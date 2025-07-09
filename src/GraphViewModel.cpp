#include <iostream>
#include <cmath>

#include "GraphViewModel.hpp"

GraphViewModel::GraphViewModel(std::mutex& mutex)
    : update_viewModel_mutex_(mutex) {}

void GraphViewModel::setPlottableSensors(const std::vector<std::string>& sensors) {
    plottable_sensors_ = sensors;
}

const std::vector<std::string>& GraphViewModel::getPlottableSensors() const {
    return plottable_sensors_;
}

AddPlotPopupState& GraphViewModel::getAddPlotPopupState() {
    return add_plot_popup_state_;
}

PlotOptionsPopupState& GraphViewModel::getPlotOptionsState(){
    return plot_options_popup_state_;
}

void GraphViewModel::updatePlotsWithData(DataManager& dataManager) {
    // Loop through all windows and renderable plots
    for (auto& window_plot_label : getWindowPlotLabels()) {
        WindowPlots& window_plot = getWindowPlot(window_plot_label);

        // Loop through all renderable plots in the window
        for (auto& renderable_plot_labels: window_plot.getRenderablePlotLabels()) {
            RenderablePlot& renderable_plot = window_plot.getRenderablePlot(renderable_plot_labels);

            // Update the data for all sensors in the plot
            for (const auto& sensor: renderable_plot.getAllSensors()) {
                std::vector<std::pair<DataManager::Timestamp, DataManager::Value>> data_in_range
                 = dataManager.getBuffersSnapshot(
                    sensor, renderable_plot.getPlotRange().first, renderable_plot.getPlotRange().second);

                renderable_plot.setData(sensor, data_in_range);
            }

        }
    }
}

std::pair<std::vector<DataManager::Timestamp>, std::vector<DataManager::Value>> GraphViewModel::getDownsampledData(
    RenderablePlot& plot, const std::string& sensor, double range, int num_pixels) {
    std::vector<DataManager::Timestamp> timestamps;
    std::vector<DataManager::Value> values;
    RenderablePlot::DataSeries data = plot.getDataSnapshot(sensor);

    // If there is no data for the sensor, return empty vectors
    if (data.empty()) {
        return {timestamps, values};
    }

    // Calculate the step size based on the range and number of pixels
    int step_size = static_cast<int>(range / num_pixels / 10);
    if (step_size <= 0) {
        step_size = 1;
    }

    // Calculate how many points we need to render based on the step size
    int num_points_to_render = data.size() / step_size;
    if (num_points_to_render <= 0) {
        num_points_to_render = data.size();
    }

    // Reserve space for the timestamps and values to avoid multiple reallocations
    timestamps.reserve(num_points_to_render);
    values.reserve(num_points_to_render);

    // Check if the axis for this sensor is log scale
    ImAxis axis = plot.getYAxisForSensor(sensor);
    bool is_log = plot.getYAxisPropertiesScaleType(axis) == RenderablePlot::ScaleType::Logirithmic;

    // Iterate through the data and downsample it
    int counter = 0;
    int data_pos = 0;
    for (int i = 0;
        i < num_points_to_render && data_pos < data.size();
        ++i, data_pos += step_size) {
        auto ts = data.at(data_pos).first;
        auto val = data.at(data_pos).second;
        if (!is_log || val > 0) { // Only include positive values for log scale
            timestamps.push_back(ts);
            values.push_back(val);
            counter++;
        }
    }

    return {timestamps, values};
}




// ============================================
// Load Window from file
// ============================================
LoadWindowFileDialogState& GraphViewModel::getLoadWindowFileDialogState() {
    return load_window_file_dialog_state_;
}

LoadWindowState& GraphViewModel::getLoadWindowState() {
    return load_window_state_;
}




// ============================================
// Windows with renderable plots
// ============================================

// Note: this method ignores the window if it already exists. It is up to the caller to check if the window exists (by calling hasWindowPlots())
void GraphViewModel::addWindowPlots(const std::string& window_label, std::unique_ptr<WindowPlots> window_plots) {
    // Check if the window already exists
    if (window_plots_.find(window_label) != window_plots_.end()) {
        return;
    }
    window_plots_.emplace(window_label, std::move(window_plots));
}

