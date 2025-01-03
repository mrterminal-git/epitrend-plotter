#pragma once
#include <vector>
#include <string>

#include "TimeSeriesBuffer.hpp"

class DataManager {
public:
    using Timestamp = double; // Or another suitable type
    using Value = double;

    DataManager();
    ~DataManager();

    const std::unordered_map<std::string, TimeSeriesBuffer<Timestamp, Value>>& getBuffers() const;

    void addSensor (const std::string& sensor_id);
    void updateSensorRange(const std::string& sensor_id, int plot_id, Timestamp start, Timestamp end);
    void addSensorData(const std::string& sensor_id, const std::vector<std::pair<Timestamp, Value>>& data);

    void startBackgroundUpdates();
    void stopBackgroundUpdates();

private:
    std::unordered_map<std::string, TimeSeriesBuffer<Timestamp, Value>> buffers_;
    std::unordered_map<std::string, std::unordered_map<int, std::pair<Timestamp, Timestamp>>> sensor_ranges_; // sensor -> plot_id -> range

    std::thread background_thread_;
    std::atomic<bool> background_thread_running_;

    void preloadData(const std::string& sensor_id, Timestamp start, Timestamp end);
    void backgroundUpdateTask();

    std::pair<Timestamp, Timestamp> mergeRanges(
        const std::unordered_map<int, std::pair<Timestamp, Timestamp>>& ranges
        );

};
