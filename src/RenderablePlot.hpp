#pragma once

#include <string>
#include <vector>
#include <map>
#include <functional>

class RenderablePlot {
public:
    using Timestamp = double;
    using Value = double;
    using RangeCallback = std::function<void(Timestamp start, Timestamp end)>;

    // Constructor
    RenderablePlot(const std::string& label, bool real_time = true);

    // Setters
    void setLabel(const std::string& label);
    void setWindowLabel(const std::string& window_label);
    void setData(const std::string& series_label, const std::map<Timestamp, Value>& data);
    void setPlotRange(Timestamp start, Timestamp end);
    void setRealTime(bool real_time);
    void setRangeCallback(const RangeCallback& callback);

    // Getters
    const std::string& getLabel() const;
    const std::string& getWindowLabel() const;
    const std::map<Timestamp, Value>& getData(const std::string& series_label) const;
    const std::map<std::string, std::map<Timestamp, Value>> getAllData() const;
    const std::pair<Timestamp, Timestamp>& getPlotRange() const;
    bool isRealTime() const;

    // Print object
    void print() const;

    // Callback invocation
    void notifyRangeChange(Timestamp start, Timestamp end) const;

private:
    std::string window_label_; // Window label
    std::string label_; // Plot label
    bool real_time_;    // Real-time plotting flag
    std::pair<Timestamp, Timestamp> plot_range_; // Plot range
    std::map<std::string, std::map<Timestamp, Value>> data_; // Data for each series
    RangeCallback range_callback_; // Callback for range changes
};
