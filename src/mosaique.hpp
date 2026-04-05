#ifndef MOSAIQUE_HPP
#define MOSAIQUE_HPP
#include <string>
#include "template.hpp"

class Mosaique {
    private:
        Image img_origin;
        Image img_mean;
        Image img_mosaique;
        std::vector<Template> modif_mosa;
        int size_bloc = 8;
        float lambda = 1.0f;
        float sigma = 0.5f;
        std::string path;
        void bloc_tmpl(std::vector<Pixel> data_tmp, const std::vector<unsigned char>& origin_data, int ORIGIN_W, int ORIGIN_H, int bloc_idx);
        std::vector<Pixel> resize_image(std::vector<Pixel>& in);
    public:
        Mosaique() = default;
        ~Mosaique() = default;

        void set_size_bloc(int size);
        void set_img(std::string path);
        void set_lambda(float lambda_);
        void set_sigma(float sigma_);
        void compute_mean();
        void compute_mosaique();
        const std::vector<Pixel> get_mosaique() const;
        void recompute_lambda_sigma();
};
#endif