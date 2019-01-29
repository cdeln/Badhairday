
#include "programs/programs.hpp"
#include <iostream>
#include "common.h"

int main(int argc, char **argv) {

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
    glutInitContextVersion(3, 2);
    glutCreateWindow ("gtltest");

    try {
        gtl::shader::Gouraud shader;
        std::cout << "Test passed" << std::endl;
        return 0;
    }
    catch ( gtl::Exception & e ) {
        std::cerr <<  e.what() << std::endl;
        return 1;
    }
}
