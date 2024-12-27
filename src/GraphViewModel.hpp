#pragma once
#include "RenderablePlot.hpp"
#include <vector>
#include <string>

class GraphViewModel {
public:
    GraphViewModel() = default;

    void addRenderablePlot(const RenderablePlot& object);
    const std::vector<RenderablePlot>& getRenderablePlots() const;

    void clear(); // Clear all renderables

    // Plottable sensors storage
    void setPlottableSensors(const std::vector<std::string>& sensors);
    const std::vector<std::string>& getPlottableSensors() const;

private:
    // Each renderable plot is a separate window
    std::vector<RenderablePlot> renderable_plots_;

    // Plottable sensors
    std::vector<std::string> plottable_sensors_;
};
