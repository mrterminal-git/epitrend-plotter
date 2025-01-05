#pragma once
#include "DataManager.hpp"
#include "GraphView.hpp"
#include "GraphViewModel.hpp"

class AppController {
public:
    using Timestamp = double;
    using Value = double;

    AppController();
    ~AppController();

    void run(); // Main application loop.

    void updatePlottableSensors();
    void updateViewModel();

private:
    DataManager dataManager;
    GraphViewModel viewModel;
    GraphView graphView;

    std::thread update_viewModel_thread_;
    std::mutex update_viewModel_mutex_;
    bool stop_update_viewModel_thread_;
};
