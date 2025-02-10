#pragma once
#include <vector>
#include <string>
#include <filesystem>

#include "RenderablePlot.hpp"
#include "DataManager.hpp"
#include "WindowPlots.hpp"
#include <shlobj.h>
#include <windows.h>

struct AddPlotPopupState {
    std::string window_label;
    std::string plot_label;
    bool is_real_time = true;
    std::vector<std::string> available_sensors;
    std::vector<std::string> selected_sensors;

    char plot_range_start_year[5] = "";
    char plot_range_start_month[3] = "";
    char plot_range_start_day[3] = "";
    char plot_range_start_hour[3] = "";
    char plot_range_start_minute[3] = "";
    char plot_range_start_second[3] = "";

    char plot_range_end_year[5] = "";
    char plot_range_end_month[3] = "";
    char plot_range_end_day[3] = "";
    char plot_range_end_hour[3] = "";
    char plot_range_end_minute[3] = "";
    char plot_range_end_second[3] = "";

    void reset() {
        window_label.clear();
        plot_label.clear();
        is_real_time = true;
        selected_sensors.clear();

        std::fill(std::begin(plot_range_start_year), std::end(plot_range_start_year), '\0');
        std::fill(std::begin(plot_range_start_month), std::end(plot_range_start_month), '\0');
        std::fill(std::begin(plot_range_start_day), std::end(plot_range_start_day), '\0');
        std::fill(std::begin(plot_range_start_hour), std::end(plot_range_start_hour), '\0');
        std::fill(std::begin(plot_range_start_minute), std::end(plot_range_start_minute), '\0');
        std::fill(std::begin(plot_range_start_second), std::end(plot_range_start_second), '\0');

        std::fill(std::begin(plot_range_end_year), std::end(plot_range_end_year), '\0');
        std::fill(std::begin(plot_range_end_month), std::end(plot_range_end_month), '\0');
        std::fill(std::begin(plot_range_end_day), std::end(plot_range_end_day), '\0');
        std::fill(std::begin(plot_range_end_hour), std::end(plot_range_end_hour), '\0');
        std::fill(std::begin(plot_range_end_minute), std::end(plot_range_end_minute), '\0');
        std::fill(std::begin(plot_range_end_second), std::end(plot_range_end_second), '\0');
    }
};

struct PlotOptionsPopupState {
    std::string window_label;
    std::string plot_label;
    bool is_real_time = true;
    std::vector<std::string> available_sensors;
    std::vector<std::string> selected_sensors;
    bool is_range_initialized = false;
    bool is_able_to_submit = false;
    std::vector<std::string> sensors_in_available_list_box;
    int selected_sensor_in_available_list_box = -1;
    std::vector<std::string> sensors_in_Y1_list_box;
    std::vector<std::string> sensors_in_Y2_list_box;
    std::vector<std::string> sensors_in_Y3_list_box;
    char search_available_sensors_buffer[255] = "";
    RenderablePlot::YAxisProperties Y1_properties;
    RenderablePlot::YAxisProperties Y2_properties;
    RenderablePlot::YAxisProperties Y3_properties;
    std::map<std::string, RenderablePlot::PlotLineProperties> sensor_to_plotline_properties;
    std::string plotline_properties_selected_sensor = "";

    char plot_range_start_year[5] = "";
    char plot_range_start_month[3] = "";
    char plot_range_start_day[3] = "";
    char plot_range_start_hour[3] = "";
    char plot_range_start_minute[3] = "";
    char plot_range_start_second[3] = "";

    char plot_range_end_year[5] = "";
    char plot_range_end_month[3] = "";
    char plot_range_end_day[3] = "";
    char plot_range_end_hour[3] = "";
    char plot_range_end_minute[3] = "";
    char plot_range_end_second[3] = "";

