#pragma once
#include "DataManager.hpp"
#include "GraphView.hpp"
#include "GraphViewModel.hpp"

class AppController {
public:
    using Timestamp = double;
    using Value = double;

    AppController();
    // ~AppController();

    void run(); // Main application loop.

    void updatePlottableSensors();

private:
    DataManager dataManager;
    GraphViewModel viewModel;
    GraphView graphView;

    void onViewRangeChanged(double start, double end);
};
