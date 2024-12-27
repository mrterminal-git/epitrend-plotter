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

void GraphView::renderAll() {
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
        static std::tm* local_time = std::localtime(&current_time);

        // Get current year
        ImGui::Text("Enter values:");
        ImGui::InputText("Window name", window_label_input, IM_ARRAYSIZE(window_label_input));
        ImGui::InputText("Plot title", plot_label_input, IM_ARRAYSIZE(plot_label_input));
        ImGui::Checkbox("Real-time", &is_real_time);
        is_able_to_submit = is_real_time;
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

            // Fill the end time textboxes with the current time
            std::strftime(plot_range_end_year, sizeof(plot_range_end_year), "%Y", local_time);
            std::strftime(plot_range_end_month, sizeof(plot_range_end_month), "%m", local_time);
            std::strftime(plot_range_end_day, sizeof(plot_range_end_day), "%d", local_time);
            std::strftime(plot_range_end_hour, sizeof(plot_range_end_hour), "%H", local_time);
            std::strftime(plot_range_end_minute, sizeof(plot_range_end_minute), "%M", local_time);
            std::strftime(plot_range_end_second, sizeof(plot_range_end_second), "%S", local_time);

            // Fill the start time textboxes with the current time minus the default range
            // Calculate start time (current time minus default range)
            static std::time_t start_time = current_time - max_data_time_range; // 1 hour default range
            static std::tm* start_local_time = std::localtime(&start_time);
            std::strftime(plot_range_start_year, sizeof(plot_range_start_year), "%Y", start_local_time);
            std::strftime(plot_range_start_month, sizeof(plot_range_start_month), "%m", start_local_time);
            std::strftime(plot_range_start_day, sizeof(plot_range_start_day), "%d", start_local_time);
            std::strftime(plot_range_start_hour, sizeof(plot_range_start_hour), "%H", start_local_time);
            std::strftime(plot_range_start_minute, sizeof(plot_range_start_minute), "%M", start_local_time);
            std::strftime(plot_range_start_second, sizeof(plot_range_start_second), "%S", start_local_time);

            ImGui::SeparatorText("");

            ImGui::Text("Enter plot range (YYYY-MM-DD HH:MM:SS)");
            ImGui::Text("Start range");

            ImGui::SetNextItemWidth(50.0f); // Set width for the next item
            ImGui::InputText("###Start year", plot_range_start_year, IM_ARRAYSIZE(plot_range_start_year));
            ImGui::SameLine();
            ImGui::Text("-");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(30.0f); // Set width for the next item
            ImGui::InputText("###Start month", plot_range_start_month, IM_ARRAYSIZE(plot_range_start_month));
            ImGui::SameLine();
            ImGui::Text("-");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(30.0f); // Set width for the next item
            ImGui::InputText("###Start day", plot_range_start_day, IM_ARRAYSIZE(plot_range_start_day));
            ImGui::SameLine();
            ImGui::Text(" ");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(30.0f); // Set width for the next item
            ImGui::InputText("###Start hour", plot_range_start_hour, IM_ARRAYSIZE(plot_range_start_hour));
            ImGui::SameLine();
            ImGui::Text(":");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(30.0f); // Set width for the next item
            ImGui::InputText("###Start minute", plot_range_start_minute, IM_ARRAYSIZE(plot_range_start_minute));
            ImGui::SameLine();
            ImGui::Text(":");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(30.0f); // Set width for the next item
            ImGui::InputText("###Start second", plot_range_start_second, IM_ARRAYSIZE(plot_range_start_second));

            ImGui::Text("End range");
           ImGui::SetNextItemWidth(50.0f); // Set width for the next item
            ImGui::InputText("###End year", plot_range_end_year, IM_ARRAYSIZE(plot_range_end_year));
            ImGui::SameLine();
            ImGui::Text("-");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(30.0f); // Set width for the next item
            ImGui::InputText("###End month", plot_range_end_month, IM_ARRAYSIZE(plot_range_end_month));
            ImGui::SameLine();
            ImGui::Text("-");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(30.0f); // Set width for the next item
            ImGui::InputText("###End day", plot_range_end_day, IM_ARRAYSIZE(plot_range_end_day));
            ImGui::SameLine();
            ImGui::Text(" ");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(30.0f); // Set width for the next item
            ImGui::InputText("###End hour", plot_range_end_hour, IM_ARRAYSIZE(plot_range_end_hour));
            ImGui::SameLine();
            ImGui::Text(":");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(30.0f); // Set width for the next item
            ImGui::InputText("###End minute", plot_range_end_minute, IM_ARRAYSIZE(plot_range_end_minute));
            ImGui::SameLine();
            ImGui::Text(":");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(30.0f); // Set width for the next item
            ImGui::InputText("###End second", plot_range_end_second, IM_ARRAYSIZE(plot_range_end_second));

            // Ensure all fields are filled
            if(plot_range_start_year[0] == '\0' || plot_range_start_month[0] == '\0' || plot_range_start_day[0] == '\0' || plot_range_start_hour[0] == '\0' || plot_range_start_minute[0] == '\0' || plot_range_start_second[0] == '\0' || plot_range_end_year[0] == '\0' || plot_range_end_month[0] == '\0' || plot_range_end_day[0] == '\0' || plot_range_end_hour[0] == '\0' || plot_range_end_minute[0] == '\0' || plot_range_end_second[0] == '\0'){
                ImGui::Text("Please enter all fields");
                is_able_to_submit = false;
            }

            // Ensure end time is after start time
            // // Convert input into unix time
            // static std::tm start_time = {};
            // start_time.tm_year = std::stoi(plot_range_start_year) - 1900;
            // start_time.tm_mon = std::stoi(plot_range_start_month) - 1;
            // start_time.tm_mday = std::stoi(plot_range_start_day);
            // start_time.tm_hour = std::stoi(plot_range_start_hour);
            // start_time.tm_min = std::stoi(plot_range_start_minute);
            // start_time.tm_sec = std::stoi(plot_range_start_second);
            // std::time_t start_unix_time = std::mktime(&start_time);

            // std::tm end_time = {};
            // end_time.tm_year = std::stoi(plot_range_end_year) - 1900;
            // end_time.tm_mon = std::stoi(plot_range_end_month) - 1;
            // end_time.tm_mday = std::stoi(plot_range_end_day);
            // end_time.tm_hour = std::stoi(plot_range_end_hour);
            // end_time.tm_min = std::stoi(plot_range_end_minute);
            // end_time.tm_sec = std::stoi(plot_range_end_second);
            // std::time_t end_unix_time = std::mktime(&end_time);

            // if (end_unix_time < start_unix_time) {
            //     ImGui::Text("End time must be after start time");
            // } else {
            //     // Create the plot
            //     if (ImGui::Button("Submit")) {
            //         // Handle submit action
            //         ImGui::CloseCurrentPopup();
            //     }

            //     ImGui::SameLine();

            //     if (ImGui::Button("Cancel")) {
            //         ImGui::CloseCurrentPopup();
            //     }
            // }


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