    void reset() {
        window_label.clear();
        plot_label.clear();
        is_real_time = true;
        selected_sensors.clear();
        is_range_initialized = false;
        is_able_to_submit = false;
        sensors_in_available_list_box.clear();
        sensors_in_Y1_list_box.clear();
        sensors_in_Y2_list_box.clear();
        selected_sensor_in_available_list_box = -1;
        std::fill(std::begin(search_available_sensors_buffer), std::end(search_available_sensors_buffer), '\0');
        sensor_to_plotline_properties.clear();
        plotline_properties_selected_sensor = "";

        std::fill(std::begin(plot_range_start_year), std::end(plot_range_start_year), '\0');
        std::fill(std::begin(plot_range_start_month), std::end(plot_range_start_month), '\0');
        std::fill(std::begin(plot_range_start_day), std::end(plot_range_start_day), '\0');
        std::fill(std::begin(plot_range_start_hour), std::end(plot_range_start_hour), '\0');
        std::fill(std::begin(plot_range_start_minute), std::end(plot_range_start_minute), '\0');
        std::fill(std::begin(plot_range_start_second), std::end(plot_range_start_second), '\0');

        std::fill(std::begin(plot_range_end_year), std::end(plot_range_end_year), '\0');
        std::fill(std::begin(plot_range_end_month), std::end(plot_range_end_month), '\0');
        std::fill(std::begin(plot_range_end_day), std::end(plot_range_end_day), '\0');
        std::fill(std::begin(plot_range_end_hour), std::end(plot_range_end_hour), '\0');
        std::fill(std::begin(plot_range_end_minute), std::end(plot_range_end_minute), '\0');
        std::fill(std::begin(plot_range_end_second), std::end(plot_range_end_second), '\0');
    }
};

struct WindowPlotAddPlotPopupState {
    std::string plot_label;
    bool is_real_time = true;
    std::vector<std::string> all_sensors;
    std::vector<std::string> available_sensors;
    std::vector<std::string> selected_sensors;
    int selected_sensor_in_available_list_box = -1;
    int selected_sensor_in_selected_list_box = -1;
    bool is_range_initialized = false;

    char plot_range_start_year[5] = "";
    char plot_range_start_month[3] = "";
    char plot_range_start_day[3] = "";
    char plot_range_start_hour[3] = "";
    char plot_range_start_minute[3] = "";
    char plot_range_start_second[3] = "";

    char plot_range_end_year[5] = "";
    char plot_range_end_month[3] = "";
    char plot_range_end_day[3] = "";
    char plot_range_end_hour[3] = "";
    char plot_range_end_minute[3] = "";
    char plot_range_end_second[3] = "";

    char plot_label_buffer[255] = "";
    char search_available_sensors_buffer[255] = "";
    std::tm input_start_time = {};
    std::tm input_end_time = {};

    void reset() {
        plot_label.clear();
        is_real_time = true;
        all_sensors.clear();
        available_sensors.clear();
        selected_sensors.clear();
        selected_sensor_in_available_list_box = -1;
        selected_sensor_in_selected_list_box = -1;
        is_range_initialized = false;

        std::fill(std::begin(plot_range_start_year), std::end(plot_range_start_year), '\0');
        std::fill(std::begin(plot_range_start_month), std::end(plot_range_start_month), '\0');
        std::fill(std::begin(plot_range_start_day), std::end(plot_range_start_day), '\0');
        std::fill(std::begin(plot_range_start_hour), std::end(plot_range_start_hour), '\0');
        std::fill(std::begin(plot_range_start_minute), std::end(plot_range_start_minute), '\0');
        std::fill(std::begin(plot_range_start_second), std::end(plot_range_start_second), '\0');

        std::fill(std::begin(plot_range_end_year), std::end(plot_range_end_year), '\0');
        std::fill(std::begin(plot_range_end_month), std::end(plot_range_end_month), '\0');
        std::fill(std::begin(plot_range_end_day), std::end(plot_range_end_day), '\0');
        std::fill(std::begin(plot_range_end_hour), std::end(plot_range_end_hour), '\0');
        std::fill(std::begin(plot_range_end_minute), std::end(plot_range_end_minute), '\0');
        std::fill(std::begin(plot_range_end_second), std::end(plot_range_end_second), '\0');

        std::fill(std::begin(plot_label_buffer), std::end(plot_label_buffer), '\0');
        std::fill(std::begin(search_available_sensors_buffer), std::end(search_available_sensors_buffer), '\0');
        input_start_time = {};
    }
};

struct FileDialogState {
    std::string current_path = "";
    std::string current_file = "";
    std::string current_folder = "";
    std::string error = "";
    char path_buffer[255] = "";
    std::vector<std::filesystem::directory_entry> files;
    std::vector<std::filesystem::directory_entry> folders;
    int file_select_index = 0;
    int folder_select_index = 0;
    bool open = false;
    bool initial_path_set = false;

