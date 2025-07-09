#pragma once
#include <fstream>     
#include <sstream>     
#include <string>      
#include <vector>      
#include "/opt/homebrew/include/SDL2/SDL.h"
#include <filesystem>
#include <iostream>

std::vector<std::vector<std::string>> import_csv_layout(const std::string& path) {
    std::vector<std::vector<std::string>> terrain_map;
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to open CSV file: " << path << std::endl;
        return terrain_map;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string cell;
        std::vector<std::string> row;

        while (std::getline(ss, cell, ',')) {
            row.push_back(cell);
        }
        terrain_map.push_back(row);
    }

    return terrain_map;
}

std::vector<SDL_Surface*> import_folder(const std::string& path) {
    std::vector<std::filesystem::directory_entry> entries;

    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        if (!entry.is_regular_file()) continue;

        std::string stem = entry.path().stem().string();

        if (!std::all_of(stem.begin(), stem.end(), ::isdigit)) {
            std::cerr << "Skipping non-numeric file: " << entry.path().filename() << std::endl;
            continue;
        }

        entries.push_back(entry);
    }

    std::sort(entries.begin(), entries.end(), [](const auto& a, const auto& b) {
        return std::stoi(a.path().stem().string()) < std::stoi(b.path().stem().string());
    });

    std::vector<SDL_Surface*> surface_list;
    for (const auto& entry : entries) {
        std::string full_path = entry.path().string();
        SDL_Surface* image = IMG_Load(full_path.c_str());
        std::cout << "Loading: " << full_path << std::endl;

        if (!image) {
            std::cerr << "Failed to load image: " << full_path << " | " << IMG_GetError() << std::endl;
            continue;
        }

        surface_list.push_back(image);
    }

    return surface_list;
}

std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\n\r");
    size_t end = s.find_last_not_of(" \t\n\r");
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}