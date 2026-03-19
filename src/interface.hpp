#ifndef INTERFACE_HPP
#define INTERFACE_HPP

#include <string>
#include <vector>

class Interface {
private:
    std::vector<std::string> images;
    int selected_img;
    int selected_algo = 0;
    float lambda;
    float sigma;

public:
    Interface();

    void load_images(const std::string& folder);
    std::string get_img() const; // chemin de l'image sélectionnée
    

    void render(); // la partie ImGui

    double get_lambda() const;
    double get_sigma()  const;
    int get_algo() const;
    void set_algo(int algo);
};

#endif