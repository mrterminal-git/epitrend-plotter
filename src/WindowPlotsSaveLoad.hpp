#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include "WindowPlots.hpp"
#include "RenderablePlot.hpp"

class WindowPlotsSaveLoad {

public:
    static nlohmann::json serialize(const WindowPlots& windowPlots);
    static WindowPlots deserialize(const nlohmann::json& j);

    static nlohmann::json serialize(const RenderablePlot& renderablePlot);
    static RenderablePlot deserializeRenderablePlot(const nlohmann::json& j);

    static void saveToFile(const WindowPlots& windowPlots, const std::string& filename);
    static WindowPlots loadFromFile(const std::string& filename);

private:
    static WindowPlots deserializeV1(const nlohmann::json& j);
};
