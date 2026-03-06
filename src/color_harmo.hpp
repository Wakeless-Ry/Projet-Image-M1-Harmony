#ifndef COLOR_HARMO_HPP
#define COLOR_HARMO_HPP

#include "template.hpp"
#include "image.hpp"

class Color_harmo
{
private:
    Template tmpl;
    Image img;

    double energie_1(int width, int height, const std::vector<Pixel>& pixels,
                                            const std::vector<int>& v) const;
    double energie_2(int width, int height, const std::vector<Pixel>& pixels,
                                            const std::vector<int>& v) const;
public:
    Color_harmo(const std::string& path);

    double F(Template& t) const;
    double bestOrientation(int steps, Template_format format) const; // M(X,Tm) dans le papier
    std::pair<Template_format, double> bestTemplate(int steps = 1000) const; // B(x)

    double distance_hue(double h1, double h2) const;
    double compute_energie(double lambda, const std::vector<int>& v) const;
};

#endif