    void reset() {
        current_path.clear();
        current_file.clear();
        current_folder.clear();
        error.clear();
        std::fill(std::begin(path_buffer), std::end(path_buffer), '\0');
        files.clear();
        folders.clear();
        file_select_index = 0;
        folder_select_index = 0;
        open = false;
        initial_path_set = false;

        // Reinitialize path_buffer with the user's "Downloads" folder path
        PWSTR default_path = NULL;
        if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Desktop, 0, NULL, &default_path))) {
            wcstombs(path_buffer, default_path, sizeof(path_buffer));
            CoTaskMemFree(default_path);
        }
    }

    FileDialogState() {
        // Initialize path_buffer with the user's "Downloads" folder path
        PWSTR default_path = NULL;
        if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Desktop, 0, NULL, &default_path))) {
            wcstombs(path_buffer, default_path, sizeof(path_buffer));
            CoTaskMemFree(default_path);
        }
    }
};

struct FileMenuState {
    bool save_as_open_popup = false;

    void reset() {
        save_as_open_popup = false;
    }
};

struct SaveWindowAsPopupState {
    std::string window_label;
    char file_path_buffer[255] = "";
    char file_name_buffer[255] = "";
    bool is_able_to_save = true;
    bool save_success_popup = false;
    std::filesystem::path save_path;
    int check_save_attempt = 0;
    int check_save_attempt_max = 600;
    bool close_after_save = false;
    bool overwrite_file_popup = false;

    void reset() {
        window_label.clear();
        std::fill(std::begin(file_path_buffer), std::end(file_path_buffer), '\0');
        std::fill(std::begin(file_name_buffer), std::end(file_name_buffer), '\0');
        is_able_to_save = true;
        save_success_popup = false;
        save_path = "";
        check_save_attempt = 0;
        check_save_attempt_max = 600;
        close_after_save = false;
        overwrite_file_popup = false;

        // Initialize path_buffer with the user's "Desktop" folder path
        PWSTR default_path = NULL;
        if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Desktop, 0, NULL, &default_path))) {
            wcstombs(file_path_buffer, default_path, sizeof(file_path_buffer));
            CoTaskMemFree(default_path);
        }
    }

    SaveWindowAsPopupState() {
        // Initialize path_buffer with the user's "Desktop" folder path
        PWSTR default_path = NULL;
        if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Desktop, 0, NULL, &default_path))) {
            wcstombs(file_path_buffer, default_path, sizeof(file_path_buffer));
            CoTaskMemFree(default_path);
        }
    }
};

class GraphViewModel {
public:
    // Constructor
    GraphViewModel(std::mutex& mutex);

    // Plottable sensors storage
    void setPlottableSensors(const std::vector<std::string>& sensors);
    const std::vector<std::string>& getPlottableSensors() const;

    // get "Add plot" popup state
    AddPlotPopupState& getAddPlotPopupState();

    // get "Plot options" popup state for a specific plot
    PlotOptionsPopupState& getPlotOptionsState();

    // Update plots with data from DataManager
    void updatePlotsWithData(DataManager& dataManager);

    // Get downsampled data for a specific sensor
    std::pair<std::vector<DataManager::Timestamp>, std::vector<DataManager::Value>> getDownsampledData(
    RenderablePlot& plot, const std::string& sensor, double range, int num_pixels);



    // ============================================
    // Windows with renderable plots
    // ============================================
    void addWindowPlots(const std::string& window_label, std::unique_ptr<WindowPlots> window_plots);
    void removeWindowPlots(const std::string& window_label);
    bool hasWindowPlots(const std::string& window_label) const;
    const std::map<std::string, std::unique_ptr<WindowPlots>>& getWindowPlots() const;
    WindowPlots& getWindowPlot(const std::string& window_label);
    std::vector<std::string> getWindowPlotLabels() const;

    WindowPlotAddPlotPopupState& getWindowPlotAddPlotPopupState();
    FileDialogState& getFileDialogState();
    FileMenuState& getFileMenuState();
    SaveWindowAsPopupState& getSaveWindowAsPopupState();


private:
    // Plottable sensors
    std::vector<std::string> plottable_sensors_;

    // "Add plot" popup state
    AddPlotPopupState add_plot_popup_state_;

    // Track the plot id
    long long next_plot_id_ = 0;

    // Mutex for updating the view model
    std::mutex& update_viewModel_mutex_;

    // "Plot options" popup state
    PlotOptionsPopupState plot_options_popup_state_;


    // ============================================
    // Windows with renderable plots
    // ============================================
    std::map<std::string, std::unique_ptr<WindowPlots>> window_plots_;
    WindowPlotAddPlotPopupState window_plot_add_plot_popup_state_;
    FileDialogState file_dialog_state_;
    FileMenuState file_menu_state_;
    SaveWindowAsPopupState save_window_as_popup_state_;

};
