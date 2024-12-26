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
private:
    std::vector<RenderablePlot> renderable_plots_;
};
