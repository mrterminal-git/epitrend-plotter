#include "DataManager.hpp"

// Initialize a buffer for a specific machine
void DataManager::addSensor (const std::string& sensor_id) {
    buffers_[sensor_id] = TimeSeriesBuffer<Timestamp, Value>();
}

// Update range for a specific machine
void DataManager::updateSensorRange(const std::string& sensor_id, Timestamp start, Timestamp end) {
    if (buffers_.find(sensor_id) != buffers_.end()) {
        buffers_[sensor_id].setRange(
            start, end,
            [this, sensor_id](Timestamp preload_start, Timestamp preload_end) {
                preloadData(sensor_id, preload_start, preload_end);
            });
    }
}

// Add new data to a machine's buffer
void DataManager::addSensorData(const std::string& sensor_id, const std::vector<std::pair<Timestamp, Value>>& data) {
    if (buffers_.find(sensor_id) != buffers_.end()) {
        buffers_[sensor_id].addData(data);
    }
}

void DataManager::preloadData(const std::string& sensor_id, Timestamp start, Timestamp end) {
    // Simulate data loading (replace with actual data loading logic)
    std::vector<std::pair<Timestamp, Value>> new_data;
    for (Timestamp t = start; t <= end; t += 1.0) {
        new_data.emplace_back(t, std::sin(t));
    }
    addSensorData(sensor_id, new_data);
}
