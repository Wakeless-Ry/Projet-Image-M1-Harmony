#include "color_harmo.hpp"

Color_harmo::Color_harmo(const std::string& path) : img(path), tmpl(Template_format::i)
{
}


double Color_harmo::F(Template& t) const
{
    double total = 0.0;
    for (const Pixel &p : this->img.get_img())
    {
        double h, s, v;
        p.toHSV(h, s, v);
        total += t.distanceToTemplate(h) * s;
    }
    return total;
}


double Color_harmo::bestOrientation(int steps, Template_format format) const {
    double best_angle = 0.0;
    double best_F = std::numeric_limits<double>::max();

    for (int i = 0; i < steps; i++)
    {
        double angle = 2.0 * M_PI * i / steps;

        Template t(format);
        t.rotate(angle);

        double f = F(t);
        if (f < best_F)
        {
            best_F = f;
            best_angle = angle;
        }
    }

    return best_angle;
}

std::pair<Template_format, double> Color_harmo::bestTemplate(int steps) const
{
    Template_format best_format = i;
    double best_angle = 0.0;
    double best_F = std::numeric_limits<double>::max();

    for (int ite = 0; ite <= 6; ite++)
    {
        Template_format t = (Template_format)ite;
        double angle = bestOrientation(steps, t);

        Template t2(t);
        t2.rotate(angle);
        double f = F(t2);

        if (f < best_F)
        {
            best_F = f;
            best_angle = angle;
            best_format = (Template_format)ite;
        }
    }

    return {best_format, best_angle};
}

double Color_harmo::distance_hue(double h1, double h2) const
{
    double d = std::fabs(h1 - h2);

    if (d > M_PI)
        d = 2 * M_PI - d;

    return d;
}

double Color_harmo::energie_1(int width, int height, const std::vector<Pixel>& pixels,
                                                     const std::vector<int>& v) const
{
    double e1 = 0.0;

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int idx = y * width + x;
            const Pixel& p = pixels[idx];

            double h, s, val;
            p.toHSV(h, s, val);

            // secteur + proche
            int sector = 0;
            double minDist = distance_hue(h, tmpl.get_center(0));
            for (int n = 1; n < tmpl.get_nbSector(); n++)
            {
                double dist = distance_hue(h, tmpl.get_center(n));
                if (dist < minDist)
                {
                    minDist = dist;
                    sector = n;
                }
            }

            // bordure
            double left  = tmpl.get_center(sector) - tmpl.get_widths(sector)/2.0;
            double right = tmpl.get_center(sector) + tmpl.get_widths(sector)/2.0;

            double h2 = (v[idx] == 0) ? left : right;

            e1 += distance_hue(h, h2) * s;
        }
    }
    return e1;
}


double Color_harmo::energie_2(int width, int height, const std::vector<Pixel>& pixels,
                                                     const std::vector<int>& v) const
{
    double e2 = 0.0;
    // voisins
    const int dx[4] = {-1, 1, 0, 0};
    const int dy[4] = {0, 0, -1, 1};

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int idx = y * width + x;
            for (int k = 0; k < 4; k++)
            {
                int nx = x + dx[k];
                int ny = y + dy[k];

                if (nx < 0 || nx >= width || ny < 0 || ny >= height)
                    continue;

                int nidx = ny * width + nx;

                if (v[idx] != v[nidx])
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

double Color_harmo::compute_energie(double lambda, const std::vector<int>& v) const
{
    const std::vector<Pixel>& pixels = this->img.get_img();
    int width = this->img.get_width();
    int height = this->img.get_height();

    double e1 = energie_1(width, height, pixels, v);
    double e2 = energie_2(width, height, pixels, v);

    return lambda * e1 + e2;
}