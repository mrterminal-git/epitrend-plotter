#include "TimeSeriesBuffer.hpp"
#include <algorithm>
#include <cmath>

template<typename Timestamp, typename Value>
TimeSeriesBuffer<Timestamp, Value>::TimeSeriesBuffer(double preload_factor)
    : preload_factor_(preload_factor), stop_background_thread_(false) {}

template<typename Timestamp, typename Value>
TimeSeriesBuffer<Timestamp, Value>::~TimeSeriesBuffer() {
    stop_background_thread_ = true;
    if (background_thread_.joinable()) {
        background_thread_.join();
    }
}

template<typename Timestamp, typename Value>
TimeSeriesBuffer<Timestamp, Value>& TimeSeriesBuffer<Timestamp, Value>::operator=(TimeSeriesBuffer&& other) noexcept {
    if (this != &other) {
        stop_background_thread_ = true;
        if (background_thread_.joinable()) {
            background_thread_.join();
        }

        data_ = std::move(other.data_);
        current_start_ = other.current_start_;
        current_end_ = other.current_end_;
        preload_factor_ = other.preload_factor_;
        preload_callback_ = std::move(other.preload_callback_);
        background_thread_ = std::move(other.background_thread_);
        stop_background_thread_ = other.stop_background_thread_;
        other.stop_background_thread_ = true;
    }
    return *this;
}

template<typename Timestamp, typename Value>
void TimeSeriesBuffer<Timestamp, Value>::initialize(const std::map<Timestamp, Value>& initial_data) {
    std::lock_guard<std::mutex> lock(data_mutex_);
    data_ = initial_data;
}

template<typename Timestamp, typename Value>
void TimeSeriesBuffer<Timestamp, Value>::setRange(Timestamp start, Timestamp end, const std::function<void(Timestamp, Timestamp)>& preload_callback) {
    std::lock_guard<std::mutex> lock(data_mutex_);
    current_start_ = start;
    current_end_ = end;

    Timestamp preload_start = start - (end - start) * preload_factor_;
    Timestamp preload_end = end + (end - start) * preload_factor_;

    preload_callback_ = preload_callback;

    if (!background_thread_.joinable()) {
        background_thread_ = std::thread([this, preload_start, preload_end]() {
            while (!stop_background_thread_) {
                {
                    std::unique_lock<std::mutex> lock(preload_mutex_);
                    preload_condition_.wait(lock);
                }
                preload_callback_(preload_start, preload_end);
            }
        });
    }
    preload_condition_.notify_one();
}

template<typename Timestamp, typename Value>
void TimeSeriesBuffer<Timestamp, Value>::addData(const std::vector<std::pair<Timestamp, Value>>& new_data) {
    std::lock_guard<std::mutex> lock(data_mutex_);
    for (const auto& entry : new_data) {
        data_[entry.first] = entry.second;
    }
    cleanup();
}

template<typename Timestamp, typename Value>
std::vector<std::pair<Timestamp, Value>> TimeSeriesBuffer<Timestamp, Value>::getData() {
    std::lock_guard<std::mutex> lock(data_mutex_);
    std::vector<std::pair<Timestamp, Value>> result;
    for (const auto& entry : data_) {
        result.emplace_back(entry.first, entry.second);
    }
    return result;
}

template<typename Timestamp, typename Value>
std::map<Timestamp,Value> TimeSeriesBuffer<Timestamp, Value>::getDataMap() {
    return data_;
}

template<typename Timestamp, typename Value>
void TimeSeriesBuffer<Timestamp, Value>::cleanup() {
    Timestamp retention_start = current_start_ - (current_end_ - current_start_) * preload_factor_;
    auto it = data_.lower_bound(retention_start);

    // Remove all entries before retention_start
    data_.erase(data_.begin(), it);
}

template class TimeSeriesBuffer<double, double>;
