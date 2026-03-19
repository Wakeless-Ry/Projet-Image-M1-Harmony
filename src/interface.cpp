#include "interface.hpp"
#include "imgui.h"
#include <filesystem>

Interface::Interface() : selected_img(0), selected_algo(0), lambda(5.0f), sigma(0.5f) {}

void Interface::load_images(const std::string& folder)
{
    images.clear();
    for (const auto& entry : std::filesystem::directory_iterator(folder))
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
    ImGui::RadioButton("Auncun algo", &selected_algo, 0);
    ImGui::RadioButton("color harmonization", &selected_algo, 1);
    ImGui::Indent();
    if (ImGui::CollapsingHeader("Parametres##1"))
    {
        ImGui::SliderFloat("Lambda", &lambda, 0.1f, 100.0f, "%.1f");
        ImGui::SliderFloat("Sigma",  &sigma, 0.1f, 1.0f, "%.2f");
    }
    ImGui::Unindent();
    ImGui::RadioButton("Algo 2", &selected_algo, 2);

    ImGui::End();
}

double Interface::get_lambda() const { return (double)lambda; }
double Interface::get_sigma()  const { return (double)sigma; }
int Interface::get_algo() const { return selected_algo; }
std::string Interface::get_img() const
{
    if (images.empty())
        return "";
    return images[selected_img];
}

void Interface::set_algo(int algo) { this->selected_algo = algo;}