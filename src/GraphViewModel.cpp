#include <iostream>
#include <cmath>

#include "GraphViewModel.hpp"

GraphViewModel::GraphViewModel(std::mutex& mutex)
    : update_viewModel_mutex_(mutex) {}
void GraphViewModel::addRenderablePlot(RenderablePlot& object) {
    // Set the plot ID
    object.setPlotId(next_plot_id_++);

    renderable_plots_.push_back(object);
}

std::vector<RenderablePlot>& GraphViewModel::getRenderablePlots() {
    return renderable_plots_;
}

void GraphViewModel::clear() {
    renderable_plots_.clear();
}

void GraphViewModel::setPlottableSensors(const std::vector<std::string>& sensors) {
    plottable_sensors_ = sensors;
}

const std::vector<std::string>& GraphViewModel::getPlottableSensors() const {
    return plottable_sensors_;
}

AddPlotPopupState& GraphViewModel::getAddPlotPopupState() {
    return add_plot_popup_state_;
}

void GraphViewModel::updatePlotsWithData(DataManager& dataManager) {
    std::vector<RenderablePlot> temp_plots;

    // Get the all time-series data for each sensor from dataManager
    const auto& buffers = dataManager.getBuffers();

    // Get all the time-series data for each from dataManager via getBuffersSnapshot
    for (auto& renderable_plot : renderable_plots_) {
        // Copy the plot to the temporary buffer
        temp_plots.push_back(renderable_plot);

        // Loop through the sensors in each renderable plot
        for (const auto& [sensor, _] : renderable_plot.getAllData()) {
            std::vector<std::pair<DataManager::Timestamp, DataManager::Value>> data_in_range
             = dataManager.getBuffersSnapshot(
                sensor, renderable_plot.getPlotRange().first, renderable_plot.getPlotRange().second);

                temp_plots.back().setData(sensor, data_in_range);
        }

    }

    // Lock the mutex only when updating the view model
    {
        std::lock_guard<std::mutex> lock(update_viewModel_mutex_);
        renderable_plots_ = std::move(temp_plots);
    }
}

std::pair<std::vector<DataManager::Timestamp>, std::vector<DataManager::Value>> GraphViewModel::getDownsampledData(
    const RenderablePlot& plot, const std::string& sensor, double range, int num_pixels) {
    std::vector<DataManager::Timestamp> timestamps;
    std::vector<DataManager::Value> values;
    const auto& data = plot.getData(sensor);

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

    std::cout << "====================================\n";
    std::cout << "sensor: " << sensor << "\n";
    std::cout << "range: " << range << "\n";
    std::cout << "data.size(): " << data.size() << "\n";
    std::cout << "num_pixels: " << num_pixels << "\n";
    std::cout << "num_points_to_render: " << num_points_to_render << "\n";
    std::cout << "step_size: " << step_size << "\n";
    std::cout << "counter: " << counter << "\n";

    return {timestamps, values};
}
