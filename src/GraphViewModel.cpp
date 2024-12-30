#include "GraphViewModel.hpp"

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

void GraphViewModel::updatePlotsWithData(const DataManager& dataManager) {
    // for (auto& renderable_plot : renderable_plots_) {
    //     for (const auto& sensor_id : plot.getSelectedSensors()) {
    //         if (dataManager.getBuffers().find(sensor_id) != dataManager.getBuffers().end()) {
    //             plot.setData(sensor_id, dataManager.getBuffers().at(sensor_id).getDataMap());
    //         }
    //     }
    // }

    // Get the all time-series data for each sensor from dataManager
    const auto& buffers = dataManager.getBuffers();

    for (auto& renderable_plot : renderable_plots_) {
        // Loop through the sensors in each renderable plot
        for (const auto& [sensor, _] : renderable_plot.getAllData()) {
            // Grab the time-series data for that sensor from dataManager
            if (buffers.find(sensor) != buffers.end()) {
                // Update the time-series data in the renderable plot
                renderable_plot.setData(sensor, buffers.at(sensor).getDataMap());

            }

        }
    }
}
