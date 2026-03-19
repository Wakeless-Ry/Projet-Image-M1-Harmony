#include "harmonization.hpp"

Harmonization::Harmonization(double lambda, double sigma)
    : lambda(lambda), sigma(sigma), tmpl(std::vector<double>{}, std::vector<double>{}) {}

void Harmonization::set_image(const std::string& path)
{
    img_path = path;
    tmpl = Template(std::vector<double>{}, std::vector<double>{});
    tmpl.set_image(path);
}

void Harmonization::compute_best_template()
{
    auto [f, a] = tmpl.bestTemplate();
    format = f;
    angle = a;

    tmpl = Template(format);
    tmpl.set_image(img_path);
    tmpl.rotate(angle);
}

void Harmonization::compute_labels()
{
    tmpl.compute_labels(lambda);
}

std::vector<Pixel> Harmonization::shift_hues()
{
    return tmpl.shift_hues(sigma);
}

double Harmonization::get_lambda() const { return lambda; }
double Harmonization::get_sigma()  const { return sigma;  }
int Harmonization::get_format() const { return (int)format; }
double Harmonization::get_angle()  const { return angle; }

void Harmonization::set_lambda(double l) { lambda = l; }
void Harmonization::set_sigma(double s)  { sigma  = s; }