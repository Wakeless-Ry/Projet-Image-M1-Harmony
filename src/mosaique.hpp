#ifndef MOSAIQUE_HPP
#define MOSAIQUE_HPP
#include <string>
#include "template.hpp"

class Mosaique {
    private:
        Image img_origin;
        Image img_mean;
        Image img_mosaique;
        int size_bloc;  
        std::string path;
        Template bloc_tmpl(std::vector<Pixel> data_tmp) const;
        std::vector<Pixel> resize_image(std::vector<Pixel>& in);
    public:
        Mosaique() = default;
        ~Mosaique() = default;

        void set_size_bloc(int size);
        void set_img(std::string path);
        void compute_mean();
        void compute_mosaique();
};
#endif