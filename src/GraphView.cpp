#include <array>
#include <vector>
#include <cmath>
#include <iostream>
#include <set>
#include <string_view>

#include <fmt/format.h>
#include <imgui.h>
#include <implot.h>

#include "GraphView.hpp"

// Constructor
GraphView::GraphView(GraphViewModel& viewModel) : viewModel_(viewModel) {}

void GraphView::Draw(const std::string label)
{
    constexpr static auto window_flags =
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;
    constexpr static auto window_size = ImVec2(1280.0F, 720.0F);
    constexpr static auto window_pos = ImVec2(0.0F, 0.0F);

    ImGui::SetNextWindowSize(window_size);
    ImGui::SetNextWindowPos(window_pos);

    ImGui::Begin(label.data(), nullptr, window_flags);

    renderAll();

    ImGui::End();
}

// ==============================
// renderAddPlotPopup
// ==============================

// Helper function to render a date/time field
void renderDateTimeField(const char* label, char* year, char* month, char* day, char* hour, char* minute, char* second) {
    ImGui::Text(label);
    ImGui::SetNextItemWidth(50.0f); ImGui::InputText((std::string("###Year") + label).c_str(), year, 5);
    ImGui::SameLine(); ImGui::Text("-"); ImGui::SameLine();
    ImGui::SetNextItemWidth(30.0f); ImGui::InputText((std::string("###Month") + label).c_str(), month, 3);
    ImGui::SameLine(); ImGui::Text("-"); ImGui::SameLine();
    ImGui::SetNextItemWidth(30.0f); ImGui::InputText((std::string("###Day") + label).c_str(), day, 3);
    ImGui::SameLine(); ImGui::Text(" "); ImGui::SameLine();
    ImGui::SetNextItemWidth(30.0f); ImGui::InputText((std::string("###Hour") + label).c_str(), hour, 3);
    ImGui::SameLine(); ImGui::Text(":"); ImGui::SameLine();
    ImGui::SetNextItemWidth(30.0f); ImGui::InputText((std::string("###Minute") + label).c_str(), minute, 3);
    ImGui::SameLine(); ImGui::Text(":"); ImGui::SameLine();
    ImGui::SetNextItemWidth(30.0f); ImGui::InputText((std::string("###Second") + label).c_str(), second, 3);
}

// Handle submit action for "Add plot" popup
void GraphView::actionSubmitAddPlotPopup(AddPlotPopupState& state) {
    // Handle submit action
    RenderablePlot plot(state.plot_label, state.is_real_time);
    plot.setWindowLabel(state.window_label);

    // Convert date/time fields to timestamps
    std::tm start_time = {};
    start_time.tm_year = std::stoi(state.plot_range_start_year) - 1900;
    start_time.tm_mon = std::stoi(state.plot_range_start_month) - 1;
    start_time.tm_mday = std::stoi(state.plot_range_start_day);
    start_time.tm_hour = std::stoi(state.plot_range_start_hour);
    start_time.tm_min = std::stoi(state.plot_range_start_minute);
    start_time.tm_sec = std::stoi(state.plot_range_start_second);
    std::time_t start = std::mktime(&start_time);

    std::tm end_time = {};
    end_time.tm_year = std::stoi(state.plot_range_end_year) - 1900;
    end_time.tm_mon = std::stoi(state.plot_range_end_month) - 1;
    end_time.tm_mday = std::stoi(state.plot_range_end_day);
    end_time.tm_hour = std::stoi(state.plot_range_end_hour);
    end_time.tm_min = std::stoi(state.plot_range_end_minute);
    end_time.tm_sec = std::stoi(state.plot_range_end_second);
    std::time_t end = std::mktime(&end_time);

    plot.setPlotRange(start, end);

    // Add selected sensors to the plot
    for (const auto& sensor : state.selected_sensors) {
        plot.setData(sensor, {});
    }

    // Add the plot to the view model
    // viewModel_.addRenderablePlot(plot);

}

