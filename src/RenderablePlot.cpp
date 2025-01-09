#include <iostream>
#include "RenderablePlot.hpp"

RenderablePlot::RenderablePlot(const std::string& label, bool real_time)
    : label_(label), real_time_(real_time), plot_range_({0, 0}) {}

// Move constructor
RenderablePlot::RenderablePlot(RenderablePlot&& other) noexcept
    : label_(std::move(other.label_)),
      window_label_(std::move(other.window_label_)),
      plot_range_(std::move(other.plot_range_)),
      real_time_(other.real_time_),
      plot_id_(other.plot_id_),
      data_(std::move(other.data_)),
      data_to_y_axis_(std::move(other.data_to_y_axis_)),
      range_callback_(std::move(other.range_callback_)) {}

// Move assignment operator
RenderablePlot& RenderablePlot::operator=(RenderablePlot&& other) noexcept {
    if (this != &other) {
        std::lock_guard<std::mutex> lock(data_mutex_);
        label_ = std::move(other.label_);
        window_label_ = std::move(other.window_label_);
        plot_range_ = std::move(other.plot_range_);
        real_time_ = other.real_time_;
        plot_id_ = other.plot_id_;
        data_ = std::move(other.data_);
        data_to_y_axis_ = std::move(other.data_to_y_axis_);
        range_callback_ = std::move(other.range_callback_);
    }
    return *this;
}

void RenderablePlot::setLabel(const std::string& label) {
    label_ = label;
}

void RenderablePlot::setWindowLabel(const std::string& window_label) {
    window_label_ = window_label;
}

void RenderablePlot::setPlotRange(Timestamp start, Timestamp end) {
    plot_range_ = {start, end};
}

void RenderablePlot::setRealTime(bool real_time) {
    real_time_ = real_time;
}

void RenderablePlot::setRangeCallback(const RangeCallback& callback) {
    range_callback_ = callback;
}

void RenderablePlot::setPlotId(long long id) { plot_id_ = id; }

const std::string& RenderablePlot::getLabel() const {
    return label_;
}

const std::string& RenderablePlot::getWindowLabel() const {
    return window_label_;
}

const std::pair<RenderablePlot::Timestamp, RenderablePlot::Timestamp>& RenderablePlot::getPlotRange() const {
    return plot_range_;
}

long long RenderablePlot::getPlotId() const { return plot_id_; }

bool RenderablePlot::isRealTime() const {
    return real_time_;
}

void RenderablePlot::notifyRangeChange(Timestamp start, Timestamp end) const {
    if (range_callback_) {
        range_callback_(start, end);
    }
}

void RenderablePlot::print() const {
    std::cout << "RenderablePlot: " << label_ << "\n";
    std::cout << "Window label: " << window_label_ << "\n";
    std::cout << "Real-time: " << real_time_ << "\n";
    std::cout << "Plot range: " << plot_range_.first << " - " << plot_range_.second << "\n";
    std::cout << "Data: " << "\n";
    for (const auto& [series_label, data] : data_) {
        std::cout << "Series: " << series_label << "\n";

        // Print all data points
        for (const auto& [timestamp, value] : data) {
            std::cout << timestamp << ": " << value << "\n";
        }
    }
}

const std::vector<std::string> RenderablePlot::getAllSensors() const {
    std::vector<std::string> sensors;
    // Pre-allocate space
    sensors.reserve(data_.size());

    for (const auto& [sensor, _] : data_) {
        sensors.push_back(sensor);
    }
    return sensors;
}




// ============================================
// Data Management
// ============================================
void RenderablePlot::setData(const std::string& series_label, const RenderablePlot::DataSeries& data) {
    // Lock the mutex
    std::lock_guard<std::mutex> lock(data_mutex_);
    data_[series_label] = data;
}

void RenderablePlot::setAllData(const std::map<std::string, RenderablePlot::DataSeries> data) {
    // Lock the mutex
    std::lock_guard<std::mutex> lock(data_mutex_);
    data_ = data;
}

// UNSAFE ACCESS
const RenderablePlot::DataSeries& RenderablePlot::getData(const std::string& sensor) const {
    return data_.at(sensor);
}

// UNSAFE ACCESS
const std::map<std::string, RenderablePlot::DataSeries> RenderablePlot::getAllData() const {
    return data_;
}

// SAFE ACCESS
RenderablePlot::DataSeries RenderablePlot::getDataSnapshot(const std::string& series_label) {
    std::lock_guard<std::mutex> lock(data_mutex_);
    auto it = data_.find(series_label);
    if (it != data_.end()) {
        return it->second;
    }
    // Return empty data series if not found
    return DataSeries(); 
}

std::vector<std::string> RenderablePlot::getSensorsForYAxis(ImAxis y_axis) const {
    std::vector<std::string> sensors;
    for (const auto& [sensor, axis] : data_to_y_axis_) {
        if (axis == y_axis) {
            sensors.push_back(sensor);
        }
    }
    return sensors;
}

// ============================================
// Multiple axis support
// ============================================
void RenderablePlot::addYAxisForSensor(const std::string& series_label, ImAxis y_axis) {
    data_to_y_axis_[series_label] = y_axis;
}

ImAxis RenderablePlot::getYAxisForSensor(const std::string& series_label) const {
    auto it = data_to_y_axis_.find(series_label);
    if (it != data_to_y_axis_.end()) {
        return it->second;
    } 
    // Default to Y1 axis
    return ImAxis_Y1;
}

void RenderablePlot::deleteYAxisForSensor(const std::string& series_label) {
    if (data_to_y_axis_.find(series_label) != data_to_y_axis_.end()) {
        data_to_y_axis_.erase(series_label);
    }
}

void RenderablePlot::clearYAxes() {
    data_to_y_axis_.clear();
}