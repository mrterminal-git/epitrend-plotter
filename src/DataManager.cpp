#include "DataManager.hpp"
#include <iostream> // FOR TESTING
#include <iomanip> // FOR TESTING


// Constructor
DataManager::DataManager() : background_thread_running_(false), config_("config.txt") {
    // Get the InfluxDB connection parameters
    const std::string host = config_.getHost();
    const int port = config_.getPort();
    const std::string org = config_.getOrg();
    const std::string epitrend_bucket = config_.getEpitrendBucket();
    const std::string user = config_.getUser();
    const std::string password = config_.getPassword();
    const std::string precision = config_.getPrecision();
    const std::string token = config_.getToken();

    // Initialize the InfluxDB connection
    influxdb_ = InfluxDatabase(host, port, org, epitrend_bucket, user, password, precision, token, true);

    // Check connection for a maximum of 10 seconds
    for (int i = 0; i < 10; i++) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        if (influxdb_.checkConnection(false)) {
            break;
        } else if (i == 9) {
            // If connection fails after 10 seconds, stop the program
            std::cerr << "Failed to connect to InfluxDB" << "\n";
            std::exit(1);
        }
    }

}

// Destructor
DataManager::~DataManager() {
    stopBackgroundUpdates();
}

// Start background updates
void DataManager::startBackgroundUpdates() {
    background_thread_running_ = true;
    background_thread_ = std::thread(&DataManager::backgroundUpdateTask, this);
}

// Stop background updates
void DataManager::stopBackgroundUpdates() {
    background_thread_running_ = false;
    if (background_thread_.joinable()) {
        background_thread_.join();
    }
}

