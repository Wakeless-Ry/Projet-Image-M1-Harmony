#ifndef TEMPLATE
#define TEMPLATE

#include <cmath>
#include <vector>
#include <image.hpp>
#include <unordered_map>
const double TEMPLATE_DEFAULT_S_WIDTH = 0.33161255787;//M_PI / 6.0f;
const double TEMPLATE_DEFAULT_M_WIDTH = M_PI / 2.0f;
const double TEMPLATE_DEFAULT_L_WIDTH = M_PI;
const double TEMPLATE_DEFAULT_CENTER = 0.0f;
struct HSVCache {
    double h, s, v;
};
struct SharedGraph {
    std::vector<int> non_fixed;
    int non_fixed_size = 0;
    std::unordered_map<int,int> local_id;
    std::vector<HSVCache> hsv_cache;
    struct CachedEdge { int i, j; double w; };
    std::vector<CachedEdge> cached_edges;
};

struct Voisinage
{
	std::vector<int> pos_x;
	std::vector<int> pos_y;
	bool has_coeff = false;
	std::vector<double> coeff;

	Voisinage(unsigned int range);
	int size();
};

enum Template_format { i = 0, V = 1, L = 2, I = 3, T = 4, Y = 5, X = 6, t = 7, q = 8 }; // t : triadique, q : quadriadique

class Template
{
	private:
  		std::vector<double> centers;
  		std::vector<double> widths;
  		Image img;
		Template_format format;
  		void autoCongru();
		std::vector<double> theta_1;
		std::vector<double> theta_2;
		std::vector<int> pixel_label;
		std::vector<double> gap_left;
		std::vector<double> gap_right;
		SharedGraph graph;
		bool graph_built = false;

		std::vector<unsigned char> demi_sectors;
		std::vector<double> distances_sectors;
		bool demi_sec_is_computed = false;
		std::vector<int> bad_pixels;
	public:
  		Template(std::vector<double> c = {}, std::vector<double> w = {});
  		Template(double c, double w = TEMPLATE_DEFAULT_S_WIDTH);
  		Template(Template_format format);
		void set_image(std::string path);
		void set_image_v2(std::vector<unsigned char> data_tmp, int height, int width);
		const std::vector<Pixel>& get_img() const;
  		const int get_nbSector() const;
  		const double get_center(int n) const;
  		const double get_widths(int n) const;
  		const std::vector<double> get_center() const;
  		const std::vector<double> get_widths() const;

		void setWidths(double w);
		void setWidths(std::vector<double> w = {});
		const Template_format get_format() const;
  		void rotate(double angle);

  		static double congru(double angle);

  		double distanceToTemplate(double hue) const;
  		bool isInsideSector(double hue, int sectorIndex) const;
		// 3.0
		double F() const;
		double bestOrientation() const; // M(X,Tm) dans le papier
		std::pair<Template_format, double> bestTemplate() const; // B(X)
		// 4.0
 		double e1(const std::vector<int>& labels, const std::vector<int>& pixel_indices) const;
		double e2(const std::vector<int>& labels, const std::vector<int>& pixel_indices) const;
		double e(const std::vector<int>& labels, const std::vector<int>& pixel_indices, double lambda = 1.0) const;
		void compute_thetas();
		void solve_graph(double lambda);
		SharedGraph build_graph();
		// 4.1
		int find_pixel_sector(int p, double h, bool & isInside) const;
		void compute_demi_sectors();
		std::vector<Pixel> shift_hues(double sigma_factor = 0.5, bool _blur_bad_pixel = true, bool test_bad_pixel = false);
		std::vector<Pixel> shift_hues2(bool _blur_bad_pixel = false, bool test_bad_pixel = true);

		void find_bad_pixels(double distance_max);
		void blur_bad_pixels(std::vector<Pixel> & result, int h, int w) const;
};



#endif