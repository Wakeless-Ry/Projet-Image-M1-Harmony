#ifndef IMAGE_H
#define IMAGE_H

#include <vector>
#include <string>

struct Pixel
{
    unsigned char r = 0, g = 0, b = 0;

    Pixel() = default;
    Pixel(unsigned char _r, unsigned char _g, unsigned char _b) : r(_r), g(_g), b(_b) {}
};

class Image 
{
    private :
        std::vector<Pixel> image_data;
        int width = 0;
        int height = 0;
        int nb_pixels = 0;
    
    public :
        Image(const std::string path);
        Image(const std::vector<unsigned char> &buffer, int width, int height);
        ~Image() = default;
        void read_ppm(const std::string path);
        void read_stb(const std::string path);
        void write_ppm(const std::string& path) const;

        double psnr(const Image& other) const;
        void histogram(const std::string name) const;
        void histogram_one_channel(const std::string name, char channel) const;

        const int get_width() const; 
        const int get_height() const;
        const std::vector<Pixel> get_img() const;
};

#endif