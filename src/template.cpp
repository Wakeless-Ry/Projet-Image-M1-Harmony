#include "template.hpp"
#include "graph.h"
#include "image.hpp"
#include <omp.h>
#include <stdexcept>
#include <unordered_map>
#include <iostream>

Template::Template(std::vector<double> c, std::vector<double> w) : format(Template_format::i)
{
    if (c.size() != w.size())
        throw std::runtime_error(
            "Il doit y avoir autant de centre de secteur que "
            "de largeur de secteur !\n");

    centers.resize(c.size());
    widths.resize(w.size());

    for (int i = 0; i < c.size(); i++) {
        centers[i] = Template::congru(c[i]);
        widths[i] = std::abs(w[i]);
    }
}

Template::Template(double c, double w) : format(Template_format::i)
{
    centers = {Template::congru(c)};
    widths = {std::abs(w)};
}

Template::Template(Template_format format) : format(format)
{
    switch (format)
    {
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
        centers = {TEMPLATE_DEFAULT_CENTER,
                   TEMPLATE_DEFAULT_CENTER - M_PI * 0.5};
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
    case t:
    {
        centers = {TEMPLATE_DEFAULT_CENTER, TEMPLATE_DEFAULT_CENTER - M_PI*2.0/3.0, TEMPLATE_DEFAULT_CENTER + M_PI*2.0/3.0};
        widths = {TEMPLATE_DEFAULT_S_WIDTH, TEMPLATE_DEFAULT_S_WIDTH, TEMPLATE_DEFAULT_S_WIDTH};
        break;
    }
    case q:
    {
        centers = {TEMPLATE_DEFAULT_CENTER, TEMPLATE_DEFAULT_CENTER - M_PI*0.5, TEMPLATE_DEFAULT_CENTER + M_PI, TEMPLATE_DEFAULT_CENTER + M_PI*0.5};
        widths = {TEMPLATE_DEFAULT_S_WIDTH, TEMPLATE_DEFAULT_S_WIDTH, TEMPLATE_DEFAULT_S_WIDTH, TEMPLATE_DEFAULT_S_WIDTH};
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
const Template_format Template::get_format() const { return this->format; }
void Template::autoCongru()
{
    for (int i = 0; i < centers.size(); i++)
    {
        centers[i] = congru(centers[i]);
        widths[i] = congru(widths[i]);
    }
}

double Template::congru(double angle) {
    double pi2 = 2 * M_PI;
    double rest = angle - double(int(angle / pi2)) * pi2;
    return (rest > M_PI) ? (rest - pi2)
                         : ((rest <= -M_PI) ? (rest + pi2) : rest);
}

void Template::setWidths(double w) { for (int i=0 ; i<widths.size() ; i++) widths[i] = w; demi_sec_is_computed=false;}
void Template::setWidths(std::vector<double> w)
{
    if (widths.size() != w.size())
        throw std::runtime_error("Le nombre de largeurs de secteurs ne doit pas changer !\n");
    for (int i=0 ; i<widths.size() ; i++) widths[i] = w[i];
    demi_sec_is_computed=false;
}

void Template::rotate(double angle)
{
    for (int i = 0; i < centers.size(); i++)
        centers[i] = Template::congru(centers[i] + angle);
    demi_sec_is_computed=false;
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
void Template::set_image(std::string path) { this->img.set_path(path); demi_sec_is_computed=false;}
void Template::set_image_v2(std::vector<unsigned char> data_tmp, int height, int width) { this->img = Image(data_tmp, width, height); demi_sec_is_computed=false;}
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
// 4.0
double Template::e1(const std::vector<int> &labels,
                    const std::vector<int> &pixel_indices) const {
    const auto &pixels = img.get_img();
    double sum = 0.0;

    for (int p_i = 0; p_i < (int)pixel_indices.size(); p_i++) {
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

double Template::e2(const std::vector<int> &labels,
                    const std::vector<int> &pixel_indices) const {
    const auto &pixels = img.get_img();
    int img_width = img.get_width();

    std::unordered_map<int, int> idx_map;
    for (int i = 0; i < (int)pixel_indices.size(); i++)
        idx_map[pixel_indices[i]] = i;

    const int dx[] = {1, 0};
    const int dy[] = {0, 1};

    double E2_sum = 0.0;

    for (int pixel_idx = 0; pixel_idx < (int)pixel_indices.size();
         pixel_idx++) {
        int flat_p = pixel_indices[pixel_idx];
        int col_p = flat_p % img_width;
        int row_p = flat_p / img_width;

        double H_p, S_p, V_p;
        pixels[flat_p].toHSV(H_p, S_p, V_p);
        H_p = congru(H_p);

        for (int d = 0; d < 2; d++) {
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

double Template::e(const std::vector<int> &labels,
                   const std::vector<int> &pixel_indices, double lambda) const {
    return lambda * e1(labels, pixel_indices) + e2(labels, pixel_indices);
}

void Template::compute_thetas() {
    demi_sec_is_computed=false;
    const auto &pixels = img.get_img();
    int N = (int)pixels.size();

    theta_1.assign(N, 0.0);
    theta_2.assign(N, 0.0);
    gap_left.assign(N, 0.0);
    gap_right.assign(N, 0.0);

    std::vector<double> borders;
    for (int k = 0; k < get_nbSector(); k++) {
        borders.push_back(congru(centers[k] - widths[k] / 2.0));
        borders.push_back(congru(centers[k] + widths[k] / 2.0));
    }
    std::sort(borders.begin(), borders.end());
    int boder_size = (int)borders.size();

    for (int i = 0; i < N; i++) {
        double h, s, v;
        pixels[i].toHSV(h, s, v);
        h = congru(h);

        int idx = (int)(std::lower_bound(borders.begin(), borders.end(), h) -
                        borders.begin());

        double t1 = borders[(idx - 1 + boder_size) % boder_size];
        double t2 = borders[idx % boder_size];

        theta_1[i] = t1;
        theta_2[i] = t2;
        gap_left[i] = t1;
        gap_right[i] = t2;
    }
}

SharedGraph Template::build_graph()
{
    compute_thetas();

    const auto &pixels = img.get_img();
    int pixel_size = (int)pixels.size();
    int img_width = img.get_width();

    SharedGraph g;

    g.non_fixed.reserve(pixel_size);
    std::vector<std::vector<int>> local_buffers(omp_get_max_threads());

    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        auto& buffer = local_buffers[tid];

        #pragma omp for nowait
        for (int i = 0; i < pixel_size; i++)
        {
            double h, s, v;
            pixels[i].toHSV(h, s, v);
            h = congru(h);

            bool fixed = false;
            for (int nb_sectors = 0; nb_sectors < get_nbSector(); nb_sectors++)
            {
                if (isInsideSector(h, nb_sectors))
                {
                    fixed = true;
                    break;
                }
            }

            if (!fixed)
                buffer.push_back(i);
        }
    }

    // merge final
    g.non_fixed.clear();
    g.non_fixed.reserve(pixel_size);
    for (const auto& buf : local_buffers)
        g.non_fixed.insert(g.non_fixed.end(), buf.begin(), buf.end());
    g.non_fixed_size = (int)g.non_fixed.size();

    g.local_id.reserve(g.non_fixed_size * 2);
    for (int i = 0; i < g.non_fixed_size; i++)
        g.local_id[g.non_fixed[i]] = i;

    g.hsv_cache.resize(pixel_size);
    #pragma omp parallel for
    for (int i = 0; i < pixel_size; i++)
    {
        double h, s, v;
        pixels[i].toHSV(h, s, v);
        g.hsv_cache[i] = {congru(h), s, v};
    }
    const int dx[] = {1, 0};
    const int dy[] = {0, 1};
    for (int i = 0; i < g.non_fixed_size; i++)
    {
        int flat_p = g.non_fixed[i];
        int col_p  = flat_p % img_width;
        int row_p  = flat_p / img_width;

        for (int d = 0; d < 2; d++)
        {
            int flat_q = (row_p + dy[d]) * img_width + (col_p + dx[d]);
            if (flat_q < 0 || flat_q >= pixel_size)
                continue;

            auto it = g.local_id.find(flat_q);
            if (it == g.local_id.end())
                continue;
            int j = it->second;

            double smax   = std::max(g.hsv_cache[flat_p].s, g.hsv_cache[flat_q].s);
            double h_dist = std::abs(congru(g.hsv_cache[flat_p].h - g.hsv_cache[flat_q].h));
            double w_pq   = smax / (h_dist + 1e-8);

            g.cached_edges.push_back({i, j, w_pq});
        }
    }
    this->graph = g;
    graph_built = true;
    return g;
}

void Template::solve_graph(double lambda) {

    demi_sec_is_computed=false;
    const auto& pixels = img.get_img();
    int pixel_size = (int)pixels.size();

    if (this->graph.non_fixed_size == 0)
    {
        pixel_label.assign(pixel_size, -1);
        return;
    }

    Graph<double,double,double> graph_cut(this->graph.non_fixed_size, (int)this->graph.cached_edges.size());
    graph_cut.add_node(this->graph.non_fixed_size);

    for (int i = 0; i < this->graph.non_fixed_size; i++)
    {
        double h = this->graph.hsv_cache[this->graph.non_fixed[i]].h;
        double s = this->graph.hsv_cache[this->graph.non_fixed[i]].s;
        double dist_1 = std::abs(congru(h - gap_left[this->graph.non_fixed[i]])) * s;
        double dist_2 = std::abs(congru(h - gap_right[this->graph.non_fixed[i]])) * s;
        graph_cut.add_tweights(i, lambda * dist_2, lambda * dist_1);
    }

    for (const auto& [i, j, w] : this->graph.cached_edges)
        graph_cut.add_edge(i, j, w, w);

    graph_cut.maxflow();

    pixel_label.assign(pixel_size, -1);
    for (int i = 0; i < this->graph.non_fixed_size; i++)
        pixel_label[this->graph.non_fixed[i]] = (graph_cut.what_segment(i) == Graph<double,double,double>::SINK) ? 1 : 0;
}



//  VOISINAGE

Voisinage::Voisinage(unsigned int range)
{
    pos_x.resize(0);
    pos_y.resize(0);
    for (int x=-range ; x<int(range+1) ; x++) for (int y=-range ; y<int(range+1) ; y++) if (x*x+y*y <= int(range*range))
    {
        pos_x.push_back(x);
        pos_y.push_back(y);
    }
}
int Voisinage::size() {return pos_x.size();}


// 4.1
int Template::find_pixel_sector(int p, double h, bool & isInside) const
{
    int nb_sectors = get_nbSector();
    isInside = false;

    int index = -1;
    for (int sector=0 ; sector<nb_sectors ; sector++)
        if (isInsideSector(h, sector))
        { 
            index = sector;
            isInside = true;
            break;
        }
    if (!isInside)
    {
        int label = pixel_label[p];
        double target_border = (label == 0) ? gap_left[p] : gap_right[p];
        for (int sector=0 ; sector<nb_sectors ; sector++)
        {
            double border_tested = centers[sector] + (label==0 ? 1 : -1)*widths[sector] / 2.0;
            double border_dist = congru(target_border - border_tested);
            if (border_dist == 0) // !!!!
            {
                index = sector;
                break;
            }
        }
    }

    return index;
}

void Template::compute_demi_sectors()
{
    const std::vector<Pixel> & pixels = img.get_img();
    if (!demi_sec_is_computed)
    {
        demi_sectors.resize(pixels.size());
        distances_sectors.resize(pixels.size());
        for (int p=0 ; p<pixels.size() ; p++)
        {
            double h, s, v;
            pixels[p].toHSV(h, s, v);
            h = congru(h);
            bool isInside = false;
            int index = find_pixel_sector(p, h, isInside);
            double C = centers[index];
            double d = congru(C-h);
            char demi = (get_nbSector()==1 && !isInside) ? (pixel_label[p]==0 ? 0 : 1) : (d<0 ? 0 : 1);
            d = isInside ? abs(d) : ((pixel_label[p]==0 ? 1.0 : -1.0) * (h-C));
            d += d>0 ? 0.0 : M_PI * 2.0;
            demi_sectors[p] = 2*index+demi;
            distances_sectors[p] = d;
        }
        demi_sec_is_computed = true;
    }
}

void Template::find_bad_pixels(double distance_max)
{
    const std::vector<Pixel> & pixels = img.get_img();
    int h = img.get_height();
    int w = img.get_width();
    compute_demi_sectors();
    bad_pixels.resize(0);
    std::vector<std::vector<int>> voisinage = {{0, -1}, {-1, 0}, {1, 0}, {0, 1}};

    for (int x=0 ; x<w ; x++) for (int y=0 ; y<h ; y++)
    {
        int p = y*w+x;
        bool is_ok = true;
        for (int v=0 ; v<voisinage.size() ; v++)
        {
            int pv = std::clamp(y+voisinage[v][1], 0, h-1)*w + std::clamp(x+voisinage[v][0], 0, w-1);
            double dist2 = pow(pixels[p].r-pixels[pv].r, 2) + pow(pixels[p].g-pixels[pv].g, 2) + pow(pixels[p].b-pixels[pv].b, 2);
            is_ok = is_ok && ((demi_sectors[pv] == demi_sectors[p]) || (dist2 > distance_max*distance_max));
        }
        if (!is_ok) bad_pixels.push_back(p);
    }
}

void Template::blur_bad_pixels(std::vector<Pixel> & result, int h, int w) const
{
    std::vector<Pixel> buff;
    buff.resize(result.size());
    for (int p=0 ; p<result.size() ; p++) buff[p] = Pixel(result[p].r, result[p].g, result[p].b);
    Voisinage vois(3);
    
    for (int pp=0 ; pp<bad_pixels.size() ; pp++)
    {
        int p = bad_pixels[pp];
        int y = p/w;
        int x = p%w;
        for (int v=0 ; v<vois.size() ; v++)
        {
            int pv = std::clamp(y+vois.pos_y[v], 0, h-1)*w + std::clamp(x+vois.pos_x[v], 0, w-1);
            int sumR = 0;
            int sumG = 0;
            int sumB = 0;
            int yv = pv/w;
            int xv = pv%w;
            for (int vv=0 ; vv<vois.size() ; vv++)
            {
                int pvv = std::clamp(yv+vois.pos_y[vv], 0, h-1)*w + std::clamp(xv+vois.pos_x[vv], 0, w-1);
                sumR += buff[pvv].r;
                sumG += buff[pvv].g;
                sumB += buff[pvv].b;
            }
            result[pv].r = sumR/vois.size();
            result[pv].g = sumG/vois.size();
            result[pv].b = sumB/vois.size();
        }
    }
}

std::vector<Pixel> Template::shift_hues(double sigma_factor, bool _blur_bad_pixel, bool test_bad_pixel) {
    const auto &pixels = img.get_img();
    int nb_pixels = pixels.size();
    std::vector<Pixel> result;
    int nb_sectors = get_nbSector();
    result.reserve(nb_pixels);
    compute_demi_sectors();

    for (int p=0 ; p<nb_pixels ; p++)
    {
        double h, s, v;
        pixels[p].toHSV(h, s, v);
        h = congru(h);

        int index = demi_sectors[p]/2;
        double C = centers[index];
        double w = widths[index];
        double sigma = sigma_factor * w;
        double d = distances_sectors[p];
        double sens = (demi_sectors[p]%2 == 0) ? 1.0 : -1.0;

        double gauss = exp(-d*d / (sigma*sigma*2.0));
        double h2 = congru(C + sens * (w / 2.0) * (1.0 - gauss));
        h2 += h2>0 ? 0 : M_PI*2.0;
        result.push_back(Pixel::toRGB(h2, s, v));
    }

    if (_blur_bad_pixel || test_bad_pixel)
        find_bad_pixels(8.0);
    if (_blur_bad_pixel)
        blur_bad_pixels(result, img.get_height(), img.get_width());
    if (test_bad_pixel) for (int i=0 ; i<bad_pixels.size() ; i++)
    {
        int p = bad_pixels[i];
        double h, s, v;
        pixels[p].toHSV(h, s, v);
        if (s>0.1) result[p] = Pixel(0, 0, 0);
    }

    return result;
}

std::vector<Pixel> Template::shift_hues2(bool _blur_bad_pixel, bool test_bad_pixel)
{
    const std::vector<Pixel> & pixels = img.get_img();
    std::vector<Pixel> result;
    result.resize(pixels.size());
    int nb_sectors = get_nbSector();
    std::vector<std::vector<int>> pixelIndexPerSector;
    pixelIndexPerSector.resize(nb_sectors*2);
    for (int i=0 ; i<pixelIndexPerSector.size() ; i++) pixelIndexPerSector[i].resize(0);
    compute_demi_sectors();
    for (int p=0 ; p<pixels.size() ; p++) {pixelIndexPerSector[demi_sectors[p]].push_back(p);}

    for (int sector=0 ; sector<nb_sectors ; sector++) for (int sensI=0 ; sensI<2 ; sensI++)
    {
        int demiSector = 2*sector + sensI;
        double distMin = 0;
        double distMax = 0;
        if (pixelIndexPerSector[demiSector].size() != 0)
        {
            distMin = distances_sectors[pixelIndexPerSector[demiSector][0]];
            distMax = distances_sectors[pixelIndexPerSector[demiSector][0]];
        }
        for (int p=1 ; p<pixelIndexPerSector[demiSector].size() ; p++)
        {
            distMin = std::min(distMin, distances_sectors[pixelIndexPerSector[demiSector][p]]);
            distMax = std::max(distMax, distances_sectors[pixelIndexPerSector[demiSector][p]]);
        }
        if (distMax-distMin != 0)
        {
            double ratio = (widths[sector]*0.5)/(distMax-distMin);
            for (int pp=0 ; pp<pixelIndexPerSector[demiSector].size() ; pp++)
            {
                int p = pixelIndexPerSector[demiSector][pp];
                double newDist = (distances_sectors[p] - distMin) * ratio;
                double sens = (sensI%2 == 0) ? 1.0 : -1.0;
                double h2 = congru(centers[sector] + sens * newDist);
                h2 += h2>0 ? 0 : M_PI*2.0;
                double h, s, v;
                pixels[p].toHSV(h, s, v);
                result[p] = Pixel::toRGB(h2, s, v);
            }
        }
        else for (int pp=0 ; pp<pixelIndexPerSector[demiSector].size() ; pp++)
        {
            int p = pixelIndexPerSector[demiSector][pp];
            double h2 = congru(centers[sector]);
            h2 += h2>0 ? 0 : M_PI*2.0;
            double h, s, v;
            pixels[p].toHSV(h, s, v);
            result[p] = Pixel::toRGB(h2, s, v);
        }
    }

    if (_blur_bad_pixel || test_bad_pixel)
        find_bad_pixels(8.0);
    if (_blur_bad_pixel)
        blur_bad_pixels(result, img.get_height(), img.get_width());
    if (test_bad_pixel) for (int i=0 ; i<bad_pixels.size() ; i++)
    {
        int p = bad_pixels[i];
        double h, s, v;
        pixels[p].toHSV(h, s, v);
        if (s>0.1) result[p] = Pixel(0, 0, 0);
    }

    return result;
}