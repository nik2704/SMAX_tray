#pragma once

#include "../ConfigManager/ConfigManager.h"
#include "../TrayManager/TrayManager.h"
#include <atomic>
#include <condition_variable>
#include <set>
#include <string>
#include <thread>
#include <memory>

namespace smax {

/**
 * @brief Manages background execution logic such as polling and HTTP GET requests.
 *
 * This class runs a background thread that periodically sends GET requests based
 * on configuration parameters. It also tracks already processed IDs to avoid duplication
 * and interacts with the system tray to show notifications or alerts.
 */
class Worker {
public:
    /**
     * @brief Constructs a Worker instance.
     * @param hInst Handle to the application instance.
     * @param config Shared pointer to the configuration manager.
     * @param tray Pointer to the tray manager for displaying notifications.
     */
    Worker(HINSTANCE hInst, std::shared_ptr<ConfigManager> config, TrayManager* tray);

    /**
     * @brief Destroys the Worker and ensures cleanup.
     */
    ~Worker();

    /**
     * @brief Starts the worker thread.
     */
    void start();

    /**
     * @brief Stops the worker thread and joins it.
     */
    void stop();

    /**
     * @brief Checks if the worker is currently running.
     * @return True if the background thread is active, false otherwise.
     */
    bool isRunning() const;

private:
    /**
     * @brief Main function executed by the background thread.
     *
     * Periodically reads configuration, sends GET requests, and triggers
     * tray notifications if necessary.
     */
    void run();

    /**
     * @brief Sends a GET request to the specified URL.
     * @param url The URL to send the GET request to.
     */
    void sendGET(const std::string& url);

    /**
     * @brief Encodes a string for safe use in a URL.
     * @param value The string to encode.
     * @return The URL-encoded version of the input string.
     */
    std::string urlEncode(const std::string& value);

    /**
     * @brief Updates the internal set of already processed IDs.
     * @param ids List of new IDs to check and add.
     * @return Number of new (unprocessed) IDs added.
     */
    size_t updateProcessedIds(const std::vector<std::string>& ids);

    HINSTANCE hInst_;                           /// Application instance handle.
    std::shared_ptr<ConfigManager> config_;     /// Shared pointer to configuration manager.
    TrayManager* tray_;                         /// Pointer to tray manager for displaying alerts.
    std::thread thread_;                        /// Background thread executing the worker logic.
    std::atomic<bool> running_;                 /// Flag indicating whether the worker is running.
    std::condition_variable cv_;                /// Condition variable for thread synchronization.
    std::mutex mutex_;                          /// Mutex for synchronizing access to shared data.
    std::set<std::string> processedIDs_;        /// Set of already processed IDs to avoid duplication.
};

} // namespace smax
