#pragma once

#include <array>
#include <set>
#include <string_view>
#include <unordered_map>
#include <map>
#include <imgui.h>

#include "TimeSeriesBuffer.hpp"

class GraphView
{
public:
    // Define types for TimeSeriesBuffer
    using Timestamp = double;
    using Value = double;

    // Plot range callback
    using RangeCallback = std::function<void(Timestamp start, Timestamp end)>;

    // Constructor
    GraphView() = default;

    // Draw method
    void Draw(std::string_view label, const std::vector<std::pair<Timestamp, Value>> &data);
    void Draw3(std::string_view label, const std::unordered_map<std::string, std::pair<std::vector<double>, std::vector<double>>> &decoupled_data);

    // Set range callback
    void setRangeCallback(const RangeCallback& callback);

private:
    RangeCallback range_callback_;

private:
    void TestingPlot();
    void TestingPlot2(const std::vector<std::pair<Timestamp,Value>> &data); // bad implementation because data is not de-coupled in Controller
    void TestingPlot3(const std::string sub_window_label,
        const std::unordered_map<std::string, std::pair<std::vector<double>, std::vector<double>>> &decoupled_data
    ); // good implementation because data is de-coupled in Controller
};
