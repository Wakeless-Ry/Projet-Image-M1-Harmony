#include "mosaique.hpp"
#include "template.hpp"
#include <stdexcept>
#include <cstdio>
#include <string>

void Mosaique::set_size_bloc(int size) { this->size_bloc = size; }

void Mosaique::set_img(std::string path_) {
    this->img_origin.set_path(path_);
    this->path = path_;
}

void Mosaique::set_lambda(float lambda_) { this->lambda = lambda_; }
void Mosaique::set_sigma(float sigma_ ) { this->sigma = sigma_; }

void Mosaique::compute_mean() {
    const int HEIGHT = this->img_origin.get_height();
    const int WIDTH = this->img_origin.get_width();
    if ((HEIGHT % this->size_bloc) != 0)
        throw std::runtime_error("Hauteur de l'image n'est pas divisible par " + std::to_string(this->size_bloc));
    else if ((WIDTH % this->size_bloc) != 0)
        throw std::runtime_error("Largeur de l'image n'est pas divisible par " + std::to_string(this->size_bloc));
    const int NB_PIXELS = this->img_origin.get_nb_pixels() * 3;
    const std::vector<Pixel> DATA_IMG = this->img_origin.get_img();
    std::vector<unsigned char> mean(NB_PIXELS);

    for (int h = 0; h < HEIGHT; h += this->size_bloc) {
        for (int w = 0; w < WIDTH; w += this->size_bloc) {
            int sum_red = 0, sum_green = 0, sum_blue = 0;
            int count = 0;

            for (int x = h; x < h + this->size_bloc; x++) {
                for (int y = w; y < w + this->size_bloc; y++) {
                    sum_red   += DATA_IMG[x * WIDTH + y].r;
                    sum_green += DATA_IMG[x * WIDTH + y].g;
                    sum_blue  += DATA_IMG[x * WIDTH + y].b;
                    count++;
                }
            }

            unsigned char mean_red = sum_red / count;
            unsigned char mean_green = sum_green / count;
            unsigned char mean_blue = sum_blue / count;

            for (int x = h; x < h + this->size_bloc; x++) {
                for (int y = w; y < w + this->size_bloc; y++) {
                    mean[(x * WIDTH + y) * 3 + 0] = mean_red;
                    mean[(x * WIDTH + y) * 3 + 1] = mean_green;
                    mean[(x * WIDTH + y) * 3 + 2] = mean_blue;
                }
            }
        }
    }
    this->img_mean = Image(mean, WIDTH, HEIGHT);
    std::string filename = this->path.substr(this->path.find_last_of('/') + 1);
    this->img_mean.write_ppm("../assets/out/mosaique/mean_" + std::to_string(this->size_bloc) + "_" + filename);
}

std::vector<Pixel> Mosaique::resize_image(std::vector<Pixel>& in)
{
    const int HEIGHT = this->img_origin.get_height();
    const int WIDTH = this->img_origin.get_width();
    double scale_x = static_cast<double>(WIDTH) / this->size_bloc;
    double scale_y = static_cast<double>(HEIGHT) / this->size_bloc;

    std::vector<Pixel> out(this->size_bloc * this->size_bloc);

    for (int y_out = 0; y_out < this->size_bloc; y_out++){
        int y_in = static_cast<int>(y_out * scale_y);
        if (y_in >= HEIGHT)
            y_in = HEIGHT - 1;

        for (int x_out = 0; x_out < this->size_bloc; x_out++) {
            int x_in = static_cast<int>(x_out * scale_x);
            if (x_in >= WIDTH)
                x_in = WIDTH - 1;

            out[y_out * this->size_bloc + x_out] = in[y_in * WIDTH + x_in];
        }
    }
    return out;
}

void Mosaique::bloc_tmpl(std::vector<Pixel> data_tmp, const std::vector<unsigned char>& origin_data, int ORIGIN_W, int ORIGIN_H, int bloc_idx) {
    int nb_pixels = this->size_bloc * this->size_bloc;
    std::vector<unsigned char> tmp(nb_pixels * 3);
    
    for (int p = 0; p < nb_pixels; p++) {
        tmp[p * 3 + 0] = data_tmp[p].r;
        tmp[p * 3 + 1] = data_tmp[p].g;
        tmp[p * 3 + 2] = data_tmp[p].b;
    }

    Template tmpl = Template(std::vector<double>{}, std::vector<double>{});
    tmpl.set_image_v2(tmp, this->size_bloc, this->size_bloc);
    
    auto [fmt, angle] = tmpl.bestTemplate();
    Template result(fmt);
    result.set_image_v2(origin_data, ORIGIN_W, ORIGIN_H);
    result.rotate(angle);
    this->modif_mosa[bloc_idx] = result;
}

