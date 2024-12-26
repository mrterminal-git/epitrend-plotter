#pragma once
#include <string>
#include <vector>
#include <utility>
#include <functional>

struct RenderablePlot {
    using Timestamp = double;
    using Value = double;

    // Plot range callback
    using RangeCallback = std::function<void(Timestamp start, Timestamp end)>;

    std::string label; // Name or identifier of the object
    std::vector<Timestamp> timestamps; // X-axis data
    std::vector<Value> values;         // Y-axis data
    RangeCallback range_callback_;

    // Additional properties (e.g., colors, line styles)
    bool real_time = true;
    std::pair<Timestamp, Timestamp> plot_range; // Optional plot range
};
