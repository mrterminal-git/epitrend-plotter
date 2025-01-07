#include "DataManager.hpp"
#include <iostream> // FOR TESTING
#include <iomanip> // FOR TESTING

// Constructor
DataManager::DataManager() : background_thread_running_(false) {}

// Destructor
DataManager::~DataManager() {
    stopBackgroundUpdates();
}

// Start background updates
void DataManager::startBackgroundUpdates() {
    background_thread_running_ = true;
    background_thread_ = std::thread(&DataManager::backgroundUpdateTask, this);
}

// Stop background updates
void DataManager::stopBackgroundUpdates() {
    background_thread_running_ = false;
    if (background_thread_.joinable()) {
        background_thread_.join();
    }
}

// Background update task
void DataManager::backgroundUpdateTask() {
    while (background_thread_running_) {
        {
            std::unordered_map<std::string, std::pair<Timestamp, Timestamp>> local_merged_ranges;

            // Copy sensor_ranges_ to a local map
            {
                std::lock_guard<std::mutex> lock(sensor_ranges_mutex_);
                for (const auto& [sensor_id, ranges] : sensor_ranges_) {
                    local_merged_ranges[sensor_id] = mergeRanges(ranges);
                }
            }

            // Update buffers_ based on the local copy of the merged ranges
            {
                std::lock_guard<std::mutex> lock(buffer_mutex_);

                // Iterate through all sensors and update the data
                for (const auto& [sensor_id, merged_range] : local_merged_ranges) {
                    if (local_merged_ranges.empty())
                        continue;

                    buffers_[sensor_id].setRange(
                        merged_range.first, merged_range.second,
                        [this, sensor_id](Timestamp start, Timestamp end) {
                            preloadData(sensor_id, start, end);
                        });
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(1)); // Update every second
    }
}

// Get all buffers. UNSAFE ACCESS
const std::unordered_map<std::string, TimeSeriesBuffer<DataManager::Timestamp, DataManager::Value>>& DataManager::getBuffers() const {
    return buffers_;
}

// Get a snapshot of the buffer for a specific sensor. SAFE ACCESS
std::vector<std::pair<DataManager::Timestamp, DataManager::Value>> DataManager::getBuffersSnapshot(
    const std::string& sensor_label, Timestamp start, Timestamp end) {
    std::lock_guard<std::mutex> lock(buffer_mutex_);

    // Find the buffer
    auto it = buffers_.find(sensor_label);
    if (it == buffers_.end()) {
        return {}; // Return empty if sensor not found
    }

    // Get the buffer and ensure it's sorted
    const auto& buffer = it->second.getData();
    if (buffer.empty()) {
        return {};
    }

    // Use binary search to find the start and end iterators
    auto lower = std::lower_bound(buffer.begin(), buffer.end(), start,
        [](const auto& entry, const Timestamp& value) {
            return entry.first < value;
        });

    auto upper = std::upper_bound(buffer.begin(), buffer.end(), end,
        [](const Timestamp& value, const auto& entry) {
            return value < entry.first;
        });

    // Create a snapshot using the range
    return std::vector<std::pair<Timestamp, Value>>(lower, upper);
}

// Initialize a buffer for a specific machine
void DataManager::addSensor(const std::string& sensor_id) {
    // Lock the buffer mutex
    std::lock_guard<std::mutex> lock(buffer_mutex_);
    buffers_.emplace(sensor_id, TimeSeriesBuffer<Timestamp, Value>());
}

// Update range for a specific machine. Callback is used to preload data and clean up old data according to the new range
void DataManager::updateSensorRange(const std::string& sensor_id, int plot_id, DataManager::Timestamp start, DataManager::Timestamp end) {
    auto& ranges = sensor_ranges_[sensor_id];
    ranges[plot_id] = {start, end}; // Update the specific plot's range

    auto [merged_start, merged_end] = mergeRanges(ranges); // Merge ranges across plots

    buffers_[sensor_id].setRange(
        merged_start, merged_end,
        [this, sensor_id](Timestamp preload_start, Timestamp preload_end) {
            preloadData(sensor_id, preload_start, preload_end);
        });
}

// Add new data to a machine's buffer
void DataManager::addSensorData(const std::string& sensor_id, const std::vector<std::pair<Timestamp, Value>>& data) {
    // Lock the mutex
    std::lock_guard<std::mutex> lock(buffer_mutex_);
    if (buffers_.find(sensor_id) != buffers_.end()) {
        buffers_[sensor_id].addData(data);
    }
}

/// Preload data outside the existing range for a specific machine. CURRENTLY IN TESTING
void DataManager::preloadData(const std::string& sensor_id, Timestamp start, Timestamp end) {
    // Simulate data loading (replace with actual data loading logic)
    std::vector<std::pair<Timestamp, Value>> new_data;
    if (sensor_id == "sensor_1") {
        for (Timestamp t = start; t <= end; t += 1.0) {
            new_data.emplace_back(t, std::sin(t));
        }

    } else if (sensor_id == "sensor_2") {
        for (Timestamp t = start; t <= end; t += 1.0) {
            new_data.emplace_back(t, 2.0 * std::cos(t / 1.5));
        }

    } else {
        // STOP PROGRAM
        std::exit(1);

    }
    addSensorData(sensor_id, new_data);
}

// Merge ranges across all plots for a specific sensor
std::pair<DataManager::Timestamp, DataManager::Timestamp> DataManager::mergeRanges(
    const std::unordered_map<int, std::pair<DataManager::Timestamp, DataManager::Timestamp>>& ranges) {
    if (ranges.empty()) return {0, 0};
    Timestamp min_start = ranges.begin()->second.first;
    Timestamp max_end = ranges.begin()->second.second;
    for (const auto& [_, range] : ranges) {
        min_start = std::min(min_start, range.first);
        max_end = std::max(max_end, range.second);
    }
    return {min_start, max_end};
}

// Set the range for a specific sensor
void DataManager::setSensorRange(const std::string& sensor_id, int plot_id, Timestamp start, Timestamp end) {
    // Lock the mutex
    std::lock_guard<std::mutex> lock(sensor_ranges_mutex_);
    sensor_ranges_[sensor_id][plot_id] = {start, end};
}
