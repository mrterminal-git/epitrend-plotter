#include "WindowPlots.hpp"

WindowPlots::WindowPlots(const std::string& label) : label_(label) {}

WindowPlots::~WindowPlots() {}

// Warning: This function moves the ownership renderable_plot object
void WindowPlots::addRenderablePlot(const std::string& plot_label, std::unique_ptr<RenderablePlot> renderable_plot) {
    // Check if the plot already exists
    if (renderable_plots_.find(plot_label) != renderable_plots_.end()) {
        // Ignore if the plot already exists
        return;
    }
    renderable_plots_.emplace(plot_label, std::move(renderable_plot));
}

bool WindowPlots::hasRenderablePlot(const std::string& plot_label) {
    return renderable_plots_.find(plot_label) != renderable_plots_.end();
}

// Note: This function returns a reference to the renderable plot object
// Note: This function should always be called after hasRenderablePlot() returns true
RenderablePlot& WindowPlots::getRenderablePlot(const std::string& plot_label) {
    if (!hasRenderablePlot(plot_label)) {
        throw std::runtime_error("Error in WindowsPlot::getRenderablePlot call: Plot does not exist");
    }
    return *renderable_plots_.at(plot_label);
}

const std::map<std::string, std::unique_ptr<RenderablePlot>>& WindowPlots::getRenderablePlots() const {
    return renderable_plots_;
}

void WindowPlots::clearAllRenderablePlots() {
    renderable_plots_.clear();
}

void WindowPlots::removeRenderablePlot(const std::string& plot_label) {
    renderable_plots_.erase(plot_label);
}

std::vector<std::string> WindowPlots::getRenderablePlotLabels() const {
    std::vector<std::string> labels;
    for (const auto& [label, plot] : renderable_plots_) {
        labels.push_back(label);
    }
    return labels;
}
