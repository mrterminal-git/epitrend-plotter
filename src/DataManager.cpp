#include "DataManager.hpp"
#include <iostream> // FOR TESTING

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

// Background update task. CURRENTLY IN TESTING
void DataManager::backgroundUpdateTask() {
    while (background_thread_running_) {
        const time_t current_time = std::time(nullptr);
        const time_t max_data_time_range = 1000;

        for (auto& [sensor_id, buffer] : buffers_) {
            // Adjust range as needed
            Timestamp start = current_time - max_data_time_range / 2;
            Timestamp end = current_time + max_data_time_range / 2; // Adjust range size
            preloadData(sensor_id, start, end);
        }
        std::this_thread::sleep_for(std::chrono::seconds(1)); // Update every second
    }
}

// Get all buffers
const std::unordered_map<std::string, TimeSeriesBuffer<DataManager::Timestamp, DataManager::Value>>& DataManager::getBuffers() const {
    return buffers_;
}

// Initialize a buffer for a specific machine
void DataManager::addSensor(const std::string& sensor_id) {
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
        std::cout << "Unknown sensor ID: " << sensor_id << "\n";
        std::exit(1);

    }
    addSensorData(sensor_id, new_data);
}

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
