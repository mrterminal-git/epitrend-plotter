#include <iostream>
#include <cmath>

#include "GraphViewModel.hpp"

GraphViewModel::GraphViewModel(std::mutex& mutex)
    : update_viewModel_mutex_(mutex) {}

void GraphViewModel::setPlottableSensors(const std::vector<std::string>& sensors) {
    plottable_sensors_ = sensors;
}

const std::vector<std::string>& GraphViewModel::getPlottableSensors() const {
    return plottable_sensors_;
}

AddPlotPopupState& GraphViewModel::getAddPlotPopupState() {
    return add_plot_popup_state_;
}

PlotOptionsPopupState& GraphViewModel::getPlotOptionsState(){
    return plot_options_popup_state_;
}

void GraphViewModel::updatePlotsWithData(DataManager& dataManager) {
    // Loop through all windows and renderable plots
    for (auto& window_plot_label : getWindowPlotLabels()) {
        WindowPlots& window_plot = getWindowPlot(window_plot_label);

        // Loop through all renderable plots in the window
        for (auto& renderable_plot_labels: window_plot.getRenderablePlotLabels()) {
            RenderablePlot& renderable_plot = window_plot.getRenderablePlot(renderable_plot_labels);

            // Update the data for all sensors in the plot
            for (const auto& sensor: renderable_plot.getAllSensors()) {
                std::vector<std::pair<DataManager::Timestamp, DataManager::Value>> data_in_range
                 = dataManager.getBuffersSnapshot(
                    sensor, renderable_plot.getPlotRange().first, renderable_plot.getPlotRange().second);

                renderable_plot.setData(sensor, data_in_range);
            }

        }
    }
}

std::pair<std::vector<DataManager::Timestamp>, std::vector<DataManager::Value>> GraphViewModel::getDownsampledData(
    RenderablePlot& plot, const std::string& sensor, double range, int num_pixels) {
    std::vector<DataManager::Timestamp> timestamps;
    std::vector<DataManager::Value> values;
    RenderablePlot::DataSeries data = plot.getDataSnapshot(sensor);

    if (data.empty()) {
        return {timestamps, values};
    }

    int step_size = static_cast<int>(range / num_pixels) * 4;
    if (step_size <= 0) {
        step_size = 1;
    }

    int num_points_to_render = data.size() / step_size;
    if (num_points_to_render <= 0) {
        num_points_to_render = data.size();
    }

    timestamps.reserve(num_points_to_render);
    values.reserve(num_points_to_render);

    int counter = 0;
    int data_pos = 0;
    for (int i = 0;
        i < num_points_to_render && data_pos < data.size();
        ++i, data_pos += step_size) {
        timestamps.push_back(data.at(data_pos).first);
        values.push_back(data.at(data_pos).second);
        counter++;
    }

    // std::cout << "====================================\n";
    // std::cout << "sensor: " << sensor << "\n";
    // std::cout << "range: " << range << "\n";
    // std::cout << "data.size(): " << data.size() << "\n";
    // std::cout << "num_pixels: " << num_pixels << "\n";
    // std::cout << "num_points_to_render: " << num_points_to_render << "\n";
    // std::cout << "step_size: " << step_size << "\n";
    // std::cout << "counter: " << counter << "\n";

    return {timestamps, values};
}



// ============================================
// Windows with renderable plots
// ============================================

// Note: this method ignores the window if it already exists. It is up to the caller to check if the window exists (by calling hasWindowPlots())
void GraphViewModel::addWindowPlots(const std::string& window_label, std::unique_ptr<WindowPlots> window_plots) {
    // Check if the window already exists
    if (window_plots_.find(window_label) != window_plots_.end()) {
        return;
    }
    window_plots_.emplace(window_label, std::move(window_plots));
}

void GraphViewModel::removeWindowPlots(const std::string& window_label) {
    window_plots_.erase(window_label);
}

bool GraphViewModel::hasWindowPlots(const std::string& window_label) const {
    return window_plots_.find(window_label) != window_plots_.end();
}

const std::map<std::string, std::unique_ptr<WindowPlots>>& GraphViewModel::getWindowPlots() const {
    return window_plots_;
}

WindowPlots& GraphViewModel::getWindowPlot(const std::string& window_label) {
    if (!hasWindowPlots(window_label)) {
        throw std::runtime_error("Error in GraphViewModel::getWindowPlots call: Window does not exist");
    }
    return *window_plots_.at(window_label);
}

std::vector<std::string> GraphViewModel::getWindowPlotLabels() const {
    std::vector<std::string> labels;
    for (const auto& [label, window] : window_plots_) {
        labels.push_back(label);
    }
    return labels;
}

WindowPlotAddPlotPopupState& GraphViewModel::getWindowPlotAddPlotPopupState() {
    return window_plot_add_plot_popup_state_;
}

FileDialogState& GraphViewModel::getFileDialogState() {
    return file_dialog_state_;
}
