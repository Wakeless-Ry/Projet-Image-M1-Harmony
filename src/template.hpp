#ifndef TEMPLATE
#define TEMPLATE

#include <cmath>
#include <vector>
#include <image.hpp>

const double TEMPLATE_DEFAULT_S_WIDTH = M_PI / 6.0f;
const double TEMPLATE_DEFAULT_M_WIDTH = M_PI / 2.0f;
const double TEMPLATE_DEFAULT_L_WIDTH = M_PI;
const double TEMPLATE_DEFAULT_CENTER = 0.0f;

enum Template_format { i = 0, V = 1, L = 2, I = 3, T = 4, Y = 5, X = 6 };

class Template
{
	private:
  		std::vector<double> centers;
  		std::vector<double> widths;
  		Image img;

  		void autoCongru();
		std::vector<double> theta_1;
		std::vector<double> theta_2;
		std::vector<int> pixel_label;
		std::vector<double> gap_left;
		std::vector<double> gap_right;

	public:
  		Template(std::vector<double> c = {}, std::vector<double> w = {});
  		Template(double c = 0, double w = 0);
  		Template(Template_format format);
		void set_image(std::string path);
		const std::vector<Pixel>& get_img() const;
  		const int get_nbSector() const;
  		const double get_center(int n) const;
  		const double get_widths(int n) const;
  		const std::vector<double> get_center() const;
  		const std::vector<double> get_widths() const;

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
		void compute_labels(double lambda = 1.0);
		// 4.1
		std::vector<Pixel> shift_hues(double sigma_factor = 0.5) const;
};



#endif