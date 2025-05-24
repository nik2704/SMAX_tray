#include "Worker.h"
#include "../NetworkClient/NetworkClient.h"
#include "../resource.h"
#include <iomanip>
#include <sstream>
#include <chrono>

namespace smax {

Worker::Worker(HINSTANCE hInst, std::shared_ptr<ConfigManager> config, TrayManager* tray)
    : hInst_(hInst), config_(std::move(config)), tray_(tray), running_(false) {}

Worker::~Worker() {
    stop();
}

void Worker::start() {
    if (running_) return;

    running_ = true;
    thread_ = std::thread(&Worker::run, this);
}

void Worker::stop() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        running_ = false;
    }
    cv_.notify_all();

    if (thread_.joinable()) {
        thread_.join();
    }
}

bool Worker::isRunning() const {
    return running_;
}

void Worker::run() {
    std::unique_lock<std::mutex> lock(mutex_);
    while (running_) {
        lock.unlock();

        std::string encoded_filter = urlEncode(config_->getFilter());
        std::string full_url = config_->getUrl() + "&filter=" + encoded_filter;
        sendGET(full_url);

        lock.lock();
        cv_.wait_for(lock, std::chrono::seconds(config_->getPeriod()), [this]() { return !running_; });
    }
}

void Worker::sendGET(const std::string& url) {
    auto result = NetworkClient::get(url, config_->getUserName(), config_->getToken());

    tray_->dismissAlert();

    if (result.has_value()) {
        auto ids = NetworkClient::extractIDsFromJSON(result.value());
        auto newCount = updateProcessedIds(ids);

        if (newCount > 0) {
            tray_->showInfo(L"New " + std::to_wstring(newCount) + L" requests found!");
            tray_->setIcon(LoadIcon(hInst_, MAKEINTRESOURCE(SMAX_TRAY_ICON_ALERT)));
        }
    } else {
        tray_->showInfo(L"Failed to fetch data from SMAX", L"Error");
        tray_->setIcon(LoadIcon(hInst_, MAKEINTRESOURCE(SMAX_TRAY_ICON_ERROR)));
        processedIDs_.clear();
    }
}

std::string Worker::urlEncode(const std::string& value) {
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;

    for (unsigned char c : value) {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
        } else {
            escaped << '%' << std::uppercase << std::setw(2) << int(c);
            escaped << std::nouppercase;
        }
    }

    return escaped.str();
}

size_t Worker::updateProcessedIds(const std::vector<std::string>& ids) {
    std::set<std::string> current(ids.begin(), ids.end());
    size_t newCount = 0;

    for (const auto& id : current) {
        if (processedIDs_.insert(id).second) {
            ++newCount;
        }
    }

    for (auto it = processedIDs_.begin(); it != processedIDs_.end();) {
        if (current.find(*it) == current.end()) {
            it = processedIDs_.erase(it);
        } else {
            ++it;
        }
    }

    return newCount;
}

} // namespace smax
