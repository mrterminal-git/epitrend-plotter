#include <iostream>
#include "RenderablePlot.hpp"

RenderablePlot::RenderablePlot(const std::string& label, bool real_time)
    : label_(label), real_time_(real_time), plot_range_({0, 0}) {}

void RenderablePlot::setLabel(const std::string& label) {
    label_ = label;
}

void RenderablePlot::setWindowLabel(const std::string& window_label) {
    window_label_ = window_label;
}

void RenderablePlot::setData(const std::string& series_label, const RenderablePlot::DataSeries& data) {
    data_[series_label] = data;
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

const RenderablePlot::DataSeries& RenderablePlot::getData(const std::string& sensor) const {
    return data_.at(sensor);
}

const std::map<std::string, RenderablePlot::DataSeries> RenderablePlot::getAllData() const {
    return data_;
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
        const auto& [timestamps, values] = data;

        if (timestamps.size() != values.size()) {
            std::cout << "Error: timestamps and values are not the same size\n";
            return;
        }

        for (size_t i = 0; i < timestamps.size(); ++i) {
            std::cout << timestamps[i] << ": " << values[i] << "\n";
        }
    }
}
