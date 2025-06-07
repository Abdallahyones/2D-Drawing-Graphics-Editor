#include <iostream>
#include <Windows.h>
#include <cmath>
#include <vector>
#include "Common.h"

using namespace std;


//function to calculate region code
int ComupteCodeRetangle(int x, int y) {
    int code = INSIDE;
    if (x < xLeft)
        code |= LEFT;
    else if (x > xRight)
        code |= RIGHT;
    if (y < yBottom)
        code |= BOTTOM;
    else if (y > yTop)
        code |= TOP;
    return code;
}

// Draw Line using DDA
void DrawLineDDAWithClippingWithClipping(HDC hdc, int x1, int y1, int x2, int y2, COLORREF c) {
    double  dx = x2 - x1;
    double  dy = y2 - y1;
    int steps = max(abs(dx), abs(dy));
    float xIncrement = dx / (double ) steps;
    float yIncrement = dy / (double ) steps;

    double  x = x1;
    double  y = y1;
    for (int i = 0; i <= steps; ++i) {
        SetPixel(hdc, round(x), round(y), c);
        x += xIncrement;
        y += yIncrement;
    }
}

// Get Slope
float GetLineSlope(int x1, int y1, int x2, int y2) {
    if (x2 - x1 == 0)
        return INFINITY;
    return (float) (y2 - y1) / (x2 - x1);
}

// Calculate InterSection of Line with clipping boundry
void getIntersection(int CodeOut, int &x, int &y, int x1, int y1, int x2, int y2) {
    float m = GetLineSlope(x1, y1, x2, y2);
    if (CodeOut & TOP) {
        y = yTop;
        if (m == INFINITY)
            x = x1;
        else
            x = (int) (x1 + (yTop - y1) / m);
    } else if (CodeOut & BOTTOM) {
        y = yBottom;
        if (m == INFINITY)
            x = x1;
        else
            x = (int) (x1 + (yBottom - y1) / m);
    } else if (CodeOut & RIGHT) {
        x = xRight;
        if (m == INFINITY)
            y = y1;
        else
            y = (int) (y1 + (xRight - x1) * m);
    } else if (CodeOut & LEFT) {
        x = xLeft;
        if (m == INFINITY)
            y = y1;
        else
            y = (int) (y1 + (xLeft - x1) * m);
    }
}

// Cohen-sutherland line clipping
void CohenSutherlandLineRectangle(HDC hdc, Point p1 ,  Point p2, COLORREF color) {
    auto[x1 , y1] = p1;
    auto[x2 , y2] = p2;
    int origX1 = x1, origY1 = y1, origX2 = x2, origY2 = y2;
    int code1 = ComupteCodeRetangle(x1, y1);
    int code2 = ComupteCodeRetangle(x2, y2);
    bool accept = false;
    while (true) {
        // if my object is on window
        if ((code1 == 0) && (code2 == 0)) {
            accept = true;
            break;
        } else if (code1 & code2) {// second base case , can't draw line on my screen
            break;
        } else {
            int codeOut, x, y;
            codeOut = code1 ? code1 : code2;
            getIntersection(codeOut, x, y, x1, y1, x2, y2);
            if (codeOut == code1) {
                x1 = x;
                y1 = y;
                code1 = ComupteCodeRetangle(x1, y1);
            } else {
                x2 = x;
                y2 = y;
                code2 = ComupteCodeRetangle(x2, y2);
            }
        }
    }
    if (accept) {
        // Outside part 1: original to clipped start
        if (x1 != origX1 || y1 != origY1)
            DrawLineDDAWithClippingWithClipping(hdc, origX1, origY1, x1, y1, RGB(255,0,0)); // red

        // Inside part
        DrawLineDDAWithClippingWithClipping(hdc, x1, y1, x2, y2, RGB(0,0,255)); // blue

        // Outside part 2: clipped end to original end
        if (x2 != origX2 || y2 != origY2)
            DrawLineDDAWithClippingWithClipping(hdc, x2, y2, origX2, origY2, RGB(255,0,0)); // red
    } else {
        // Line completely outside, draw full line in red
        DrawLineDDAWithClippingWithClipping(hdc, origX1, origY1, origX2, origY2, color);
    }
}

LRESULT drawLineRectangle(HWND hwnd, UINT m, WPARAM wp, LPARAM lp, COLORREF color, DrawCommand &cmd) {
    HDC hdc;
    PAINTSTRUCT ps;
    static int x1, y1, x2, y2;
    static bool isFirstClick = true;

    switch (m) {
        case WM_LBUTTONDOWN: {
            hdc = GetDC(hwnd);
            int x = LOWORD(lp);
            int y = HIWORD(lp);
            if (isFirstClick) {
                x1 = x;
                y1 = y;
                isFirstClick = false;
            } else {
                x2 = x;
                y2 = y;
                cmd.points.emplace_back(x1,y1);
                cmd.points.emplace_back(x2,y2);
                CohenSutherlandLineRectangle(hdc,cmd.points[0] , cmd.points[1], color); // Clip and draw
                drawHistory.emplace_back(cmd);
                isFirstClick = true;
            }

            ReleaseDC(hwnd, hdc);
            break;
        }

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hwnd, m, wp, lp);
    }

    return 0;
}