// Render the "Add plot" popup
void GraphView::renderAddPlotPopup() {
    // Example of creating a popup window with textbox input fields
    if (ImGui::Button("Add new plot")) {
        ImGui::OpenPopup("Add new plot");
        auto& add_plot_pop_up_state = viewModel_.getAddPlotPopupState();
        add_plot_pop_up_state.available_sensors = viewModel_.getPlottableSensors();
    }

    if (ImGui::BeginPopup("Add new plot")) {
        auto& add_plot_pop_up_state = viewModel_.getAddPlotPopupState();
        static bool is_able_to_submit = false;

        // Get current year
        ImGui::Text("Enter values:");

        // Temporary buffers to hold input text
        static char window_label_buffer[128] = "";
        static char plot_label_buffer[128] = "";

        // Copy current values into buffers
        std::strncpy(window_label_buffer, add_plot_pop_up_state.window_label.c_str(), sizeof(window_label_buffer));
        std::strncpy(plot_label_buffer, add_plot_pop_up_state.plot_label.c_str(), sizeof(plot_label_buffer));

        // Render the input text fields
        if (ImGui::InputText("Window name", window_label_buffer, sizeof(window_label_buffer))) {
            add_plot_pop_up_state.window_label = std::string(window_label_buffer); // Sync changes back to std::string
        }

        if (ImGui::InputText("Plot title", plot_label_buffer, sizeof(plot_label_buffer))) {
            add_plot_pop_up_state.plot_label = std::string(plot_label_buffer); // Sync changes back to std::string
        }

        // List plottable sensors
        static std::vector<std::string> sensors = add_plot_pop_up_state.available_sensors;
        static int selected_sensor = -1;

        ImGui::Text("Select a sensor:");
        if (ImGui::BeginListBox("##Sensors")) {
            for (size_t i = 0; i < sensors.size(); ++i) {
                const bool is_selected = (selected_sensor == static_cast<int>(i));
                if (ImGui::Selectable(sensors[i].c_str(), is_selected)) {
                    // Remove the selected sensor from the list and move them into the selected sensors list
                    selected_sensor = static_cast<int>(i);
                    add_plot_pop_up_state.selected_sensors.push_back(sensors.at(selected_sensor));
                    sensors.erase(sensors.begin() + selected_sensor);
                }
            }
            ImGui::EndListBox();
        }

        ImGui::SeparatorText("");

    if (ImGui::BeginListBox("##SelectedSensors")) {
        static int selected_sensor_in_selected = -1;
        for (size_t i = 0; i < add_plot_pop_up_state.selected_sensors.size(); ++i) {
            const bool is_selected = (selected_sensor_in_selected == static_cast<int>(i));
            if (ImGui::Selectable(add_plot_pop_up_state.selected_sensors[i].c_str(), is_selected)) {
                selected_sensor_in_selected = static_cast<int>(i);
            }
        }
        ImGui::EndListBox();

        // Move selected sensor back to "Sensors" list
        if (selected_sensor_in_selected != -1 && ImGui::Button("Deselect Sensor")) {
            const std::string& sensor_to_return = add_plot_pop_up_state.selected_sensors[selected_sensor_in_selected];

            // Find position in available_sensors only if it is non-empty
            if (!add_plot_pop_up_state.available_sensors.empty()) {
                auto it = std::find(
                    add_plot_pop_up_state.available_sensors.begin(),
                    add_plot_pop_up_state.available_sensors.end(),
                    sensor_to_return
                );

                // Check if found within bounds
                if (it != add_plot_pop_up_state.available_sensors.end()) {
                    auto index = std::distance(add_plot_pop_up_state.available_sensors.begin(), it);

                    // Ensure the calculated index is valid for sensors
                    if (index <= static_cast<int>(sensors.size())) {
                        sensors.insert(sensors.begin() + index, sensor_to_return);
                    } else {
                        sensors.push_back(sensor_to_return);
                    }
                } else {
                    sensors.push_back(sensor_to_return); // Append if not found
                }
            } else {
                sensors.push_back(sensor_to_return); // Append if available_sensors is empty
            }

            // Remove the sensor from selected_sensors
            if (selected_sensor_in_selected >= 0 && selected_sensor_in_selected < static_cast<int>(add_plot_pop_up_state.selected_sensors.size())) {
                add_plot_pop_up_state.selected_sensors.erase(add_plot_pop_up_state.selected_sensors.begin() + selected_sensor_in_selected);
                selected_sensor_in_selected = -1; // Reset selection
            }
        }
    }

        ImGui::SeparatorText("");

        // Real-time plotting checkbox
        ImGui::Checkbox("Real-time", &add_plot_pop_up_state.is_real_time);
        is_able_to_submit = add_plot_pop_up_state.is_real_time;

        // Real-time plot range
        static bool is_range_initialized = false; // Initialization flag
        if(add_plot_pop_up_state.is_real_time && !is_range_initialized) {
            // Get current time for default plot range
            std::time_t current_time = std::time(nullptr);
            std::tm local_time;
            localtime_s(&local_time, &current_time);
            std::time_t max_data_time_range = 3600; // 1 hour

            std::time_t start_time = std::time(nullptr) - max_data_time_range;
            std::tm start_local_time;
            localtime_s(&start_local_time, &start_time);

            // Set the end plot year, month, day, hour, minute, second to current time
            std::strftime(add_plot_pop_up_state.plot_range_end_year, sizeof(add_plot_pop_up_state.plot_range_end_year), "%Y", &local_time);
            std::strftime(add_plot_pop_up_state.plot_range_end_month, sizeof(add_plot_pop_up_state.plot_range_end_month), "%m", &local_time);
            std::strftime(add_plot_pop_up_state.plot_range_end_day, sizeof(add_plot_pop_up_state.plot_range_end_day), "%d", &local_time);
            std::strftime(add_plot_pop_up_state.plot_range_end_hour, sizeof(add_plot_pop_up_state.plot_range_end_hour), "%H", &local_time);
            std::strftime(add_plot_pop_up_state.plot_range_end_minute, sizeof(add_plot_pop_up_state.plot_range_end_minute), "%M", &local_time);
            std::strftime(add_plot_pop_up_state.plot_range_end_second, sizeof(add_plot_pop_up_state.plot_range_end_second), "%S", &local_time);

            // Set the start plot year, month, day, hour, minute, second to current time - max_data_time_range
            std::strftime(add_plot_pop_up_state.plot_range_start_year, sizeof(add_plot_pop_up_state.plot_range_start_year), "%Y", &start_local_time);
            std::strftime(add_plot_pop_up_state.plot_range_start_month, sizeof(add_plot_pop_up_state.plot_range_start_month), "%m", &start_local_time);
            std::strftime(add_plot_pop_up_state.plot_range_start_day, sizeof(add_plot_pop_up_state.plot_range_start_day), "%d", &start_local_time);
            std::strftime(add_plot_pop_up_state.plot_range_start_hour, sizeof(add_plot_pop_up_state.plot_range_start_hour), "%H", &start_local_time);
            std::strftime(add_plot_pop_up_state.plot_range_start_minute, sizeof(add_plot_pop_up_state.plot_range_start_minute), "%M", &start_local_time);
            std::strftime(add_plot_pop_up_state.plot_range_start_second, sizeof(add_plot_pop_up_state.plot_range_start_second), "%S", &start_local_time);

        }

        // User defined plot range
        if(!add_plot_pop_up_state.is_real_time) {
            // Initialize textboxes with default values only once
                if (!is_range_initialized) {
                // Get current time for default plot range
                std::time_t current_time = std::time(nullptr);
                std::tm local_time;
                localtime_s(&local_time, &current_time);
                std::time_t max_data_time_range = 3600; // 1 hour

                std::time_t start_time = std::time(nullptr) - max_data_time_range;
                std::tm start_local_time;
                localtime_s(&start_local_time, &start_time);

                std::strftime(add_plot_pop_up_state.plot_range_start_year, sizeof(add_plot_pop_up_state.plot_range_start_year), "%Y", &start_local_time);
                std::strftime(add_plot_pop_up_state.plot_range_start_month, sizeof(add_plot_pop_up_state.plot_range_start_month), "%m", &start_local_time);
                std::strftime(add_plot_pop_up_state.plot_range_start_day, sizeof(add_plot_pop_up_state.plot_range_start_day), "%d", &start_local_time);
                std::strftime(add_plot_pop_up_state.plot_range_start_hour, sizeof(add_plot_pop_up_state.plot_range_start_hour), "%H", &start_local_time);
                std::strftime(add_plot_pop_up_state.plot_range_start_minute, sizeof(add_plot_pop_up_state.plot_range_start_minute), "%M", &start_local_time);
                std::strftime(add_plot_pop_up_state.plot_range_start_second, sizeof(add_plot_pop_up_state.plot_range_start_second), "%S", &start_local_time);

                std::strftime(add_plot_pop_up_state.plot_range_end_year, sizeof(add_plot_pop_up_state.plot_range_end_year), "%Y", &local_time);
                std::strftime(add_plot_pop_up_state.plot_range_end_month, sizeof(add_plot_pop_up_state.plot_range_end_month), "%m", &local_time);
                std::strftime(add_plot_pop_up_state.plot_range_end_day, sizeof(add_plot_pop_up_state.plot_range_end_day), "%d", &local_time);
                std::strftime(add_plot_pop_up_state.plot_range_end_hour, sizeof(add_plot_pop_up_state.plot_range_end_hour), "%H", &local_time);
                std::strftime(add_plot_pop_up_state.plot_range_end_minute, sizeof(add_plot_pop_up_state.plot_range_end_minute), "%M", &local_time);
                std::strftime(add_plot_pop_up_state.plot_range_end_second, sizeof(add_plot_pop_up_state.plot_range_end_second), "%S", &local_time);

                is_range_initialized = true;
            }

            ImGui::Text("Enter plot range (YYYY-MM-DD HH:MM:SS)");

            renderDateTimeField("Plot start date", add_plot_pop_up_state.plot_range_start_year,
                add_plot_pop_up_state.plot_range_start_month,
                add_plot_pop_up_state.plot_range_start_day,
                add_plot_pop_up_state.plot_range_start_hour,
                add_plot_pop_up_state.plot_range_start_minute,
                add_plot_pop_up_state.plot_range_start_second);
            renderDateTimeField("Plot end date", add_plot_pop_up_state.plot_range_end_year,
                add_plot_pop_up_state.plot_range_end_month,
                add_plot_pop_up_state.plot_range_end_day,
                add_plot_pop_up_state.plot_range_end_hour,
                add_plot_pop_up_state.plot_range_end_minute,
                add_plot_pop_up_state.plot_range_end_second
                );

            if (std::strlen(add_plot_pop_up_state.plot_range_start_year) == 0 ||
                std::strlen(add_plot_pop_up_state.plot_range_start_month) == 0 ||
                std::strlen(add_plot_pop_up_state.plot_range_start_day) == 0 ||
                std::strlen(add_plot_pop_up_state.plot_range_start_hour) == 0 ||
                std::strlen(add_plot_pop_up_state.plot_range_start_minute) == 0 ||
                std::strlen(add_plot_pop_up_state.plot_range_start_second) == 0 ||
                std::strlen(add_plot_pop_up_state.plot_range_end_year) == 0 ||
                std::strlen(add_plot_pop_up_state.plot_range_end_month) == 0 ||
                std::strlen(add_plot_pop_up_state.plot_range_end_day) == 0 ||
                std::strlen(add_plot_pop_up_state.plot_range_end_hour) == 0 ||
                std::strlen(add_plot_pop_up_state.plot_range_end_minute) == 0 ||
                std::strlen(add_plot_pop_up_state.plot_range_end_second) == 0) {
                ImGui::Text("Please enter all fields");
                is_able_to_submit = false;
            } else {
                is_able_to_submit = true;
            }

            // Ensure end time is after start time
            // Convert input into unix time
            try {
                static std::tm input_start_time = {};
                input_start_time.tm_year = std::stoi(add_plot_pop_up_state.plot_range_start_year) - 1900;
                input_start_time.tm_mon = std::stoi(add_plot_pop_up_state.plot_range_start_month) - 1;
                input_start_time.tm_mday = std::stoi(add_plot_pop_up_state.plot_range_start_day);
                input_start_time.tm_hour = std::stoi(add_plot_pop_up_state.plot_range_start_hour);
                input_start_time.tm_min = std::stoi(add_plot_pop_up_state.plot_range_start_minute);
                input_start_time.tm_sec = std::stoi(add_plot_pop_up_state.plot_range_start_second);
                std::time_t start_unix_time = std::mktime(&input_start_time);

                std::tm input_end_time = {};
                input_end_time.tm_year = std::stoi(add_plot_pop_up_state.plot_range_end_year) - 1900;
                input_end_time.tm_mon = std::stoi(add_plot_pop_up_state.plot_range_end_month) - 1;
                input_end_time.tm_mday = std::stoi(add_plot_pop_up_state.plot_range_end_day);
                input_end_time.tm_hour = std::stoi(add_plot_pop_up_state.plot_range_end_hour);
                input_end_time.tm_min = std::stoi(add_plot_pop_up_state.plot_range_end_minute);
                input_end_time.tm_sec = std::stoi(add_plot_pop_up_state.plot_range_end_second);
                std::time_t end_unix_time = std::mktime(&input_end_time);

                if (end_unix_time < start_unix_time) {
                    ImGui::Text("End time must be after start time");
                    is_able_to_submit = false;
                } else {
                    is_able_to_submit = true;
                }
            } catch (const std::exception& e) {
                ImGui::Text("Invalid date/time format");
                is_able_to_submit = false;
            }

        }

        if(is_able_to_submit) {
            if (ImGui::Button("Submit")) {
                // Handle submit action
                actionSubmitAddPlotPopup(add_plot_pop_up_state);

                add_plot_pop_up_state.reset();
                sensors = add_plot_pop_up_state.available_sensors;
                is_range_initialized = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();
        }

        if (ImGui::Button("Cancel")) {
            add_plot_pop_up_state.reset();
            sensors = add_plot_pop_up_state.available_sensors;
            is_range_initialized = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

// ==============================
// ==============================


void GraphView::renderAll() {
    // Render "Add plot" button and popup
    renderAddPlotPopup();

    // Plot all RenderablePlots in individual windows
    for (const auto& renderable_plot : viewModel_.getRenderablePlots()){
        // Create a sub-window with each plot
        ImGui::Begin(renderable_plot.getWindowLabel().c_str());

        // Create the plot
        std::time_t plot_start, plot_end;
        if(renderable_plot.isRealTime()){
            std::time_t current_time = std::time(nullptr);
            std::time_t max_data_time_range = 1000;

            plot_start = current_time - max_data_time_range;
            plot_end = current_time;

        } else {
            plot_start = renderable_plot.getPlotRange().first;
            plot_end = renderable_plot.getPlotRange().second;

        }

        if (ImPlot::BeginPlot("")) {
            ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
            for (const auto& [series_label, data] : renderable_plot.getAllData()) {
                std::vector<double> xs, ys;
                for (const auto& [timestamp, value] : data) {
                    xs.push_back(timestamp);
                    ys.push_back(value);
                }
                ImPlot::PlotLine(series_label.c_str(), xs.data(), ys.data(), xs.size());
            }
            ImPlot::EndPlot();
        }

        ImGui::End();
    }
}
