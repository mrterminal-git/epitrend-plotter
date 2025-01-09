#pragma once

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <implot.h>
#include <mutex>

class RenderablePlot {
public:
    using Timestamp = double;
    using Value = double;
    using RangeCallback = std::function<void(Timestamp start, Timestamp end)>;
    using DataSeries = std::vector<std::pair<Timestamp, Value>>;

    // Constructor
    RenderablePlot(const std::string& label, bool real_time = true);

    // Delete copy constructor and copy assignment operator
    RenderablePlot(const RenderablePlot&) = delete;
    RenderablePlot& operator=(const RenderablePlot&) = delete;

    // Define move constructor and move assignment operator
    RenderablePlot(RenderablePlot&& other) noexcept;
    RenderablePlot& operator=(RenderablePlot&& other) noexcept;

    // Setters
    void setLabel(const std::string& label);
    void setWindowLabel(const std::string& window_label);
    void setPlotRange(Timestamp start, Timestamp end);
    void setRealTime(bool real_time);
    void setPlotId(long long id);

    // Getters
    const std::string& getLabel() const;
    const std::string& getWindowLabel() const;
    const std::pair<Timestamp, Timestamp>& getPlotRange() const;
    bool isRealTime() const;
    long long getPlotId() const;
    const std::vector<std::string> getAllSensors() const;

    // Print object
    void print() const;

    // ============================================
    // Data Management
    // ============================================
    void setData(const std::string& series_label, const DataSeries& data);
    void setAllData(const std::map<std::string, DataSeries> data);
    const DataSeries& getData(const std::string& series_label) const; // UNSAFE ACCESS
    const std::map<std::string, DataSeries> getAllData() const;
    DataSeries getDataSnapshot(const std::string& series_label); // SAFE ACCESS

    // ============================================
    // Callback invocation
    // ============================================
    void notifyRangeChange(Timestamp start, Timestamp end) const;
    void setRangeCallback(const RangeCallback& callback);


    // ============================================
    // Multiple axis support
    // ============================================
    void addYAxisForSensor(const std::string& series_label, ImAxis y_axis);
    ImAxis getYAxisForSensor(const std::string& series_label) const;
    void deleteYAxisForSensor(const std::string& series_label);
    void clearYAxes();

private:
    std::string window_label_; // Window label
    std::string label_; // Plot label
    bool real_time_;    // Real-time plotting flag
    std::pair<Timestamp, Timestamp> plot_range_; // Plot range
    std::map<std::string, DataSeries> data_; // Data for each sensor and corresponding time-series
    RangeCallback range_callback_; // Callback for range changes
    long long plot_id_;
    
    // ============================================
    // Data Management
    // ============================================
    std::mutex data_mutex_;

    // ============================================
    // Multiple axis support
    // ============================================
    ImAxis primary_x_axis_ = ImAxis_X1;
    std::map<std::string, ImAxis> data_to_y_axis_;
    std::map<ImAxis, std::string> y_axis_labels_;

    
};
