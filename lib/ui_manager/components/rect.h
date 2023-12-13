#ifndef LIB_UI_MANAGER_RECT_H_
#define LIB_UI_MANAGER_RECT_H_

struct Rect {
    int x;
    int y;
    int width;
    int height;

    Rect();
    Rect(int x, int y);
    Rect(int x, int y, int width, int height);
};


#endif