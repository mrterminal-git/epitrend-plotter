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
    using Timestamp = double;
    using Value = double;

    GraphView() = default;

    void Draw(std::string_view label, const std::vector<std::pair<Timestamp, Value>> &data);

private:
    void TestingPlot();
    void TestingPlot2(const std::vector<std::pair<Timestamp,Value>> &data);
};
