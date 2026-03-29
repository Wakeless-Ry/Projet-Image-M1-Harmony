#include "template.hpp"
#include "graph.h"
#include <omp.h>
#include <stdexcept>
#include <unordered_map>

Template::Template(std::vector<double> c, std::vector<double> w)
{
    if (c.size() != w.size())
        throw std::runtime_error(
            "Il doit y avoir autant de centre de secteur que "
            "de largeur de secteur !\n");

    centers.resize(c.size());
    widths.resize(w.size());

    for (int i = 0; i < c.size(); i++)
    {
        centers[i] = Template::congru(c[i]);
        widths[i] = std::abs(w[i]);
    }
}

Template::Template(double c, double w)
{
    centers = {Template::congru(c)};
    widths = {std::abs(w)};
}

Template::Template(Template_format format)
{
    switch (format)
    {
    case i: {
        centers = {TEMPLATE_DEFAULT_CENTER};
        widths = {TEMPLATE_DEFAULT_S_WIDTH};
        break;
    }
    case V:
    {
        centers = {TEMPLATE_DEFAULT_CENTER};
        widths = {TEMPLATE_DEFAULT_M_WIDTH};
        break;
    }
    case L:
    {
        centers = {TEMPLATE_DEFAULT_CENTER,
                   TEMPLATE_DEFAULT_CENTER - M_PI / 2.0};
        widths = {TEMPLATE_DEFAULT_S_WIDTH, TEMPLATE_DEFAULT_M_WIDTH};
        break;
    }
    case I:
    {
        centers = {TEMPLATE_DEFAULT_CENTER, TEMPLATE_DEFAULT_CENTER - M_PI};
        widths = {TEMPLATE_DEFAULT_S_WIDTH, TEMPLATE_DEFAULT_S_WIDTH};
        break;
    }
    case T:
    {
        centers = {TEMPLATE_DEFAULT_CENTER};
        widths = {TEMPLATE_DEFAULT_L_WIDTH};
        break;
    }
    case Y:
    {
        centers = {TEMPLATE_DEFAULT_CENTER, TEMPLATE_DEFAULT_CENTER - M_PI};
        widths = {TEMPLATE_DEFAULT_M_WIDTH, TEMPLATE_DEFAULT_S_WIDTH};
        break;
    }
    case X:
    {
        centers = {TEMPLATE_DEFAULT_CENTER, TEMPLATE_DEFAULT_CENTER - M_PI};
        widths = {TEMPLATE_DEFAULT_M_WIDTH, TEMPLATE_DEFAULT_M_WIDTH};
        break;
    }
    }
    autoCongru();
}

// getters
const int Template::get_nbSector() const { return centers.size(); }
const double Template::get_center(int n) const { return centers[n]; }
const double Template::get_widths(int n) const { return widths[n]; }
const std::vector<double> Template::get_center() const { return centers; }
const std::vector<double> Template::get_widths() const { return widths; }

void Template::autoCongru()
{
    for (int i = 0; i < centers.size(); i++)
    {
        centers[i] = congru(centers[i]);
        widths[i] = congru(widths[i]);
    }
}

double Template::congru(double angle)
{
    double pi2 = 2 * M_PI;
    double rest = angle - double(int(angle / pi2)) * pi2;
    return (rest > M_PI) ? (rest - pi2) : ((rest <= -M_PI) ? (rest + pi2) : rest);
}

void Template::rotate(double angle)
{
    for (int i = 0; i < centers.size(); i++)
        centers[i] = Template::congru(centers[i] + angle);
}

bool Template::isInsideSector(double hue, int n) const
{
    double diff = std::abs(congru(hue - centers[n]));
    return diff <= widths[n] / 2.0;
}