void Mosaique::compute_mosaique() {
    const std::vector<Pixel> DATA_MEAN = this->img_mean.get_img();
    const std::vector<Pixel> DATA_ORIGIN = this->img_origin.get_img();
    const int ORIGIN_H = this->img_origin.get_height();
    const int ORIGIN_W = this->img_origin.get_width();
    const int HEIGHT = this->img_mean.get_height();
    const int WIDTH = this->img_mean.get_width();

    std::vector<unsigned char> origin_data(ORIGIN_W * ORIGIN_H * 3);
    for (int i = 0; i < (int)DATA_ORIGIN.size(); i++) {
        origin_data[i * 3 + 0] = DATA_ORIGIN[i].r;
        origin_data[i * 3 + 1] = DATA_ORIGIN[i].g;
        origin_data[i * 3 + 2] = DATA_ORIGIN[i].b;
    }

    std::vector<unsigned char> mosaique_data(HEIGHT * WIDTH * 3);
    int count = 0;

    int nb_blocs_w = WIDTH / this->size_bloc;
    int nb_blocs_h = HEIGHT / this->size_bloc;
    int total = nb_blocs_w * nb_blocs_h;
    this->modif_mosa.resize(total, Template(std::vector<double>{}, std::vector<double>{}));
    #pragma omp parallel for schedule(dynamic)
    for (int bloc_idx = 0; bloc_idx < total; bloc_idx++) {
        int h = (bloc_idx / nb_blocs_w) * this->size_bloc;
        int w = (bloc_idx % nb_blocs_w) * this->size_bloc;

        std::vector<Pixel> bloc(this->size_bloc * this->size_bloc);
        for (int x = h; x < h + this->size_bloc; x++)
            for (int y = w; y < w + this->size_bloc; y++)
                bloc[(x - h) * this->size_bloc + (y - w)] = DATA_MEAN[x * WIDTH + y];

        bloc_tmpl(bloc, origin_data, ORIGIN_W, ORIGIN_H, bloc_idx);
        Template tmpl = this->modif_mosa[bloc_idx];
        tmpl.build_graph();
        tmpl.solve_graph(this->lambda);
        std::vector<Pixel> resultat_bloc = tmpl.shift_hues(this->sigma);
        resultat_bloc = resize_image(resultat_bloc);

        for (int x = h; x < h + this->size_bloc; x++)
            for (int y = w; y < w + this->size_bloc; y++) {
                Pixel p = resultat_bloc[(x - h) * this->size_bloc + (y - w)];
                mosaique_data[(x * WIDTH + y) * 3 + 0] = p.r;
                mosaique_data[(x * WIDTH + y) * 3 + 1] = p.g;
                mosaique_data[(x * WIDTH + y) * 3 + 2] = p.b;
            }

        // #pragma omp critical
        // {
        //     count++;
        //     printf("%d/%d\n", count, total);
        // }
    }

    this->img_mosaique = Image(mosaique_data, WIDTH, HEIGHT);
    std::string filename = this->path.substr(this->path.find_last_of('/') + 1);
    this->img_mosaique.write_ppm("../assets/out/mosaique/mosaique_" + std::to_string(this->size_bloc) + "_" + filename);
}


void Mosaique::recompute_lambda_sigma() {
    const int HEIGHT = this->img_mean.get_height();
    const int WIDTH = this->img_mean.get_width();
    int nb_blocs_w = WIDTH / this->size_bloc;
    int nb_blocs_h = HEIGHT / this->size_bloc;
    int total = nb_blocs_w * nb_blocs_h;
    std::vector<unsigned char> mosaique_data(HEIGHT * WIDTH * 3);
    int count = 0;
    #pragma omp parallel for schedule(dynamic)
    for (int bloc_idx = 0; bloc_idx < total; bloc_idx++) {
        int h = (bloc_idx / nb_blocs_w) * this->size_bloc;
        int w = (bloc_idx % nb_blocs_w) * this->size_bloc;

        Template tmpl = this->modif_mosa[bloc_idx];
        tmpl.build_graph();
        tmpl.solve_graph(this->lambda);
        std::vector<Pixel> resultat_bloc = tmpl.shift_hues(this->sigma);
        resultat_bloc = resize_image(resultat_bloc);

        for (int x = h; x < h + this->size_bloc; x++)
            for (int y = w; y < w + this->size_bloc; y++) {
                Pixel p = resultat_bloc[(x - h) * this->size_bloc + (y - w)];
                mosaique_data[(x * WIDTH + y) * 3 + 0] = p.r;
                mosaique_data[(x * WIDTH + y) * 3 + 1] = p.g;
                mosaique_data[(x * WIDTH + y) * 3 + 2] = p.b;
            }
        #pragma omp critical
        {
            count++;
            printf("%d/%d\n", count, total);
        }
    }

    this->img_mosaique = Image(mosaique_data, WIDTH, HEIGHT);
    std::string filename = this->path.substr(this->path.find_last_of('/') + 1);
    this->img_mosaique.write_ppm("../assets/out/mosaique/mosaique_" + std::to_string(this->size_bloc) + "_" + filename);
}