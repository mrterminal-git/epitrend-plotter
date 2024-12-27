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

// Render the "Add plot" popup
void GraphView::renderAddPlotPopup() {
    // Example of creating a popup window with textbox input fields
    if (ImGui::Button("Add new plot")) {
        ImGui::OpenPopup("Add new plot");
    }

    if (ImGui::BeginPopup("Add new plot")) {
        static char window_label_input[128] = "";
        static char plot_label_input[128] = "";
        static bool is_real_time = false;
        static bool is_able_to_submit = false;

        // Get current time for default plot range
        static std::time_t current_time = std::time(nullptr);
        static std::time_t max_data_time_range = 3600; // 1 hour
        static std::tm local_time;
        localtime_s(&local_time, &current_time);

        // Get current year
        ImGui::Text("Enter values:");
        ImGui::InputText("Window name", window_label_input, IM_ARRAYSIZE(window_label_input));
        ImGui::InputText("Plot title", plot_label_input, IM_ARRAYSIZE(plot_label_input));

        // List plottable sensors
        const auto& sensors = viewModel_.getPlottableSensors();
        static int selected_sensor = -1;

        ImGui::Text("Select a sensor:");
        if (ImGui::BeginListBox("##Sensors")) {
            for (size_t i = 0; i < sensors.size(); ++i) {
                const bool is_selected = (selected_sensor == static_cast<int>(i));
                if (ImGui::Selectable(sensors[i].c_str(), is_selected)) {
                    selected_sensor = static_cast<int>(i);
                }
                if (is_selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndListBox();
        }

        // Real-time plotting checkbox
        ImGui::Checkbox("Real-time", &is_real_time);
        is_able_to_submit = is_real_time;

        // User defined plot range
        if(!is_real_time){
            static char plot_range_start_year[5] = "";
            static char plot_range_start_month[3] = "";
            static char plot_range_start_day[3] = "";
            static char plot_range_start_hour[3] = "";
            static char plot_range_start_minute[3] = "";
            static char plot_range_start_second[3] = "";

            static char plot_range_end_year[5] = "";
            static char plot_range_end_month[3] = "";
            static char plot_range_end_day[3] = "";
            static char plot_range_end_hour[3] = "";
            static char plot_range_end_minute[3] = "";
            static char plot_range_end_second[3] = "";

            static bool is_range_initialized = false; // Initialization flag

            // Initialize textboxes with default values only once
            if (!is_range_initialized) {
                static std::time_t start_time = current_time - max_data_time_range;
                static std::tm start_local_time;
                localtime_s(&start_local_time, &start_time);

                std::strftime(plot_range_start_year, sizeof(plot_range_start_year), "%Y", &start_local_time);
                std::strftime(plot_range_start_month, sizeof(plot_range_start_month), "%m", &start_local_time);
                std::strftime(plot_range_start_day, sizeof(plot_range_start_day), "%d", &start_local_time);
                std::strftime(plot_range_start_hour, sizeof(plot_range_start_hour), "%H", &start_local_time);
                std::strftime(plot_range_start_minute, sizeof(plot_range_start_minute), "%M", &start_local_time);
                std::strftime(plot_range_start_second, sizeof(plot_range_start_second), "%S", &start_local_time);

                std::strftime(plot_range_end_year, sizeof(plot_range_end_year), "%Y", &local_time);
                std::strftime(plot_range_end_month, sizeof(plot_range_end_month), "%m", &local_time);
                std::strftime(plot_range_end_day, sizeof(plot_range_end_day), "%d", &local_time);
                std::strftime(plot_range_end_hour, sizeof(plot_range_end_hour), "%H", &local_time);
                std::strftime(plot_range_end_minute, sizeof(plot_range_end_minute), "%M", &local_time);
                std::strftime(plot_range_end_second, sizeof(plot_range_end_second), "%S", &local_time);

                is_range_initialized = true;
            }

            ImGui::SeparatorText("");

            ImGui::Text("Enter plot range (YYYY-MM-DD HH:MM:SS)");

            renderDateTimeField("Plot start date", plot_range_start_year, plot_range_start_month, plot_range_start_day, plot_range_start_hour, plot_range_start_minute, plot_range_start_second);
            renderDateTimeField("Plot end date", plot_range_end_year, plot_range_end_month, plot_range_end_day, plot_range_end_hour, plot_range_end_minute, plot_range_end_second);

            if (std::strlen(plot_range_start_year) == 0 ||
                std::strlen(plot_range_start_month) == 0 ||
                std::strlen(plot_range_start_day) == 0 ||
                std::strlen(plot_range_start_hour) == 0 ||
                std::strlen(plot_range_start_minute) == 0 ||
                std::strlen(plot_range_start_second) == 0 ||
                std::strlen(plot_range_end_year) == 0 ||
                std::strlen(plot_range_end_month) == 0 ||
                std::strlen(plot_range_end_day) == 0 ||
                std::strlen(plot_range_end_hour) == 0 ||
                std::strlen(plot_range_end_minute) == 0 ||
                std::strlen(plot_range_end_second) == 0) {
                ImGui::Text("Please enter all fields");
                is_able_to_submit = false;
            } else {
                is_able_to_submit = true;
            }

            // Ensure end time is after start time
            // Convert input into unix time
            try {
                static std::tm input_start_time = {};
                input_start_time.tm_year = std::stoi(plot_range_start_year) - 1900;
                input_start_time.tm_mon = std::stoi(plot_range_start_month) - 1;
                input_start_time.tm_mday = std::stoi(plot_range_start_day);
                input_start_time.tm_hour = std::stoi(plot_range_start_hour);
                input_start_time.tm_min = std::stoi(plot_range_start_minute);
                input_start_time.tm_sec = std::stoi(plot_range_start_second);
                std::time_t start_unix_time = std::mktime(&input_start_time);

                std::tm input_end_time = {};
                input_end_time.tm_year = std::stoi(plot_range_end_year) - 1900;
                input_end_time.tm_mon = std::stoi(plot_range_end_month) - 1;
                input_end_time.tm_mday = std::stoi(plot_range_end_day);
                input_end_time.tm_hour = std::stoi(plot_range_end_hour);
                input_end_time.tm_min = std::stoi(plot_range_end_minute);
                input_end_time.tm_sec = std::stoi(plot_range_end_second);
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
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();
        }

        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

// Handle submit action for "Add plot" popup
void actionSubmitAddPlotPopup() {
    // Handle submit action
}

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
