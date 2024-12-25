#pragma once
#include "DataManager.hpp"
#include "GraphView.hpp"

class AppController {
public:
    using Timestamp = double;
    using Value = double;

    AppController();
    // ~AppController();

    void run(); // Main application loop.

private:
    DataManager dataManager;
    GraphView graphView;

    void onViewRangeChanged(double start, double end);
};
