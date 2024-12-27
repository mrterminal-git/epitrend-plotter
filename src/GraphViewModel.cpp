#include "GraphViewModel.hpp"

void GraphViewModel::addRenderablePlot(const RenderablePlot& object) {
    renderable_plots_.push_back(object);
}

const std::vector<RenderablePlot>& GraphViewModel::getRenderablePlots() const {
    return renderable_plots_;
}

void GraphViewModel::clear() {
    renderable_plots_.clear();
}

void GraphViewModel::setPlottableSensors(const std::vector<std::string>& sensors) {
    plottable_sensors_ = sensors;
}

const std::vector<std::string>& GraphViewModel::getPlottableSensors() const {
    return plottable_sensors_;
}
