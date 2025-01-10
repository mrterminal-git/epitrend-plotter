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
    enum class ScaleType {
        Linear,
        Logirithmic
    };
    struct YAxisProperties {
        std::string label = "";
        Value min = 0.1;
        Value max = 1.0;
        ScaleType scale_type = ScaleType::Linear;
        double log_base = 10;
        bool user_set_range = false; // important to let rendering logic know if the user has set the range

        void reset() {
            label = "";
            min = 0.1;
            max = 1.0;
            ScaleType scale_type = ScaleType::Linear;
            log_base = 10;
            user_set_range = false;
        }
    };
    void addYAxisForSensor(const std::string& series_label, ImAxis y_axis);
    ImAxis getYAxisForSensor(const std::string& series_label) const;
    std::vector<std::string> getSensorsForYAxis(ImAxis y_axis) const;
    void deleteYAxisForSensor(const std::string& series_label);
    void clearYAxes();
    void setYAxisProperties(ImAxis y_axis, const YAxisProperties& properties);
    void setYAxisPropertiesMin(ImAxis y_axis, Value min);
    void setYAxisPropertiesMax(ImAxis y_axis, Value max);
    void setYAxisPropertiesLabel(ImAxis y_axis, const std::string& label);
    void setYAxisPropertiesScale(ImAxis y_axis, ScaleType scale_type);
    void setYAxisPropertiesLogBase(ImAxis y_axis, double log_base);
    void setYAxisPropertiesUserSetRange(ImAxis y_axis, bool user_set_range);
    YAxisProperties getYAxisProperties(ImAxis y_axis);
    Value getYAxisPropertiesMin(ImAxis y_axis);
    Value getYAxisPropertiesMax(ImAxis y_axis);
    ScaleType getYAxisPropertiesScaleType(ImAxis y_axis);
    double getYAxisPropertiesLogBase(ImAxis y_axis);
    bool getYAxisPropertiesUserSetRange(ImAxis y_axis);

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
    std::map<ImAxis, YAxisProperties> y_axis_properties_;

};
