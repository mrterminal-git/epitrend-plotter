#include "AppController.hpp"

// Constructor
AppController::AppController() {
    dataManager.addSensor("sensor_1"); // for testing
    dataManager.addSensor("sensor_2"); // for testing

    // Start background updates on the DataManager
    dataManager.startBackgroundUpdates();

    // Set the range callback
    graphView.setRangeCallback([this](double start, double end) {
        onViewRangeChanged(start, end);
    });
}

void AppController::onViewRangeChanged(double start, double end) {
    // Update the range
    // For testing, we update the range for sensor_1 and sensor_2
    dataManager.updateSensorRange("sensor_1", start, end);
    dataManager.updateSensorRange("sensor_2", start, end);
}

void AppController::run() {
    // auto data = dataManager.buffers_["sensor_1"].getData();
    // graphView.Draw("Data Viewer", data);

    // De-couple data from the DataManager
    std::unordered_map<std::string, std::pair<std::vector<double>, std::vector<double>>> data;
    for (const auto &sensor : dataManager.buffers_) {
        auto& non_const_sensor = const_cast<TimeSeriesBuffer<Timestamp, Value>&>(sensor.second);
        const auto& sensor_time_series = non_const_sensor.getDataMap();

        std::vector<double> t, y;
        for (const auto &entry : sensor_time_series) {
            t.push_back(entry.first);
            y.push_back(entry.second);
        }
        data[sensor.first] = std::make_pair(t, y);
    }

    graphView.Draw3("Data Viewer", data);
}
