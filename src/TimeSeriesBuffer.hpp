#pragma once
#include <vector>
#include <mutex>
#include <utility>
#include <thread>
#include <condition_variable>
#include <functional>

template<typename Timestamp, typename Value>
class TimeSeriesBuffer {
public:
    TimeSeriesBuffer(double preload_factor = 0.2);
    ~TimeSeriesBuffer();

    // Delete copy semantics
    TimeSeriesBuffer(const TimeSeriesBuffer&) = delete;
    TimeSeriesBuffer& operator=(const TimeSeriesBuffer&) = delete;

    // Define move semantics
    TimeSeriesBuffer(TimeSeriesBuffer&& other) noexcept;
    TimeSeriesBuffer& operator=(TimeSeriesBuffer&& other) noexcept;

    void initialize(const std::vector<std::pair<Timestamp, Value>>& initial_data);
    void setRange(Timestamp start, Timestamp end, const std::function<void(Timestamp, Timestamp)>& preload_callback);
    void addData(const std::vector<std::pair<Timestamp, Value>>& new_data);
    std::vector<std::pair<Timestamp, Value>> getData();

private:
    void cleanup();

    std::vector<std::pair<Timestamp, Value>> data_;
    Timestamp current_start_{}, current_end_{};
    double preload_factor_;
    std::mutex data_mutex_;

    std::function<void(Timestamp, Timestamp)> preload_callback_;
    std::thread background_thread_;
    std::mutex preload_mutex_;
    std::condition_variable preload_condition_;
    bool stop_background_thread_;
};
