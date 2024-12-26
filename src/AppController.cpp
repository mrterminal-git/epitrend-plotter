#include "AppController.hpp"

// Constructor
AppController::AppController() {
    dataManager.addSensor("sensor_1"); // for testing
    dataManager.addSensor("sensor_2"); // for testing

    // Start background updates on the DataManager
    dataManager.startBackgroundUpdates();
}

void AppController::onViewRangeChanged(double start, double end) {
    // Update the range
    // For testing, we update the range for sensor_1 and sensor_2
    dataManager.updateSensorRange("sensor_1", start, end);
    dataManager.updateSensorRange("sensor_2", start, end);
}

void AppController::run() {
    graphView.Draw("Main window");
}
