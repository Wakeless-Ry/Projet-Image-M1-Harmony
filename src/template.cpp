#include "template.hpp"
#include <stdexcept>
Template::Template(std::vector<double> c, std::vector<double> w) {
  if (c.size() != w.size())
    throw std::runtime_error("Il doit y avoir autant de centre de secteur que "
                             "de largeur de secteur !\n");

  centers.resize(c.size());
  widths.resize(w.size());

  for (int i = 0; i < c.size(); i++) {
    centers[i] = Template::congru(c[i]);
    widths[i] = std::abs(w[i]);
  }
}

Template::Template(double c, double w) {
  centers = {Template::congru(c)};
  widths = {std::abs(w)};
}

Template::Template(Template_format format) {
  switch (format) {
  case i: {
    centers = {TEMPLATE_DEFAULT_CENTER};
    widths = {TEMPLATE_DEFAULT_S_WIDTH};
    break;
  }
  case V: {
    centers = {TEMPLATE_DEFAULT_CENTER};
    widths = {TEMPLATE_DEFAULT_M_WIDTH};
    break;
  }
  case L: {
    centers = {TEMPLATE_DEFAULT_CENTER, TEMPLATE_DEFAULT_CENTER - M_PI / 2.0};
    widths = {TEMPLATE_DEFAULT_S_WIDTH, TEMPLATE_DEFAULT_M_WIDTH};
    break;
  }
  case I: {
    centers = {TEMPLATE_DEFAULT_CENTER, TEMPLATE_DEFAULT_CENTER - M_PI};
    widths = {TEMPLATE_DEFAULT_S_WIDTH, TEMPLATE_DEFAULT_S_WIDTH};
    break;
  }
  case T: {
    centers = {TEMPLATE_DEFAULT_CENTER};
    widths = {TEMPLATE_DEFAULT_L_WIDTH};
    break;
  }
  case Y: {
    centers = {TEMPLATE_DEFAULT_CENTER, TEMPLATE_DEFAULT_CENTER - M_PI};
    widths = {TEMPLATE_DEFAULT_M_WIDTH, TEMPLATE_DEFAULT_S_WIDTH};
    break;
  }
  case X: {
    centers = {TEMPLATE_DEFAULT_CENTER, TEMPLATE_DEFAULT_CENTER - M_PI};
    widths = {TEMPLATE_DEFAULT_M_WIDTH, TEMPLATE_DEFAULT_M_WIDTH};
    break;
  }
  }
}

// getters
const int Template::get_nbSector() const { return centers.size(); }
const double Template::get_center(int n) const { return centers[n]; }
const double Template::get_widths(int n) const { return widths[n]; }
const std::vector<double> Template::get_center() const { return centers; }
const std::vector<double> Template::get_widths() const { return widths; }

void Template::autoCongru() {
  for (int i = 0; i < centers.size(); i++) {
    centers[i] = congru(centers[i]);
    widths[i] = congru(widths[i]);
  }
}

double Template::congru(double angle) {
  double pi2 = 2 * M_PI;
  double rest = angle - double(int(angle / pi2)) * pi2;
  return (rest > M_PI) * (rest - pi2) + (rest <= -M_PI) * (rest + pi2) +
         (rest <= M_PI && rest > -M_PI) * rest;
}

void Template::rotate(double angle) {
  for (int i = 0; i < centers.size(); i++)
    centers[i] = Template::congru(centers[i] + angle);
}

bool Template::isInsideSector(double hue, int n) const {
  double diff = std::abs(congru(hue - centers[n]));
  return diff <= widths[n] / 2.0;
}

double Template::distanceToTemplate(double hue) const {
  double minDist = 2 * M_PI;

  for (int n = 0; n < get_nbSector(); n++) {
    if (isInsideSector(hue, n))
      return 0.0;

    double LeftBorder = centers[n] - widths[n] / 2.0;
    double RightBorder = centers[n] + widths[n] / 2.0;

    double distLeft = std::abs(congru(hue - LeftBorder));
    double distRight = std::abs(congru(hue - RightBorder));

    minDist = std::min(minDist, std::min(distLeft, distRight));
  }

  return minDist;
}

double Template::F(const Image &image) const {
  double total = 0.0;
  for (const Pixel &p : image.get_img()) {
    double h, s, v;
    p.toHSV(h, s, v);
    total += distanceToTemplate(h) * s;
  }
  return total;
}

double Template::bestOrientation(const Image &image, int steps) const {
  double best_angle = 0.0;
  double best_F = std::numeric_limits<double>::max();

  for (int i = 0; i < steps; i++) {
    double angle = 2.0 * M_PI * i / steps;

    Template t = *this;
    t.rotate(angle);

    double f = t.F(image);
    if (f < best_F) {
      best_F = f;
      best_angle = angle;
    }
  }

  return best_angle;
}

std::pair<Template_format, double> Template::bestTemplate(const Image &image) {
  Template_format best_format = i;
  double best_angle = 0.0;
  double best_F = std::numeric_limits<double>::max();

  for (int ite = 0; ite <= 6; ite++) {
    Template t((Template_format)ite);
    double angle = t.bestOrientation(image);

    Template t2((Template_format)ite);
    t2.rotate(angle);
    double f = t2.F(image);

    if (f < best_F) {
      best_F = f;
      best_angle = angle;
      best_format = (Template_format)ite;
    }
  }

  return {best_format, best_angle};
}