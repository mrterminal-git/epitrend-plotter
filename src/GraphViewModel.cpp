#include <iostream>
#include <cmath>

#include "GraphViewModel.hpp"

GraphViewModel::GraphViewModel(std::mutex& mutex)
    : update_viewModel_mutex_(mutex) {}
void GraphViewModel::addRenderablePlot(RenderablePlot& object) {
    // Set the plot ID
    object.setPlotId(next_plot_id_++);

    renderable_plots_.push_back(object);
}

std::vector<RenderablePlot>& GraphViewModel::getRenderablePlots() {
    return renderable_plots_;
}

void GraphViewModel::clear() {
    renderable_plots_.clear();
}

void GraphViewModel::setPlottableSensors(const std::vector<std::string>& sensors) {
    plottable_sensors_ = sensors;
}

const std::vector<std::string>& GraphViewModel::getPlottableSensors() const {
    return plottable_sensors_;
}

AddPlotPopupState& GraphViewModel::getAddPlotPopupState() {
    return add_plot_popup_state_;
}

void GraphViewModel::updatePlotsWithData(const DataManager& dataManager) {
    std::vector<RenderablePlot> temp_plots;

    // Get the all time-series data for each sensor from dataManager
    const auto& buffers = dataManager.getBuffers();

    for (auto& renderable_plot : renderable_plots_) {
        // Copy the plot to the temporary buffer
        temp_plots.push_back(renderable_plot);

        // Loop through the sensors in each renderable plot
        for (const auto& [sensor, _] : renderable_plot.getAllData()) {
            // Grab the time-series data for that sensor from dataManager
            if (buffers.find(sensor) != buffers.end()) {
                // Update the time-series data in the renderable plot
                // Only update the data within the plot range
                auto plot_range = renderable_plot.getPlotRange();
                auto data = buffers.at(sensor).getDataMap();
                RenderablePlot::DataSeries data_in_range;

                // Find the lowest timestamps in the plot range
                auto start = data.begin();
                auto end = data.begin();

                // Deduce the end iterator from the start iterator and the Timestamp spacing
                DataManager::Timestamp spacing;
                if (data.size() > 1) {
                    spacing = data.rbegin()->first - data.begin()->first;
                    spacing /= data.size() - 1;
                } else {
                    return;
                }

                // Find the start iterator using the spacing and the plot range
                for (int search_start_loop_counter = 0;
                start->first < plot_range.first && search_start_loop_counter < 10;
                ++search_start_loop_counter) {
                    int num_elements = (start->first - plot_range.first) / spacing;

                    // Check if the start iterator is already pointing the correct plot range start
                    if (num_elements <= 0) {
                        break;
                    }

                    // Check the distance between the current start iterator and the beginning of data object
                    if (distance(start, data.end()) > num_elements) {
                        start = next(start, num_elements);
                    } else {
                        // Perform a bi-section search from the beginning of data object to the current start iterator
                        auto it = data.begin();
                        auto it2 = start;
                        while (distance(it, it2) > 1) {
                            auto mid = next(it, distance(it, it2) / 2);
                            if (mid->first < plot_range.first) {
                                it = mid;
                            } else {
                                it2 = mid;
                            }
                        }
                        start = it;

                        break;
                    }
                }

                // Linear search the rest of the way
                if (start->first < plot_range.first) {
                    for (; start != data.end(); ++start) {
                        if (start->first >= plot_range.first) {
                            start = prev(start, 1);
                            break;
                        }
                    }
                }

                // Find the end iterator using the spacing and the plot range
                for (int search_end_loop_counter = 0;
                end->first < plot_range.second && search_end_loop_counter < 10;
                ++search_end_loop_counter) {
                    int num_elements = (plot_range.second - end->first) / spacing;

                    // Check the distance between the current end iterator and the end of data object
                    if (distance(end, data.end()) > num_elements) {
                        end = next(end, num_elements);
                    } else {
                        // Perform a bi-section search from the current end iterator to the end of data object
                        auto it = end;
                        auto it2 = data.end();
                        while (distance(it, it2) > 1) {
                            auto mid = next(it, distance(it, it2) / 2);
                            if (mid->first > plot_range.second) {
                                it2 = mid;
                            } else {
                                it = mid;
                            }
                        }
                        end = it;

                        break;
                    }
                    search_end_loop_counter++;
                }

                // Linear search the rest of the way
                if (end->first < plot_range.second) {
                    for (; end != data.end(); ++end) {
                        if (end->first > plot_range.second) {
                            break;
                        }
                    }
                }

                // Copy the data in the plot range
                for (auto it = start; it != end; ++it) {
                    data_in_range.push_back({it->first, it->second});
                }

                temp_plots.back().setData(sensor, data_in_range);
            }
        }
    }

    // Lock the mutex only when updating the view model
    {
        std::lock_guard<std::mutex> lock(update_viewModel_mutex_);
        renderable_plots_ = std::move(temp_plots);
    }
}

std::pair<std::vector<DataManager::Timestamp>, std::vector<DataManager::Value>> GraphViewModel::getDownsampledData(
    const RenderablePlot& plot, const std::string& sensor, double range, int num_pixels) {
    std::vector<DataManager::Timestamp> timestamps;
    std::vector<DataManager::Value> values;
    const auto& data = plot.getData(sensor);

    if (data.empty()) {
        return {timestamps, values};
    }

    int step_size = static_cast<int>(range / num_pixels) * 4;
    if (step_size <= 0) {
        step_size = 1;
    }

    int num_points_to_render = data.size() / step_size;
    if (num_points_to_render <= 0) {
        num_points_to_render = data.size();
    }

    timestamps.reserve(num_points_to_render);
    values.reserve(num_points_to_render);

    int counter = 0;
    int data_pos = 0;
    for (int i = 0;
        i < num_points_to_render && data_pos < data.size();
        ++i, data_pos += step_size) {
        timestamps.push_back(data.at(data_pos).first);
        values.push_back(data.at(data_pos).second);
        counter++;
    }

    std::cout << "====================================\n";
    std::cout << "sensor: " << sensor << "\n";
    std::cout << "range: " << range << "\n";
    std::cout << "data.size(): " << data.size() << "\n";
    std::cout << "num_pixels: " << num_pixels << "\n";
    std::cout << "num_points_to_render: " << num_points_to_render << "\n";
    std::cout << "step_size: " << step_size << "\n";
    std::cout << "counter: " << counter << "\n";

    return {timestamps, values};
}
