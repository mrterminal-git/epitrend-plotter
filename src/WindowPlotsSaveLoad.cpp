#include "WindowPlotsSaveLoad.hpp"
#include <fstream>
#include <iostream>

// ==============================
// Ensure that the CURRENT_VERSION is updated whenever
// the serialization format changes i.e. when the WindowPlots
// class members change
// ==============================
const int CURRENT_VERSION = 1;

nlohmann::json WindowPlotsSaveLoad::serialize(const WindowPlots& windowPlots) {
    nlohmann::json j;
    j["type"] = "WindowPlots"; // Add type identifier
    j["version"] = CURRENT_VERSION; // Add version identifier
    j["label"] = windowPlots.getLabel();
    j["pos_x"] = windowPlots.getPosition().first;
    j["pos_y"] = windowPlots.getPosition().second;
    j["width"] = windowPlots.getSize().first;
    j["height"] = windowPlots.getSize().second;
    j["renderable_plots"] = nlohmann::json::array();
    for (const auto& [label, plot] : windowPlots.getRenderablePlots()) {
        j["renderable_plots"].push_back({{"label", label}, {"plot", serialize(*plot)}});
    }
    return j;
}

WindowPlots WindowPlotsSaveLoad::deserialize(const nlohmann::json& j) {
    if(CURRENT_VERSION == 1) {
        return deserializeV1(j);
    } else {
        throw std::runtime_error("Unsupported version: " + std::to_string(j.at("version").get<int>()));
    }
}

nlohmann::json WindowPlotsSaveLoad::serialize(const RenderablePlot& renderablePlot) {
    nlohmann::json j;
    j["type"] = "RenderablePlot";
    j["label"] = renderablePlot.getLabel();
    j["window_label"] = renderablePlot.getWindowLabel();
    j["plot_range"] = {renderablePlot.getPlotRange().first, renderablePlot.getPlotRange().second};
    j["real_time"] = renderablePlot.isRealTime();

    // Serialize Y axis labels
    nlohmann::json y_axis_labels_mappings;
    // Loop through YAxis_Y1, YAxis_Y2, and YAxis_Y3 to serialize the labels
    for (const auto implot_y_axis : {ImAxis_Y1, ImAxis_Y2, ImAxis_Y3}) {
        const auto label = renderablePlot.getYAxisLabel(implot_y_axis);
        if (!label.empty()) {
            if (implot_y_axis == ImAxis_Y1) {
                y_axis_labels_mappings["Y1"] = label;
            } else if (implot_y_axis == ImAxis_Y2) {
                y_axis_labels_mappings["Y2"] = label;
            } else if (implot_y_axis == ImAxis_Y3) {
                y_axis_labels_mappings["Y3"] = label;
            }
        }
    }
    j["y_axis_labels"] = y_axis_labels_mappings;

    // Serialize Y axis properties
    nlohmann::json y_axis_properties_mappings;
    for (const auto implot_y_axis : {ImAxis_Y1, ImAxis_Y2, ImAxis_Y3}) {
        RenderablePlot::Value min = renderablePlot.getYAxisPropertiesMin(implot_y_axis);
        RenderablePlot::Value max = renderablePlot.getYAxisPropertiesMax(implot_y_axis);
        RenderablePlot::ScaleType scale_type = renderablePlot.getYAxisPropertiesScaleType(implot_y_axis);
        double log_base = renderablePlot.getYAxisPropertiesLogBase(implot_y_axis);
        bool user_set_range = renderablePlot.getYAxisPropertiesUserSetRange(implot_y_axis);

        if (implot_y_axis == ImAxis_Y1) {
            y_axis_properties_mappings["Y1"] =
            {   {"min", min},
                {"max", max},
                {"scale_type", scale_type},
                {"log_base", log_base},
                {"user_set_range", user_set_range}};
        } else if (implot_y_axis == ImAxis_Y2) {
            y_axis_properties_mappings["Y2"] =
            {   {"min", min},
                {"max", max},
                {"scale_type", scale_type},
                {"log_base", log_base},
                {"user_set_range", user_set_range}};
        } else if (implot_y_axis == ImAxis_Y3) {
            y_axis_properties_mappings["Y3"] =
            {   {"min", min},
                {"max", max},
                {"scale_type", scale_type},
                {"log_base", log_base},
                {"user_set_range", user_set_range}};
        }
    }
    j["y_axis_properties"] = y_axis_properties_mappings;

    // Serialize data series names for each Y axis
    nlohmann::json data_to_y_axis_mappings;
    for (const auto implot_y_axis : {ImAxis_Y1, ImAxis_Y2, ImAxis_Y3}) {
        if (implot_y_axis == ImAxis_Y1) {
            // Add data series label mappings for Y1
            data_to_y_axis_mappings["Y1"] = renderablePlot.getSensorsForYAxis(implot_y_axis);
        } else if (implot_y_axis == ImAxis_Y2) {
            data_to_y_axis_mappings["Y2"] = renderablePlot.getSensorsForYAxis(implot_y_axis);
        } else if (implot_y_axis == ImAxis_Y3) {
            data_to_y_axis_mappings["Y3"] = renderablePlot.getSensorsForYAxis(implot_y_axis);
        }
    }
    j["data_to_y_axis"] = data_to_y_axis_mappings;

    // Serialize data series properties
    nlohmann::json data_to_plotline_properties_mappings;
    for (const auto implot_y_axis : {ImAxis_Y1, ImAxis_Y2, ImAxis_Y3}) {
        for (std::string data_series_label : renderablePlot.getSensorsForYAxis(implot_y_axis)) {
            ImVec4 colour = renderablePlot.getPlotLinePropertiesColour(data_series_label);
            double thickness = renderablePlot.getPlotLinePropertiesThickness(data_series_label);
            ImPlotMarker marker_style = renderablePlot.getPlotLinePropertiesMarkerStyle(data_series_label);
            double marker_size = renderablePlot.getPlotLinePropertiesMarkerSize(data_series_label);
            ImVec4 fill = renderablePlot.getPlotLinePropertiesFill(data_series_label);
            double fill_weight = renderablePlot.getPlotLinePropertiesFillWeight(data_series_label);
            ImVec4 fill_outline = renderablePlot.getPlotLinePropertiesFillOutline(data_series_label);

            if (implot_y_axis == ImAxis_Y1) {
                data_to_plotline_properties_mappings["Y1"].push_back({
                    {"data_series_label", data_series_label},
                    {"colour", {colour.x, colour.y, colour.z, colour.w}},
                    {"thickness", thickness},
                    {"marker_style", marker_style},
                    {"marker_size", marker_size},
                    {"fill", {fill.x, fill.y, fill.z, fill.w}},
                    {"fill_weight", fill_weight},
                    {"fill_outline", {fill_outline.x, fill_outline.y, fill_outline.z, fill_outline.w}}
                });
            } else if (implot_y_axis == ImAxis_Y2) {
                data_to_plotline_properties_mappings["Y2"].push_back({
                    {"data_series_label", data_series_label},
                    {"colour", {colour.x, colour.y, colour.z, colour.w}},
                    {"thickness", thickness},
                    {"marker_style", marker_style},
                    {"marker_size", marker_size},
                    {"fill", {fill.x, fill.y, fill.z, fill.w}},
                    {"fill_weight", fill_weight},
                    {"fill_outline", {fill_outline.x, fill_outline.y, fill_outline.z, fill_outline.w}}
                });
            } else if (implot_y_axis == ImAxis_Y3) {
                data_to_plotline_properties_mappings["Y3"].push_back({
                    {"data_series_label", data_series_label},
                    {"colour", {colour.x, colour.y, colour.z, colour.w}},
                    {"thickness", thickness},
                    {"marker_style", marker_style},
                    {"marker_size", marker_size},
                    {"fill", {fill.x, fill.y, fill.z, fill.w}},
                    {"fill_weight", fill_weight},
                    {"fill_outline", {fill_outline.x, fill_outline.y, fill_outline.z, fill_outline.w}}
                });
            }

        }
    }
    j["data_to_plotline_properties"] = data_to_plotline_properties_mappings;

    return j;
}

