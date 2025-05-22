#pragma once

#include "../ConfigManager/ConfigManager.h"
#include "../TrayManager/TrayManager.h"
#include <thread>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <set>
#include <vector>
#include <string>
#include <memory>

namespace smax {

class Worker {
public:
    Worker(HINSTANCE hInst, std::shared_ptr<ConfigManager> config, TrayManager* tray);

    ~Worker();

    void start();
    void stop();

    bool isRunning() const;

private:
    void run();
    void sendGET(const std::string& url);
    std::string urlEncode(const std::string& value);
    size_t updateProcessedIds(const std::vector<std::string>& ids);

    HINSTANCE hInst_;
    std::shared_ptr<ConfigManager> config_;
    TrayManager* tray_;

    std::thread thread_;
    std::atomic<bool> running_;
    std::condition_variable cv_;
    std::mutex mutex_;
    std::set<std::string> processedIDs_;
};

} // namespace smax