// Background update task
void DataManager::backgroundUpdateTask() {
    while (background_thread_running_) {
        {
            std::unordered_map<std::string, std::pair<Timestamp, Timestamp>> local_merged_ranges;

            // Copy sensor_ranges_ to a local map
            {
                std::lock_guard<std::mutex> lock(sensor_ranges_mutex_);
                for (const auto& [sensor_id, ranges] : sensor_ranges_) {
                    local_merged_ranges[sensor_id] = mergeRanges(ranges);
                }
            }

            // Update buffers_ based on the local copy of the merged ranges
            {
                std::lock_guard<std::mutex> lock(buffer_mutex_);

                // Iterate through all sensors and update the data
                for (const auto& [sensor_id, merged_range] : local_merged_ranges) {
                    if (local_merged_ranges.empty())
                        continue;

                    buffers_[sensor_id].setRange(
                        merged_range.first, merged_range.second,
                        [this, sensor_id](Timestamp start, Timestamp end) {
                            preloadData(sensor_id, start, end);
                        });
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(1)); // Update every second
    }
}

// Get all buffers. UNSAFE ACCESS
const std::unordered_map<std::string, TimeSeriesBuffer<DataManager::Timestamp, DataManager::Value>>& DataManager::getBuffers() const {
    return buffers_;
}

// Get a snapshot of the buffer for a specific sensor. SAFE ACCESS
std::vector<std::pair<DataManager::Timestamp, DataManager::Value>> DataManager::getBuffersSnapshot(
    const std::string& sensor_label, Timestamp start, Timestamp end) {
    std::lock_guard<std::mutex> lock(buffer_mutex_);

    // Find the buffer
    auto it = buffers_.find(sensor_label);
    if (it == buffers_.end()) {
        return {}; // Return empty if sensor not found
    }

    // Get the buffer and ensure it's sorted
    const auto& buffer = it->second.getData();
    if (buffer.empty()) {
        return {};
    }

    // Use binary search to find the start and end iterators
    auto lower = std::lower_bound(buffer.begin(), buffer.end(), start,
        [](const auto& entry, const Timestamp& value) {
            return entry.first < value;
        });

    auto upper = std::upper_bound(buffer.begin(), buffer.end(), end,
        [](const Timestamp& value, const auto& entry) {
            return value < entry.first;
        });

    // Create a snapshot using the range
    return std::vector<std::pair<Timestamp, Value>>(lower, upper);
}

// Initialize a buffer for a specific machine
void DataManager::addSensor(const std::string& sensor_id) {
    // Lock the buffer mutex
    std::lock_guard<std::mutex> lock(buffer_mutex_);
    buffers_.emplace(sensor_id, TimeSeriesBuffer<Timestamp, Value>());
}

// Update range for a specific machine. Callback is used to preload data and clean up old data according to the new range
void DataManager::updateSensorRange(const std::string& sensor_id, int plot_id, DataManager::Timestamp start, DataManager::Timestamp end) {
    // Lock the buffer and sensor_ranges mutex
    std::lock_guard<std::mutex> lock1(buffer_mutex_);
    std::lock_guard<std::mutex> lock2(sensor_ranges_mutex_);

    auto& ranges = sensor_ranges_[sensor_id];
    ranges[plot_id] = {start, end}; // Update the specific plot's range

    auto [merged_start, merged_end] = mergeRanges(ranges); // Merge ranges across plots

    buffers_[sensor_id].setRange(
        merged_start, merged_end,
        [this, sensor_id](Timestamp preload_start, Timestamp preload_end) {
            preloadData(sensor_id, preload_start, preload_end);
        });
}

// Add new data to a machine's buffer
void DataManager::addSensorData(const std::string& sensor_id, const std::vector<std::pair<Timestamp, Value>>& data) {
    // Lock the mutex
    std::lock_guard<std::mutex> lock(buffer_mutex_);
    if (buffers_.find(sensor_id) != buffers_.end()) {
        buffers_[sensor_id].addData(data);
    }
}

/// Preload data outside the existing range for a specific machine. CURRENTLY IN TESTING
void DataManager::preloadData(std::string sensor_id, Timestamp start, Timestamp end) {
    // Simulate data loading (replace with actual data loading logic)
    std::vector<std::pair<Timestamp, Value>> new_data;
    if (sensor_id == "sensor_1") {
        // FOR DEVELOPMENT PURPOSES ONLY, REMOVE IF STATEMENT AND REPLACE WITH ACTUAL DATA LOADING LOGIC
        for (Timestamp t = start; t <= end; t += 1.0) {
            new_data.emplace_back(t, std::sin(t));
        }

    } else if (sensor_id == "sensor_2") {
        // FOR DEVELOPMENT PURPOSES ONLY, REMOVE IF STATEMENT AND REPLACE WITH ACTUAL DATA LOADING LOGIC
        for (Timestamp t = start; t <= end; t += 1.0) {
            new_data.emplace_back(t, 2.0 * std::cos(t / 1.5));
        }

    } else {
        // *** Load data from InfluxDB ***
        // Prepare the time-series (ts) query to read data statement
        struct ts_read_struct {
            std::string bucket;
            std::string sensor_id;
            std::string timestamp_start;
            std::string timestamp_end;
            std::string read_query;
            void set_read_query(){read_query = "from(bucket: \"" + bucket + "\") "
                "|> range(start: " + timestamp_start + ", stop: " + timestamp_end + ")"
                "|> filter(fn: (r) => r[\"_measurement\"] == \"ts\")"
                "|> filter(fn: (r) => r[\"sensor_id_\"] == \"" + sensor_id + "\")";
            }
        };

        // Prepare influx_time_struct and object
        struct influx_time_struct {
            Timestamp unix_timestamp;
            std::string influx_timestamp;

            void set_influx_timestamp() {
                // Convert unix_timestamp to influx_timestamp
                std::time_t time = unix_timestamp;
                std::tm* tm = std::gmtime(&time);
                int year = tm->tm_year + 1900;
                int month = tm->tm_mon + 1;
                int day = tm->tm_mday;
                int hour = tm->tm_hour;
                int minute = tm->tm_min;
                double second = tm->tm_sec + (unix_timestamp - std::floor(unix_timestamp));

                std::ostringstream oss;
                oss << std::setfill('0') << std::setw(4) << year << "-"
                    << std::setw(2) << month << "-"
                    << std::setw(2) << day << "T"
                    << std::setw(2) << hour << ":"
                    << std::setw(2) << minute << ":"
                    << std::fixed << std::setprecision(2) << std::setw(5) << second << "Z";
                influx_timestamp = oss.str();
            }
        };
        influx_time_struct start_time = {.unix_timestamp = start};
        start_time.set_influx_timestamp();
        influx_time_struct end_time = {.unix_timestamp = end};
        end_time.set_influx_timestamp();

        // Prepare the ts query object
        // Lock the sensor_name_to_id_ mutex
        ts_read_struct ts_read;
        {
            std::lock_guard<std::mutex> lock(sensor_name_to_id_mutex_);
            ts_read = {
                    .bucket = "ALL", // REPLACE WITH CONFIG FILE
                    .sensor_id = sensor_name_to_id_[sensor_id],
                    .timestamp_start = start_time.influx_timestamp,
                    .timestamp_end = end_time.influx_timestamp
            };
        }
        ts_read.set_read_query();

        // Query the data from the database
        std::string response;
        std::cout << "Querying data for sensor: " << sensor_id << "\n";
        std::cout << "Query: " << ts_read.read_query << "\n";
        influxdb_.queryData2(response, ts_read.read_query);

        // Parse the response
        std::vector<std::unordered_map<std::string,std::string>> parsed_response =
            influxdb_.parseQueryResponse(response);

        // Extract the data from the parsed response
        for (const auto& element: parsed_response) {
            // Convert the time string to Unix time double
            std::string time_str = element.at("_time");
            std::tm tm = {};
            std::istringstream ss(time_str);
            ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S.%fZ");
            std::time_t time = std::mktime(&tm);

            // Convert from UTC to AEST time
            time += 10 * 3600; // 10 hours

            // Append time and value to the data vector
            new_data.push_back(std::make_pair(time, std::stod(element.at("_value"))));
        }

    }

    // Create a copy of sensor_id
    std::string sensor_id_copy = sensor_id;

    std::cout << "Preloaded data for sensor: " << sensor_id_copy << "\n";
    // Set precision for output
    std::cout << std::fixed << std::setprecision(10);
    std::cout << "Start: " << start << ", End: " << end << "\n";

    addSensorData(sensor_id_copy, new_data);
}

// Merge ranges across all plots for a specific sensor
std::pair<DataManager::Timestamp, DataManager::Timestamp> DataManager::mergeRanges(
    const std::unordered_map<int, std::pair<DataManager::Timestamp, DataManager::Timestamp>>& ranges) {
    if (ranges.empty()) return {0, 0};
    Timestamp min_start = ranges.begin()->second.first;
    Timestamp max_end = ranges.begin()->second.second;
    for (const auto& [_, range] : ranges) {
        min_start = std::min(min_start, range.first);
        max_end = std::max(max_end, range.second);
    }
    return {min_start, max_end};
}

// Set the range for a specific sensor
void DataManager::setSensorRange(const std::string& sensor_id, int plot_id, Timestamp start, Timestamp end) {
    // Lock the mutex
    std::lock_guard<std::mutex> lock(sensor_ranges_mutex_);
    sensor_ranges_[sensor_id][plot_id] = {start, end};
}




// ==================================================
// InfluxDB connection
// ==================================================
void DataManager::setInfluxDBSensors() {
    // Prepare name-series (ns) query read all data statement
    struct ns_read_all_struct {
        std::string bucket;
        std::string read_query;
        void set_read_query(){read_query = "from(bucket: \"" + bucket + "\") "
            "|> range(start: -50y, stop: 100y)"
            "|> filter(fn: (r) => r[\"_measurement\"] == \"ns\")";
        }
    };

    // Prepare the query structs. REPLACE WITH CONFIG FILE
    ns_read_all_struct ns_read_all_epitrend = {.bucket = "ALL"};
    ns_read_all_epitrend.set_read_query();

    // Read the InfluxDB "ns" table for all data
    std::string response;
    response = "";
    influxdb_.queryData2(response, ns_read_all_epitrend.read_query);

    // Parse the response
    std::vector<std::unordered_map<std::string, std::string>> parsed_response = influxdb_.parseQueryResponse(response);

    // Add the sensors to the DataManager
    // Lock the sensor_name_to_id_ and sensor_id_to_name_ mutexes
    {
        std::lock_guard<std::mutex> lock1(sensor_name_to_id_mutex_);
        std::lock_guard<std::mutex> lock2(sensor_id_to_name_mutex_);

        for(const auto& element : parsed_response) {
            // Check sensor_ and sensor_id_ keys exist (ns table should contain these keys)
            if(element.find("sensor_") == element.end() || element.find("_value") == element.end()) {
                std::cerr << "Error in DataManager::setInfluxDBSensors call: "
                "sensor_ or sensor_id key not found in ns table\n";
                throw std::runtime_error("Error in DataManager::setInfluxDBSensors call: "
                "sensor_ or sensor_id key not found in ns table\n");
            }

            // Cache the sensor-name and sensor-id pairs
            sensor_name_to_id_[element.at("sensor_")] = element.at("_value");
            sensor_id_to_name_[element.at("_value")] = element.at("sensor_");

        }
    }

    // Lock the buffer and sensor_name_to_id_ mutexes
    {
        std::lock_guard<std::mutex> lock1(buffer_mutex_);
        std::lock_guard<std::mutex> lock2(sensor_name_to_id_mutex_);

        // Add the sensors to the DataManager
        for(const auto& [sensor_name, sensor_id] : sensor_name_to_id_) {
            buffers_.emplace(sensor_name, TimeSeriesBuffer<Timestamp, Value>());
        }
    }
}
