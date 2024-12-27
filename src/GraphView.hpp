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
    explicit GraphView(GraphViewModel& viewModel);

    // Draw method
    void Draw(const std::string label);

private:
    GraphViewModel& viewModel_;

    // ==============================
    // renderAll
    // ==============================
    void renderAll();

    // ==============================
    // renderAddPlotPopup
    // ==============================
    void actionSubmitAddPlotPopup(AddPlotPopupState& state);
    void renderAddPlotPopup();

};
