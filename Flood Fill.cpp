#include <windows.h>
#include <queue>
#include "Common.h"

void DrawRec(const Algorithm &algo, HDC hdc, Point p ,COLORREF boundaryColor);

using namespace std;





// --- Bresenham Circle Drawing ---
void CircleBresenhamM(HDC hdc, int xc, int yc, int radius, COLORREF c) {
    int x = 0, y = radius, d = 1 - radius;
    Draw8Points(hdc, xc, yc, x, y, c);
    while (x < y) {
        if (d < 0)
            d += 2 * x + 3;
        else {
            d += 2 * (x - y) + 5;
            y--;
        }
        x++;
        Draw8Points(hdc, xc, yc, x, y, c);
    }
}

void FloodFillOneOctantRecursive(HDC hdc, int xc, int yc, int x, int y, COLORREF boundaryColor, COLORREF fillColor) {

    // Only work in first octant (y ≤ x)
    if (x < 0 || y < 0 || y > x) return;

    // Check if this point has already been filled
    COLORREF currentColor = GetPixel(hdc, xc + x, yc + y);
    if (currentColor == boundaryColor || currentColor == fillColor) return;

    // Fill symmetric points
    Draw8Points(hdc, xc, yc, x, y, fillColor);

    // Recurse within the 1/8 region only
    FloodFillOneOctantRecursive(hdc, xc, yc, x + 1, y, boundaryColor, fillColor);
    FloodFillOneOctantRecursive(hdc, xc, yc, x, y + 1, boundaryColor, fillColor);
}



// --- Non-Recursive Flood Fill ---
void FloodFillNonRecursive(HDC hdc, int x, int y, COLORREF boundaryColor, COLORREF fillColor) {
    queue<Point> Q;
    Q.push(Point(x, y));
    while (!Q.empty()) {
        Point v = Q.front();
        Q.pop();
        COLORREF current = GetPixel(hdc, v.x, v.y);
        if (current == boundaryColor || current == fillColor) continue;
        SetPixel(hdc, v.x, v.y, fillColor);
        Q.push(Point(v.x + 1, v.y));
        Q.push(Point(v.x - 1, v.y));
        Q.push(Point(v.x, v.y + 1));
        Q.push(Point(v.x, v.y - 1));
    }
}


// --- Global state for toggle ---


// --- Window Procedure ---
LRESULT WINAPI drawRecursive(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp ,Algorithm algo, COLORREF color , DrawCommand& cmd) {
    switch (msg) {
    case WM_LBUTTONDOWN: {
        HDC hdc = GetDC(hwnd);
        int x = LOWORD(lp);
        int y = HIWORD(lp);
        cmd.points.emplace_back(x,y);
        cmd.fillColor = RGB(0, 0, 0);
        cmd.radius= 150;
        COLORREF boundaryColor =color; // black boundary
        // Draw the circle centered at (x, y) with radius 150
        DrawRec(algo, hdc, cmd.points.back(), boundaryColor);
        drawHistory.emplace_back(cmd);
        ReleaseDC(hwnd, hdc);
        break;
    }
    case WM_RBUTTONDOWN: {

              break;
    }
    case WM_CLOSE:
        DestroyWindow(hwnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wp, lp);
    }
    return 0;
}

void DrawRec(const Algorithm &algo, HDC hdc, Point p ,COLORREF boundaryColor) {
    auto[x,y] = p;
    CircleBresenhamM(hdc, x, y, 150, RGB(0, 0, 0));
    if (algo == ALGO_Recursive) {
        FloodFillOneOctantRecursive(hdc, x, y, 0, 0, boundaryColor, RGB(0,255,0));
    }
    else {
        FloodFillNonRecursive(hdc, x, y, boundaryColor, RGB(0,255,0));
    }
}

