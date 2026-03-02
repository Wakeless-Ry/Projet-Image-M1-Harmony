#include "src/image.hpp"
#include "src/template.hpp"
#include <iostream>

int main() {
  Pixel p(255, 0, 0);
  double h, s, v;
  p.toHSV(h, s, v);
  std::cout << "Red HSV: h=" << h << " s=" << s << " v=" << v << "\n";

  Pixel back = Pixel::toRGB(h, s, v);
  std::cout << "retour vers RGB: r=" << (int)back.r << " g=" << (int)back.g
            << " b=" << (int)back.b << "\n";

  Template t(Template_format::I);
  std::cout << "I-template sectors: " << t.get_nbSector() << "\n";
  std::cout << "center0=" << t.get_center(0) << " width0=" << t.get_widths(0)
            << "\n";
  std::cout << "center1=" << t.get_center(1) << " width1=" << t.get_widths(1)
            << "\n";

  std::cout << "Distance of h=0 (inside sector): " << t.distanceToTemplate(0.0)
            << "\n";

  std::cout << "Distance of h=π/2 (outside): "
            << t.distanceToTemplate(M_PI / 2.0) << "\n";

  Image img("../assets/img/baboon.ppm");
  std::cout << "F(I-template): " << t.F(img) << "\n";
}