#pragma once

#include <string>
#include <map>

#include "RenderablePlot.hpp"

class WindowPlots
{
    public:
        // Constructor
        WindowPlots(const std::string& label);

        // Destructor
        ~WindowPlots();

        // ============================================
        // Renderable Plot Management
        // ============================================
        void addRenderablePlot(const std::string& plot_label, std::unique_ptr<RenderablePlot> renderable_plot); // Warning: This function moves the renderable_plot object
        bool hasRenderablePlot(const std::string& plot_label);
        RenderablePlot& getRenderablePlot(const std::string& plot_label);
        const std::map<std::string, std::unique_ptr<RenderablePlot>>& getRenderablePlots() const;
        void clearAllRenderablePlots();
        void removeRenderablePlot(const std::string& plot_label);
        std::vector<std::string> getRenderablePlotLabels() const;



        // ============================================
        // Window Properties
        // ============================================
        void setXPosition(float x);
        void setYPosition(float y);
        void setWidth(float width);
        void setHeight(float height);
        std::pair<float, float> getPosition() const;
        std::pair<float, float> getSize() const;



    private:
        // ============================================
        // Renderable Plot Management
        // ============================================
        std::string label_;
        std::map<std::string, std::unique_ptr<RenderablePlot>> renderable_plots_;



        // ============================================
        // Window Properties
        // ============================================
        float pos_x_;
        float pos_y_;
        float width_;
        float height_;

};
