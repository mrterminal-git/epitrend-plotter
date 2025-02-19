#include "Config.hpp"
#include <filesystem>

Config::Config(const std::string& configFilePath) {
    loadConfig(configFilePath);
}

// Helper function to convert non-visible characters to their visible representations
std::string makeVisible(const std::string& str) {
    std::string visibleStr;
    for (char ch : str) {
        switch (ch) {
            case '\t': visibleStr += "\\t"; break;
            case '\n': visibleStr += "\\n"; break;
            case '\r': visibleStr += "\\r"; break;
            case '\v': visibleStr += "\\v"; break;
            case '\f': visibleStr += "\\f"; break;
            case '\b': visibleStr += "\\b"; break;
            case '\a': visibleStr += "\\a"; break;
            case '\0': visibleStr += "\\0"; break;
            default:
                if (ch < 32 || ch == 127) {
                    visibleStr += "\\x" + std::to_string(static_cast<unsigned char>(ch));
                } else {
                    visibleStr += ch;
                }
                break;
        }
    }
    return visibleStr;
}

// Helper function to remove non-visible characters
std::string removeNonVisible(const std::string& str) {
    std::string visibleStr;
    for (char ch : str) {
        if (ch >= 32 && ch != 127) {
            visibleStr += ch;
        }
    }
    return visibleStr;
}

void Config::loadConfig(const std::string& configFilePath) {
    // // DEBUG Print the current directory
    // std::cout << "Current directory: " << std::filesystem::current_path() << "\n";

    // // DEBUG Print all the files in the current directory
    // for (const auto& entry : std::filesystem::directory_iterator(".")) {
    //     std::cout << entry.path() << "\n";
    // }

    // // DEBUG Check if configFilePath file exists using filesystem
    // if (!std::filesystem::exists(configFilePath)) {
    //     std::cout << "Config file does not exist: " + configFilePath << "\n";
    // } else {
    //     std::cout << "Config file exists: " + configFilePath << "\n";
    // }

    std::fstream configFile(configFilePath);
    // Wait for the file to be opened
    int attempts = 0;
    while (!configFile.is_open() && attempts < 10) {
        configFile.open(configFilePath);
        attempts++;
    }
    if (!configFile.is_open()) {
        // Stop the program with an exception
        std::cerr << "Could not open config file: " + configFilePath << "\n";
        std::cerr << "Current directory: " << std::filesystem::current_path() << "\n";
        throw std::runtime_error("Could not open config file: " + configFilePath);
    }

    std::string line;
    while (std::getline(configFile, line)) {
        std::istringstream lineStream(line);
        std::string key;
        if (std::getline(lineStream, key, '=')) {
            std::string value;
            if (std::getline(lineStream, value)) {
                configMap[key] = value;
            }
        }
    }

    // DEBUG Print the config map
    // std::cout << "Print configMap: \n";
    // for (const auto& element : configMap) {
    //     // Explicity print whitespace and newline characters as they are not visible
    //     configMap[element.first] = removeNonVisible(element.second);
    //     std::cout << element.first << " = " << makeVisible(element.second) << "\n";
    // }


    configFile.close();
}

std::string Config::getDataDir() const {
    return configMap.at("DATA_DIR");
}

std::string Config::getOutputDir() const {
    return configMap.at("OUTPUT_DIR");
}

std::string Config::getServerEpitrendDataDir() const {
    return configMap.at("SERVER_EPITREND_DATA_DIR");
}

std::string Config::getServerRGADataDir() const {
    return configMap.at("SERVER_RGA_DATA_DIR");
}

std::string Config::getOrg() const {
    return configMap.at("ORG");
}

std::string Config::getHost() const {
    return configMap.at("HOST");
}

int Config::getPort() const {
    return std::stoi(configMap.at("PORT"));
}

std::string Config::getRgaBucket() const {
    return configMap.at("RGA_BUCKET");
}

std::string Config::getEpitrendBucket() const {
    return configMap.at("EPITREND_BUCKET");
}

std::string Config::getUser() const {
    return configMap.at("USER");
}

std::string Config::getPassword() const {
    return configMap.at("PASSWORD");
}

std::string Config::getPrecision() const {
    return configMap.at("PRECISION");
}

std::string Config::getToken() const {
    return configMap.at("TOKEN");
}

void Config::debugPrintconfigMap() const {
    std::cout << "KEYS: \n";
    for (const auto& element : configMap) {
        std::cout << makeVisible(element.first) << "\n";
    }
    std::cout << "VALUES: \n";
    for (const auto& element : configMap) {
        std::cout << makeVisible(element.second) << "\n";
    }
}
