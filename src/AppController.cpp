#include <iostream>

#include "AppController.hpp"

// Constructor
AppController::AppController()
    : dataManager(), viewModel(update_viewModel_mutex_), graphView(viewModel) {
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

    // Start the update viewModel thread
    update_viewModel_thread_ = std::thread(&AppController::updateViewModel, this);
    stop_update_viewModel_thread_ = false;

}

// Destructor
AppController::~AppController() {
    stop_update_viewModel_thread_ = true;
    if (update_viewModel_thread_.joinable()) {
        update_viewModel_thread_.join();
    }
}

// Update the plottable sensors in the viewModel from the dataManager buffers
void AppController::updatePlottableSensors() {
    std::vector<std::string> sensor_names;
    for (const auto& [sensor_id, _] : dataManager.getBuffers()) {
        sensor_names.push_back(sensor_id);
    }
    viewModel.setPlottableSensors(sensor_names);

}

// Update the viewModel with data from the dataManager in a separate thread
void AppController::updateViewModel() {
    while (!stop_update_viewModel_thread_) {
        viewModel.updatePlotsWithData(dataManager);
        std::this_thread::sleep_for(std::chrono::seconds(10)); // Update every second
    }
}

void AppController::run() {
    // Render the graph view
    {
        // Lock the mutex when updating the view model in updateViewModel()
        std::lock_guard<std::mutex> lock(update_viewModel_mutex_);
        graphView.Draw("Main window");
    }
}

