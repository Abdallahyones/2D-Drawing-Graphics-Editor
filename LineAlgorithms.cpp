#include <Windows.h>
#include <cmath>
#include <string>
#include <sstream>
#include "Common.h"

using namespace std;

// --------------------------------------------------------------------- Utilities  -----------------------------------------------------------------
void swap(int& a, int& b) {
    int temp = a;
    a = b;
    b = temp;
}





// ---------------------------------------------------------------- Line Drawing Algorithms functions -------------------------------------------------

// 1) DDA
void DrawLineDDA(HDC hdc, int startX, int startY, int endX, int endY, COLORREF color) {
    int dx = endX - startX;
    int dy = endY - startY;
    SetPixel(hdc, startX, startY, color);

    if (abs(dx) >= abs(dy)) { // X-dominant case
        int currentX = startX;
        int xIncrement = dx > 0 ? 1 : -1;
        double currentY = startY;
        double yIncrement = dx != 0 ? (double)dy / dx * xIncrement : 0;
        while (currentX != endX) {
            currentX += xIncrement;
            currentY += yIncrement;
            SetPixel(hdc, currentX, Round(currentY), color);
        }
    }
    else
    {
        // Y-dominant case (not used for horizontal scanlines)
        int currentY = startY;
        int yIncrement = dy > 0 ? 1 : -1;
        double currentX = startX;
        double xIncrement = dy != 0 ? (double)dx / dy * yIncrement : 0;
        while (currentY != endY) {
            currentY += yIncrement;
            currentX += xIncrement;
            SetPixel(hdc, Round(currentX), currentY, color);
        }
    }
}

// 2) LineBresenham
void DrawLineBresenham(HDC hdc, int x1, int y1, int x2, int y2, COLORREF color) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int xStep = x1 < x2 ? 1 : -1;
    int yStep = y1 < y2 ? 1 : -1;
    int x = x1, y = y1;

    // Adjust for |slope| <= 1 or |slope| > 1
    if (dx >= dy)
    { // |slope| <= 1, increment x, decide y
        int d = dx - 2 * dy;
        int d1 = 2 * (dx - dy); // Change when y increments
        int d2 = -2 * dy;        // Change when y doesn't increment
        SetPixel(hdc, x, y, color);
        while (true) {
            if (x == x2) break;
            x += xStep;
            if (d < 0) {
                d += d1; 
                y += yStep; // Move in both x and y directions
            }
            else {
                d += d2; // Only move in x direction 
            }
            SetPixel(hdc, x, y, color);
        }
    }
    else
    { // |slope| > 1, increment y, decide x
        int d = 2 * dx - dy;
        int d1 = 2 * (dx - dy);
        int d2 = 2 * dx;
        SetPixel(hdc, x, y, color);
        while (true) {
            if (y == y2) break; // Stop when y reaches y2
            y += yStep;
            if (d < 0) {
                d += d2; // Only move in y direction
            }
            else {
                x += xStep;
                d += d1; // Move in both x and y directions
            }
            SetPixel(hdc, x, y, color);
        }
    }
}
// 3) LineParametric
void DrawLineParametric(HDC hdc, int x1, int y1, int x2, int y2, COLORREF color) {
    // X = alpha1*T + Beta1
    // Y = alpha2*T + Beta2
    int dx = x2 - x1;    //alpha1
    int dy = y2 - y1;    //alpha2

    double step = 1.0 / max(abs(dx), abs(dy));

    double x, y;

    for (double t = 0; t <= 1; t += step) {
        x = dx * t + x1;
        y = dy * t + y1;
        SetPixel(hdc, Round(x), Round(y), color);
    }
}

// Global variables
Point points[2];
int pointCount = 0;
bool drawing = false;

LRESULT WINAPI drawLine(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam   , Algorithm currentAlgorithm , COLORREF color , DrawCommand &cmd) {
    HDC hdc;

    switch (msg) {
    case WM_LBUTTONDOWN:
        if (pointCount < 2) {
            points[pointCount].x = LOWORD(lParam);
            points[pointCount].y = HIWORD(lParam);
            pointCount++;

            cmd.points.emplace_back(points[0]);
            hdc = GetDC(hwnd);
            if (pointCount == 1) {
                DrawPoint(hdc, points[0].x, points[0].y, color);
            }
            else if (pointCount == 2) {
                cmd.points.emplace_back(points[1]);
                // Draw line between the two points
                switch (currentAlgorithm) {
                case ALGO_LINE_DDA:
                    DrawLineDDA(hdc, points[0].x, points[0].y, points[1].x, points[1].y,color );
                    break;
                case ALGO_LINE_BRESENHAM:
                    DrawLineBresenham(hdc, points[0].x, points[0].y, points[1].x, points[1].y, color );
                    break;
                case ALGO_LINE_MIDPOINT:
                    DrawLineParametric(hdc, points[0].x, points[0].y, points[1].x, points[1].y, color);
                    break;
                }
                // Draw second point
                DrawPoint(hdc, points[1].x, points[1].y,color);
                pointCount = 0; // Reset for next line
            }

            ReleaseDC(hwnd, hdc);
        }
        break;

    case WM_KEYDOWN:
        hdc = GetDC(hwnd);
        ReleaseDC(hwnd, hdc);
        break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        hdc = BeginPaint(hwnd, &ps);
        EndPaint(hwnd, &ps);
    }
    break;

    case WM_CLOSE:
        DestroyWindow(hwnd);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}


