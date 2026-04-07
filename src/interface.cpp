#include "interface.hpp"
#include "imgui.h"
#include <filesystem>
const char *format_names[] = {"i", "V", "L", "I", "T", "Y", "X", "t", "q"};
Interface::Interface()
    : selected_img(0), selected_algo(0), lambda(1.0f), sigma(0.5f) {}

void Interface::load_images(const std::string &folder) {
    images.clear();
    for (const auto &entry : std::filesystem::directory_iterator(folder)) {
        if (entry.is_regular_file()) {
            std::string path = entry.path().string();
            if (path.size() >= 4) {
                std::string ext = path.substr(path.size() - 4);
                if (ext == ".ppm" || ext == ".jpg" || ext == ".png")
                    images.push_back(path);
            }
        }
    }
}

void Interface::render() {
    ImGui::Begin("Interface Harmony");

    ImGui::Text("Choisir une image :");
    if (!images.empty()) {
        const char *preview = images[selected_img].c_str();
        if (ImGui::BeginCombo("Images", preview)) {
            for (size_t i = 0; i < images.size(); i++) {
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
    if (ImGui::CollapsingHeader("Parametres##1")) {
        ImGui::InputFloat("Angle", &angle, 0.01f, 100.0f, "%.001f");

        for (int i = 0; i < IM_ARRAYSIZE(format_names); i++) {
            if (ImGui::RadioButton(format_names[i], static_cast<int>(fmt) == i))
                fmt = static_cast<Template_format>(i);
            if (i < 8)
                ImGui::SameLine();
        }
        ImGui::SliderFloat("width", &width, 0.1f, 1.0f, "%.1f");
        ImGui::SliderFloat("Lambda", &lambda, 0.1f, 100.0f, "%.1f");
        ImGui::SliderFloat("Sigma", &sigma, 0.1f, 1.0f, "%.2f");
    }
    ImGui::Unindent();
    ImGui::RadioButton("Mosaïque", &selected_algo, 2);
    ImGui::Indent();
    if (ImGui::CollapsingHeader("Parametres##1")) {
        ImGui::Text("Bloc size");
        ImGui::SameLine();
        if (ImGui::RadioButton("4", bloc_size == 4))
            bloc_size = 4;
        ImGui::SameLine();
        if (ImGui::RadioButton("8", bloc_size == 8))
            bloc_size = 8;
        ImGui::SameLine();
        if (ImGui::RadioButton("16", bloc_size == 16))
            bloc_size = 16;
        ImGui::SliderFloat("Lambda", &lambda_2, 0.1f, 100.0f, "%.1f");
        ImGui::SliderFloat("Sigma", &sigma_2, 0.1f, 1.0f, "%.2f");
    }
    ImGui::Unindent();
    ImGui::RadioButton("Benchmark", &selected_algo, 3);
    ImGui::End();
}

double Interface::get_lambda() const { return (double)this->lambda; }
double Interface::get_sigma() const { return (double)this->sigma; }
double Interface::get_lambda_2() const { return (double)this->lambda_2; }
double Interface::get_sigma_2() const { return (double)this->sigma_2; }
int Interface::get_algo() const { return this->selected_algo; }
int Interface::get_bloc_size() const { return this->bloc_size; }
double Interface::get_angle() const { return (double)this->angle; }
Template_format Interface::get_fmt() const { return this->fmt; }
double Interface::get_width() const { return (double)this->width; }
void Interface::set_width() { this->width = 1.0f; }
std::string Interface::get_img() const {
    if (images.empty())
        return "";
    return images[selected_img];
}

void Interface::set_algo(int algo) { this->selected_algo = algo; }
void Interface::set_angle(double angle_) { this->angle = angle_; }
void Interface::set_fmt(Template_format fmt_) { this->fmt = fmt_; }