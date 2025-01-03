#include <iostream>

#include "AppController.hpp"

// Constructor
AppController::AppController()
: dataManager(), viewModel(), graphView(viewModel) {
    dataManager.addSensor("sensor_1"); // for testing
    dataManager.addSensor("sensor_2"); // for testing

    // Start background updates on the DataManager
    dataManager.startBackgroundUpdates();

    // Update the plottable sensors in the viewModel from the dataManager buffers
    updatePlottableSensors();

    // Set the callback for when the view range changes in the graphView
    graphView.setUpdateRangeCallback(
    [this](const std::string& sensor_id, int plot_id, double start, double end) {
        dataManager.updateSensorRange(sensor_id, plot_id, static_cast<DataManager::Timestamp>(start), static_cast<DataManager::Timestamp>(end));
    });
}

// Update the plottable sensors in the viewModel from the dataManager buffers
void AppController::updatePlottableSensors() {
    std::vector<std::string> sensor_names;
    for (const auto& [sensor_id, _] : dataManager.getBuffers()) {
        sensor_names.push_back(sensor_id);
    }
    viewModel.setPlottableSensors(sensor_names);

}

void AppController::run() {
    // Update the viewModel with data from the dataManager
    // viewModel.updateFromDataManager(dataManager); // IMPLEMENT THIS

    graphView.Draw("Main window");
}
