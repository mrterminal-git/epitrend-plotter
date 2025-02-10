#include <array>
#include <vector>
#include <cmath>
#include <iostream>
#include <set>
#include <string_view>
#include <sstream>
#include <iomanip>

#include <fmt/format.h>
#include <imgui.h>
#include <implot.h>

#include "GraphView.hpp"
#include "L2DFileDialog.hpp"

// Constructor
GraphView::GraphView(GraphViewModel& viewModel) : viewModel_(viewModel) {}

// Draw
void GraphView::Draw(const std::string label)
{
    //        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
    constexpr static auto window_flags =
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoBringToFrontOnFocus;
    constexpr static auto window_size = ImVec2(1280.0F, 720.0F);
    constexpr static auto window_pos = ImVec2(0.0F, 0.0F);

    ImGui::SetNextWindowSize(window_size, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(window_pos, ImGuiCond_FirstUseEver);

    ImGui::Begin(label.data(), nullptr, window_flags);

    renderAll();

    ImGui::End();

}




// ==============================
// Misc && Helper Functions
// ==============================
void GraphView::setUpdateRangeCallback(UpdateRangeCallback callback) {
    update_range_callback_ = callback;
}




// ==============================
// renderAddPlotPopup
// ==============================

// Helper function to initialize plot range for Brisbane time
void SetPlotRangeState(AddPlotPopupState& add_plot_pop_up_state) {
    // Get current time as a chrono time_point
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::chrono::hours max_data_time_range(1); // 1 hour

    // Calculate start time
    std::chrono::system_clock::time_point start_time = now - max_data_time_range;

    // Convert times to time_t for UTC
    std::time_t end_time_t = std::chrono::system_clock::to_time_t(now);
    std::time_t start_time_t = std::chrono::system_clock::to_time_t(start_time);

    // Adjust for Brisbane time (UTC+10)
    const int brisbane_offset_seconds = 10 * 3600; // +10 hours
    end_time_t += brisbane_offset_seconds;
    start_time_t += brisbane_offset_seconds;

    // Convert to tm structure (UTC+10 manually applied)
    std::tm end_local_time;
    std::tm start_local_time;
    gmtime_s(&end_local_time, &end_time_t); // Use gmtime for consistency
    gmtime_s(&start_local_time, &start_time_t);

    // Format the times using std::ostringstream
    auto formatTime = [](const std::tm& time_struct, char* buffer, size_t size, const char* format) {
        std::ostringstream oss;
        oss << std::put_time(&time_struct, format);
        std::strncpy(buffer, oss.str().c_str(), size - 1);
        buffer[size - 1] = '\0'; // Ensure null termination
    };

    // Set the end plot range
    formatTime(end_local_time, add_plot_pop_up_state.plot_range_end_year, sizeof(add_plot_pop_up_state.plot_range_end_year), "%Y");
    formatTime(end_local_time, add_plot_pop_up_state.plot_range_end_month, sizeof(add_plot_pop_up_state.plot_range_end_month), "%m");
    formatTime(end_local_time, add_plot_pop_up_state.plot_range_end_day, sizeof(add_plot_pop_up_state.plot_range_end_day), "%d");
    formatTime(end_local_time, add_plot_pop_up_state.plot_range_end_hour, sizeof(add_plot_pop_up_state.plot_range_end_hour), "%H");
    formatTime(end_local_time, add_plot_pop_up_state.plot_range_end_minute, sizeof(add_plot_pop_up_state.plot_range_end_minute), "%M");
    formatTime(end_local_time, add_plot_pop_up_state.plot_range_end_second, sizeof(add_plot_pop_up_state.plot_range_end_second), "%S");

    // Set the start plot range
    formatTime(start_local_time, add_plot_pop_up_state.plot_range_start_year, sizeof(add_plot_pop_up_state.plot_range_start_year), "%Y");
    formatTime(start_local_time, add_plot_pop_up_state.plot_range_start_month, sizeof(add_plot_pop_up_state.plot_range_start_month), "%m");
    formatTime(start_local_time, add_plot_pop_up_state.plot_range_start_day, sizeof(add_plot_pop_up_state.plot_range_start_day), "%d");
    formatTime(start_local_time, add_plot_pop_up_state.plot_range_start_hour, sizeof(add_plot_pop_up_state.plot_range_start_hour), "%H");
    formatTime(start_local_time, add_plot_pop_up_state.plot_range_start_minute, sizeof(add_plot_pop_up_state.plot_range_start_minute), "%M");
    formatTime(start_local_time, add_plot_pop_up_state.plot_range_start_second, sizeof(add_plot_pop_up_state.plot_range_start_second), "%S");
}

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
    std::time_t start = std::mktime(&start_time) + 10 * 3600; // Convert to Brisbane time + 10hrs

    std::tm end_time = {};
    end_time.tm_year = std::stoi(state.plot_range_end_year) - 1900;
    end_time.tm_mon = std::stoi(state.plot_range_end_month) - 1;
    end_time.tm_mday = std::stoi(state.plot_range_end_day);
    end_time.tm_hour = std::stoi(state.plot_range_end_hour);
    end_time.tm_min = std::stoi(state.plot_range_end_minute);
    end_time.tm_sec = std::stoi(state.plot_range_end_second);
    std::time_t end = std::mktime(&end_time) + 10 * 3600; // Convert to Brisbane time + 10hrs;

    plot.setPlotRange(start, end);

    // Add selected sensors to the plot
    for (const auto& sensor : state.selected_sensors) {
        plot.setData(sensor, {});
    }

    // Add selected sensors to Y1 axis by default
    for(const auto& sensor : state.selected_sensors) {
        plot.addYAxisForSensor(sensor, ImAxis_Y1);
    }

    // Add selected sensors into the list of plotline properties
    for(const auto& sensor : state.selected_sensors) {
        plot.addPlotLineProperties(sensor, RenderablePlot::PlotLineProperties());
    }

    // Add selected sensors into the list of plotline properties
    // Rotate through color pallette
    int color_index = 0;
    for(const auto& sensor : state.selected_sensors) {
        plot.addPlotLineProperties(sensor, RenderablePlot::PlotLineProperties());
        ImVec4 color = ImPlot::GetColormapColor(color_index);
        plot.setPlotLinePropertiesColour(sensor, color);
        color_index = (color_index + 1) % 10;
    }

    // Initialize a window and add the plot to a window
    WindowPlots window(state.window_label);
    window.addRenderablePlot(state.plot_label, std::make_unique<RenderablePlot>(std::move(plot)));
    viewModel_.addWindowPlots(state.window_label, std::make_unique<WindowPlots>(std::move(window)));

}

