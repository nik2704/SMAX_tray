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
 * @struct FetchStats
 * @brief Holds statistics about the number of requests, tasks, and approvals processed.
 */
struct FetchStats {
    int requests;
    int tasks;
    int approvals;
};

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
     * @param processedIDs Reference to a processed IDs structure to update.
     * @return number of new records.
     */
    int sendGET(const std::string& url, std::set<std::string>& processedIDs);

    /**
     * @brief Checks for new requests, tasks, and approvals.
     *
     * This function will be called periodically to check for updates
     * in requests, tasks, and approvals based on the configuration.
     */
    void checkCategory(bool enabled, const std::string& url, const std::string& filter, std::set<std::string>& processedIDs, int& resultField);
    // void checkRequests();
    // void checkTasks();
    // void checkApprovals();

    void doNotification();
    /**
     * @brief Encodes a string for safe use in a URL.
     * @param value The string to encode.
     * @return The URL-encoded version of the input string.
     */
    std::string urlEncode(const std::string& value);

    /**
     * @brief Updates the internal set of already processed IDs.
     * @param ids List of new IDs to check and add.
     * @param processedIDs Reference to a processed IDs structure to update.
     * @return Number of new (unprocessed) IDs added.
     */
    int updateProcessedIds(const std::vector<std::string>& ids, std::set<std::string>& processedIDs);

    HINSTANCE hInst_;                           /// Application instance handle.
    std::shared_ptr<ConfigManager> config_;     /// Shared pointer to configuration manager.
    TrayManager* tray_;                         /// Pointer to tray manager for displaying alerts.
    std::thread thread_;                        /// Background thread executing the worker logic.
    std::atomic<bool> running_;                 /// Flag indicating whether the worker is running.
    std::condition_variable cv_;                /// Condition variable for thread synchronization.
    std::mutex mutex_;                          /// Mutex for synchronizing access to shared data.
    std::set<std::string> processedRequestsIDs_;        /// Set of already processed Requests IDs to avoid duplication.
    std::set<std::string> processedTasksIDs_;           /// Set of already processed Requests IDs to avoid duplication.
    std::set<std::string> processedApprovalsIDs_;       /// Set of already processed Requests IDs to avoid duplication.
    FetchStats fetchStats_;                  /// Structure to hold counts of requests, tasks, and approvals.
};

} // namespace smax