double Template::distanceToTemplate(double hue) const
{
    double min_dist = 2 * M_PI;

    for (int n = 0; n < get_nbSector(); n++)
    {
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
void Template::set_image(std::string path) { this->img.set_path(path); }
const std::vector<Pixel> &Template::get_img() const { return this->img.get_img(); }
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

double Template::bestOrientation() const
{
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
                p < q * (b - x))
                {
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
            if (fu <= fw || w == x)
            {
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

std::pair<Template_format, double> Template::bestTemplate() const
{
    Template_format best_format = i;
    double best_angle = 0.0;
    double best_F = std::numeric_limits<double>::max();

    for (int ite = 0; ite <= 6; ite++)
    {
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
// 4.0
double Template::e1(const std::vector<int>& labels, const std::vector<int>& pixel_indices) const
{
    const auto& pixels = img.get_img();
    double sum = 0.0;

    for (int p_i = 0; p_i < (int)pixel_indices.size(); p_i++)
    {
        double h, s, v;
        pixels[pixel_indices[p_i]].toHSV(h, s, v);
        double border_hue = 0;
        if (labels[p_i] == 0)
            border_hue = theta_1[pixel_indices[p_i]];
        else
            border_hue = theta_2[pixel_indices[p_i]];
        sum += std::abs(congru(h - border_hue)) * s;
    }
    return sum;
}


double Template::e2(const std::vector<int>& labels, const std::vector<int>& pixel_indices) const
{
    const auto& pixels = img.get_img();
    int img_width = img.get_width();

    std::unordered_map<int, int> idx_map;
    for (int i = 0; i < (int)pixel_indices.size(); i++)
        idx_map[pixel_indices[i]] = i;

    const int dx[] = {1, 0};
    const int dy[] = {0, 1};

    double E2_sum = 0.0;

    for (int pixel_idx = 0; pixel_idx < (int)pixel_indices.size(); pixel_idx++)
    {
        int flat_p = pixel_indices[pixel_idx];
        int col_p = flat_p % img_width;
        int row_p = flat_p / img_width;

        double H_p, S_p, V_p;
        pixels[flat_p].toHSV(H_p, S_p, V_p);
        H_p = congru(H_p);

        for (int d = 0; d < 2; d++)
        {
            int flat_q = (row_p + dy[d]) * img_width + (col_p + dx[d]);

            auto it = idx_map.find(flat_q);
            if (it == idx_map.end())
                continue;

            int j = it->second;

            if (labels[pixel_idx] == labels[j])
                continue;

            double H_q, S_q, V_q;
            pixels[flat_q].toHSV(H_q, S_q, V_q);
            H_q = congru(H_q);

            double S_max = std::max(S_p, S_q);
            double hue_dist = std::abs(congru(H_p - H_q));
            double w_pq = S_max / (hue_dist + 1e-8);

            E2_sum += w_pq;
        }
    }
    return E2_sum;
}

double Template::e(const std::vector<int>& labels, const std::vector<int>& pixel_indices, double lambda) const
{
    return lambda * e1(labels, pixel_indices) + e2(labels, pixel_indices);
}

void Template::compute_thetas()
{
    const auto& pixels = img.get_img();
    int N = (int)pixels.size();

    theta_1.assign(N, 0.0);
    theta_2.assign(N, 0.0);
    gap_left.assign(N, 0.0);
    gap_right.assign(N, 0.0);

    std::vector<double> borders;
    for (int k = 0; k < get_nbSector(); k++)
    {
        borders.push_back(congru(centers[k] - widths[k] / 2.0));
        borders.push_back(congru(centers[k] + widths[k] / 2.0));
    }
    std::sort(borders.begin(), borders.end());
    int boder_size = (int)borders.size();

    for (int i = 0; i < N; i++) {
        double h, s, v;
        pixels[i].toHSV(h, s, v);
        h = congru(h);

        int idx = (int)(std::lower_bound(borders.begin(), borders.end(), h) - borders.begin());

        double t1 = borders[(idx - 1 + boder_size) % boder_size];
        double t2 = borders[idx % boder_size];

        theta_1[i] = t1;
        theta_2[i] = t2;
        gap_left[i] = t1;
        gap_right[i] = t2;
    }
}

void Template::compute_labels(double lambda)
{
    compute_thetas();

    const auto& pixels = img.get_img();
    int pixel_size = (int)pixels.size();
    int img_width = img.get_width();

    std::vector<int> non_fixed;
    non_fixed.reserve(pixel_size);
    for (int i = 0; i < pixel_size; i++)
    {
        double h, s, v;
        pixels[i].toHSV(h, s, v);
        h = congru(h);
        bool fixed = false;
        for (int nb_sectors = 0; nb_sectors < get_nbSector(); nb_sectors++)
            if (isInsideSector(h, nb_sectors))
            {
                fixed = true;
                break;
            }
        if (!fixed)
            non_fixed.push_back(i);
    }

    int non_fixed_size = (int)non_fixed.size();
    if (non_fixed_size == 0)
    { 
        pixel_label.assign(pixel_size, -1);
        return; 
    }

    std::unordered_map<int,int> local_id;
    local_id.reserve(non_fixed_size * 2);
    for (int i = 0; i < non_fixed_size; i++)
        local_id[non_fixed[i]] = i;

    Graph<double,double,double> graph_cut(non_fixed_size, non_fixed_size * 2);
    graph_cut.add_node(non_fixed_size);

    for (int i = 0; i < non_fixed_size; i++)
    {
        double h, s, val;
        pixels[non_fixed[i]].toHSV(h, s, val);
        h = congru(h);
        double dist_1 = std::abs(congru(h - gap_left[non_fixed[i]]))  * s;
        double dist_2 = std::abs(congru(h - gap_right[non_fixed[i]])) * s;
        graph_cut.add_tweights(i, lambda * dist_2, lambda * dist_1);
    }

    const int dx[] = {1, 0};
    const int dy[] = {0, 1};
    int edge_count = 0;
    for (int i = 0; i < non_fixed_size; i++)
    {
        int flat_p = non_fixed[i];
        int col_p = flat_p  % img_width;
        int row_p = flat_p  / img_width;

        double h_p, s_p, v_p;
        pixels[flat_p ].toHSV(h_p, s_p, v_p);
        h_p = congru(h_p);

        for (int d = 0; d < 2; d++)
        {
            int flat_q  = (row_p + dy[d]) * img_width + (col_p + dx[d]);
            if (flat_q  < 0 || flat_q >= pixel_size)
                continue;

            auto it = local_id.find(flat_q );
            if (it == local_id.end())
                continue;
            int j = it->second;

            double h_q, s_q, v_q;
            pixels[flat_q ].toHSV(h_q, s_q, v_q);
            h_q = congru(h_q);

            double smax = std::max(s_p, s_q);
            double h_dist = std::abs(congru(h_p - h_q));
            double w_pq = std::min(smax / (h_dist + 1e-8), 5.0 * lambda);

            graph_cut.add_edge(i, j, w_pq, w_pq);
            edge_count++;
        }
    }
    graph_cut.maxflow();

    pixel_label.assign(pixel_size, -1);
    for (int i = 0; i < non_fixed_size; i++)
    {
        int label = 0;
        if (graph_cut.what_segment(i) == Graph<double,double,double>::SINK)
            label = 1;
        else
            label = 0;
        pixel_label[non_fixed[i]] = label;
    }
}

// 4.1
std::vector<Pixel> Template::shift_hues(double sigma_factor) const
{
    const auto& pixels = img.get_img();
    int nb_pixels = pixels.size();
    std::vector<Pixel> result;
    int nb_sectors = get_nbSector();
    result.reserve(nb_pixels);
    const double pi2 = M_PI*2.0;

    for (int i = 0; i < nb_pixels; i++)
    {
        double h, s, v;
        pixels[i].toHSV(h, s, v);
        h = congru(h);
        bool isInside = false;

        int index = -1;
        for (int sector = 0; sector < nb_sectors; sector++)
            if (isInsideSector(h, sector))
            { 
                index = sector;
                isInside = true;
                break;
            }
        if (!isInside)
        {
            int label = pixel_label[i];
            double target_border = (label == 0) ? gap_left[i] : gap_right[i];
            for (int sector = 0; sector < nb_sectors; sector++)
            {
                double border_tested = centers[sector] + (label==0 ? 1 : -1)*widths[sector] / 2.0;
                double border_dist = congru(target_border - border_tested);
                if (border_dist == 0)
                {
                    index = sector;
                    break;
                }
            }
        }

        double C = centers[index];
        double w = widths[index];
        double sigma = sigma_factor * w;
        double d = congru(C-h);
        double sens = (nb_sectors==1 && !isInside) ? (pixel_label[i]==0 ? 1.0 : -1.0) : (d<0 ? 1.0 : -1.0);
        double gauss = exp(-d*d / (sigma*sigma*2.0));
        double h2  = congru(C + sens * (w / 2.0) * (1.0 - gauss));
        h2 += h2>0 ? 0 : pi2;

        result.push_back(Pixel::toRGB(h2, s, v));
    }

    return result;
}