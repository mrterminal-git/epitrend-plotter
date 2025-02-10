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
    j["label"] = renderablePlot.getLabel();
    j["real_time"] = renderablePlot.isRealTime();
    // Serialize other members...
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
