#include "WindowPlots.hpp"

WindowPlots::WindowPlots(const std::string& label) : label_(label) {}

WindowPlots::~WindowPlots() {}

// Move constructor
WindowPlots::WindowPlots(WindowPlots&& other) noexcept
    : label_(std::move(other.label_)),
      renderable_plots_(std::move(other.renderable_plots_)),
      pos_x_(other.pos_x_),
      pos_y_(other.pos_y_),
      width_(other.width_),
      height_(other.height_) {}

// Move assignment operator
WindowPlots& WindowPlots::operator=(WindowPlots&& other) noexcept {
    if (this != &other) {
        label_ = std::move(other.label_);
        renderable_plots_ = std::move(other.renderable_plots_);
        pos_x_ = other.pos_x_;
        pos_y_ = other.pos_y_;
        width_ = other.width_;
        height_ = other.height_;
    }
    return *this;
}

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

std::string WindowPlots::getLabel() const {
    return label_;
}




// ============================================
// Window Properties
// ============================================

void WindowPlots::setXPosition(float x) {
    if (x < 0) {
        pos_x_ = 0;
    } else {
        pos_x_ = x;
    }
}

void WindowPlots::setYPosition(float y) {
    if (y < 0) {
        pos_y_ = 0;
    } else {
        pos_y_ = y;
    }
}

void WindowPlots::setWidth(float width) {
    if (width < 0) {
        width_ = 1080;
    } else {
        width_ = width;
    }
}

void WindowPlots::setHeight(float height){
    if (height < 0) {
        height_ = 720;
    } else {
        height_ = height;
    }
}

std::pair<float,float> WindowPlots::getPosition() const {
    return {pos_x_, pos_y_};
}

std::pair<float,float> WindowPlots::getSize() const {
    return {width_,height_};
}
