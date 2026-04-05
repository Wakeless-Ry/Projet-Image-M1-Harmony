#include "harmonization.hpp"

Harmonization::Harmonization(double lambda, double sigma)
    : lambda(lambda), sigma(sigma), tmpl(std::vector<double>{}, std::vector<double>{}) {}

void Harmonization::set_image(const std::string& path)
{
    img_path = path;
    tmpl = Template(std::vector<double>{}, std::vector<double>{});
    tmpl.set_image(path);
}

void Harmonization::compute_best_template(double& angle, Template_format& format)
{
    auto [f, a] = tmpl.bestTemplate();
    format = f;
    angle = a;
    tmpl = Template(format);
    tmpl.set_image(img_path);
    tmpl.rotate(angle);
}

void Harmonization::build_graph() { tmpl.build_graph(); }
void Harmonization::solve_graph() { tmpl.solve_graph(this->lambda); }

std::vector<Pixel> Harmonization::shift_hues()
{
    return tmpl.shift_hues(sigma);
}

void Harmonization::new_template(double angle, Template_format fmt)
{
    Template new_tmpl = Template(fmt);
    new_tmpl.set_image(img_path);
    new_tmpl.rotate(angle);
    new_tmpl.build_graph();
    this->tmpl = new_tmpl;
}

double Harmonization::get_lambda() const { return lambda; }
double Harmonization::get_sigma()  const { return sigma;  }
int Harmonization::get_format() const { return (int)format; }
double Harmonization::get_angle()  const { return angle; }

void Harmonization::set_lambda(double l) { lambda = l; }
void Harmonization::set_sigma(double s)  { sigma  = s; }