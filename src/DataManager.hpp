#pragma once
#include <vector>
#include <string>

#include "TimeSeriesBuffer.hpp"

class DataManager {
public:
    using Timestamp = double; // Or another suitable type
    using Value = double;

    void addSensor (const std::string& sensor_id);
    void updateSensorRange(const std::string& sensor_id, Timestamp start, Timestamp end);
    void addSensorData(const std::string& sensor_id, const std::vector<std::pair<Timestamp, Value>>& data);

public:
    std::unordered_map<std::string, TimeSeriesBuffer<Timestamp, Value>> buffers_;

    void preloadData(const std::string& sensor_id, Timestamp start, Timestamp end);
};
