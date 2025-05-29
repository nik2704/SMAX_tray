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

// void Worker::run() {
//     std::unique_lock<std::mutex> lock(mutex_);
//     while (running_) {
//         lock.unlock();

//         fetchStats_ = {0 ,0, 0};

//         checkCategory(config_->getCheckRequests(), config_->getRequestUrl(), config_->getRequestFilter(), processedRequestsIDs_, fetchStats_.requests);
//         checkCategory(config_->getCheckTasks(), config_->getTaskUrl(), config_->getTaskFilter(), processedTasksIDs_, fetchStats_.tasks);
//         checkCategory(config_->getCheckApprovals(), config_->getApprovalUrl(), config_->getApprovalFilter(), processedApprovalsIDs_, fetchStats_.approvals);

//         doNotification();

//         lock.lock();
//         cv_.wait_for(lock, std::chrono::seconds(config_->getPeriod()), [this]() { return !running_; });
//     }
// }

void Worker::run() {
    while (running_) {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            if (!running_) break;
        }

        fetchStats_ = {0 ,0, 0};

        checkCategory(config_->getCheckRequests(), config_->getRequestUrl(), config_->getRequestFilter(), processedRequestsIDs_, fetchStats_.requests);
        checkCategory(config_->getCheckTasks(), config_->getTaskUrl(), config_->getTaskFilter(), processedTasksIDs_, fetchStats_.tasks);
        checkCategory(config_->getCheckApprovals(), config_->getApprovalUrl(), config_->getApprovalFilter(), processedApprovalsIDs_, fetchStats_.approvals);

        doNotification();

        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait_for(lock, std::chrono::seconds(config_->getPeriod()), [this]() { return !running_; });
    }
}

void Worker::checkCategory(bool enabled, const std::string& url, const std::string& filter,
                           std::set<std::string>& processedIDs, int& resultField) {
    if (!enabled) return;

    std::string encoded_filter = urlEncode(filter);
    std::string full_url = url + "&filter=" + encoded_filter;
    resultField = sendGET(full_url, processedIDs);
}

void Worker::doNotification() {
    tray_->dismissAlert();

    if (fetchStats_.requests == 0 && fetchStats_.tasks == 0 && fetchStats_.approvals == 0) {
        return;
    }

    if (fetchStats_.requests == -1 || fetchStats_.tasks == -1 || fetchStats_.approvals == -1) {
        tray_->updateTooltip(0, 0, 0);
        tray_->showInfo(L"Failed to fetch data from SMAX", L"Error");
        tray_->setIcon(LoadIcon(hInst_, MAKEINTRESOURCE(SMAX_TRAY_ICON_ERROR)));
        
        processedRequestsIDs_.clear();
        processedTasksIDs_.clear();
        processedApprovalsIDs_.clear();
        
        return;
    }

    std::string message = "";

    if (fetchStats_.requests > 0) {
        message += std::to_string(fetchStats_.requests) + " requests";
    }

    if (fetchStats_.tasks > 0) {
        if (!message.empty()) {
            message += ", ";
        }

        message += std::to_string(fetchStats_.tasks) + " tasks";
    }

    if (fetchStats_.approvals > 0) {
        if (!message.empty()) {
            message += ", ";
        }

        message += std::to_string(fetchStats_.approvals) + " approvals";
    }

    message = "New " + message + " found!";

    tray_->updateTooltip(fetchStats_.requests, fetchStats_.tasks, fetchStats_.approvals);
    tray_->showInfo(std::wstring(message.begin(), message.end()));
    tray_->setIcon(LoadIcon(hInst_, MAKEINTRESOURCE(SMAX_TRAY_ICON_ALERT)));
}

int Worker::sendGET(const std::string& url, std::set<std::string>& processedIDs) {
    auto result = NetworkClient::get(url, config_->getUserName(), config_->getToken());

    if (result.has_value()) {
        auto ids = NetworkClient::extractIDsFromJSON(result.value());
        return updateProcessedIds(ids, processedIDs);
    } else {
        return -1;
    }

    return 0;
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

int Worker::updateProcessedIds(const std::vector<std::string>& ids, std::set<std::string>& processedIDs) {
    std::set<std::string> current(ids.begin(), ids.end());
    int newCount = 0;

    for (const auto& id : current) {
        if (processedIDs.insert(id).second) {
            ++newCount;
        }
    }

    for (auto it = processedIDs.begin(); it != processedIDs.end();) {
        if (current.find(*it) == current.end()) {
            it = processedIDs.erase(it);
        } else {
            ++it;
        }
    }

    return newCount;
}

} // namespace smax
