#include "template.hpp"
#include <omp.h>
#include <stdexcept>
#include "graph.h"

Template::Template(std::vector<double> c, std::vector<double> w)
{
  	if (c.size() != w.size())
    	throw std::runtime_error("Il doit y avoir autant de centre de secteur que "
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
  	case i:
	{
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
    	centers = {TEMPLATE_DEFAULT_CENTER, TEMPLATE_DEFAULT_CENTER - M_PI / 2.0};
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
}

// getters
const int Template::get_nbSector() const { return centers.size(); }
const double Template::get_center(int n) const { return centers[n]; }
const double Template::get_widths(int n) const { return widths[n]; }
const std::vector<double> Template::get_center() const { return centers; }
const std::vector<double> Template::get_widths() const { return widths; }

void Template::autoCongru() {
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
  	return (rest > M_PI) * (rest - pi2) + (rest <= -M_PI) * (rest + pi2) +
         	(rest <= M_PI && rest > -M_PI) * rest;
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


double Template::distance_hue(double h, double c) const
{
    double d = congru(h - c);
    return d;
}

// 3.0
double Template::F() const
{
    double total = 0.0;
    for (const Pixel &p : this->img.get_img())
    {
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

    for (int iter = 0; iter < 100; iter++)
    {
        double midpoint = 0.5 * (a + b);
        double tol1 = tol * std::abs(x) + 1e-10;
        double tol2 = 2.0 * tol1;

        if (std::abs(x - midpoint) <= tol2 - 0.5 * (b - a))
            break;

        bool parabolic_ok = false;
        double u;

        if (std::abs(e) > tol1)
        {
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

        if (!parabolic_ok)
        {
            e = (x < midpoint) ? b - x : a - x;
            d = golden * e;
        }

        u = x + d;
        Template tu = *this;
        tu.rotate(u);
        double fu = tu.F();

        if (fu <= fx)
        {
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
        } else
        {
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
            } else if (fu <= fv || v == x || v == w)
            {
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

    for (int ite = 0; ite <= 6; ite++) {
        Template t((Template_format)ite);
        t.img = this->img;
        double angle = t.bestOrientation();

        Template t2((Template_format)ite);
        t2.img = this->img;
        t2.rotate(angle);
        double f = t2.F();

        if (f < best_F)
        {
            best_F = f;
            best_angle = angle;
            best_format = (Template_format)ite;
        }
    }

    return {best_format, best_angle};
}
// 4.0

double Template::energie_1(int width, int height,
                           const std::vector<Pixel>& pixels,
                           const std::vector<int>& v) const
{
    double e1 = 0.0;

    for (int idx = 0; idx < pixels.size(); idx++)
    {
        double h, s, val;
        pixels[idx].toHSV(h, s, val);

        double theta = (v[idx] == 1 ? this->theta2[idx] : this->theta1[idx]);
        double d = fabs(distance_hue(h, theta));

        e1 += d * s;
    }
    return e1;
}



double Template::energie_2(int width, int height,
                            const std::vector<Pixel>& pixels,
                            const std::vector<int>& v) const
{
    double e2 = 0.0;
    const int dx[4] = {1, 0, 1,  1};
    const int dy[4] = {0, 1, 1, -1};

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

                if (v[idx] == v[nidx])
                    continue;

                double h1, s1, val1;
                double h2, s2, val2;
                pixels[idx ].toHSV(h1, s1, val1);
                pixels[nidx].toHSV(h2, s2, val2);

                double smax = std::max(s1, s2);
                double dist = fabs(distance_hue(h1, h2));

                if (dist > 1e-6)
                    e2 += smax / dist;
            }
        }
    }
    return e2;
}


double Template::compute_energie(double lambda, const std::vector<int>& v) const
{
    const std::vector<Pixel>& pixels = img.get_img();
    int width  = img.get_width();
    int height = img.get_height();

    double e1 = energie_1(width, height, pixels, v);
    double e2 = energie_2(width, height, pixels, v);

    return lambda * e1 + e2;
}


void Template::set_image(std::string path)
{ 
  	this->img.set_path(path);
}


void Template::compute_thetas(const std::vector<Pixel>& pixels,  std::vector<int>& v)
{
    int N = pixels.size();
    this->theta1.resize(N);
    this->theta2.resize(N);
    this->is_fixed.assign(N, false);
    v.assign(N, 1);
    for (int idx = 0; idx < N; idx++)
    {
        double h, s, vpx;
        pixels[idx].toHSV(h, s, vpx);

        bool inside = false;
        int best_sector = 0;
        double best_d = 1e9;

        for (int n = 0; n < get_nbSector(); n++)
        {
            if (isInsideSector(h, n))
            {
                inside = true;
                double d = fabs(distance_hue(h, get_center(n)));
                if (d < best_d)
                {
                    best_d = d;
                    best_sector = n;
                }
            }
        }

        if (inside)
        {
            this->is_fixed[idx] = true;
            double d = distance_hue(h, get_center(best_sector));
            v[idx] = (d < 0 ? -1 : 1);
            continue;
        }

        double best_left = 1e9;
        double best_right = 1e9;

        for (int n = 0; n < get_nbSector(); n++)
        {
            double c = get_center(n);
            double w2 = get_widths(n) / 2.0;

            double left = c - w2;
            double right = c + w2;

            double dl = distance_hue(h, left);
            double dr = distance_hue(h, right);

            if (dl < 0 && fabs(dl) < best_left)
            {
                best_left = fabs(dl);
                this->theta1[idx] = left;
            }

            if (dr > 0 && fabs(dr) < best_right)
            {
                best_right = fabs(dr);
                this->theta2[idx] = right;
            }
        }
        if (!this->is_fixed[idx])
        {
            if (best_left == 1e9)
                this->theta1[idx] = get_center(0) - get_widths(0) / 2.0;

            if (best_right == 1e9)
                this->theta2[idx] = get_center(0) + get_widths(0) / 2.0;
        }
    }
}



void Template::run_graphcut(const std::vector<Pixel>& pixels, double lambda, std::vector<int>& v) const
{
    int N = pixels.size();
    int width = img.get_width();
    int height = img.get_height();

    typedef Graph<double,double,double> GraphType;
    GraphType* g = new GraphType(N, N * 4);
    g->add_node(N);

    for (int idx = 0; idx < N; idx++)
    {
        double h, s, val;
        pixels[idx].toHSV(h, s, val);

        if (this->is_fixed[idx])
        {
            double INF = 1e12;
            if (v[idx] == -1)
                g->add_tweights(idx, INF, 0);
            else
                g->add_tweights(idx, 0, INF);
            continue;
        }

        double cost_minus = lambda * fabs(distance_hue(h, theta1[idx])) * s;
        double cost_plus  = lambda * fabs(distance_hue(h, theta2[idx])) * s;

        g->add_tweights(idx, cost_plus, cost_minus);
    }

    const int dx[4] = {1, 0, 1, 1};
    const int dy[4] = {0, 1, 1, -1};

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

                double h1, s1, v1;
                double h2, s2, v2;
                pixels[idx].toHSV(h1, s1, v1);
                pixels[nidx].toHSV(h2, s2, v2);

                double smax = std::max(s1, s2);
                double dist = fabs(distance_hue(h1, h2));

                double w = (dist > 1e-6 ? smax / dist : 1e6);

                g->add_edge(idx, nidx, w, w);
            }
        }
    }

    g->maxflow();

    for (int idx = 0; idx < N; idx++)
        if (!this->is_fixed[idx])
            v[idx] = (g->what_segment(idx) == GraphType::SOURCE ? -1 : 1);

    delete g;
}

const std::vector<Pixel>&  Template::get_img() const
{ 
	return this->img.get_img();
}

// 4.1
double Template::gaussien(double esp, double st_dev, double x)
{
    return exp(pow((x - esp) / st_dev, 2.0) / -2.0);
}

double Template::mod2pi(double angle) // return angle in [0, 2pi[
{
  double pi2 = 2 * M_PI;
  double rest = angle - double(int(angle / pi2)) * pi2;
  return rest + 2*M_PI*(rest<0);
}

std::vector<Pixel> Template::projectPixels(std::vector<Pixel>& dataIn,
                                           Template& temp,
                                           std::vector<int>& V) const
{
    int N = dataIn.size();
    std::vector<Pixel> out(N);

    for (int p = 0; p < N; p++)
    {
        double h, s, v;
        dataIn[p].toHSV(h, s, v);

        int index = 0;
        double distMin = 2.0*M_PI;
        for (int i=0 ; i<temp.get_nbSector() ; i++)
        {
            double Cp = temp.get_center(i);
            double w2 = temp.get_widths(i)/2.0;
            double bord = Template::congru(Cp - V[p]*w2);
            double d = mod2pi((bord-h)*V[p]);
            if (d<distMin) {distMin = d; index = i;}
        }
        /*
        if (this->is_fixed[p])
        {
            out[p] = dataIn[p];
            continue;
        }

        double h_proj = h;
        if (V[p] == -1)
            h_proj = this->theta1[p];
        else
            h_proj = this->theta2[p];

        int best = 0;
        double best_d = 1e9;
        for (int i = 0; i < temp.get_nbSector(); i++)
        {
            double d = fabs(distance_hue(h_proj, temp.get_center(i)));
            if (d < best_d)
            {
                best_d = d;
                best = i;
            }
        }*/

        double c  = temp.get_center(index);
        double w  = temp.get_widths(index);
        double w2 = w / 2.0;

        //double d   = fabs(distance_hue(h_proj, c));
        double d = mod2pi((c-h)*V[p]);
        /*double sig = w2;
        double g   = exp(-(d * d) / (2.0 * sig * sig));

        double sign = (distance_hue(h_proj, c) >= 0 ? 1.0 : -1.0);

        double h2 = c + sign * w2 * (1.0 - g);*/
        double h2 = c - V[p]*w2*(1.0-gaussien(0.0, w2, d));
        h2 = c;


        out[p] = Pixel::toRGB(h2, s, v);
    }

    return out;
}
