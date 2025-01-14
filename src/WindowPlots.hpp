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

        // Deleted copy constructor and copy assignment operator
        WindowPlots(const WindowPlots&) = delete;
        WindowPlots& operator=(const WindowPlots&) = delete;

        // Move constructorand move assignment operator
        WindowPlots(WindowPlots&& other) noexcept;
        WindowPlots& operator=(WindowPlots&& other) noexcept;



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
        std::string getLabel() const;



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
    // ===================!!!=========================
    // NOTE: The move constructor and move assignment operator is implemented.
    // If you need to add more data members, you need to update these functions.
    // ===================!!!=========================

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
