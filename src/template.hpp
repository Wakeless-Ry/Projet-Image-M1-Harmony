#ifndef TEMPLATE
#define TEMPLATE

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
};

#endif