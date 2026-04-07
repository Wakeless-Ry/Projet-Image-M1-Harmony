#ifndef INTERFACE_HPP
#define INTERFACE_HPP

#include <string>
#include <vector>
#include "template.hpp"

class Interface {
private:
    std::vector<std::string> images;
    int selected_img;
    int selected_algo = 0;
    float lambda = 1.0f;
    float sigma;
    float angle;
    Template_format fmt = Template_format::I;
    float sigma_2 = 0.5f;
    float lambda_2 = 1.0f;
    int bloc_size = 8;
    float width = 1.0f;
public:
    Interface();

    void load_images(const std::string& folder);
    std::string get_img() const; // chemin de l'image sélectionnée
    

    void render(); // la partie ImGui

    double get_lambda() const;
    double get_sigma() const;
    double get_lambda_2() const;
    double get_sigma_2() const;
    double get_angle() const;
    Template_format get_fmt() const;
    void set_angle(double angle_);
    void set_fmt(Template_format fmt_);
    int get_bloc_size() const;
    int get_algo() const;
    void set_algo(int algo);
    double get_width() const;
    void set_width();
};

#endif