void GraphViewModel::removeWindowPlots(const std::string& window_label) {
    window_plots_.erase(window_label);
}

bool GraphViewModel::hasWindowPlots(const std::string& window_label) const {
    return window_plots_.find(window_label) != window_plots_.end();
}

const std::map<std::string, std::unique_ptr<WindowPlots>>& GraphViewModel::getWindowPlots() const {
    return window_plots_;
}

WindowPlots& GraphViewModel::getWindowPlot(const std::string& window_label) {
    if (!hasWindowPlots(window_label)) {
        throw std::runtime_error("Error in GraphViewModel::getWindowPlots call: Window does not exist");
    }
    return *window_plots_.at(window_label);
}

std::vector<std::string> GraphViewModel::getWindowPlotLabels() const {
    std::vector<std::string> labels;
    for (const auto& [label, window] : window_plots_) {
        labels.push_back(label);
    }
    return labels;
}

WindowPlotAddPlotPopupState& GraphViewModel::getWindowPlotAddPlotPopupState() {
    return window_plot_add_plot_popup_state_;
}

FileDialogState& GraphViewModel::getFileDialogState() {
    return file_dialog_state_;
}

FileMenuState& GraphViewModel::getFileMenuState() {
    return file_menu_state_;
}

SaveWindowAsPopupState& GraphViewModel::getSaveWindowAsPopupState() {
    return save_window_as_popup_state_;
}

std::pair<std::vector<double>, std::vector<std::string>> GraphViewModel::getLogTicks(
    double min, double max, double log_base) const
{
    std::vector<double> ticks;
    std::vector<std::string> labels;

    if (min <= 0 || max <= 0 || log_base <= 1.0) {
        return {ticks, labels}; // Invalid input for log scale
    }

    int min_exp = static_cast<int>(std::floor(std::log(min) / std::log(log_base)));
    int max_exp = static_cast<int>(std::ceil(std::log(max) / std::log(log_base)));

    // Track if any major ticks are found
    bool has_major = false;

    for (int exp = min_exp; exp <= max_exp; ++exp) {
        double major_tick = std::pow(log_base, exp);
        if (major_tick >= min && major_tick <= max) {
            has_major = true;
            ticks.push_back(major_tick);
            std::ostringstream oss;
            oss << std::scientific << std::setprecision(4) << major_tick;
            labels.push_back(oss.str());
        }
        // Minor ticks
        for (int minor = 2; minor < static_cast<int>(log_base); ++minor) {
            double minor_tick = major_tick * minor;
            double next_major_tick = std::pow(log_base, exp + 1);
            if (minor_tick >= min && minor_tick <= max && minor_tick < next_major_tick) {
                ticks.push_back(minor_tick);
                labels.push_back(""); // No label for minor ticks
            }
        }
    }

    // If no major ticks, interpolate log-spaced ticks between min and max
    if (!has_major) {
        int N = 5; // Number of ticks (adjust as needed)
        double log_min = std::log(min);
        double log_max = std::log(max);
        for (int i = 0; i < N; ++i) {
            double frac = (N == 1) ? 0.5 : static_cast<double>(i) / (N - 1);
            double tick = std::exp(log_min + frac * (log_max - log_min));
            ticks.push_back(tick);
            std::ostringstream oss;
            oss << std::scientific << std::setprecision(4) << tick;
            labels.push_back(oss.str());
        }
    }

    // Sort ticks and labels together
    std::vector<std::pair<double, std::string>> tick_pairs;
    for (size_t i = 0; i < ticks.size(); ++i) {
        tick_pairs.emplace_back(ticks[i], labels[i]);
    }
    std::sort(tick_pairs.begin(), tick_pairs.end(),
              [](const auto& a, const auto& b) { return a.first < b.first; });

    // Unpack
    ticks.clear();
    labels.clear();
    for (const auto& pair : tick_pairs) {
        ticks.push_back(pair.first);
        labels.push_back(pair.second);
    }

    return {ticks, labels};
}