RenderablePlot WindowPlotsSaveLoad::deserializeRenderablePlot(const nlohmann::json& j) {
    RenderablePlot plot(j.at("label").get<std::string>(), j.at("real_time").get<bool>());
    // Deserialize other members...
    return plot;
}

void WindowPlotsSaveLoad::saveToFile(const WindowPlots& windowPlots, const std::string& filename) {
    // Append .json extension if not present
    std::string json_filename = filename;
    if (json_filename.substr(json_filename.find_last_of(".") + 1) != "json") {
        json_filename += ".json";
    }

    std::ofstream file(json_filename);
    if (file.is_open()) {
        file << serialize(windowPlots).dump(4); // Pretty print with 4 spaces
        file.close();
    } else {
        throw std::runtime_error("Unable to open file for writing: " + filename);
    }
}

WindowPlots WindowPlotsSaveLoad::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (file.is_open()) {
        nlohmann::json j;
        file >> j;
        file.close();
        return deserialize(j);
    } else {
        throw std::runtime_error("Unable to open file for reading: " + filename);
    }
}

WindowPlots WindowPlotsSaveLoad::deserializeV1(const nlohmann::json& j) {
    WindowPlots window(j.at("label").get<std::string>());
    for (const auto& item : j.at("renderable_plots")) {
        auto plot = std::make_unique<RenderablePlot>(deserializeRenderablePlot(item.at("plot")));
        window.addRenderablePlot(item.at("label").get<std::string>(), std::move(plot));
    }
    return window;
}
