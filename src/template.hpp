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
		std::vector<double> theta1;
		std::vector<double> theta2;
		std::vector<bool> is_fixed;

	public:
  		Template(std::vector<double> c = {}, std::vector<double> w = {});
  		Template(double c = 0, double w = 0);
  		Template(Template_format format);
		void set_image(std::string path);
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
  		double distance_hue(double h1, double h2) const;
  		double energie_1(int width, int height,
  		                 const std::vector<Pixel>& pixels,
  		                 const std::vector<int>& v) const;
		
  		double energie_2(int width, int height,
  		                 const std::vector<Pixel>& pixels,
  		                 const std::vector<int>& v) const;
		
  		double compute_energie(double lambda, const std::vector<int>& v) const;
  		
  		void compute_thetas(const std::vector<Pixel>& pixels, std::vector<int>& v);
		void run_graphcut(const std::vector<Pixel>& pixels, double lambda, std::vector<int>& v) const;
  		const std::vector<Pixel>& get_img() const;
		// 4.1
		static double gaussien(double esp, double st_dev, double x);
		static double mod2pi(double angle);
		std::vector<Pixel> projectPixels(std::vector<Pixel> & dataIn, Template & temp, std::vector<int> & V) const;
};



#endif