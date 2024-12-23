#include "AppController.hpp"

void AppController::run() {
    // Get the current time and create faux data using DataManager::preloadData
    const time_t current_time = std::time(nullptr);
    const time_t max_data_time_range = 1000000;
    dataManager.addSensor("sensor_1");
    dataManager.preloadData("sensor_1", current_time - max_data_time_range / 2, current_time + max_data_time_range / 2);
    auto data = dataManager.buffers_["sensor_1"].getData();

    // Render the view
    graphView.Draw("Data Viewer", data);
}

