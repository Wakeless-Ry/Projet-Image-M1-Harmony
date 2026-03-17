#ifndef IMAGE_H
#define IMAGE_H

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

struct Pixel
{
 	unsigned char r = 0, g = 0, b = 0;

  	Pixel() = default;
  	Pixel(unsigned char _r, unsigned char _g, unsigned char _b)
      	: r(_r), g(_g), b(_b) {}

  	void toHSV(double &h, double &s, double &v) const
	{
    	double rf = r / 255.0;
    	double gf = g / 255.0;
    	double bf = b / 255.0;

    	double cmax = std::max({rf, gf, bf});
    	double cmin = std::min({rf, gf, bf});
    	double delta = cmax - cmin;

    	v = cmax;

    	s = (cmax == 0.0) ? 0.0 : delta / cmax;

    	if (delta == 0.0)
      		h = 0.0;
    	else if (cmax == rf)
      		h = M_PI / 3.0 * fmod((g - b) / delta + 6.0, 6.0);
    	else if (cmax == gf)
      		h = M_PI / 3.0 * ((bf - rf) / delta + 2.0);
    	else
      		h = M_PI / 3.0 * ((rf - gf) / delta + 4.0);

    	if (h < 0)
      	h += 2 * M_PI;
  	}

  static Pixel toRGB(double h, double s, double v)
  {
    double c = v * s;
    double x = c * (1.0 - std::abs(fmod(h / (M_PI / 3.0), 2.0) - 1.0));
    double m = v - c;

    double rf, gf, bf;
    if (h < 1 * M_PI / 3.0)
	{
      	rf = c;
      	gf = x;
      	bf = 0;
    } else if (h < 2 * M_PI / 3.0)
	{
      	rf = x;
      	gf = c;
      	bf = 0;
    } else if (h < 3 * M_PI / 3.0)
	{
      	rf = 0;
      	gf = c;
      	bf = x;
    } else if (h < 4 * M_PI / 3.0)
	{
      	rf = 0;
      	gf = x;
      	bf = c;
    } else if (h < 5 * M_PI / 3.0)
	{
      	rf = x;
      	gf = 0;
      	bf = c;
    } else
	{
      	rf = c;
      	gf = 0;
      	bf = x;
    }

    return Pixel((unsigned char)((rf + m) * 255.0),
                 (unsigned char)((gf + m) * 255.0),
                 (unsigned char)((bf + m) * 255.0));
  	}
};

class Image
{
	private:
  		std::vector<Pixel> image_data;
  		int width = 0;
  		int height = 0;
  		int nb_pixels = 0;

	public:
  		Image() = default;
  		Image(const std::string path);
  		Image(const std::vector<unsigned char> &buffer, int width, int height);
  		~Image() = default;
  		void read_ppm(const std::string path);
  		void read_stb(const std::string path);
  		void write_ppm(const std::string &path) const;

  		double psnr(const Image &other) const;
  		void histogram(const std::string name) const;
  		void histogram_one_channel(const std::string name, char channel) const;

  const int get_width() const;
  const int get_height() const;
  const int get_nb_pixels() const;
  const std::vector<Pixel> get_img() const;
};
  		const int get_width() const;
  		const int get_height() const;
  		const std::vector<Pixel>& get_img() const;

  		std::vector<Pixel>::const_iterator begin() const;
  		std::vector<Pixel>::const_iterator end() const;

  		void set_path(const std::string& path);
};	

#endif