// Render the "Add plot" popup
void GraphView::renderAddPlotPopup() {
    // Example of creating a popup window with textbox input fields
    if (ImGui::Button("Add new plot", ImVec2(100, 30))) {
        ImGui::OpenPopup("Add new plot");
    }

    if (ImGui::BeginPopup("Add new plot")) {
        auto& add_plot_pop_up_state = viewModel_.getAddPlotPopupState();
        add_plot_pop_up_state.available_sensors = viewModel_.getPlottableSensors();

        static bool is_able_to_submit = false;

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
            // Initialize plot range for Brisbane time
            SetPlotRangeState(add_plot_pop_up_state);
        }

        // User defined plot range
        if(!add_plot_pop_up_state.is_real_time) {
            if (!is_range_initialized) {
                // Initialize textboxes with default values only once
                SetPlotRangeState(add_plot_pop_up_state);
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
// renderAllWindowPlots
// ==============================

// ********** HELPER FUNCTIONS **********

// Helper function to initialize plot range for Brisbane time in the plot options popup
void SetPlotRangeState(PlotOptionsPopupState& plot_option_pop_up_state) {
    // Get current time as a chrono time_point
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::chrono::hours max_data_time_range(1); // 1 hour

    // Calculate start time
    std::chrono::system_clock::time_point start_time = now - max_data_time_range;

    // Convert times to time_t for UTC
    std::time_t end_time_t = std::chrono::system_clock::to_time_t(now);
    std::time_t start_time_t = std::chrono::system_clock::to_time_t(start_time);

    // Adjust for Brisbane time (UTC+10)
    const int brisbane_offset_seconds = 10 * 3600; // +10 hours
    end_time_t += brisbane_offset_seconds;
    start_time_t += brisbane_offset_seconds;

    // Convert to tm structure (UTC+10 manually applied)
    std::tm end_local_time;
    std::tm start_local_time;
    gmtime_s(&end_local_time, &end_time_t); // Use gmtime for consistency
    gmtime_s(&start_local_time, &start_time_t);

    // Format the times using std::ostringstream
    auto formatTime = [](const std::tm& time_struct, char* buffer, size_t size, const char* format) {
        std::ostringstream oss;
        oss << std::put_time(&time_struct, format);
        std::strncpy(buffer, oss.str().c_str(), size - 1);
        buffer[size - 1] = '\0'; // Ensure null termination
    };

    // Set the end plot range
    formatTime(end_local_time, plot_option_pop_up_state.plot_range_end_year, sizeof(plot_option_pop_up_state.plot_range_end_year), "%Y");
    formatTime(end_local_time, plot_option_pop_up_state.plot_range_end_month, sizeof(plot_option_pop_up_state.plot_range_end_month), "%m");
    formatTime(end_local_time, plot_option_pop_up_state.plot_range_end_day, sizeof(plot_option_pop_up_state.plot_range_end_day), "%d");
    formatTime(end_local_time, plot_option_pop_up_state.plot_range_end_hour, sizeof(plot_option_pop_up_state.plot_range_end_hour), "%H");
    formatTime(end_local_time, plot_option_pop_up_state.plot_range_end_minute, sizeof(plot_option_pop_up_state.plot_range_end_minute), "%M");
    formatTime(end_local_time, plot_option_pop_up_state.plot_range_end_second, sizeof(plot_option_pop_up_state.plot_range_end_second), "%S");

    // Set the start plot range
    formatTime(start_local_time, plot_option_pop_up_state.plot_range_start_year, sizeof(plot_option_pop_up_state.plot_range_start_year), "%Y");
    formatTime(start_local_time, plot_option_pop_up_state.plot_range_start_month, sizeof(plot_option_pop_up_state.plot_range_start_month), "%m");
    formatTime(start_local_time, plot_option_pop_up_state.plot_range_start_day, sizeof(plot_option_pop_up_state.plot_range_start_day), "%d");
    formatTime(start_local_time, plot_option_pop_up_state.plot_range_start_hour, sizeof(plot_option_pop_up_state.plot_range_start_hour), "%H");
    formatTime(start_local_time, plot_option_pop_up_state.plot_range_start_minute, sizeof(plot_option_pop_up_state.plot_range_start_minute), "%M");
    formatTime(start_local_time, plot_option_pop_up_state.plot_range_start_second, sizeof(plot_option_pop_up_state.plot_range_start_second), "%S");
}

// Handle submit action for plot options popup
void ActionSubmitPlotOptionsPopup(RenderablePlot& renderable_plot, PlotOptionsPopupState& plot_options_popup_state) {
    // Add all the selected sensors to their respective Y-axis
    renderable_plot.clearYAxes();
    std::map<std::string, RenderablePlot::DataSeries> temp_data_series;

    for (const auto& Y1_sensor : plot_options_popup_state.sensors_in_Y1_list_box) {
        renderable_plot.addYAxisForSensor(Y1_sensor, ImAxis_Y1);
        temp_data_series[Y1_sensor] = RenderablePlot::DataSeries();
    }
    for (const auto& Y2_sensor : plot_options_popup_state.sensors_in_Y2_list_box) {
        renderable_plot.addYAxisForSensor(Y2_sensor, ImAxis_Y2);
        temp_data_series[Y2_sensor] = RenderablePlot::DataSeries();
    }
    for (const auto& Y3_sensor : plot_options_popup_state.sensors_in_Y3_list_box) {
        renderable_plot.addYAxisForSensor(Y3_sensor, ImAxis_Y3);
        temp_data_series[Y3_sensor] = RenderablePlot::DataSeries();
    }
    renderable_plot.setAllData(temp_data_series);

    // Set the Y-axis properties
    renderable_plot.setYAxisPropertiesLabel(ImAxis_Y1, plot_options_popup_state.Y1_properties.label);
    renderable_plot.setYAxisPropertiesMin(ImAxis_Y1, plot_options_popup_state.Y1_properties.min);
    renderable_plot.setYAxisPropertiesMax(ImAxis_Y1, plot_options_popup_state.Y1_properties.max);
    renderable_plot.setYAxisPropertiesScale(ImAxis_Y1, plot_options_popup_state.Y1_properties.scale_type);
    renderable_plot.setYAxisPropertiesLogBase(ImAxis_Y1, plot_options_popup_state.Y1_properties.log_base);
    renderable_plot.setYAxisPropertiesUserSetRange(ImAxis_Y1, true);

    renderable_plot.setYAxisPropertiesLabel(ImAxis_Y2, plot_options_popup_state.Y2_properties.label);
    renderable_plot.setYAxisPropertiesMin(ImAxis_Y2, plot_options_popup_state.Y2_properties.min);
    renderable_plot.setYAxisPropertiesMax(ImAxis_Y2, plot_options_popup_state.Y2_properties.max);
    renderable_plot.setYAxisPropertiesScale(ImAxis_Y2, plot_options_popup_state.Y2_properties.scale_type);
    renderable_plot.setYAxisPropertiesLogBase(ImAxis_Y2, plot_options_popup_state.Y2_properties.log_base);
    renderable_plot.setYAxisPropertiesUserSetRange(ImAxis_Y2, true);

    renderable_plot.setYAxisPropertiesLabel(ImAxis_Y3, plot_options_popup_state.Y3_properties.label);
    renderable_plot.setYAxisPropertiesMin(ImAxis_Y3, plot_options_popup_state.Y3_properties.min);
    renderable_plot.setYAxisPropertiesMax(ImAxis_Y3, plot_options_popup_state.Y3_properties.max);
    renderable_plot.setYAxisPropertiesScale(ImAxis_Y3, plot_options_popup_state.Y3_properties.scale_type);
    renderable_plot.setYAxisPropertiesLogBase(ImAxis_Y3, plot_options_popup_state.Y3_properties.log_base);
    renderable_plot.setYAxisPropertiesUserSetRange(ImAxis_Y3, true);

    // Set the plotline properties for each sensor
    for (const auto& [sensor, properties] : plot_options_popup_state.sensor_to_plotline_properties) {
        //  Check if plotline properties exist for the sensor
        if (!renderable_plot.hasPlotLineProperties(sensor)) {
            renderable_plot.addPlotLineProperties(sensor, RenderablePlot::PlotLineProperties());
        }

        // Set the plotline properties
        renderable_plot.setPlotLinePropertiesColour(sensor, properties.colour);
        renderable_plot.setPlotLinePropertiesMarkerStyle(sensor, properties.marker_style);
        renderable_plot.setPlotLinePropertiesMarkerSize(sensor, properties.marker_size);
        renderable_plot.setPlotLinePropertiesFill(sensor, properties.fill);
        renderable_plot.setPlotLinePropertiesFillWeight(sensor, properties.fill_weight);
        renderable_plot.setPlotLinePropertiesFillOutline(sensor, properties.fill_outline);
    }

    // Remove any plotline properties of RenderablePlot that are not in either Y1, Y2, Y3
    std::vector<std::string> sensors_in_Y_axes;
    sensors_in_Y_axes.insert(sensors_in_Y_axes.end(), plot_options_popup_state.sensors_in_Y1_list_box.begin(), plot_options_popup_state.sensors_in_Y1_list_box.end());
    sensors_in_Y_axes.insert(sensors_in_Y_axes.end(), plot_options_popup_state.sensors_in_Y2_list_box.begin(), plot_options_popup_state.sensors_in_Y2_list_box.end());
    sensors_in_Y_axes.insert(sensors_in_Y_axes.end(), plot_options_popup_state.sensors_in_Y3_list_box.begin(), plot_options_popup_state.sensors_in_Y3_list_box.end());
    for (const auto& [sensor, properties] : renderable_plot.getAllPlotLineProperties()) {
        if (std::find(sensors_in_Y_axes.begin(), sensors_in_Y_axes.end(), sensor) == sensors_in_Y_axes.end()) {
            renderable_plot.removePlotLineProperties(sensor);
        }
    }
}

// Search bar logic
bool IsSearchBarMatch(const std::string& search_text, const std::string& sensor) {
    // Delimiter for search text and split by delimiter
    const std::string delimiter = " ";
    std::vector<std::string> search_tokens;

    // Split search text by delimiter
    size_t start = 0;
    size_t end = search_text.find(delimiter);
    while (end != std::string::npos) {
        search_tokens.push_back(search_text.substr(start, end - start));
        start = end + delimiter.length();
        end = search_text.find(delimiter, start);
    }
    search_tokens.push_back(search_text.substr(start, end));

    // Return true if sensor contains any of the search tokens
    for (const auto& token : search_tokens) {
        if (sensor.find(token) != std::string::npos) {
            return true;
        }
    }

    // Return false if no match
    return false;
}

// Helper transform functions for logarithmic scale
static inline double TransformForward_Log(double v, void* user_data) {
    double log_base = *static_cast<double*>(user_data);
    if (log_base <= 0) {
        log_base = 10;
    }
    if (v <= 0) {
        return 1E-17;
    }
    return log(v) / log(log_base);
}

// Helper transform functions for logarithmic scale
static inline double TransformInverse_Log(double v, void* user_data) {
    double log_base = *static_cast<double*>(user_data);
    if (log_base <= 0) {
        log_base = 10;
    }
    return pow(log_base, v);
}

// Helper function to initialize plot range for Brisbane time
void SetPlotRangeState(WindowPlotAddPlotPopupState& add_plot_pop_up_state) {
    // Get current time as a chrono time_point
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::chrono::hours max_data_time_range(1); // 1 hour

    // Calculate start time
    std::chrono::system_clock::time_point start_time = now - max_data_time_range;

    // Convert times to time_t for UTC
    std::time_t end_time_t = std::chrono::system_clock::to_time_t(now);
    std::time_t start_time_t = std::chrono::system_clock::to_time_t(start_time);

    // Adjust for Brisbane time (UTC+10)
    const int brisbane_offset_seconds = 10 * 3600; // +10 hours
    end_time_t += brisbane_offset_seconds;
    start_time_t += brisbane_offset_seconds;

    // Convert to tm structure (UTC+10 manually applied)
    std::tm end_local_time;
    std::tm start_local_time;
    gmtime_s(&end_local_time, &end_time_t); // Use gmtime for consistency
    gmtime_s(&start_local_time, &start_time_t);

    // Format the times using std::ostringstream
    auto formatTime = [](const std::tm& time_struct, char* buffer, size_t size, const char* format) {
        std::ostringstream oss;
        oss << std::put_time(&time_struct, format);
        std::strncpy(buffer, oss.str().c_str(), size - 1);
        buffer[size - 1] = '\0'; // Ensure null termination
    };

    // Set the end plot range
    formatTime(end_local_time, add_plot_pop_up_state.plot_range_end_year, sizeof(add_plot_pop_up_state.plot_range_end_year), "%Y");
    formatTime(end_local_time, add_plot_pop_up_state.plot_range_end_month, sizeof(add_plot_pop_up_state.plot_range_end_month), "%m");
    formatTime(end_local_time, add_plot_pop_up_state.plot_range_end_day, sizeof(add_plot_pop_up_state.plot_range_end_day), "%d");
    formatTime(end_local_time, add_plot_pop_up_state.plot_range_end_hour, sizeof(add_plot_pop_up_state.plot_range_end_hour), "%H");
    formatTime(end_local_time, add_plot_pop_up_state.plot_range_end_minute, sizeof(add_plot_pop_up_state.plot_range_end_minute), "%M");
    formatTime(end_local_time, add_plot_pop_up_state.plot_range_end_second, sizeof(add_plot_pop_up_state.plot_range_end_second), "%S");

    // Set the start plot range
    formatTime(start_local_time, add_plot_pop_up_state.plot_range_start_year, sizeof(add_plot_pop_up_state.plot_range_start_year), "%Y");
    formatTime(start_local_time, add_plot_pop_up_state.plot_range_start_month, sizeof(add_plot_pop_up_state.plot_range_start_month), "%m");
    formatTime(start_local_time, add_plot_pop_up_state.plot_range_start_day, sizeof(add_plot_pop_up_state.plot_range_start_day), "%d");
    formatTime(start_local_time, add_plot_pop_up_state.plot_range_start_hour, sizeof(add_plot_pop_up_state.plot_range_start_hour), "%H");
    formatTime(start_local_time, add_plot_pop_up_state.plot_range_start_minute, sizeof(add_plot_pop_up_state.plot_range_start_minute), "%M");
    formatTime(start_local_time, add_plot_pop_up_state.plot_range_start_second, sizeof(add_plot_pop_up_state.plot_range_start_second), "%S");
}

void ActionSubmitWindowPlotAddPlotPopup(WindowPlotAddPlotPopupState& state, WindowPlots& window) {
    // Handle submit action
    RenderablePlot plot(state.plot_label, state.is_real_time);
    plot.setWindowLabel(window.getLabel());

    // Convert date/time fields to timestamps
    std::tm start_time = {};
    start_time.tm_year = std::stoi(state.plot_range_start_year) - 1900;
    start_time.tm_mon = std::stoi(state.plot_range_start_month) - 1;
    start_time.tm_mday = std::stoi(state.plot_range_start_day);
    start_time.tm_hour = std::stoi(state.plot_range_start_hour);
    start_time.tm_min = std::stoi(state.plot_range_start_minute);
    start_time.tm_sec = std::stoi(state.plot_range_start_second);
    std::time_t start = std::mktime(&start_time) + 10 * 3600; // Convert to Brisbane time + 10hrs

    std::tm end_time = {};
    end_time.tm_year = std::stoi(state.plot_range_end_year) - 1900;
    end_time.tm_mon = std::stoi(state.plot_range_end_month) - 1;
    end_time.tm_mday = std::stoi(state.plot_range_end_day);
    end_time.tm_hour = std::stoi(state.plot_range_end_hour);
    end_time.tm_min = std::stoi(state.plot_range_end_minute);
    end_time.tm_sec = std::stoi(state.plot_range_end_second);
    std::time_t end = std::mktime(&end_time) + 10 * 3600; // Convert to Brisbane time + 10hrs;

    plot.setPlotRange(start, end);

    // Add selected sensors to the plot
    for (const auto& sensor : state.selected_sensors) {
        plot.setData(sensor, {});
    }

    // Add selected sensors to Y1 axis by default
    for(const auto& sensor : state.selected_sensors) {
        plot.addYAxisForSensor(sensor, ImAxis_Y1);
    }

    // Add selected sensors into the list of plotline properties
    for(const auto& sensor : state.selected_sensors) {
        plot.addPlotLineProperties(sensor, RenderablePlot::PlotLineProperties());
    }

    // Add selected sensors into the list of plotline properties
    // Rotate through color pallette
    int color_index = 0;
    for(const auto& sensor : state.selected_sensors) {
        plot.addPlotLineProperties(sensor, RenderablePlot::PlotLineProperties());
        ImVec4 color = ImPlot::GetColormapColor(color_index);
        plot.setPlotLinePropertiesColour(sensor, color);
        color_index = (color_index + 1) % 10;
    }

    // Add the plot to the window object
    window.addRenderablePlot(state.plot_label, std::make_unique<RenderablePlot>(std::move(plot)));
}

void ActionSubmitSaveWindowAsPlotPopup(WindowPlots& window, SaveWindowAsPopupState& save_window_as_popup_state) {
    // Logic for submitting "Save Window As" popup
    // Save the file into file location if it exists
    if (std::filesystem::exists(save_window_as_popup_state.file_path_buffer)) {
        // Convert window into JSON format and save file into folder
        std::filesystem::path file_path = std::filesystem::path(save_window_as_popup_state.file_path_buffer) / save_window_as_popup_state.file_name_buffer;
        WindowPlotsSaveLoad::saveToFile(window, file_path.string());
    }
}

// ********** RENDERING **********
// Render plots within window object
void GraphView::renderAllPlotsInWindow(WindowPlots* window) {
    // Loop through all renderable plots in the window
    for (const std::string& plot_label : window->getRenderablePlotLabels()) {
        RenderablePlot& renderable_plot = window->getRenderablePlot(plot_label);

        // Create the plot
        std::time_t plot_start, plot_end;
        if(renderable_plot.isRealTime()){
            std::time_t current_time = std::time(nullptr) + 10 * 3600; // Brisbane time
            std::time_t max_data_time_range = 1000;

            plot_start = current_time - max_data_time_range;
            plot_end = current_time;

        } else {
            plot_start = renderable_plot.getPlotRange().first;
            plot_end = renderable_plot.getPlotRange().second;

        }

        // Title for the plot
        ImGui::SeparatorText(renderable_plot.getLabel().c_str());

        // Store the real-time flag in a local variable
        bool is_real_time = renderable_plot.isRealTime();
        if (ImGui::Checkbox(("Real-time (AEST)###" + renderable_plot.getLabel() + "IsRealTimeCheckBox").c_str(), &is_real_time)) {
            // Update the real-time flag if it changes
            renderable_plot.setRealTime(is_real_time);
        }
        ImGui::SameLine();

        // Create a button to open the plot options popup
        renderPlotOptions(("###" + renderable_plot.getLabel()), renderable_plot);

        // Render the plot
        if (ImPlot::BeginPlot(("###" + renderable_plot.getLabel()).c_str())) {
            // Get all sensors in the plot
            const std::vector<std::string> sensors = renderable_plot.getAllSensors();

            // Set up the plot Y-axis before any setup locking functions
            for (const auto& series_label : sensors) {
                // Get the axis (Y1, Y2, Y3) for the sensor
                ImAxis plot_axis = renderable_plot.getYAxisForSensor(series_label);
                ImPlot::SetupAxis(plot_axis, nullptr, ImPlotAxisFlags_AuxDefault);
                if (renderable_plot.getYAxisPropertiesUserSetRange(plot_axis)) {
                    std::cout << "Setting axis limits for " << series_label << " to " << renderable_plot.getYAxisPropertiesMin(plot_axis) << " - " << renderable_plot.getYAxisPropertiesMax(plot_axis) << "\n";
                    ImPlot::SetupAxisLimits(plot_axis, renderable_plot.getYAxisPropertiesMin(plot_axis),
                        renderable_plot.getYAxisPropertiesMax(plot_axis), ImGuiCond_Always);
                    renderable_plot.setYAxisPropertiesUserSetRange(plot_axis, false);
                } else {
                    ImPlot::SetupAxisLimits(plot_axis, renderable_plot.getYAxisPropertiesMin(plot_axis),
                        renderable_plot.getYAxisPropertiesMax(plot_axis), ImGuiCond_Once);
                }

                // Set the axis scale type
                if (renderable_plot.getYAxisPropertiesScaleType(plot_axis) == RenderablePlot::ScaleType::Logirithmic) {
                    double log_base = renderable_plot.getYAxisPropertiesLogBase(plot_axis);
                    ImPlot::SetupAxisScale(plot_axis, TransformForward_Log, TransformInverse_Log, &log_base);
                } else {
                    ImPlot::SetupAxisScale(plot_axis, ImPlotScale_Linear);
                }
            }

            // Set the plot X_axis to time
            ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
            if(renderable_plot.isRealTime()){
                ImPlot::SetupAxisLimits(ImAxis_X1, plot_start, plot_end, ImGuiCond_Always);
            } else {
                ImPlot::SetupAxisLimits(ImAxis_X1, plot_start, plot_end, ImGuiCond_Once);
            }

            ImPlotRect limits = ImPlot::GetPlotLimits();
            double range = limits.X.Max - limits.X.Min;
            int num_pixels = ImPlot::GetPlotPos().x + ImPlot::GetPlotSize().x;

            // Plot all sensors
            for (const auto& series_label : sensors) {
                // Get downsampled data
                auto [xs, ys] = viewModel_.getDownsampledData(renderable_plot, series_label, range, num_pixels);

                // Apply the plotline properties
                // Line colour
                ImPlot::SetNextLineStyle(renderable_plot.getPlotLineProperties(series_label).colour);

                // Marker style
                ImPlot::SetNextMarkerStyle(renderable_plot.getPlotLineProperties(series_label).marker_style,
                    renderable_plot.getPlotLineProperties(series_label).marker_size,
                    renderable_plot.getPlotLineProperties(series_label).fill, renderable_plot.getPlotLineProperties(series_label).fill_weight,
                    renderable_plot.getPlotLineProperties(series_label).fill_outline);

                // Get the axis (Y1, Y2, Y3) for the sensor
                ImAxis plot_axis = renderable_plot.getYAxisForSensor(series_label);
                ImPlot::SetAxes(ImAxis_X1, plot_axis);
                ImPlot::PlotStairs(series_label.c_str(), xs.data(), ys.data(), xs.size());
            }

            // Callback plot range if plot range changes
            // ImPlotRect limits = ImPlot::GetPlotLimits();
            if (limits.X.Min != renderable_plot.getPlotRange().first || limits.X.Max != renderable_plot.getPlotRange().second) {
                renderable_plot.notifyRangeChange(limits.X.Min, limits.X.Max);

                // Update the plot range
                renderable_plot.setPlotRange(limits.X.Min, limits.X.Max);

                // Callback to update the range in the data manager
                if (update_range_callback_) {
                    // Update the range for all sensors in the plot
                    for (const auto& sensor : sensors) {
                        update_range_callback_(sensor, renderable_plot.getPlotId(), limits.X.Min, limits.X.Max);
                    }
                }
            }

            // Callback the Y1, Y2, Y3 min and max values
            for (const auto& axis : {ImAxis_Y1, ImAxis_Y2, ImAxis_Y3}) {
                // Check if the axis exists in the plot
                bool axis_exists = false;
                for (const auto& sensor: sensors) {
                    if (renderable_plot.getYAxisForSensor(sensor) == axis) {
                        axis_exists = true;
                        break;
                    }
                }
                if (!axis_exists) {
                    continue;
                }

                // Get the min and max values for the axis
                ImPlotRect y_axis_limits = ImPlot::GetPlotLimits(ImAxis_X1, axis);
                double axis_min = y_axis_limits.Y.Min;
                double axis_max = y_axis_limits.Y.Max;

                // Update the axis properties in the renderable plot
                renderable_plot.setYAxisPropertiesMin(axis, axis_min);
                renderable_plot.setYAxisPropertiesMax(axis, axis_max);

            }

            ImPlot::EndPlot();
        }
        ImGui::Separator();
    }
}

// Render menu bar for the window
void GraphView::renderWindowMenuBar(WindowPlots* window) {
    // Get the file menu state
    auto& file_menu_state = viewModel_.getFileMenuState();

    // Menu bar for the window
    if (ImGui::BeginMenuBar()) {
        // ********** File menu **********
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Save Window")) {
                // Save plot
            }

            if (ImGui::MenuItem("Save Window As...")) {
                // Open the "Save Window As" popup
                file_menu_state.save_as_open_popup = true;
            }

            ImGui::EndMenu();
        }

        // ********** Options menu **********
        if (ImGui::BeginMenu("Options")) {
            if (ImGui::MenuItem("Add Plot")) {
                // Add plot popup
            }

            if (ImGui::MenuItem("Remove Plot")) {
                // Remove plot
            }

            if (ImGui::MenuItem("Configure Window")) {
                // Configure window popup
            }
            ImGui::EndMenu();
        }

        // End the menu bar
        ImGui::EndMenuBar();
    }

    // Check if the "Save Window As" popup should be rendered
    if (file_menu_state.save_as_open_popup) {
        ImGui::OpenPopup(("Save Window As###" + window->getLabel()).c_str());

        // Get the SaveWindowAsPopupState
        auto& save_window_as_popup_state = viewModel_.getSaveWindowAsPopupState();

        // Reset the SaveWindowAsPopupState
        save_window_as_popup_state.reset();

        // Set the window label
        save_window_as_popup_state.window_label = window->getLabel();

        // Set the file name buffer to the window label
        std::strcpy(save_window_as_popup_state.file_name_buffer, window->getLabel().c_str());

    }

    // Render the "Save Window As" popup
    if (ImGui::BeginPopupModal(("Save Window As###" + window->getLabel()).c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        // Get the file dialog state and save window as popup state
        auto& save_window_as_popup_state = viewModel_.getSaveWindowAsPopupState();
        auto& file_dialog_state = viewModel_.getFileDialogState();

        // Input text for the folder path to save the window.
        // Check if the path is valid
        if (!std::filesystem::exists(save_window_as_popup_state.file_path_buffer)) {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Path does not exist");
            save_window_as_popup_state.is_able_to_save = false;
        } else {
            save_window_as_popup_state.is_able_to_save = true;
        }
        ImGui::Text("Folder path:");
        ImGui::SameLine();
        float input_text_width = ImGui::CalcTextSize(save_window_as_popup_state.file_path_buffer).x + ImGui::GetStyle().FramePadding.x * 2.0f;
        ImGui::SetNextItemWidth(input_text_width);
        ImGui::InputText("###Folder path", save_window_as_popup_state.file_path_buffer,
            sizeof(save_window_as_popup_state.file_path_buffer));

        ImGui::SameLine();

        // Browse button
        if (ImGui::Button("Browse")) {
            // Open the file dialog
            FileDialog::file_dialog_open = true;
            FileDialog::file_dialog_open_type = FileDialog::FileDialogType::SelectFolder;

            // Copy the current path to the file dialog path buffer if it exists
            if (std::filesystem::exists(save_window_as_popup_state.file_path_buffer)) {
                std::strcpy(file_dialog_state.path_buffer, save_window_as_popup_state.file_path_buffer);
            }
        }

        // Input text for title but disabled
        ImGui::Text("File name");
        input_text_width = std::max(
            ImGui::CalcTextSize(save_window_as_popup_state.file_name_buffer).x + ImGui::GetStyle().FramePadding.x * 2.0f,
            input_text_width);
        ImGui::SetNextItemWidth(input_text_width);
        ImGui::SameLine();
        ImGui::InputText("###File name", save_window_as_popup_state.file_name_buffer,
            sizeof(save_window_as_popup_state.file_name_buffer), ImGuiInputTextFlags_ReadOnly);

        // Render the file dialog if it is open
        if (FileDialog::file_dialog_open) {
            // Render the file dialog
            FileDialog::ShowFileDialog(&FileDialog::file_dialog_open, file_dialog_state.path_buffer, sizeof(file_dialog_state.path_buffer), FileDialog::file_dialog_open_type);

            // Copy the selected path to the save window as popup state if it exists
            if (!FileDialog::file_dialog_open && std::filesystem::exists(file_dialog_state.path_buffer)) {
                std::cout << "Selected path: " << file_dialog_state.path_buffer << "\n";
                std::strcpy(save_window_as_popup_state.file_path_buffer, file_dialog_state.path_buffer);
            }
        }

        // Close the popup
        if (ImGui::Button("Cancel")) {
            // Close the popup
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        // Save to folder
        if (!save_window_as_popup_state.is_able_to_save) {
            // Disable the save button if cannot be saved
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
        }
        if (ImGui::Button("Save to folder")) {
            // Save the folder action function
            ActionSubmitSaveWindowAsPlotPopup(*window, save_window_as_popup_state);

            // Close the popup
            ImGui::CloseCurrentPopup();
        }
        if (!save_window_as_popup_state.is_able_to_save) {
            ImGui::PopItemFlag();
            ImGui::PopStyleVar();
        }

        // End the popup
        ImGui::EndPopup();
    }

    // Reset the file dialog state
    file_menu_state.reset();
}

// Render the "Plot options" popup
void GraphView::renderPlotOptions(const std::string& popup_label, RenderablePlot& renderable_plot) {
    if (ImGui::Button(("Options###" + renderable_plot.getLabel() + "OptionsMenu").c_str())) {
        ImGui::OpenPopup(("###" + renderable_plot.getLabel()).c_str());

        // Set the current state of the popup
        auto& plot_option_pop_up_state = viewModel_.getPlotOptionsState();
        plot_option_pop_up_state.window_label = renderable_plot.getWindowLabel();
        plot_option_pop_up_state.plot_label = renderable_plot.getLabel();
        plot_option_pop_up_state.available_sensors = viewModel_.getPlottableSensors();
        plot_option_pop_up_state.is_real_time = renderable_plot.isRealTime();
        plot_option_pop_up_state.sensors_in_available_list_box
            = plot_option_pop_up_state.available_sensors;
        plot_option_pop_up_state.selected_sensor_in_available_list_box = -1;
        plot_option_pop_up_state.sensors_in_Y1_list_box = renderable_plot.getSensorsForYAxis(ImAxis_Y1);
        plot_option_pop_up_state.sensors_in_Y2_list_box = renderable_plot.getSensorsForYAxis(ImAxis_Y2);
        plot_option_pop_up_state.sensors_in_Y3_list_box = renderable_plot.getSensorsForYAxis(ImAxis_Y3);

        // Remove all elements from the available sensors list that are already in the Y1, Y2, Y3 list
        for (const auto& sensor : plot_option_pop_up_state.sensors_in_Y1_list_box) {
            plot_option_pop_up_state.sensors_in_available_list_box.erase(
                std::remove(plot_option_pop_up_state.sensors_in_available_list_box.begin(),
                    plot_option_pop_up_state.sensors_in_available_list_box.end(),
                    sensor),
                plot_option_pop_up_state.sensors_in_available_list_box.end());
        }
        for (const auto& sensor : plot_option_pop_up_state.sensors_in_Y2_list_box) {
            plot_option_pop_up_state.sensors_in_available_list_box.erase(
                std::remove(plot_option_pop_up_state.sensors_in_available_list_box.begin(),
                    plot_option_pop_up_state.sensors_in_available_list_box.end(),
                    sensor),
                plot_option_pop_up_state.sensors_in_available_list_box.end());
        }
        for (const auto& sensor : plot_option_pop_up_state.sensors_in_Y3_list_box) {
            plot_option_pop_up_state.sensors_in_available_list_box.erase(
                std::remove(plot_option_pop_up_state.sensors_in_available_list_box.begin(),
                    plot_option_pop_up_state.sensors_in_available_list_box.end(),
                    sensor),
                plot_option_pop_up_state.sensors_in_available_list_box.end());
        }

        plot_option_pop_up_state.Y1_properties = renderable_plot.getYAxisProperties(ImAxis_Y1);
        plot_option_pop_up_state.Y2_properties = renderable_plot.getYAxisProperties(ImAxis_Y2);
        plot_option_pop_up_state.Y3_properties = renderable_plot.getYAxisProperties(ImAxis_Y3);

        // Set the current plotline properties for each sensor
        plot_option_pop_up_state.sensor_to_plotline_properties = renderable_plot.getAllPlotLineProperties();
    }

    // Start a Popup Modal
    if (ImGui::BeginPopupModal(popup_label.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        // Get the current state of the popup
        auto& plot_option_pop_up_state = viewModel_.getPlotOptionsState();

        bool is_able_to_submit = true;

        // ******* Real-time toggle ********
        ImGui::Checkbox("Real-time", &plot_option_pop_up_state.is_real_time);

        // ******* Input boxes for plot range *******
        if(!plot_option_pop_up_state.is_real_time) {
            // Enter options for plot range if not real-time
            if (!plot_option_pop_up_state.is_range_initialized) {
                // Initialize textboxes with default values only once
                SetPlotRangeState(plot_option_pop_up_state);
                plot_option_pop_up_state.is_range_initialized = true;
            }

            ImGui::Text("Enter plot range (YYYY-MM-DD HH:MM:SS)");

            renderDateTimeField("Plot start date", plot_option_pop_up_state.plot_range_start_year,
                plot_option_pop_up_state.plot_range_start_month,
                plot_option_pop_up_state.plot_range_start_day,
                plot_option_pop_up_state.plot_range_start_hour,
                plot_option_pop_up_state.plot_range_start_minute,
                plot_option_pop_up_state.plot_range_start_second);
            renderDateTimeField("Plot end date", plot_option_pop_up_state.plot_range_end_year,
                plot_option_pop_up_state.plot_range_end_month,
                plot_option_pop_up_state.plot_range_end_day,
                plot_option_pop_up_state.plot_range_end_hour,
                plot_option_pop_up_state.plot_range_end_minute,
                plot_option_pop_up_state.plot_range_end_second
                );

            if (std::strlen(plot_option_pop_up_state.plot_range_start_year) == 0 ||
                std::strlen(plot_option_pop_up_state.plot_range_start_month) == 0 ||
                std::strlen(plot_option_pop_up_state.plot_range_start_day) == 0 ||
                std::strlen(plot_option_pop_up_state.plot_range_start_hour) == 0 ||
                std::strlen(plot_option_pop_up_state.plot_range_start_minute) == 0 ||
                std::strlen(plot_option_pop_up_state.plot_range_start_second) == 0 ||
                std::strlen(plot_option_pop_up_state.plot_range_end_year) == 0 ||
                std::strlen(plot_option_pop_up_state.plot_range_end_month) == 0 ||
                std::strlen(plot_option_pop_up_state.plot_range_end_day) == 0 ||
                std::strlen(plot_option_pop_up_state.plot_range_end_hour) == 0 ||
                std::strlen(plot_option_pop_up_state.plot_range_end_minute) == 0 ||
                std::strlen(plot_option_pop_up_state.plot_range_end_second) == 0) {
                ImGui::Text("Please enter all fields");
                plot_option_pop_up_state.is_able_to_submit = false;
            } else {
                plot_option_pop_up_state.is_able_to_submit = true;
            }

            // Ensure end time is after start time
            // Convert input into unix time
            try {
                static std::tm input_start_time = {};
                input_start_time.tm_year = std::stoi(plot_option_pop_up_state.plot_range_start_year) - 1900;
                input_start_time.tm_mon = std::stoi(plot_option_pop_up_state.plot_range_start_month) - 1;
                input_start_time.tm_mday = std::stoi(plot_option_pop_up_state.plot_range_start_day);
                input_start_time.tm_hour = std::stoi(plot_option_pop_up_state.plot_range_start_hour);
                input_start_time.tm_min = std::stoi(plot_option_pop_up_state.plot_range_start_minute);
                input_start_time.tm_sec = std::stoi(plot_option_pop_up_state.plot_range_start_second);
                std::time_t start_unix_time = std::mktime(&input_start_time);

                std::tm input_end_time = {};
                input_end_time.tm_year = std::stoi(plot_option_pop_up_state.plot_range_end_year) - 1900;
                input_end_time.tm_mon = std::stoi(plot_option_pop_up_state.plot_range_end_month) - 1;
                input_end_time.tm_mday = std::stoi(plot_option_pop_up_state.plot_range_end_day);
                input_end_time.tm_hour = std::stoi(plot_option_pop_up_state.plot_range_end_hour);
                input_end_time.tm_min = std::stoi(plot_option_pop_up_state.plot_range_end_minute);
                input_end_time.tm_sec = std::stoi(plot_option_pop_up_state.plot_range_end_second);
                std::time_t end_unix_time = std::mktime(&input_end_time);

                if (end_unix_time < start_unix_time) {
                    ImGui::Text("End time must be after start time");
                    plot_option_pop_up_state.is_able_to_submit = false;
                } else {
                    plot_option_pop_up_state.is_able_to_submit = true;
                }
            } catch (const std::exception& e) {
                ImGui::Text("Invalid date/time format");
                plot_option_pop_up_state.is_able_to_submit = false;
            }

        }

        // ******* Text boxes with Drag-and-Drop for sensors *******
        // Search bar for available sensors
        ImGui::InputText("Search", plot_option_pop_up_state.search_available_sensors_buffer,
            sizeof(plot_option_pop_up_state.search_available_sensors_buffer));

        if (ImGui::BeginListBox("Available Sensors")) {
            // If no sensors are available, display a message to drop sensors here
            if (plot_option_pop_up_state.sensors_in_available_list_box.empty()) {
                ImGui::Selectable("Drop here to add sensors...", false, NULL);

                // FROM Y1 SENSORS TO AVAILABLE SENSORS
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_FROM_Y1_SENSORS")) {
                        int payload_n = *(const int*)payload->Data;
                        std::cout << "Payload received from Y1 Sensors into Available Sensors: " << plot_option_pop_up_state.sensors_in_Y1_list_box.at(payload_n) << "\n";

                        // Delete the sensor from the Y1 list and add it to the available list
                        plot_option_pop_up_state.sensors_in_available_list_box.push_back(plot_option_pop_up_state.sensors_in_Y1_list_box.at(payload_n));
                        plot_option_pop_up_state.sensors_in_Y1_list_box.erase(plot_option_pop_up_state.sensors_in_Y1_list_box.begin() + payload_n);
                    }
                    ImGui::EndDragDropTarget();
                }

                // FROM Y2 SENSORS TO AVAILABLE SENSORS
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_FROM_Y2_SENSORS")) {
                        int payload_n = *(const int*)payload->Data;
                        std::cout << "Payload received from Y2 Sensors into Available Sensors: " << plot_option_pop_up_state.sensors_in_Y2_list_box.at(payload_n) << "\n";

                        // Delete the sensor from the Y2 list and add it to the available list
                        plot_option_pop_up_state.sensors_in_available_list_box.push_back(plot_option_pop_up_state.sensors_in_Y2_list_box.at(payload_n));
                        plot_option_pop_up_state.sensors_in_Y2_list_box.erase(plot_option_pop_up_state.sensors_in_Y2_list_box.begin() + payload_n);
                    }
                    ImGui::EndDragDropTarget();
                }

                // FROM Y3 SENSORS TO AVAILABLE SENSORS
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_FROM_Y3_SENSORS")) {
                        int payload_n = *(const int*)payload->Data;
                        std::cout << "Payload received from Y3 Sensors into Available Sensors: " << plot_option_pop_up_state.sensors_in_Y3_list_box.at(payload_n) << "\n";

                        // Delete the sensor from the Y3 list and add it to the available list
                        plot_option_pop_up_state.sensors_in_available_list_box.push_back(plot_option_pop_up_state.sensors_in_Y3_list_box.at(payload_n));
                        plot_option_pop_up_state.sensors_in_Y3_list_box.erase(plot_option_pop_up_state.sensors_in_Y3_list_box.begin() + payload_n);
                    }
                    ImGui::EndDragDropTarget();
                }
            }

            // List all available sensors that have not been selected
            for (size_t i = 0;
            i < plot_option_pop_up_state.sensors_in_available_list_box.size();
            ++i) {
                // Check if the sensor matches the search text
                if (!IsSearchBarMatch(plot_option_pop_up_state.search_available_sensors_buffer,
                    plot_option_pop_up_state.sensors_in_available_list_box.at(i)))
                    continue;

                ImGui::Selectable(plot_option_pop_up_state.sensors_in_available_list_box.at(i).c_str());

                ImGuiDragDropFlags src_flags = 0;
                // src_flags |= ImGuiDragDropFlags_SourceNoDisableHover;     // Keep the source displayed as hovered
                // src_flags |= ImGuiDragDropFlags_SourceNoHoldToOpenOthers; // Because our dragging is local, we disable the feature of opening foreign treenodes/tabs while dragging

                if (ImGui::BeginDragDropSource(src_flags)) {
                    if (!(src_flags))
                        ImGui::Text("Moving \"%s\"", plot_option_pop_up_state.sensors_in_available_list_box.at(i).c_str());
                    ImGui::SetDragDropPayload("DND_FROM_AVAILABLE_SENSORS", &i, sizeof(int));
                    ImGui::EndDragDropSource();
                }

                // FROM Y1 SENSORS TO AVAILABLE SENSORS
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_FROM_Y1_SENSORS")) {
                        int payload_n = *(const int*)payload->Data;
                        std::cout << "Payload received from Y1 Sensors into Available Sensors: " << plot_option_pop_up_state.sensors_in_Y1_list_box.at(payload_n) << "\n";

                        // Delete the sensor from the Y1 list and add it to the available list
                        plot_option_pop_up_state.sensors_in_available_list_box.push_back(plot_option_pop_up_state.sensors_in_Y1_list_box.at(payload_n));
                        plot_option_pop_up_state.sensors_in_Y1_list_box.erase(plot_option_pop_up_state.sensors_in_Y1_list_box.begin() + payload_n);

                        // Re-arrange the sensors in the available list
                        std::map<int, std::string> sensor_pos_map;
                        std::vector<std::string> ordered_by_sensor;
                        for (const std::string sensor : plot_option_pop_up_state.sensors_in_available_list_box) {
                            auto it = std::find(
                                plot_option_pop_up_state.available_sensors.begin(),
                                plot_option_pop_up_state.available_sensors.end(),
                                sensor
                            );

                            if (it == plot_option_pop_up_state.available_sensors.end()) {
                                // Exit program
                                std::cerr << "Error: Sensor not found in available sensors\n";
                                exit(1);
                            }

                            long int distance_to_sensor = std::distance(
                                plot_option_pop_up_state.available_sensors.begin(),
                                it
                            );

                            sensor_pos_map[distance_to_sensor] = sensor;
                        }
                        for (const auto& [pos, sensor] : sensor_pos_map) {
                            ordered_by_sensor.push_back(sensor);
                        }
                        plot_option_pop_up_state.sensors_in_available_list_box = ordered_by_sensor;
                    }

                    ImGui::EndDragDropTarget();
                }

                // FROM Y2 SENSORS TO AVAILABLE SENSORS
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_FROM_Y2_SENSORS")) {
                        int payload_n = *(const int*)payload->Data;
                        std::cout << "Payload received from Y1 Sensors into Available Sensors: " << plot_option_pop_up_state.sensors_in_Y2_list_box.at(payload_n) << "\n";

                        // Delete the sensor from the Y1 list and add it to the available list
                        plot_option_pop_up_state.sensors_in_available_list_box.push_back(plot_option_pop_up_state.sensors_in_Y2_list_box.at(payload_n));
                        plot_option_pop_up_state.sensors_in_Y2_list_box.erase(plot_option_pop_up_state.sensors_in_Y2_list_box.begin() + payload_n);

                        // Re-arrange the sensors in the available list
                        std::map<int, std::string> sensor_pos_map;
                        std::vector<std::string> ordered_by_sensor;
                        for (const std::string sensor : plot_option_pop_up_state.sensors_in_available_list_box) {
                            auto it = std::find(
                                plot_option_pop_up_state.available_sensors.begin(),
                                plot_option_pop_up_state.available_sensors.end(),
                                sensor
                            );

                            if (it == plot_option_pop_up_state.available_sensors.end()) {
                                // Exit program
                                std::cerr << "Error: Sensor not found in available sensors\n";
                                exit(1);
                            }

                            long int distance_to_sensor = std::distance(
                                plot_option_pop_up_state.available_sensors.begin(),
                                it
                            );

                            sensor_pos_map[distance_to_sensor] = sensor;
                        }
                        for (const auto& [pos, sensor] : sensor_pos_map) {
                            ordered_by_sensor.push_back(sensor);
                        }
                        plot_option_pop_up_state.sensors_in_available_list_box = ordered_by_sensor;
                    }
                    ImGui::EndDragDropTarget();
                }

                // FROM Y3 SENSORS TO AVAILABLE SENSORS
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_FROM_Y3_SENSORS")) {
                        int payload_n = *(const int*)payload->Data;
                        std::cout << "Payload received from Y3 Sensors into Available Sensors: " << plot_option_pop_up_state.sensors_in_Y3_list_box.at(payload_n) << "\n";

                        // Delete the sensor from the Y3 list and add it to the available list
                        plot_option_pop_up_state.sensors_in_available_list_box.push_back(plot_option_pop_up_state.sensors_in_Y3_list_box.at(payload_n));
                        plot_option_pop_up_state.sensors_in_Y3_list_box.erase(plot_option_pop_up_state.sensors_in_Y3_list_box.begin() + payload_n);

                        // Re-arrange the sensors in the available list
                        std::map<int, std::string> sensor_pos_map;
                        std::vector<std::string> ordered_by_sensor;
                        for (const std::string sensor : plot_option_pop_up_state.sensors_in_available_list_box) {
                            auto it = std::find(
                                plot_option_pop_up_state.available_sensors.begin(),
                                plot_option_pop_up_state.available_sensors.end(),
                                sensor
                            );

                            if (it == plot_option_pop_up_state.available_sensors.end()) {
                                // Exit program
                                std::cerr << "Error: Sensor not found in available sensors\n";
                                exit(1);
                            }

                            long int distance_to_sensor = std::distance(
                                plot_option_pop_up_state.available_sensors.begin(),
                                it
                            );

                            sensor_pos_map[distance_to_sensor] = sensor;
                        }
                        for (const auto& [pos, sensor] : sensor_pos_map) {
                            ordered_by_sensor.push_back(sensor);
                        }
                        plot_option_pop_up_state.sensors_in_available_list_box = ordered_by_sensor;
                    }
                    ImGui::EndDragDropTarget();
                }
            }
            ImGui::EndListBox();
        }

        if (ImGui::BeginListBox("Y1 Sensors")) {
            // If no sensors are available, display a message to drop sensors here
            if (plot_option_pop_up_state.sensors_in_Y1_list_box.empty()) {
                ImGui::Selectable("Drop here to add sensors...", false, NULL);

                // FROM AVAILABLE SENSORS TO Y1 SENSORS
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_FROM_AVAILABLE_SENSORS")) {
                        int payload_n = *(const int*)payload->Data;
                        std::cout << "Payload received from Available Sensors: " << plot_option_pop_up_state.sensors_in_available_list_box.at(payload_n) << "\n";

                        // Delete the sensor from the available list and add it to the Y1 list
                        plot_option_pop_up_state.sensors_in_Y1_list_box.push_back(plot_option_pop_up_state.sensors_in_available_list_box.at(payload_n));
                        plot_option_pop_up_state.sensors_in_available_list_box.erase(plot_option_pop_up_state.sensors_in_available_list_box.begin() + payload_n);
                    }
                    ImGui::EndDragDropTarget();
                }

                // FROM Y2 SENSORS TO Y1 SENSORS
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_FROM_Y2_SENSORS")) {
                        int payload_n = *(const int*)payload->Data;
                        std::cout << "Payload received from Y2 Sensors into Y1 Sensors: " << plot_option_pop_up_state.sensors_in_Y2_list_box.at(payload_n) << "\n";

                        // Delete the sensor from the Y2 list and add it to the Y1 list
                        plot_option_pop_up_state.sensors_in_Y1_list_box.push_back(plot_option_pop_up_state.sensors_in_Y2_list_box.at(payload_n));
                        plot_option_pop_up_state.sensors_in_Y2_list_box.erase(plot_option_pop_up_state.sensors_in_Y2_list_box.begin() + payload_n);
                    }
                    ImGui::EndDragDropTarget();
                }

                // FROM Y3 SENSORS TO Y1 SENSORS
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_FROM_Y3_SENSORS")) {
                        int payload_n = *(const int*)payload->Data;
                        std::cout << "Payload received from Y3 Sensors into Y1 Sensors: " << plot_option_pop_up_state.sensors_in_Y3_list_box.at(payload_n) << "\n";

                        // Delete the sensor from the Y3 list and add it to the Y1 list
                        plot_option_pop_up_state.sensors_in_Y1_list_box.push_back(plot_option_pop_up_state.sensors_in_Y3_list_box.at(payload_n));
                        plot_option_pop_up_state.sensors_in_Y3_list_box.erase(plot_option_pop_up_state.sensors_in_Y3_list_box.begin() + payload_n);
                    }
                    ImGui::EndDragDropTarget();
                }
            }

            // Loop through existing sensors in Y1 and allow drag-and-drop
            for (size_t i = 0; i < plot_option_pop_up_state.sensors_in_Y1_list_box.size(); ++i) {
                ImGui::Selectable(plot_option_pop_up_state.sensors_in_Y1_list_box.at(i).c_str());

                ImGuiDragDropFlags src_flags = 0;
                if (ImGui::BeginDragDropSource(src_flags)) {
                    if (!(src_flags))
                        ImGui::Text("Moving \"%s\"", plot_option_pop_up_state.sensors_in_Y1_list_box.at(i).c_str());
                    ImGui::SetDragDropPayload("DND_FROM_Y1_SENSORS", &i, sizeof(int));
                    ImGui::EndDragDropSource();
                }

                // FROM AVAILABLE SENSORS TO Y1 SENSORS
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_FROM_AVAILABLE_SENSORS")) {
                        int payload_n = *(const int*)payload->Data;
                        std::cout << "Payload received from Available Sensors into Y1 Sensors: " << plot_option_pop_up_state.sensors_in_available_list_box.at(payload_n) << "\n";

                        // Delete the sensor from the available list and add it to the Y1 list
                        plot_option_pop_up_state.sensors_in_Y1_list_box.push_back(plot_option_pop_up_state.sensors_in_available_list_box.at(payload_n));
                        plot_option_pop_up_state.sensors_in_available_list_box.erase(plot_option_pop_up_state.sensors_in_available_list_box.begin() + payload_n);
                    }
                    ImGui::EndDragDropTarget();
                }

                // FROM Y1 SENSORS TO Y1 SENSORS
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_FROM_Y1_SENSORS")) {
                        int payload_n = *(const int*)payload->Data;
                        std::cout << "Payload received from Y1 Sensors: " << plot_option_pop_up_state.sensors_in_Y1_list_box.at(payload_n) << "\n";

                        // Switch the positions of the sensors in the Y1 list into the current position
                        std::swap(plot_option_pop_up_state.sensors_in_Y1_list_box.at(payload_n), plot_option_pop_up_state.sensors_in_Y1_list_box.at(i));
                    }
                    ImGui::EndDragDropTarget();
                }

                // FROM Y2 SENSORS TO Y1 SENSORS
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_FROM_Y2_SENSORS")) {
                        int payload_n = *(const int*)payload->Data;
                        std::cout << "Payload received from Y2 Sensors into Y1 Sensors: " << plot_option_pop_up_state.sensors_in_Y2_list_box.at(payload_n) << "\n";

                        // Delete the sensor from the Y2 list and add it to the Y1 list
                        plot_option_pop_up_state.sensors_in_Y1_list_box.push_back(plot_option_pop_up_state.sensors_in_Y2_list_box.at(payload_n));
                        plot_option_pop_up_state.sensors_in_Y2_list_box.erase(plot_option_pop_up_state.sensors_in_Y2_list_box.begin() + payload_n);
                    }
                    ImGui::EndDragDropTarget();
                }

                // FROM Y3 SENSORS TO Y1 SENSORS
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_FROM_Y3_SENSORS")) {
                        int payload_n = *(const int*)payload->Data;
                        std::cout << "Payload received from Y3 Sensors into Y1 Sensors: " << plot_option_pop_up_state.sensors_in_Y3_list_box.at(payload_n) << "\n";

                        // Delete the sensor from the Y3 list and add it to the Y1 list
                        plot_option_pop_up_state.sensors_in_Y1_list_box.push_back(plot_option_pop_up_state.sensors_in_Y3_list_box.at(payload_n));
                        plot_option_pop_up_state.sensors_in_Y3_list_box.erase(plot_option_pop_up_state.sensors_in_Y3_list_box.begin() + payload_n);
                    }
                    ImGui::EndDragDropTarget();
                }
            }

            ImGui::EndListBox();
        }

        if (ImGui::BeginListBox("Y2 Sensors")) {
            // If no sensors are available, display a message to drop sensors here
            if (plot_option_pop_up_state.sensors_in_Y2_list_box.empty()) {
                ImGui::Selectable("Drop here to add sensors...", false, NULL);

                // FROM AVAILABLE SENSORS TO Y2 SENSORS
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_FROM_AVAILABLE_SENSORS")) {
                        int payload_n = *(const int*)payload->Data;
                        std::cout << "Payload received from Available Sensors into Y2 Sensors: " << plot_option_pop_up_state.sensors_in_available_list_box.at(payload_n) << "\n";

                        // Delete the sensor from the available list and add it to the Y2 list
                        plot_option_pop_up_state.sensors_in_Y2_list_box.push_back(plot_option_pop_up_state.sensors_in_available_list_box.at(payload_n));
                        plot_option_pop_up_state.sensors_in_available_list_box.erase(plot_option_pop_up_state.sensors_in_available_list_box.begin() + payload_n);
                    }
                    ImGui::EndDragDropTarget();
                }

                // FROM Y1 SENSORS TO Y2 SENSORS
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_FROM_Y1_SENSORS")) {
                        int payload_n = *(const int*)payload->Data;
                        std::cout << "Payload received from Y1 Sensors into Y2 Sensors: " << plot_option_pop_up_state.sensors_in_Y1_list_box.at(payload_n) << "\n";

                        // Delete the sensor from the available list and add it to the Y2 list
                        plot_option_pop_up_state.sensors_in_Y2_list_box.push_back(plot_option_pop_up_state.sensors_in_Y1_list_box.at(payload_n));
                        plot_option_pop_up_state.sensors_in_Y1_list_box.erase(plot_option_pop_up_state.sensors_in_Y1_list_box.begin() + payload_n);
                    }
                    ImGui::EndDragDropTarget();
                }

                // FROM Y3 SENSORS TO Y2 SENSORS
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_FROM_Y3_SENSORS")) {
                        int payload_n = *(const int*)payload->Data;
                        std::cout << "Payload received from Y3 Sensors into Y2 Sensors: " << plot_option_pop_up_state.sensors_in_Y3_list_box.at(payload_n) << "\n";

                        // Delete the sensor from the Y3 list and add it to the Y1 list
                        plot_option_pop_up_state.sensors_in_Y2_list_box.push_back(plot_option_pop_up_state.sensors_in_Y3_list_box.at(payload_n));
                        plot_option_pop_up_state.sensors_in_Y3_list_box.erase(plot_option_pop_up_state.sensors_in_Y3_list_box.begin() + payload_n);
                    }
                    ImGui::EndDragDropTarget();
                }
            }

            // Loop through existing sensors in Y2 and allow drag-and-drop
            for (size_t i = 0; i < plot_option_pop_up_state.sensors_in_Y2_list_box.size(); ++i) {
                ImGui::Selectable(plot_option_pop_up_state.sensors_in_Y2_list_box.at(i).c_str());

                ImGuiDragDropFlags src_flags = 0;
                if (ImGui::BeginDragDropSource(src_flags)) {
                    if (!(src_flags))
                        ImGui::Text("Moving \"%s\"", plot_option_pop_up_state.sensors_in_Y2_list_box.at(i).c_str());
                    ImGui::SetDragDropPayload("DND_FROM_Y2_SENSORS", &i, sizeof(int));
                    ImGui::EndDragDropSource();
                }

                // FROM Y2 SENSORS TO Y2 SENSORS
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_FROM_Y2_SENSORS")) {
                        int payload_n = *(const int*)payload->Data;
                        std::cout << "Payload received from Y2 Sensors to Y2 Sensors: " << plot_option_pop_up_state.sensors_in_Y2_list_box.at(payload_n) << "\n";

                        // Switch the positions of the sensors in the Y1 list into the current position
                        std::swap(plot_option_pop_up_state.sensors_in_Y2_list_box.at(payload_n), plot_option_pop_up_state.sensors_in_Y2_list_box.at(i));
                    }
                    ImGui::EndDragDropTarget();
                }

                // FROM AVAILABLE SENSORS TO Y2 SENSORS
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_FROM_AVAILABLE_SENSORS")) {
                        int payload_n = *(const int*)payload->Data;
                        std::cout << "Payload received from Available Sensors into Y2 Sensors: " << plot_option_pop_up_state.sensors_in_available_list_box.at(payload_n) << "\n";

                        // Delete the sensor from the available list and add it to the Y2 list
                        plot_option_pop_up_state.sensors_in_Y2_list_box.push_back(plot_option_pop_up_state.sensors_in_available_list_box.at(payload_n));
                        plot_option_pop_up_state.sensors_in_available_list_box.erase(plot_option_pop_up_state.sensors_in_available_list_box.begin() + payload_n);
                    }
                    ImGui::EndDragDropTarget();
                }

                // FROM Y1 SENSORS TO Y2 SENSORS
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_FROM_Y1_SENSORS")) {
                        int payload_n = *(const int*)payload->Data;
                        std::cout << "Payload received from Y1 Sensors into Y2 Sensors: " << plot_option_pop_up_state.sensors_in_Y1_list_box.at(payload_n) << "\n";

                        // Delete the sensor from the available list and add it to the Y2 list
                        plot_option_pop_up_state.sensors_in_Y2_list_box.push_back(plot_option_pop_up_state.sensors_in_Y1_list_box.at(payload_n));
                        plot_option_pop_up_state.sensors_in_Y1_list_box.erase(plot_option_pop_up_state.sensors_in_Y1_list_box.begin() + payload_n);
                    }
                    ImGui::EndDragDropTarget();
                }

                // FROM Y3 SENSORS TO Y2 SENSORS
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_FROM_Y3_SENSORS")) {
                        int payload_n = *(const int*)payload->Data;
                        std::cout << "Payload received from Y3 Sensors into Y2 Sensors: " << plot_option_pop_up_state.sensors_in_Y3_list_box.at(payload_n) << "\n";

                        // Delete the sensor from the Y3 list and add it to the Y1 list
                        plot_option_pop_up_state.sensors_in_Y2_list_box.push_back(plot_option_pop_up_state.sensors_in_Y3_list_box.at(payload_n));
                        plot_option_pop_up_state.sensors_in_Y3_list_box.erase(plot_option_pop_up_state.sensors_in_Y3_list_box.begin() + payload_n);
                    }
                    ImGui::EndDragDropTarget();
                }
            }

            ImGui::EndListBox();
        }

        if (ImGui::BeginListBox("Y3 Sensors")) {
            // If no sensors are available, display a message to drop sensors here
            if (plot_option_pop_up_state.sensors_in_Y3_list_box.empty()) {
                ImGui::Selectable("Drop here to add sensors...", false, NULL);

                // FROM AVAILABLE SENSORS TO Y3 SENSORS
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_FROM_AVAILABLE_SENSORS")) {
                        int payload_n = *(const int*)payload->Data;
                        std::cout << "Payload received from Available Sensors into Y3 Sensors: " << plot_option_pop_up_state.sensors_in_available_list_box.at(payload_n) << "\n";

                        // Delete the sensor from the available list and add it to the Y2 list
                        plot_option_pop_up_state.sensors_in_Y3_list_box.push_back(plot_option_pop_up_state.sensors_in_available_list_box.at(payload_n));
                        plot_option_pop_up_state.sensors_in_available_list_box.erase(plot_option_pop_up_state.sensors_in_available_list_box.begin() + payload_n);
                    }
                    ImGui::EndDragDropTarget();
                }

                // FROM Y1 SENSORS TO Y3 SENSORS
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_FROM_Y1_SENSORS")) {
                        int payload_n = *(const int*)payload->Data;
                        std::cout << "Payload received from Y1 Sensors into Y3 Sensors: " << plot_option_pop_up_state.sensors_in_Y1_list_box.at(payload_n) << "\n";

                        // Delete the sensor from the Y2 list and add it to the Y2 list
                        plot_option_pop_up_state.sensors_in_Y3_list_box.push_back(plot_option_pop_up_state.sensors_in_Y1_list_box.at(payload_n));
                        plot_option_pop_up_state.sensors_in_Y1_list_box.erase(plot_option_pop_up_state.sensors_in_Y1_list_box.begin() + payload_n);
                    }
                    ImGui::EndDragDropTarget();
                }

                // FROM Y2 SENSORS TO Y3 SENSORS
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_FROM_Y2_SENSORS")) {
                        int payload_n = *(const int*)payload->Data;
                        std::cout << "Payload received from Y2 Sensors into Y3 Sensors: " << plot_option_pop_up_state.sensors_in_Y2_list_box.at(payload_n) << "\n";

                        // Delete the sensor from the Y2 list and add it to the Y3 list
                        plot_option_pop_up_state.sensors_in_Y3_list_box.push_back(plot_option_pop_up_state.sensors_in_Y2_list_box.at(payload_n));
                        plot_option_pop_up_state.sensors_in_Y2_list_box.erase(plot_option_pop_up_state.sensors_in_Y2_list_box.begin() + payload_n);
                    }
                    ImGui::EndDragDropTarget();
                }
            }

            // Loop through existing sensors in Y3 and allow drag-and-drop
            for (size_t i = 0; i < plot_option_pop_up_state.sensors_in_Y3_list_box.size(); ++i) {
                ImGui::Selectable(plot_option_pop_up_state.sensors_in_Y3_list_box.at(i).c_str());

                ImGuiDragDropFlags src_flags = 0;
                if (ImGui::BeginDragDropSource(src_flags)) {
                    if (!(src_flags))
                        ImGui::Text("Moving \"%s\"", plot_option_pop_up_state.sensors_in_Y3_list_box.at(i).c_str());
                    ImGui::SetDragDropPayload("DND_FROM_Y3_SENSORS", &i, sizeof(int));
                    ImGui::EndDragDropSource();
                }

                // FROM Y3 SENSORS TO Y3 SENSORS
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_FROM_Y3_SENSORS")) {
                        int payload_n = *(const int*)payload->Data;
                        std::cout << "Payload received from Y3 Sensors to Y3 Sensors: " << plot_option_pop_up_state.sensors_in_Y3_list_box.at(payload_n) << "\n";

                        // Switch the positions of the sensors in the Y3 list into the current position
                        std::swap(plot_option_pop_up_state.sensors_in_Y3_list_box.at(payload_n), plot_option_pop_up_state.sensors_in_Y3_list_box.at(i));
                    }
                    ImGui::EndDragDropTarget();
                }

                // FROM AVAILABLE SENSORS TO Y3 SENSORS
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_FROM_AVAILABLE_SENSORS")) {
                        int payload_n = *(const int*)payload->Data;
                        std::cout << "Payload received from Available Sensors into Y3 Sensors: " << plot_option_pop_up_state.sensors_in_available_list_box.at(payload_n) << "\n";

                        // Delete the sensor from the available list and add it to the Y2 list
                        plot_option_pop_up_state.sensors_in_Y3_list_box.push_back(plot_option_pop_up_state.sensors_in_available_list_box.at(payload_n));
                        plot_option_pop_up_state.sensors_in_available_list_box.erase(plot_option_pop_up_state.sensors_in_available_list_box.begin() + payload_n);
                    }
                    ImGui::EndDragDropTarget();
                }

                // FROM Y1 SENSORS TO Y3 SENSORS
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_FROM_Y1_SENSORS")) {
                        int payload_n = *(const int*)payload->Data;
                        std::cout << "Payload received from Y1 Sensors into Y3 Sensors: " << plot_option_pop_up_state.sensors_in_Y1_list_box.at(payload_n) << "\n";

                        // Delete the sensor from the Y2 list and add it to the Y2 list
                        plot_option_pop_up_state.sensors_in_Y3_list_box.push_back(plot_option_pop_up_state.sensors_in_Y1_list_box.at(payload_n));
                        plot_option_pop_up_state.sensors_in_Y1_list_box.erase(plot_option_pop_up_state.sensors_in_Y1_list_box.begin() + payload_n);
                    }
                    ImGui::EndDragDropTarget();
                }

                // FROM Y2 SENSORS TO Y3 SENSORS
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_FROM_Y2_SENSORS")) {
                        int payload_n = *(const int*)payload->Data;
                        std::cout << "Payload received from Y2 Sensors into Y3 Sensors: " << plot_option_pop_up_state.sensors_in_Y2_list_box.at(payload_n) << "\n";

                        // Delete the sensor from the Y2 list and add it to the Y3 list
                        plot_option_pop_up_state.sensors_in_Y3_list_box.push_back(plot_option_pop_up_state.sensors_in_Y2_list_box.at(payload_n));
                        plot_option_pop_up_state.sensors_in_Y2_list_box.erase(plot_option_pop_up_state.sensors_in_Y2_list_box.begin() + payload_n);
                    }
                    ImGui::EndDragDropTarget();
                }
            }

            ImGui::EndListBox();
        }

        is_able_to_submit = is_able_to_submit
            && (!plot_option_pop_up_state.sensors_in_Y1_list_box.empty()
            || !plot_option_pop_up_state.sensors_in_Y2_list_box.empty()
            || !plot_option_pop_up_state.sensors_in_Y3_list_box.empty());

        // ******* Collapsibles with Y-axis properties *******
        if (ImGui::CollapsingHeader("Y1 Axis Properties")) {
            // Y1 Axis properties
            ImGui::Text("Y1 Axis Properties");

            // Input boxes for minimum and maximum values
            ImGui::InputDouble("Y1 Minimum", &plot_option_pop_up_state.Y1_properties.min);
            ImGui::SameLine();
            ImGui::InputDouble("Y1 Maximum", &plot_option_pop_up_state.Y1_properties.max);

            // Drop-down menu for scale properties
            const char* scale_options[] = { "Linear", "Logarithmic" };
            int scale_option_index = static_cast<int>(plot_option_pop_up_state.Y1_properties.scale_type);
            const char* selected_scale = scale_options[scale_option_index];
            if (ImGui::BeginCombo("Y1 Scale", selected_scale)) {
                for (int n = 0; n < IM_ARRAYSIZE(scale_options); n++) {
                    const bool is_selected = (selected_scale == scale_options[n]);
                    if (ImGui::Selectable(scale_options[n], is_selected)) {
                        selected_scale = scale_options[n];
                        plot_option_pop_up_state.Y1_properties.scale_type = static_cast<RenderablePlot::ScaleType>(n);
                    }
                    if (is_selected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            // Input box for log base if scale is logarithmic
            if (plot_option_pop_up_state.Y1_properties.scale_type == RenderablePlot::ScaleType::Logirithmic) {
                ImGui::InputDouble("Y1 Log base", &plot_option_pop_up_state.Y1_properties.log_base);
            }
        }
        ImGui::Separator();

        if (ImGui::CollapsingHeader("Y2 Axis Properties")) {
            // Y2 Axis properties
            ImGui::Text("Y2 Axis Properties");

            // Input boxes for minimum and maximum values
            ImGui::InputDouble("Y2 Minimum", &plot_option_pop_up_state.Y2_properties.min);
            ImGui::SameLine();
            ImGui::InputDouble("Y2 Maximum", &plot_option_pop_up_state.Y2_properties.max);

            // Drop-down menu for scale properties
            const char* scale_options[] = { "Linear", "Logarithmic" };
            int scale_option_index = static_cast<int>(plot_option_pop_up_state.Y2_properties.scale_type);
            const char* selected_scale = scale_options[scale_option_index];
            if (ImGui::BeginCombo("Y2 Scale", selected_scale)) {
                for (int n = 0; n < IM_ARRAYSIZE(scale_options); n++) {
                    const bool is_selected = (selected_scale == scale_options[n]);
                    if (ImGui::Selectable(scale_options[n], is_selected)) {
                        selected_scale = scale_options[n];
                        plot_option_pop_up_state.Y2_properties.scale_type = static_cast<RenderablePlot::ScaleType>(n);
                    }
                    if (is_selected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            // Input box for log base if scale is logarithmic
            if (plot_option_pop_up_state.Y2_properties.scale_type == RenderablePlot::ScaleType::Logirithmic) {
                ImGui::InputDouble("Y2 Log base", &plot_option_pop_up_state.Y2_properties.log_base);
            }
        }
        ImGui::Separator();

        if (ImGui::CollapsingHeader("Y3 Axis Properties")) {
            // Y3 Axis properties
            ImGui::Text("Y3 Axis Properties");

            // Input boxes for minimum and maximum values
            ImGui::InputDouble("Y3 Minimum", &plot_option_pop_up_state.Y3_properties.min);
            ImGui::SameLine();
            ImGui::InputDouble("Y3 Maximum", &plot_option_pop_up_state.Y3_properties.max);

            // Drop-down menu for scale properties
            const char* scale_options[] = { "Linear", "Logarithmic" };
            int scale_option_index = static_cast<int>(plot_option_pop_up_state.Y3_properties.scale_type);
            const char* selected_scale = scale_options[scale_option_index];
            if (ImGui::BeginCombo("Y3 Scale", selected_scale)) {
                for (int n = 0; n < IM_ARRAYSIZE(scale_options); n++) {
                    const bool is_selected = (selected_scale == scale_options[n]);
                    if (ImGui::Selectable(scale_options[n], is_selected)) {
                        selected_scale = scale_options[n];
                        plot_option_pop_up_state.Y3_properties.scale_type = static_cast<RenderablePlot::ScaleType>(n);
                    }
                    if (is_selected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            // Input box for log base if scale is logarithmic
            if (plot_option_pop_up_state.Y3_properties.scale_type == RenderablePlot::ScaleType::Logirithmic) {
                ImGui::InputDouble("Y3 Log base", &plot_option_pop_up_state.Y3_properties.log_base);
            }
        }
        ImGui::Separator();

        // ******* Drop-down menu of sensors and plotline properties *******
        //  Get all sensors in Y1, Y2, and Y3 list boxes
        std::vector<std::string> all_plot_sensors;
        all_plot_sensors.insert(all_plot_sensors.end(), plot_option_pop_up_state.sensors_in_Y1_list_box.begin(), plot_option_pop_up_state.sensors_in_Y1_list_box.end());
        all_plot_sensors.insert(all_plot_sensors.end(), plot_option_pop_up_state.sensors_in_Y2_list_box.begin(), plot_option_pop_up_state.sensors_in_Y2_list_box.end());
        all_plot_sensors.insert(all_plot_sensors.end(), plot_option_pop_up_state.sensors_in_Y3_list_box.begin(), plot_option_pop_up_state.sensors_in_Y3_list_box.end());

        // Drop-down menu for sensors
        std::string drop_down_label = "Select a sensor...";
        if (!plot_option_pop_up_state.plotline_properties_selected_sensor.empty()) {
            drop_down_label = plot_option_pop_up_state.plotline_properties_selected_sensor;
        }
        if (plot_option_pop_up_state.sensors_in_Y1_list_box.empty()
            && plot_option_pop_up_state.sensors_in_Y2_list_box.empty()
            && plot_option_pop_up_state.sensors_in_Y3_list_box.empty()) {
                drop_down_label = "Please allocate a sensor to a Y-axis...";
        }
        if (ImGui::BeginCombo("Available Sensors", drop_down_label.c_str(), ImGuiComboFlags_WidthFitPreview)) {
            // Loop through all the sensors inside Y1, Y2, and Y3 list boxes
            for (const std::string& sensor : all_plot_sensors) {
                bool is_selected = false;
                ImGui::Selectable(sensor.c_str(), &is_selected);
                if (is_selected) {
                    plot_option_pop_up_state.plotline_properties_selected_sensor = sensor;

                    // Check if plotline properties exists for the selected sensor. If not, create a default one
                    if (plot_option_pop_up_state.sensor_to_plotline_properties.find(plot_option_pop_up_state.plotline_properties_selected_sensor) == plot_option_pop_up_state.sensor_to_plotline_properties.end()) {
                        plot_option_pop_up_state.sensor_to_plotline_properties[plot_option_pop_up_state.plotline_properties_selected_sensor] = RenderablePlot::PlotLineProperties();
                    }
                }
            }
            ImGui::EndCombo();
        }

        // Detect if the selected sensor has been removed from all Y-axis
        if (std::find(all_plot_sensors.begin(), all_plot_sensors.end(), plot_option_pop_up_state.plotline_properties_selected_sensor) == all_plot_sensors.end()) {
            plot_option_pop_up_state.plotline_properties_selected_sensor.clear();
        }

        // Plotline properties of selected sensor
        if(!plot_option_pop_up_state.plotline_properties_selected_sensor.empty()) {
            ImGui::Text("Plotline properties of %s", plot_option_pop_up_state.plotline_properties_selected_sensor.c_str());

            // Colour picker for plotline color
            ImVec4& color = plot_option_pop_up_state.sensor_to_plotline_properties[plot_option_pop_up_state.plotline_properties_selected_sensor].colour;
            ImGui::ColorEdit4("Plotline Color", &color.x);

            // Marker drop-down for plotline marker
            const char* marker_options[] = { "None", "Circle", "Square", "Diamond", "Up", "Down", "Left", "Right", "Cross", "Plus", "Asterisk" };
            int marker_option_index = static_cast<int>(plot_option_pop_up_state.sensor_to_plotline_properties[plot_option_pop_up_state.plotline_properties_selected_sensor].marker_style) + 1;
            const char* selected_marker = marker_options[marker_option_index];
            if (ImGui::BeginCombo("Plotline Marker", selected_marker)) {
                for (int n = 0; n < IM_ARRAYSIZE(marker_options); n++) {
                    const bool is_selected = (selected_marker == marker_options[n]);
                    if (ImGui::Selectable(marker_options[n], is_selected)) {
                        selected_marker = marker_options[n];
                        plot_option_pop_up_state.sensor_to_plotline_properties[plot_option_pop_up_state.plotline_properties_selected_sensor].marker_style = static_cast<ImPlotMarker>(n) - 1;
                    }
                    if (is_selected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            // Marker options if marker is not "None"
            if (plot_option_pop_up_state.sensor_to_plotline_properties[plot_option_pop_up_state.plotline_properties_selected_sensor].marker_style != ImPlotMarker_None) {
                // Marker size input box
                float marker_size = static_cast<float>(plot_option_pop_up_state.sensor_to_plotline_properties[plot_option_pop_up_state.plotline_properties_selected_sensor].marker_size);
                if (ImGui::InputFloat("Marker Size", &marker_size)) {
                    plot_option_pop_up_state.sensor_to_plotline_properties[plot_option_pop_up_state.plotline_properties_selected_sensor].marker_size = static_cast<double>(marker_size);
                }

                // Marker fill color
                ImVec4& marker_fill_color = plot_option_pop_up_state.sensor_to_plotline_properties[plot_option_pop_up_state.plotline_properties_selected_sensor].fill;
                ImGui::ColorEdit4("Marker Fill Color", &marker_fill_color.x);

                // Marker weight
                float marker_weight = static_cast<float>(plot_option_pop_up_state.sensor_to_plotline_properties[plot_option_pop_up_state.plotline_properties_selected_sensor].fill_weight);
                if (ImGui::InputFloat("Marker Weight", &marker_weight)) {
                    plot_option_pop_up_state.sensor_to_plotline_properties[plot_option_pop_up_state.plotline_properties_selected_sensor].fill_weight = static_cast<double>(marker_weight);
                }

                // Marker outline color
                ImVec4& marker_outline_color = plot_option_pop_up_state.sensor_to_plotline_properties[plot_option_pop_up_state.plotline_properties_selected_sensor].fill_outline;
                ImGui::ColorEdit4("Marker Outline Color", &marker_outline_color.x);
            }
        }

        ImGui::Separator();

        if (is_able_to_submit) {
            if (ImGui::Button("Submit")) {
                // Close the popup
                ImGui::CloseCurrentPopup();

                // Execute submit action
                ActionSubmitPlotOptionsPopup(renderable_plot, plot_option_pop_up_state);

                // Reset the state of the popup
                plot_option_pop_up_state.reset();
            }
        } else {
            ImGui::Text("Please ensure at least one sensors is allocated to a Y-axis.");
        }

        if (ImGui::Button("Close")) {
            ImGui::CloseCurrentPopup();

            // Reset the state of the popup
            plot_option_pop_up_state.reset();
        }
        ImGui::EndPopup();
    }
}

// Render the add plot popup for a specific window
void GraphView::renderWindowPlotAddPlotPopup(WindowPlots *window){
    std::string popup_label = "Add new plot to window \"" + window->getLabel() + "\"";
    // Example of creating a popup window with textbox input fields
    if (ImGui::Button("+")) {
        ImGui::OpenPopup(popup_label.c_str());
        auto& window_plot_add_plot_popup_state = viewModel_.getWindowPlotAddPlotPopupState();

        // Reset the state of the popup
        window_plot_add_plot_popup_state.reset();

        // Set the available sensors from the view model
        window_plot_add_plot_popup_state.all_sensors = viewModel_.getPlottableSensors(); // For ordering purposes
        window_plot_add_plot_popup_state.available_sensors = viewModel_.getPlottableSensors();
    }

    // Measure the width of the title text
    ImVec2 title_size = ImGui::CalcTextSize(popup_label.c_str());
    // Set minimum size constraints for the popup modal based on the title text width
    ImGui::SetNextWindowSizeConstraints(ImVec2(title_size.x + 20, 100), ImVec2(FLT_MAX, FLT_MAX));

    // Begin the popup modal
    if (ImGui::BeginPopupModal(popup_label.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        auto& window_plot_add_plot_popup_state = viewModel_.getWindowPlotAddPlotPopupState();

        bool is_able_to_submit = true;

        // ********** Input text for plot label **********
        // std::strncpy(window_plot_add_plot_popup_state.plot_label_buffer, window_plot_add_plot_popup_state.plot_label.c_str(), sizeof(window_plot_add_plot_popup_state.plot_label_buffer));
        ImGui::Text("Enter plot title");
        if (ImGui::InputText("###Plot title", window_plot_add_plot_popup_state.plot_label_buffer, sizeof(window_plot_add_plot_popup_state.plot_label_buffer))) {
            window_plot_add_plot_popup_state.plot_label = std::string(window_plot_add_plot_popup_state.plot_label_buffer); // Sync changes back to std::string
        }
        ImGui::Separator();

        // ********** List plottable sensors **********
        // Search bar for available sensors
        ImGui::InputText("Search", window_plot_add_plot_popup_state.search_available_sensors_buffer,
            sizeof(window_plot_add_plot_popup_state.search_available_sensors_buffer));
        std::vector<std::string>& sensors = window_plot_add_plot_popup_state.available_sensors;
        int& selected_sensor = window_plot_add_plot_popup_state.selected_sensor_in_available_list_box;

        ImGui::Text("Select sensors to plot");
        if (ImGui::BeginListBox("##Sensors")) {
            for (size_t i = 0; i < sensors.size(); ++i) {
                // Check if the sensor matches the search text
                if (!IsSearchBarMatch(window_plot_add_plot_popup_state.search_available_sensors_buffer,
                    window_plot_add_plot_popup_state.available_sensors.at(i)))
                    continue;

                const bool is_selected = (selected_sensor == static_cast<int>(i));
                if (ImGui::Selectable(sensors[i].c_str(), is_selected)) {
                    // Remove the selected sensor from the list and move them into the selected sensors list
                    selected_sensor = static_cast<int>(i);
                    window_plot_add_plot_popup_state.selected_sensors.push_back(sensors.at(selected_sensor));
                    sensors.erase(sensors.begin() + selected_sensor);
                }
            }
            ImGui::EndListBox();
        }

        if (ImGui::BeginListBox("##WindowPlotSelectedSensors")) {
            int& selected_sensor_in_selected = window_plot_add_plot_popup_state.selected_sensor_in_selected_list_box;
            for (size_t i = 0; i < window_plot_add_plot_popup_state.selected_sensors.size(); ++i) {
                const bool is_selected = (selected_sensor_in_selected == static_cast<int>(i));
                if (ImGui::Selectable(window_plot_add_plot_popup_state.selected_sensors[i].c_str(), is_selected)) {
                    selected_sensor_in_selected = static_cast<int>(i);
                }
            }
            ImGui::EndListBox();

            // Move selected sensor back to "available sensors" list
            if (selected_sensor_in_selected != -1 && ImGui::Button("Deselect Sensor")) {
                const std::string& sensor_to_return = window_plot_add_plot_popup_state.selected_sensors[selected_sensor_in_selected];

                // Find position in all_sensors only if it is non-empty
                if (!window_plot_add_plot_popup_state.all_sensors.empty()) {
                    auto it = std::find(
                        window_plot_add_plot_popup_state.all_sensors.begin(),
                        window_plot_add_plot_popup_state.all_sensors.end(),
                        sensor_to_return
                    );

                    // Check if found within bounds
                    if (it != window_plot_add_plot_popup_state.all_sensors.end()) {
                        auto index = std::distance(window_plot_add_plot_popup_state.all_sensors.begin(), it);

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
                    sensors.push_back(sensor_to_return); // Append if all_sensors is empty
                }

                // Remove the sensor from selected_sensors
                if (selected_sensor_in_selected >= 0 && selected_sensor_in_selected < static_cast<int>(window_plot_add_plot_popup_state.selected_sensors.size())) {
                    window_plot_add_plot_popup_state.selected_sensors.erase(window_plot_add_plot_popup_state.selected_sensors.begin() + selected_sensor_in_selected);
                    selected_sensor_in_selected = -1; // Reset selection
                }
            }
        }

        ImGui::SeparatorText("");

        // ********** Real-time/User-set-time logic **********
                // Real-time plotting checkbox
        ImGui::Checkbox("Real-time", &window_plot_add_plot_popup_state.is_real_time);
        is_able_to_submit = window_plot_add_plot_popup_state.is_real_time;

        // Real-time plot range
        if(window_plot_add_plot_popup_state.is_real_time && !window_plot_add_plot_popup_state.is_range_initialized) {
            // Initialize plot range for Brisbane time
            SetPlotRangeState(window_plot_add_plot_popup_state);
        }

        // User defined plot range
        if(!window_plot_add_plot_popup_state.is_real_time) {
            if (!window_plot_add_plot_popup_state.is_range_initialized) {
                // Initialize textboxes with default values only once
                SetPlotRangeState(window_plot_add_plot_popup_state);
                window_plot_add_plot_popup_state.is_range_initialized = true;
            }

            ImGui::Text("Enter plot range (YYYY-MM-DD HH:MM:SS)");

            renderDateTimeField("Plot start date", window_plot_add_plot_popup_state.plot_range_start_year,
                window_plot_add_plot_popup_state.plot_range_start_month,
                window_plot_add_plot_popup_state.plot_range_start_day,
                window_plot_add_plot_popup_state.plot_range_start_hour,
                window_plot_add_plot_popup_state.plot_range_start_minute,
                window_plot_add_plot_popup_state.plot_range_start_second);
            renderDateTimeField("Plot end date", window_plot_add_plot_popup_state.plot_range_end_year,
                window_plot_add_plot_popup_state.plot_range_end_month,
                window_plot_add_plot_popup_state.plot_range_end_day,
                window_plot_add_plot_popup_state.plot_range_end_hour,
                window_plot_add_plot_popup_state.plot_range_end_minute,
                window_plot_add_plot_popup_state.plot_range_end_second
                );

            if (std::strlen(window_plot_add_plot_popup_state.plot_range_start_year) == 0 ||
                std::strlen(window_plot_add_plot_popup_state.plot_range_start_month) == 0 ||
                std::strlen(window_plot_add_plot_popup_state.plot_range_start_day) == 0 ||
                std::strlen(window_plot_add_plot_popup_state.plot_range_start_hour) == 0 ||
                std::strlen(window_plot_add_plot_popup_state.plot_range_start_minute) == 0 ||
                std::strlen(window_plot_add_plot_popup_state.plot_range_start_second) == 0 ||
                std::strlen(window_plot_add_plot_popup_state.plot_range_end_year) == 0 ||
                std::strlen(window_plot_add_plot_popup_state.plot_range_end_month) == 0 ||
                std::strlen(window_plot_add_plot_popup_state.plot_range_end_day) == 0 ||
                std::strlen(window_plot_add_plot_popup_state.plot_range_end_hour) == 0 ||
                std::strlen(window_plot_add_plot_popup_state.plot_range_end_minute) == 0 ||
                std::strlen(window_plot_add_plot_popup_state.plot_range_end_second) == 0) {
                ImGui::Text("Please enter all fields");
                is_able_to_submit = false;
            } else {
                is_able_to_submit = true;
            }

            // Ensure end time is after start time
            // Convert input into unix time
            try {
                window_plot_add_plot_popup_state.input_start_time.tm_year = std::stoi(window_plot_add_plot_popup_state.plot_range_start_year) - 1900;
                window_plot_add_plot_popup_state.input_start_time.tm_mon = std::stoi(window_plot_add_plot_popup_state.plot_range_start_month) - 1;
                window_plot_add_plot_popup_state.input_start_time.tm_mday = std::stoi(window_plot_add_plot_popup_state.plot_range_start_day);
                window_plot_add_plot_popup_state.input_start_time.tm_hour = std::stoi(window_plot_add_plot_popup_state.plot_range_start_hour);
                window_plot_add_plot_popup_state.input_start_time.tm_min = std::stoi(window_plot_add_plot_popup_state.plot_range_start_minute);
                window_plot_add_plot_popup_state.input_start_time.tm_sec = std::stoi(window_plot_add_plot_popup_state.plot_range_start_second);
                std::time_t start_unix_time = std::mktime(&window_plot_add_plot_popup_state.input_start_time);

                window_plot_add_plot_popup_state.input_end_time.tm_year = std::stoi(window_plot_add_plot_popup_state.plot_range_end_year) - 1900;
                window_plot_add_plot_popup_state.input_end_time.tm_mon = std::stoi(window_plot_add_plot_popup_state.plot_range_end_month) - 1;
                window_plot_add_plot_popup_state.input_end_time.tm_mday = std::stoi(window_plot_add_plot_popup_state.plot_range_end_day);
                window_plot_add_plot_popup_state.input_end_time.tm_hour = std::stoi(window_plot_add_plot_popup_state.plot_range_end_hour);
                window_plot_add_plot_popup_state.input_end_time.tm_min = std::stoi(window_plot_add_plot_popup_state.plot_range_end_minute);
                window_plot_add_plot_popup_state.input_end_time.tm_sec = std::stoi(window_plot_add_plot_popup_state.plot_range_end_second);
                std::time_t end_unix_time = std::mktime(&window_plot_add_plot_popup_state.input_end_time);

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

        // ********** "Close" button logic **********
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();

            // Reset the state of the popup
            window_plot_add_plot_popup_state.reset();
        }

        // ********** "Submit" button logic **********
        if(is_able_to_submit) {
            if (ImGui::Button("Submit")) {
                // Handle submit action
                ActionSubmitWindowPlotAddPlotPopup(window_plot_add_plot_popup_state, *window);

                window_plot_add_plot_popup_state.reset();
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();
        }

        ImGui::EndPopup();
    }

}

// Render all plots in their respective windows
void GraphView::renderAllWindowPlots(){
    // Loop through all the windows inside the view model
    for (const std::string& window_label : viewModel_.getWindowPlotLabels()) {
        // Set up the window
        WindowPlots& window = viewModel_.getWindowPlot(window_label);
        ImGui::SetNextWindowSize(ImVec2(1080, 600), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_FirstUseEver);
        ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;

        // Render the window
        if (ImGui::Begin(window_label.c_str(), nullptr, flags)) {
            // Menu bar for the window
            renderWindowMenuBar(&window);

            // Loop through all the plots inside the window and render them
            renderAllPlotsInWindow(&window);

            // Add plot button
            renderWindowPlotAddPlotPopup(&window);
        }

        ImGui::End();

    }
}




// ==============================
// renderAll
// ==============================

void GraphView::renderAll() {
    // Render "Add plot" button and popup
    renderAddPlotPopup();

    // Render all window plots
    renderAllWindowPlots();
}
