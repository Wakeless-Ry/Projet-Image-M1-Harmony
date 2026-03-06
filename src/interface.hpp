#ifndef INTERFACE_HPP
#define INTERFACE_HPP

#include <string>
#include <vector>

class Interface {
private:
    std::vector<std::string> images;
    int selected_img;
    int selected_algo;

public:
    Interface();

    void load_images(const std::string& folder);
    std::string get_img() const; // chemin de l'image sélectionnée
    int get_algo() const;

    void render(); // la partie ImGui
};

#endif