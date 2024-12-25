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

void GraphView::setRangeCallback(const RangeCallback& callback) {
    range_callback_ = callback;
}

void GraphView::Draw(std::string_view label, const std::vector<std::pair<Timestamp, Value>> &data)
{
    constexpr static auto window_flags =
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;
    constexpr static auto window_size = ImVec2(1280.0F, 720.0F);
    constexpr static auto window_pos = ImVec2(0.0F, 0.0F);

    ImGui::SetNextWindowSize(window_size);
    ImGui::SetNextWindowPos(window_pos);

    ImGui::Begin(label.data(), nullptr, window_flags);

    TestingPlot2(data);

    ImGui::End();
}

void GraphView::Draw3(std::string_view label, const std::unordered_map<std::string, std::pair<std::vector<double>, std::vector<double>>> &decoupled_data)
{
    constexpr static auto window_flags =
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;
    constexpr static auto window_size = ImVec2(1280.0F, 720.0F);
    constexpr static auto window_pos = ImVec2(0.0F, 0.0F);

    ImGui::SetNextWindowSize(window_size);
    ImGui::SetNextWindowPos(window_pos);

    ImGui::Begin(label.data(), nullptr, window_flags);

    TestingPlot3("Sub-window with plot 1", decoupled_data);
    TestingPlot3("Sub-window with plot 2", decoupled_data);

    ImGui::End();
}

void GraphView::TestingPlot() {
    static std::vector<double> t, y;
    static const time_t plot_start_time = std::time(nullptr);
    static const time_t max_time_range = 100;
    const time_t current_time = std::time(nullptr);

    // Create a sin-wave as time progresses (once every second)
    if (t.empty()) {
        t.push_back(current_time);
        y.push_back(std::sin(current_time / 10.0));
    }

    if (current_time != t.back()) {
        t.push_back(current_time);
        y.push_back(std::sin(current_time / 10.0));
    }

    // Remove old data
    if (!t.empty() && t.front() - t.back() > max_time_range) {
        t.erase(t.begin());
        y.erase(y.begin());
    }

    ImGui::BulletText("Current plot x-axis lower limit: %ld", current_time - max_time_range);
    ImGui::BulletText("Current plot x-axis upper limit: %ld", current_time);

    ImGui::BulletText("Current size of t: %ld", t.size());

    static bool set_axis_to_current_time = true;
    ImGui::Checkbox("Real-time",&set_axis_to_current_time);

    if (ImPlot::BeginPlot("My Plot", "t", "y")) {
        ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
        if (set_axis_to_current_time) {
            ImPlot::SetupAxisLimits(ImAxis_X1, current_time - max_time_range, current_time, ImGuiCond_Always);
        }
        else {
            ImPlot::SetupAxisLimits(ImAxis_X1, current_time - max_time_range, current_time);
        }
        ImPlot::PlotLine("sin(x)", t.data(), y.data(), t.size());

        // Display the current plot limits
        ImGui::BulletText("X-axis range: [%ld, %ld]", (int)(ImPlot::GetPlotLimits().X.Min), (int)(ImPlot::GetPlotLimits().X.Max));
        ImGui::BulletText("Y-axis range: [%ld, %ld]", (int)(ImPlot::GetPlotLimits().Y.Min), (int)(ImPlot::GetPlotLimits().Y.Max));

        ImPlot::EndPlot();
    }
}

void GraphView::TestingPlot2(const std::vector<std::pair<Timestamp,Value>> &data) {
    static const time_t real_time_max_time_range = 100;
    const time_t current_time = std::time(nullptr);

    // Get the t- and y- vectors from the data
    std::vector<double> t, y;
    for (const auto &point : data) {
        t.push_back(point.first);
        y.push_back(point.second);
    }


    ImGui::BulletText("Current plot x-axis lower limit: %ld", current_time - real_time_max_time_range);
    ImGui::BulletText("Current plot x-axis upper limit: %ld", current_time);

    ImGui::BulletText("Current size of t: %ld", t.size());

    static bool set_axis_to_current_time = true;
    ImGui::Checkbox("Real-time",&set_axis_to_current_time);

    if (ImPlot::BeginPlot("My Plot", "t", "y")) {
        ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
        if (set_axis_to_current_time) {
            ImPlot::SetupAxisLimits(ImAxis_X1, current_time - real_time_max_time_range, current_time, ImGuiCond_Always);
        }
        else {
            ImPlot::SetupAxisLimits(ImAxis_X1, current_time - real_time_max_time_range, current_time);
        }
        ImPlot::PlotLine("sin(x)", t.data(), y.data(), t.size());

        // Display the current plot limits
        ImGui::BulletText("X-axis range: [%ld, %ld]", (int)(ImPlot::GetPlotLimits().X.Min), (int)(ImPlot::GetPlotLimits().X.Max));
        ImGui::BulletText("Y-axis range: [%ld, %ld]", (int)(ImPlot::GetPlotLimits().Y.Min), (int)(ImPlot::GetPlotLimits().Y.Max));

        // Set the range callback
        if (range_callback_) {
            range_callback_(ImPlot::GetPlotLimits().X.Min, ImPlot::GetPlotLimits().X.Max);
        }
        ImPlot::EndPlot();
    }
}

void GraphView::TestingPlot3(const std::string sub_window_label,
    const std::unordered_map<std::string, std::pair<std::vector<double>, std::vector<double>>> &decoupled_data) {
    // Create a movable sub-window
    ImGui::Begin(sub_window_label.c_str());

    static const time_t real_time_max_time_range = 100;
    const time_t current_time = std::time(nullptr);

    ImGui::BulletText("Current plot x-axis lower limit: %ld", current_time - real_time_max_time_range);
    ImGui::BulletText("Current plot x-axis upper limit: %ld", current_time);

    for (const auto &sensor : decoupled_data) {
        ImGui::BulletText("Current size of t for sensor %s: %ld", sensor.first.c_str(), sensor.second.first.size());
    }

    static bool set_axis_to_current_time = true;
    ImGui::Checkbox("Real-time",&set_axis_to_current_time);

    if (ImPlot::BeginPlot("My Plot", "t", "y")) {
        ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
        if (set_axis_to_current_time) {
            ImPlot::SetupAxisLimits(ImAxis_X1, current_time - real_time_max_time_range, current_time, ImGuiCond_Always);
        }
        else {
            ImPlot::SetupAxisLimits(ImAxis_X1, current_time - real_time_max_time_range, current_time);
        }

        // Plot the data for each sensor
        for (const auto &sensor : decoupled_data) {
            ImPlot::PlotLine(fmt::format("{}", sensor.first).c_str(), sensor.second.first.data(), sensor.second.second.data(), sensor.second.first.size());
        }

        // Display the current plot limits
        ImGui::BulletText("X-axis range: [%ld, %ld]", (int)(ImPlot::GetPlotLimits().X.Min), (int)(ImPlot::GetPlotLimits().X.Max));
        ImGui::BulletText("Y-axis range: [%ld, %ld]", (int)(ImPlot::GetPlotLimits().Y.Min), (int)(ImPlot::GetPlotLimits().Y.Max));

        // Set the range callback
        if (range_callback_) {
            range_callback_(ImPlot::GetPlotLimits().X.Min, ImPlot::GetPlotLimits().X.Max);
        }
        ImPlot::EndPlot();
    }

    ImGui::End(); // Close the movable window

}
