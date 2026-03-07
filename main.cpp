#include "src/image.hpp"
#include "src/template.hpp"
#include <iostream>
#include <stdexcept>
#include <vector>
#include <cmath>

double gaussien(double esp, double st_dev, double x) {
  return exp(pow((x-esp)/st_dev, 2.0)/-2.0) / (st_dev*sqrt(2.0*M_PI));
}

Image projectPixels(Image & imIn, Template & temp, std::vector<int> & sectorTarget)
{
  if (sectorTarget.size() != imIn.get_nb_pixels()) throw std::runtime_error("Buffer doit avoir la même taille que image");
  std::vector<Pixel> dataIn = imIn.get_img();
  std::vector<unsigned char> dataOut;
  dataOut.resize(sectorTarget.size()*3);
  for (int p=0 ; p<sectorTarget.size() ; p++)
  {
    double h, s, v, h2;
    dataIn[p].toHSV(h, s, v);
    double Cp = temp.get_center(sectorTarget[p]);
    double w2 = temp.get_widths(sectorTarget[p])/2.0;
    double dist = Template::congru(h-Cp);
    double sens = (dist > 0)*2-1;
    h2 = Cp + /*sens**/w2*(1.0-gaussien(0.0, w2, dist));
    Pixel pix = Pixel::toRGB(h2, s, v);
    dataOut[3*p] = pix.r;
    dataOut[3*p+1] = pix.g;
    dataOut[3*p+2] = pix.b;
  }
  Image imOut = Image(dataOut, imIn.get_width(), imIn.get_height());
  return imOut;
}

int main() {
  Image imIn("../assets/img/peacock.jpg");
  Template temp(i);
  std::vector<int> sectorTarget(imIn.get_nb_pixels(), 0);
  Image imOut = projectPixels(imIn, temp, sectorTarget);
  imOut.write_ppm("../assets/out/peacock_red_withoutSens.ppm");

  // Pixel p(255, 0, 0);
  // double h, s, v;
  // p.toHSV(h, s, v);
  // std::cout << "Red HSV: h=" << h << " s=" << s << " v=" << v << "\n";

  // Pixel tmp = Pixel::toRGB(h, s, v);
  // std::cout << "retour vers RGB: r=" << (int)tmp.r << " g=" << (int)tmp.g
  //           << " b=" << (int)tmp.b << "\n";

  // Template t(Template_format::I);
  // std::cout << "I-template sectors: " << t.get_nbSector() << "\n";
  // std::cout << "center0=" << t.get_center(0) << " width0=" << t.get_widths(0)
  //           << "\n";
  // std::cout << "center1=" << t.get_center(1) << " width1=" << t.get_widths(1)
  //           << "\n";

  // std::cout << "Distance of h=0 (inside sector): " <<
  // t.distanceToTemplate(0.0)
  //           << "\n";

  // std::cout << "Distance of h=π/2 (outside): "
  //           << t.distanceToTemplate(M_PI / 2.0) << "\n";

  // Image img("../assets/img/baboon.ppm");
  // Image img("../assets/img/peacock.jpg");
  // std::cout << "F(I-template): " << t.F(img) << "\n";

  // auto [format, angle] = Template::bestTemplate(img);

  // std::cout << "Best template: " << format << "\n";
  // std::cout << "Best angle: " << angle << "\n";

  // double angle = 2.64522;
  // std::cout << "Degrees: " << angle * 180.0 / M_PI << "\n";

  // Pixel color = Pixel::toRGB(angle, 1.0, 1.0);
  // std::cout << "Color: r=" << (int)color.r << " g=" << (int)color.g
  //           << " b=" << (int)color.b << "\n";
}