#pragma once
#include "DataManager.hpp"
#include "GraphView.hpp"

class AppController {
public:
    void run(); // Main application loop.

private:
    DataManager dataManager;
    GraphView graphView;
};
