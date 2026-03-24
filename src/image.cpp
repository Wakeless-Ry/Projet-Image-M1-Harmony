#include "image.hpp"

// ereur
#include <stdexcept>

// extention
#include <algorithm>

// lecture & write
#include "../common/stb_image.h"
#include <fstream>

// histogramme
#include <cmath>
#include <cstdlib>
#include <map>

Image::Image(const std::string path) {
    auto dot = path.find_last_of('.');
    if (dot == std::string::npos)
        throw std::runtime_error("Pas d'extension");

    std::string ext = path.substr(dot + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    if (ext == "ppm")
        read_ppm(path);
    else if (ext == "png" || ext == "jpg" || ext == "jpeg")
        read_stb(path);
    else
        throw std::runtime_error("Format non supporté");
}

Image::Image(const std::vector<unsigned char> &buffer, int width, int height) {
    this->width = width;
    this->height = height;
    this->nb_pixels = width * height;
    if (buffer.size() != this->nb_pixels * 3)
        throw std::runtime_error("Taille du buffer incorrecte");

    this->image_data.resize(this->nb_pixels);
    for (int i = 0; i < nb_pixels; ++i)
        this->image_data[i] =
            Pixel(buffer[3 * i + 0], buffer[3 * i + 1], buffer[3 * i + 2]);
}

// read
void Image::read_ppm(const std::string path) {
    std::ifstream file(path, std::ios::binary);
    if (!file)
        throw std::runtime_error("Impossible d'ouvrir " + path);
    // format
    std::string p;
    file >> p;
    if (p != "P6")
        throw std::runtime_error("Format PPM invalide");
    // com
    char c;
    file >> std::ws;
    while (file.peek() == '#') {
        std::string comment;
        std::getline(file, comment);
    }
    // val
    int max_val;
    file >> this->width >> this->height >> max_val;
    file.get();

    if (max_val != 255)
        throw std::runtime_error("PPM non supporté (max_val != 255)");

    this->nb_pixels = this->width * this->height;
    this->image_data.resize(this->nb_pixels);

    // lecture
    std::vector<unsigned char> buffer(this->nb_pixels * 3);
    file.read(reinterpret_cast<char *>(buffer.data()), buffer.size());
    if (file.gcount() != static_cast<std::streamsize>(buffer.size()))
        throw std::runtime_error("Erreur lecture données PPM");

    for (int i = 0; i < this->nb_pixels; ++i)
        this->image_data[i] =
            Pixel(buffer[3 * i + 0], buffer[3 * i + 1], buffer[3 * i + 2]);
}

void Image::read_stb(const std::string path) {
    int n;
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &n, 3);
    if (!data)
        throw std::runtime_error("Impossible de charger " + path);

    this->nb_pixels = this->width * this->height;
    this->image_data.resize(this->nb_pixels);

    for (int p = 0; p < this->nb_pixels; p++)
        this->image_data[p] =
            Pixel(data[3 * p + 0], data[3 * p + 1], data[3 * p + 2]);

    stbi_image_free(data);
}

// write
void Image::write_ppm(const std::string &path) const {
    std::ofstream file(path, std::ios::binary);
    if (!file)
        throw std::runtime_error("Impossible d'ouvrir " + path);

    // P6
    file << "P6\n" << width << " " << height << "\n255\n";

    // Pixels
    for (int p = 0; p < nb_pixels; p++) {
        file.put(image_data[p].r);
        file.put(image_data[p].g);
        file.put(image_data[p].b);
    }
}

// psnr
double Image::psnr(const Image &other) const {
    if (this->width != other.width || this->height != other.height)
        throw std::runtime_error("Images de tailles différentes");

    double mse = 0.0;
    for (int p = 0; p < nb_pixels; p++) {
        double dr =
            static_cast<double>(image_data[p].r) - other.image_data[p].r;
        double dg =
            static_cast<double>(image_data[p].g) - other.image_data[p].g;
        double db =
            static_cast<double>(image_data[p].b) - other.image_data[p].b;
        mse += (dr * dr + dg * dg + db * db) / 3.0;
    }
    mse /= nb_pixels;
    if (mse == 0.0)
        return 100.0;
    return 10.0 * log10(255.0 * 255.0 / mse);
}

