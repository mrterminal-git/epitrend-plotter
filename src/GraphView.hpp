#pragma once

#include <array>
#include <set>
#include <string_view>
#include <unordered_map>
#include <map>
#include <imgui.h>

#include "TimeSeriesBuffer.hpp"
#include "GraphViewModel.hpp"
#include "WindowPlots.hpp"
#include "WindowPlotsSaveLoad.hpp"

class GraphView
{
public:
    // Constructor
    explicit GraphView(GraphViewModel& viewModel);

    // Draw method
    void Draw(const std::string label);

    // ==============================
    // renderAddPlotPopup
    // ==============================
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
    // renderLoadWindowPopup
    // ==============================
    void actionSubmitLoadWindowPopup(LoadWindowFileDialogState& file_dialog_state);
    void renderLoadWindowPopup();



    // ==============================
    // renderAllWindowPlots
    // ==============================
    void renderWindowPlotAddPlotPopup(WindowPlots* window);
    void renderPlotOptions(
        const std::string& popup_label, RenderablePlot& renderable_plot
    );
    void renderAllPlotsInWindow(WindowPlots* window);
    void renderWindowMenuBar(WindowPlots* window);
    void renderAllWindowPlots();


};
