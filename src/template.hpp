#ifndef TEMPLATE
#define TEMPLATE

#include "image.hpp"
#include <cmath>
#include <vector>

const double TEMPLATE_DEFAULT_S_WIDTH = M_PI / 6.0f;
const double TEMPLATE_DEFAULT_M_WIDTH = M_PI / 2.0f;
const double TEMPLATE_DEFAULT_L_WIDTH = M_PI;
const double TEMPLATE_DEFAULT_CENTER = 0.0f;

enum Template_format { i = 0, V = 1, L = 2, I = 3, T = 4, Y = 5, X = 6 };

class Template {
private:
  std::vector<double> centers;
  std::vector<double> widths;

  void autoCongru();

public:
  Template(std::vector<double> c = {}, std::vector<double> w = {});
  Template(double c = 0, double w = 0);
  Template(Template_format format);

  const int get_nbSector() const;
  const double get_center(int n) const;
  const double get_widths(int n) const;
  const std::vector<double> get_center() const;
  const std::vector<double> get_widths() const;

  void rotate(double angle);

  static double congru(double angle);

  double distanceToTemplate(double hue) const;
  bool isInsideSector(double hue, int sectorIndex) const;
  double F(const Image &image) const;
  double bestOrientation(const Image &image,
                         int steps = 1000) const; // M(X,Tm) dans le papier
  static std::pair<Template_format, double>
  bestTemplate(const Image &image); // B(X)
};

#endif