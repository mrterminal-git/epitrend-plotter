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

    using UpdateRangeCallback = std::function<void(const std::string& sensor_id, int plot_id, double start, double end)>;

    void setUpdateRangeCallback(UpdateRangeCallback callback);
private:
    UpdateRangeCallback update_range_callback_;
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

    // ==============================
    // renderAllPlots
    // ==============================
    void renderAllPlots();


};
