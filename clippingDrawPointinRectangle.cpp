#include <Windows.h>
#include <vector>
#include "Common.h"

void ClippingPoint(COLORREF color, DrawCommand &cmd, HDC hdc, int x, int y);

using namespace std;

// Draw Point in Rectangle

void DrawPointRectangle(HDC hdc, Point p, COLORREF c) {
    auto [x, y] = p;
    if (x >= xLeft && x <= xRight && y >= yBottom && y <= yTop) {
        SetPixel(hdc, x, y, c);
    }
}

void DrawRectangle(HWND hwnd, HDC hdc) {
    PAINTSTRUCT ps;
    hdc = BeginPaint(hwnd, &ps);
    Rectangle(hdc, xLeft, yTop, xRight, yBottom); // draw only once
    EndPaint(hwnd, &ps);
}

LRESULT drawPointRectangle(HWND hwnd, UINT m, WPARAM wp, LPARAM lp, COLORREF color, DrawCommand &cmd) {
    HDC hdc;
    PAINTSTRUCT ps;
    switch (m) {
        case WM_PAINT:
            hdc = BeginPaint(hwnd, &ps);
            Rectangle(hdc, xLeft, yBottom, xRight, yTop); // Draw clipping rectangle
            EndPaint(hwnd, &ps);
            break;
        case WM_LBUTTONDOWN: {
            hdc = BeginPaint(hwnd, &ps);
            Rectangle(hdc, xLeft, yTop, xRight, yBottom);
            EndPaint(hwnd, &ps);
            hdc = GetDC(hwnd);
            int x = LOWORD(lp);
            int y = HIWORD(lp);
            cmd.points.emplace_back(x, y);
            ClippingPoint(color, cmd, hdc, x, y);
            drawHistory.emplace_back(cmd);
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

void ClippingPoint(COLORREF color, DrawCommand &cmd,HDC hdc,int x, int y) {
    //HDC hdc;
    DrawPointRectangle(hdc, cmd.points.back(), color);

}
