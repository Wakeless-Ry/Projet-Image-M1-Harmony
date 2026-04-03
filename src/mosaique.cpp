#include "mosaique.hpp"
#include <stdexcept>
#include <cstdio>

void Mosaique::set_size_bloc(int size)
{
    this->size_bloc = size;
}

void Mosaique::set_img(std::string path_) {
    this->img_origin.set_path(path_);
    this->path = path_;
}

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
    this->img_mean.write_ppm("../assets/out/algo2/mean.ppm");
}

Template Mosaique::bloc_tmpl(std::vector<Pixel> data_tmp) const {
    Template tmpl = Template(std::vector<double>{}, std::vector<double>{});
    int nb_pixels = this->size_bloc * this->size_bloc * 3;
    std::vector<unsigned char> tmp(nb_pixels);
    for (int p = 0; p < nb_pixels; p += 3)
    {
        tmp[p + 0] = data_tmp[p / 3].r;
        tmp[p + 1] = data_tmp[p / 3].g;
        tmp[p + 2] = data_tmp[p / 3].b;
    }
    tmpl.set_image_v2(tmp, this->size_bloc, this->size_bloc);
    double angle = tmpl.bestOrientation();
    Template resultat = Template(Template_format::i);
    resultat.set_image(this->path);
    resultat.rotate(angle);
    return resultat;
}

std::vector<Pixel> Mosaique::resize_image(std::vector<Pixel>& in)
{
    const int HEIGHT = this->img_origin.get_height();
    const int WIDTH = this->img_origin.get_width();
    double scale_x = static_cast<double>(WIDTH) / this->size_bloc;
    double scale_y = static_cast<double>(HEIGHT) / this->size_bloc;

    std::vector<Pixel> out(this->size_bloc * this->size_bloc);

    for (int y_out = 0; y_out < this->size_bloc; y_out++)
    {
        int y_in = static_cast<int>(y_out * scale_y);
        if (y_in >= HEIGHT) y_in = HEIGHT - 1;

        for (int x_out = 0; x_out < this->size_bloc; x_out++)
        {
            int x_in = static_cast<int>(x_out * scale_x);
            if (x_in >= WIDTH) x_in = WIDTH - 1;

            out[y_out * this->size_bloc + x_out] = in[y_in * WIDTH + x_in];
        }
    }
    return out;
}

void Mosaique::compute_mosaique() {
    const std::vector<Pixel> DATA_MEAN = this->img_mean.get_img();
    const int HEIGHT = this->img_mean.get_height();
    const int WIDTH = this->img_mean.get_width();

    std::vector<unsigned char> mosaique_data(HEIGHT * WIDTH * 3);
    int count = 0;
    for (int h = 0; h < HEIGHT; h += this->size_bloc)
    {
        for (int w = 0; w < WIDTH; w += this->size_bloc)
        {
            std::vector<Pixel> bloc(this->size_bloc * this->size_bloc);
            for (int x = h; x < h + this->size_bloc; x++) {
                for (int y = w; y < w + this->size_bloc; y++) {
                    int local_x = x - h;
                    int local_y = y - w;
                    bloc[local_x * this->size_bloc + local_y] = DATA_MEAN[x * WIDTH + y];
                }
            }
            Template tmpl = bloc_tmpl(bloc);
            tmpl.compute_labels();
            std::vector<Pixel> resultat_bloc = tmpl.shift_hues();
            resultat_bloc = resize_image(resultat_bloc);

            for (int x = h; x < h + this->size_bloc; x++) {
                for (int y = w; y < w + this->size_bloc; y++) {
                    int local_x = x - h;
                    int local_y = y - w;
                    Pixel p = resultat_bloc[local_x * this->size_bloc + local_y];
                    mosaique_data[(x * WIDTH + y) * 3 + 0] = p.r;
                    mosaique_data[(x * WIDTH + y) * 3 + 1] = p.g;
                    mosaique_data[(x * WIDTH + y) * 3 + 2] = p.b;
                }
            }
            count++;
            printf("%d / 4096\n", count);
        }
    }

    this->img_mosaique = Image(mosaique_data, WIDTH, HEIGHT);
    this->img_mosaique.write_ppm("../assets/out/algo2/mosaique.ppm");
}
