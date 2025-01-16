#include "WindowPlotsSaveLoad.hpp"
#include <fstream>

nlohmann::json WindowPlotsSaveLoad::serialize(const WindowPlots& windowPlots) {
    nlohmann::json j;
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
    WindowPlots window(j.at("label").get<std::string>());
    for (const auto& item : j.at("renderable_plots")) {
        auto plot = std::make_unique<RenderablePlot>(deserializeRenderablePlot(item.at("plot")));
        window.addRenderablePlot(item.at("label").get<std::string>(), std::move(plot));
    }
    return window;
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
    std::ofstream file(filename);
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
