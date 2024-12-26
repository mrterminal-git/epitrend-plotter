#include "GraphViewModel.hpp"

GraphViewModel::GraphViewModel() {}

void GraphViewModel::addRenderablePlot(const RenderablePlot& object) {
    renderable_plots_.push_back(object);
}

const std::vector<RenderablePlot>& GraphViewModel::getRenderablePlots() const {
    return renderable_plots_;
}

void GraphViewModel::clear() {
    renderable_plots_.clear();
}
