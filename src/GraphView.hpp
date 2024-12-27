#pragma once

#include <array>
#include <set>
#include <string_view>
#include <unordered_map>
#include <map>
#include <imgui.h>

#include "TimeSeriesBuffer.hpp"
#include "GraphViewModel.hpp"

class GraphView
{
public:
    // Constructor
    GraphView(const GraphViewModel& viewModel) : viewModel_(viewModel) {}

    // Draw method
    void Draw(const std::string label);

private:
    GraphViewModel viewModel_;

    void renderAll();
};
