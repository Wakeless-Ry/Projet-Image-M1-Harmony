#include "template.hpp"
#include "graph.h"
#include <omp.h>
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
  double min_dist = 2 * M_PI;

  for (int n = 0; n < get_nbSector(); n++) {
    if (isInsideSector(hue, n))
      return 0.0;

    double LeftBorder = centers[n] - widths[n] / 2.0;
    double RightBorder = centers[n] + widths[n] / 2.0;

    double distLeft = std::abs(congru(hue - LeftBorder));
    double distRight = std::abs(congru(hue - RightBorder));

    min_dist = std::min(min_dist, std::min(distLeft, distRight));
  }

  return min_dist;
}

double Template::distance_hue(double h1, double h2) const {
  double d = std::fabs(h1 - h2);

  if (d > M_PI)
    d = 2 * M_PI - d;

  return d;
}
// 3.0
double Template::F() const {
  double total = 0.0;
  for (const Pixel &p : this->img.get_img()) {
    double h, s, v;
    p.toHSV(h, s, v);
    total += distanceToTemplate(h) * s;
  }
  return total;
}
double Template::bestOrientation() const {
  const double golden = 0.3819660;
  const double tol = 1e-4;
  double a = 0.0, b = 2 * M_PI;

  double x = a + golden * (b - a);
  double w = x, v = x;

  Template tx = *this;
  tx.rotate(x);
  double fx = tx.F();
  double fw = fx, fv = fx;

  double d = 0.0, e = 0.0;

  for (int iter = 0; iter < 100; iter++) {
    double midpoint = 0.5 * (a + b);
    double tol1 = tol * std::abs(x) + 1e-10;
    double tol2 = 2.0 * tol1;

    if (std::abs(x - midpoint) <= tol2 - 0.5 * (b - a))
      break;

    bool parabolic_ok = false;
    double u;

    if (std::abs(e) > tol1) {
      double r = (x - w) * (fx - fv);
      double q = (x - v) * (fx - fw);
      double p = (x - v) * q - (x - w) * r;
      q = 2.0 * (q - r);
      if (q > 0)
        p = -p;
      q = std::abs(q);
      r = e;
      e = d;

      if (std::abs(p) < std::abs(0.5 * q * r) && p > q * (a - x) &&
          p < q * (b - x)) {
        d = p / q;
        u = x + d;
        parabolic_ok = true;
      }
    }

    if (!parabolic_ok) {
      e = (x < midpoint) ? b - x : a - x;
      d = golden * e;
    }

    u = x + d;
    Template tu = *this;
    tu.rotate(u);
    double fu = tu.F();

    if (fu <= fx) {
      if (u < x)
        b = x;
      else
        a = x;
      v = w;
      fv = fw;
      w = x;
      fw = fx;
      x = u;
      fx = fu;
    } else {
      if (u < x)
        a = u;
      else
        b = u;
      if (fu <= fw || w == x) {
        v = w;
        fv = fw;
        w = u;
        fw = fu;
      } else if (fu <= fv || v == x || v == w) {
        v = u;
        fv = fu;
      }
    }
  }

  return x;
}

std::pair<Template_format, double> Template::bestTemplate() const {
  Template_format best_format = i;
  double best_angle = 0.0;
  double best_F = std::numeric_limits<double>::max();

  for (int ite = 0; ite <= 6; ite++) {
    Template t((Template_format)ite);
    t.img = this->img;
    double angle = t.bestOrientation();

    Template t2((Template_format)ite);
    t2.img = this->img;
    t2.rotate(angle);
    double f = t2.F();

    if (f < best_F) {
      best_F = f;
      best_angle = angle;
      best_format = (Template_format)ite;
    }
  }

  return {best_format, best_angle};
}
// 4.1
double Template::energie_1(int width, int height,
                           const std::vector<Pixel> &pixels,
                           const std::vector<int> &v) const {
  double e1 = 0.0;

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      int idx = y * width + x;
      double h, s, val;
      pixels[idx].toHSV(h, s, val);

      double theta_1 = 0.0;
      double theta_2 = 0.0;
      double min_dist_1 = 2 * M_PI;
      double min_dist_2 = 2 * M_PI;

      for (int n = 0; n < get_nbSector(); n++) {
        double left = get_center(n) - get_widths(n) / 2.0;
        double right = get_center(n) + get_widths(n) / 2.0;

        double d_left = congru(h - left);
        double d_right = congru(h - right);

        if (d_left > 0 && d_left < min_dist_1) {
          min_dist_1 = d_left;
          theta_1 = left;
        }
        if (d_right < 0 && -d_right < min_dist_2) {
          min_dist_2 = -d_right;
          theta_2 = right;
        }
      }

      double h_border = (v[idx] == 1) ? theta_2 : theta_1;

      e1 += distance_hue(h, h_border) * s;
    }
  }
  return e1;
}

