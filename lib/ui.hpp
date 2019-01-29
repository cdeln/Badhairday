#ifndef __UI__
#define __UI__

class UI {
    public:
        UI();
        static void onKeyDown(unsigned char key, int x, int y);
        static void onKeyUp(unsigned char key, int x, int y);
};

#endif
