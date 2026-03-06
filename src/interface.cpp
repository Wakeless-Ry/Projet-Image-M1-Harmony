#include "interface.hpp"
#include "imgui.h"
#include <filesystem>

namespace fs = std::filesystem;

Interface::Interface() : selected_img(0), selected_algo(0) {}

void Interface::load_images(const std::string& folder)
{
    images.clear();
    for (const auto& entry : fs::directory_iterator(folder))
    {
        if (entry.is_regular_file())
        {
            std::string path = entry.path().string();
            if (path.size() >= 4)
            {
                std::string ext = path.substr(path.size() - 4);
                if (ext == ".ppm" || ext == ".jpg" || ext == ".png")
                    images.push_back(path);
            }
        }
    }
}

std::string Interface::get_img() const
{
    if (images.empty())
        return "";
    return images[selected_img];
}

int Interface::get_algo() const
{
    return selected_algo;
}

void Interface::render()
{
    ImGui::Begin("Interface Harmony");

    ImGui::Text("Choisir une image :");
    if (!images.empty())
    {
        const char* preview = images[selected_img].c_str();
        if (ImGui::BeginCombo("Images", preview))
        {
            for (size_t i = 0; i < images.size(); i++)
            {
                bool is_selected = (selected_img == i);
                if (ImGui::Selectable(images[i].c_str(), is_selected))
                    selected_img = static_cast<int>(i);
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
    }

    ImGui::Text("Choisir l'algorithme :");
    ImGui::RadioButton("Algo 1", &selected_algo, 0);
    ImGui::RadioButton("Algo 2", &selected_algo, 1);

    ImGui::End();
}