double Template::energie_2(int width, int height,
                           const std::vector<Pixel> &pixels,
                           const std::vector<int> &v) const {
  double e2 = 0.0;
  const int dx[4] = {1, 0, 1, 1};
  const int dy[4] = {0, 1, 1, -1};

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      int idx = y * width + x;

      for (int k = 0; k < 4; k++) {
        int nx = x + dx[k];
        int ny = y + dy[k];

        if (nx < 0 || nx >= width || ny < 0 || ny >= height)
          continue;

        int nidx = ny * width + nx;

        if (v[idx] == v[nidx])
          continue;

        double h1, s1, val1;
        double h2, s2, val2;
        pixels[idx].toHSV(h1, s1, val1);
        pixels[nidx].toHSV(h2, s2, val2);

        double smax = std::max(s1, s2);
        double dist = distance_hue(h1, h2);

        if (dist > 1e-6)
          e2 += smax / dist;
      }
    }
  }
  return e2;
}

double Template::compute_energie(double lambda,
                                 const std::vector<int> &v) const {
  const std::vector<Pixel> &pixels = img.get_img();
  int width = img.get_width();
  int height = img.get_height();

  double e1 = energie_1(width, height, pixels, v);
  double e2 = energie_2(width, height, pixels, v);

  return lambda * e1 + e2;
}

void Template::set_image(std::string path) { this->img.set_path(path); }

void Template::compute_thetas(const std::vector<Pixel> &pixels,
                              std::vector<double> &theta_1,
                              std::vector<double> &theta2,
                              std::vector<bool> &is_fixed,
                              std::vector<int> &v) const {
  int n_pixels = pixels.size();
  theta_1.resize(n_pixels);
  theta2.resize(n_pixels);
  is_fixed.resize(n_pixels, false);
  v.resize(n_pixels, 1);

  for (int idx = 0; idx < n_pixels; idx++) {
    double h, s, val;
    pixels[idx].toHSV(h, s, val);

    bool inside = false;
    for (int n = 0; n < get_nbSector(); n++)
      if (isInsideSector(h, n)) {
        inside = true;
        break;
      }
    if (inside) {
      is_fixed[idx] = true;
      int sector = 0;
      double min_d = distance_hue(h, get_center(0));
      for (int n = 1; n < get_nbSector(); n++) {
        double d = distance_hue(h, get_center(n));
        if (d < min_d) {
          min_d = d;
          sector = n;
        }
      }
      v[idx] = (congru(h - get_center(sector)) < 0) ? -1 : 1;
      continue;
    }

    double min_dist_1 = 2 * M_PI;
    double min_dist_2 = 2 * M_PI;

    for (int n = 0; n < get_nbSector(); n++) {
      double left = get_center(n) - get_widths(n) / 2.0;
      double right = get_center(n) + get_widths(n) / 2.0;

      double d_left = congru(h - left);
      double d_right = congru(h - right);

      if (d_left > 0 && d_left < min_dist_1) {
        min_dist_1 = d_left;
        theta_1[idx] = left;
      }

      if (d_right < 0 && -d_right < min_dist_2) {
        min_dist_2 = -d_right;
        theta2[idx] = right;
      }
    }
  }
}

void Template::run_graphcut(const std::vector<Pixel> &pixels,
                            const std::vector<double> &theta1,
                            const std::vector<double> &theta2,
                            const std::vector<bool> &is_fixed, double lambda,
                            std::vector<int> &v) const {
  int width = img.get_width();
  int height = img.get_height();
  int n_pixels = pixels.size();

  typedef Graph<double, double, double> GraphType;
  GraphType *g = new GraphType(n_pixels, n_pixels * 4);
  g->add_node(n_pixels);

  for (int idx = 0; idx < n_pixels; idx++) {
    double h, s, val;
    pixels[idx].toHSV(h, s, val);

    if (is_fixed[idx]) {
      double INF = 1e10;
      if (v[idx] == -1)
        g->add_tweights(idx, INF, 0);
      else
        g->add_tweights(idx, 0, INF);
      continue;
    }

    double cost_plus1 = lambda * distance_hue(h, theta2[idx]) * s;
    double cost_minus1 = lambda * distance_hue(h, theta1[idx]) * s;

    g->add_tweights(idx, cost_plus1, cost_minus1);
  }

  const int dx[4] = {1, 0, 1, 1};
  const int dy[4] = {0, 1, 1, -1};

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      int idx = y * width + x;

      for (int k = 0; k < 4; k++) {
        int nx = x + dx[k];
        int ny = y + dy[k];

        if (nx < 0 || nx >= width || ny < 0 || ny >= height)
          continue;

        int nidx = ny * width + nx;

        double h1, s1, val1;
        double h2, s2, val2;
        pixels[idx].toHSV(h1, s1, val1);
        pixels[nidx].toHSV(h2, s2, val2);

        double smax = std::max(s1, s2);
        double dist = distance_hue(h1, h2);

        double w_pq = (dist > 1e-6) ? smax / dist : 1e6;

        g->add_edge(idx, nidx, w_pq, w_pq);
      }
    }
  }

  g->maxflow();

  for (int idx = 0; idx < n_pixels; idx++) {
    if (!is_fixed[idx])
      v[idx] = (g->what_segment(idx) == GraphType::SOURCE) ? -1 : 1;
  }

  delete g;
}
const std::vector<Pixel> &Template::get_img() const {
  return this->img.get_img();
}