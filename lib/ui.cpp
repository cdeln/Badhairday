#include <iostream>
#include "ui.hpp"
#include "common.h"

UI::UI() {
    glutKeyboardFunc(onKeyDown);
}

void UI::onKeyDown(unsigned char key, int x, int y) {
    std::cout << "Pressed key " << key << ", x = " << x << ", y = " << std::endl;
}
void UI::onKeyUp(unsigned char key, int x, int y) {
    std::cout << "Released key " << key << ", x = " << x << ", y = " << y << std::endl;
}
