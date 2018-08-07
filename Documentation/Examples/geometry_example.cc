/*
  from within ngc/build/ directory:
  g++ -O3 -I ../third_party/ -I<path to eigen> -I ../ -Wall -pedantic \
   -std=c++14 ../Main/geometry_example.cc \
   ../Framework/Geometry/CoordinateSystem.cc -o geometry_example
*/

#include <Geometry/Vector.h>
#include <Geometry/Sphere.h>
#include <Geometry/Point.h>
#include <Geometry/CoordinateSystem.h>
#include <Units/PhysicalUnits.h>

#include <iostream>
#include <cstdlib>

int main()
{
    using namespace phys::units;
    using namespace phys::units::io;
    using namespace phys::units::literals;
    
    CoordinateSystem root;
    Point const p1(root, {0_m, 0_m, 0_m});
    CoordinateSystem cs2 = root.translate({0_m, 0_m, 1_m});
    Point const p2(cs2, {0_m, 0_m, 0_m});
    
    auto const diff = p2 - p1;
    auto const norm = diff.squaredNorm();
    
    std::cout << "p2-p1 components: " << diff.getComponents() << std::endl;
    std::cout << "p2-p1 norm^2: " << norm << std::endl;
    
    Sphere s(p1, 10_m);
    std::cout << "p1 inside s:  " << s.isInside(p2) << std::endl;    
    
    Sphere s2(p1, 3_um);
    std::cout << "p1 inside s2: " << s2.isInside(p2) << std::endl;    
    
    return EXIT_SUCCESS;
}
