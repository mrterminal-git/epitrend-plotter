#include "TimeSeriesBuffer.hpp"
#include <iostream>
#include <algorithm>
#include <cmath>

template<typename Timestamp, typename Value>
TimeSeriesBuffer<Timestamp, Value>::TimeSeriesBuffer(double preload_factor)
    : preload_factor_(preload_factor), stop_background_thread_(false) {}

template<typename Timestamp, typename Value>
TimeSeriesBuffer<Timestamp, Value>::~TimeSeriesBuffer() {
    stop_background_thread_ = true;
    preload_condition_.notify_one();

    if (background_thread_.joinable()) {
        background_thread_.join();
    }
}

// Move constructor
template<typename Timestamp, typename Value>
TimeSeriesBuffer<Timestamp, Value>::TimeSeriesBuffer(TimeSeriesBuffer&& other) noexcept
    : data_(std::move(other.data_)),
      current_start_(std::move(other.current_start_)),
      current_end_(std::move(other.current_end_)),
      preload_factor_(other.preload_factor_),
      preload_callback_(std::move(other.preload_callback_)),
      stop_background_thread_(other.stop_background_thread_) {
    // Move the background thread if it is joinable
    if (other.background_thread_.joinable()) {
        background_thread_ = std::move(other.background_thread_);
    }
    other.stop_background_thread_ = true;
}

// Move assignment operator
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

    preload_callback_ = preload_callback;

    if (!background_thread_.joinable()) {
        background_thread_ = std::thread([this]() {
            while (!stop_background_thread_) {
                {
                    std::unique_lock<std::mutex> lock(preload_mutex_);
                    preload_condition_.wait(lock);
                }

                Timestamp preload_start = current_start_ - (current_end_ - current_start_) * preload_factor_;
                Timestamp preload_end = current_end_ + (current_end_ - current_start_) * preload_factor_;

                preload_callback_(preload_start, preload_end);
            }
        });
    }
    preload_condition_.notify_one();
}

template<typename Timestamp, typename Value>
void TimeSeriesBuffer<Timestamp, Value>::addData(const std::vector<std::pair<Timestamp, Value>>& new_data) {
    // Lock the data mutex and copy the temporary buffer into the main buffer
    {
        std::lock_guard<std::mutex> lock(data_mutex_);
        data_.insert(new_data.begin(), new_data.end());
    }
    cleanup();
    enforceSizeLimit();
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
std::map<Timestamp,Value> TimeSeriesBuffer<Timestamp, Value>::getDataMap() const {
    return data_;
}

template<typename Timestamp, typename Value>
void TimeSeriesBuffer<Timestamp, Value>::cleanup() {
    std::lock_guard<std::mutex> lock(data_mutex_);
    Timestamp retention_start = current_start_ - (current_end_ - current_start_) * preload_factor_;
    auto it = data_.lower_bound(retention_start);

    Timestamp retention_end = current_end_ + (current_end_ - current_start_) * preload_factor_;

    // Remove all entries before retention_start
    data_.erase(data_.begin(), it);

    // Remove all entries after retention_end
    it = data_.upper_bound(retention_end);

    // Check if the iterator is valid
    if (it != data_.end()) {
        data_.erase(it, data_.end());
    }
}

template<typename Timestamp, typename Value>
void TimeSeriesBuffer<Timestamp, Value>::enforceSizeLimit() {
    // Check if the data size exceeds the maximum limit
    if (data_.size() <= max_data_points_) {
        return;
    }

    // DEBUG
    std::cout << "-----------------------------Enforcing size limit-----------------------------\n";

    // Convert the map to a vector of TimeSeriesPoint
    std::vector<TimeSeriesPoint> points;
    points.reserve(data_.size());
    for (const auto& entry : data_) {
        points.push_back({static_cast<double>(entry.first), static_cast<double>(entry.second)});
    }

    // Prepare a vector to hold the downsampled data
    std::vector<TimeSeriesPoint> downsampled_points;
    downsampled_points.reserve(max_data_points_);

    // Apply the LTTB algorithm
    LargestTriangleThreeBuckets<TimeSeriesPoint, double, &TimeSeriesPoint::timestamp, &TimeSeriesPoint::value>::Downsample(
        points.begin(), points.size(), std::back_inserter(downsampled_points), max_data_points_);

    // Convert the downsampled vector back to a map
    std::map<Timestamp, Value> downsampled_data;
    for (const auto& point : downsampled_points) {
        downsampled_data[static_cast<Timestamp>(point.timestamp)] = static_cast<Value>(point.value);
    }

    // Lock the data mutex and replace the data with the downsampled data
    std::lock_guard<std::mutex> lock(data_mutex_);
    data_ = std::move(downsampled_data);
}

template class TimeSeriesBuffer<double, double>;
