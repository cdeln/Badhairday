#ifndef __UTIL__
#define __UTIL__

#include <string>
#include "common_ext.hpp"

namespace util {

    Model * quad();
    Model * cube();
    Model * ball(float scale, int number = 2); 
    Model * lattice(int resolution);
    Model * model(const std::string & path); 

    float rad2deg(float rad);
    float deg2rad(float deg);
    float clamp(float val, float min, float max);
}

#endif
