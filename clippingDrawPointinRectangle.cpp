#include <Windows.h>
#include <vector>
#include "Common.h"

void ClippingPoint(COLORREF color, DrawCommand &cmd, HDC hdc, int x, int y);

using namespace std;

// Draw Point in Rectangle

void DrawPointRectangle(HDC hdc, Point p, COLORREF c) {
    auto [x, y] = p;
    if (x >= xLeft && x <= xRight && y >= yBottom && y <= yTop) {
        DrawPoint(hdc, x, y, RGB(0,0,0));

    }
}



LRESULT drawPointRectangle(HWND hwnd, UINT m, WPARAM wp, LPARAM lp, COLORREF color, DrawCommand &cmd) {
    static bool isRectangleDrawn = false;
    HDC hdc;
    PAINTSTRUCT ps;

    switch (m) {
        case WM_LBUTTONDOWN: {
            hdc = GetDC(hwnd);
            int x = LOWORD(lp);
            int y = HIWORD(lp);
            Rectangle(hdc, xLeft, yTop, xRight, yBottom);
            cmd.points.emplace_back(x, y);
            ClippingPoint(color, cmd, hdc, x, y);
            drawHistory.emplace_back(cmd);
            ReleaseDC(hwnd, hdc);
            break;
        }

        case WM_DESTROY:
            break;

        default:
            return DefWindowProc(hwnd, m, wp, lp);
    }

    return 0;
}


void ClippingPoint(COLORREF color, DrawCommand &cmd,HDC hdc,int x, int y) {
    //HDC hdc;
    cout<<cmd.points.back().x<<" "<<cmd.points.back().y;
    DrawPointRectangle(hdc, cmd.points.back(), color);

}
