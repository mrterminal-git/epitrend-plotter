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

void RenderablePlot::setData(const std::string& series_label, const std::map<Timestamp, Value>& data) {
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

const std::string& RenderablePlot::getLabel() const {
    return label_;
}

const std::string& RenderablePlot::getWindowLabel() const {
    return window_label_;
}

const std::map<RenderablePlot::Timestamp, RenderablePlot::Value>& RenderablePlot::getData(const std::string& series_label) const {
    static const std::map<Timestamp, Value> empty_data;
    auto it = data_.find(series_label);
    if (it != data_.end()) {
        return it->second;
    }
    return empty_data;
}

const std::map<std::string, std::map<RenderablePlot::Timestamp, RenderablePlot::Value>> RenderablePlot::getAllData() const {
    return data_;
}

const std::pair<RenderablePlot::Timestamp, RenderablePlot::Timestamp>& RenderablePlot::getPlotRange() const {
    return plot_range_;
}

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
        for (const auto& [timestamp, value] : data) {
            std::cout << timestamp << ": " << value << "\n";
        }
    }
}
