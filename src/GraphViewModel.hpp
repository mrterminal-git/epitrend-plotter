#pragma once
#include "RenderablePlot.hpp"
#include <vector>
#include <string>

struct AddPlotPopupState {
    std::string window_label;
    std::string plot_label;
    bool is_real_time = true;
    std::vector<std::string> available_sensors;
    std::vector<std::string> selected_sensors;

    char plot_range_start_year[5] = "";
    char plot_range_start_month[3] = "";
    char plot_range_start_day[3] = "";
    char plot_range_start_hour[3] = "";
    char plot_range_start_minute[3] = "";
    char plot_range_start_second[3] = "";

    char plot_range_end_year[5] = "";
    char plot_range_end_month[3] = "";
    char plot_range_end_day[3] = "";
    char plot_range_end_hour[3] = "";
    char plot_range_end_minute[3] = "";
    char plot_range_end_second[3] = "";

    void reset() {
        window_label.clear();
        plot_label.clear();
        is_real_time = true;
        selected_sensors.clear();

        std::fill(std::begin(plot_range_start_year), std::end(plot_range_start_year), '\0');
        std::fill(std::begin(plot_range_start_month), std::end(plot_range_start_month), '\0');
        std::fill(std::begin(plot_range_start_day), std::end(plot_range_start_day), '\0');
        std::fill(std::begin(plot_range_start_hour), std::end(plot_range_start_hour), '\0');
        std::fill(std::begin(plot_range_start_minute), std::end(plot_range_start_minute), '\0');
        std::fill(std::begin(plot_range_start_second), std::end(plot_range_start_second), '\0');

        std::fill(std::begin(plot_range_end_year), std::end(plot_range_end_year), '\0');
        std::fill(std::begin(plot_range_end_month), std::end(plot_range_end_month), '\0');
        std::fill(std::begin(plot_range_end_day), std::end(plot_range_end_day), '\0');
        std::fill(std::begin(plot_range_end_hour), std::end(plot_range_end_hour), '\0');
        std::fill(std::begin(plot_range_end_minute), std::end(plot_range_end_minute), '\0');
        std::fill(std::begin(plot_range_end_second), std::end(plot_range_end_second), '\0');
    }
};

class GraphViewModel {
public:
    GraphViewModel() = default;

    void addRenderablePlot(const RenderablePlot& object);
    const std::vector<RenderablePlot>& getRenderablePlots() const;

    void clear(); // Clear all renderables

    // Plottable sensors storage
    void setPlottableSensors(const std::vector<std::string>& sensors);
    const std::vector<std::string>& getPlottableSensors() const;

    // Add plot popup state
    AddPlotPopupState& getAddPlotPopupState();

private:
    // Each renderable plot is a separate window
    std::vector<RenderablePlot> renderable_plots_;

    // Plottable sensors
    std::vector<std::string> plottable_sensors_;

    // Add plot popup state
    AddPlotPopupState add_plot_popup_state_;
};