// histogramme
void Image::histogram(const std::string name) const {
    std::map<unsigned char, int> histo_r, histo_g, histo_b;
    for (int i = 0; i < 256; i++) {
        histo_r[i] = 0;
        histo_g[i] = 0;
        histo_b[i] = 0;
    }

    for (const Pixel &p : image_data) {
        histo_r[p.r]++;
        histo_g[p.g]++;
        histo_b[p.b]++;
    }

    // csv
    std::string csv_file = "../assets/data/csv/" + name + ".csv";
    std::ofstream csv(csv_file);
    if (!csv)
        throw std::runtime_error("Impossible d'ouvrir " + csv_file);

    for (int i = 0; i < 256; ++i)
        csv << i << " " << histo_r[i] << " " << histo_g[i] << " " << histo_b[i]
            << "\n";
    csv.close();

    // png
    std::string png = "../assets/data/histo/" + name + ".png";

    // PNG global
    std::string cmd =
        "gnuplot -e \""
        "set terminal pngcairo size 800,600 enhanced font 'Arial,12'; "
        "set output '" +
        png +
        "'; "
        "set title 'Histogramme'; "
        "set xlabel 'Intensité'; "
        "set ylabel 'Nombre de pixels'; "
        "plot '" +
        csv_file +
        "' using 1:2 with lines lc rgb 'red' title 'R', "
        "'" +
        csv_file +
        "' using 1:3 with lines lc rgb 'green' title 'G', "
        "'" +
        csv_file + "' using 1:4 with lines lc rgb 'blue' title 'B'\"";

    if (std::system(cmd.c_str()) != 0)
        printf("Erreur lors de la génération de l'image avec Gnuplot\n");
}

void Image::histogram_one_channel(const std::string name, char channel) const {
    std::map<unsigned char, int> histo;

    // Initialisation
    for (int i = 0; i < 256; i++)
        histo[i] = 0;

    // Remplissage selon le canal
    for (const Pixel &p : image_data) {
        switch (channel) {
        case 'R':
            histo[p.r]++;
            break;
        case 'G':
            histo[p.g]++;
            break;
        case 'B':
            histo[p.b]++;
            break;
        default:
            throw std::invalid_argument("Canal invalide (R, G ou B)");
        }
    }

    // Définition couleur
    std::string couleur;
    switch (channel) {
    case 'R':
        couleur = "red";
        break;
    case 'G':
        couleur = "green";
        break;
    case 'B':
        couleur = "blue";
        break;
    }

    // CSV
    std::string csv_file = "../assets/data/csv/" + name + ".csv";
    std::ofstream csv(csv_file);
    if (!csv)
        throw std::runtime_error("Impossible d'ouvrir " + csv_file);

    for (int i = 0; i < 256; ++i)
        csv << i << " " << histo[i] << "\n";

    csv.close();

    // PNG
    std::string png = "../assets/data/histo/" + name + ".png";

    std::string cmd =
        "gnuplot -e \""
        "set terminal pngcairo size 800,600 enhanced font 'Arial,12'; "
        "set output '" +
        png +
        "'; "
        "set title 'Histogramme " +
        std::string(1, channel) +
        "'; "
        "set xlabel 'Intensité'; "
        "set ylabel 'Nombre de pixels'; "
        "plot '" +
        csv_file + "' using 1:2 with lines lc rgb '" + couleur + "' title '" +
        std::string(1, channel) + "'\"";

    if (std::system(cmd.c_str()) != 0)
        printf("Erreur lors de la génération avec Gnuplot\n");
}

// getter
const int Image::get_width() const { return this->width; }
const int Image::get_height() const { return this->height; }
const int Image::get_nb_pixels() const { return this->nb_pixels; }
const std::vector<Pixel> &Image::get_img() const { return this->image_data; }

std::vector<Pixel>::const_iterator Image::begin() const {
    return this->image_data.begin();
}
std::vector<Pixel>::const_iterator Image::end() const {
    return this->image_data.end();
}

void Image::set_path(const std::string &path) {
    if (path.size() >= 4 && path.substr(path.size() - 4) == ".ppm")
        read_ppm(path);
    else
        read_stb(path);
}