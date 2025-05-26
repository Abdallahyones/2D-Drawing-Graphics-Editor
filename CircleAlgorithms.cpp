#include <Windows.h>
#include<sstream>
#include <cmath>
#include "Common.h"
using namespace std;


// ------------------------------------------------------------------------ Circle Algorithms ----------------------------------------------------------

// 1) Direct : using Cartesian equation (x-xc)^2 + (y-yc)^2 = R^2
void CircleDirect(HDC hdc, int xc, int yc, int R, COLORREF color)
{
    // Second Octant
    int x = 0, y = R;
    int R2 = R * R;
    Draw8Points(hdc, xc, yc, x, y, color);
    while (x < y)
    {
        x++;
        y = Round(sqrt((double)(R2 - x * x)));
        Draw8Points(hdc, xc, yc, x, y, color);
    }
}

// 2) Polar : x = xc + R*Cos(theta) y = yc + R*Sin(theta)
void CirclePolar(HDC hdc, int xc, int yc, int R, COLORREF color)
{
    // First Octant
    int x = R, y = 0;
    double theta = 0, dtheta = 1.0 / R;
    Draw8Points(hdc, xc, yc, x, y, color);
    while (x > y)
    {
        theta += dtheta;
        x = Round(R * cos(theta));
        y = Round(R * sin(theta));
        Draw8Points(hdc, xc, yc, x, y, color);
    }
}

// 3) IterativePolar 
/*
     x' = R (cosθ cosΔθ - sinθ sinΔθ)
        = (R cosθ) cosΔθ - (R sinθ) sinΔθ
        = x cosΔθ - y sinΔθ

     y' = R (cosθ sinΔθ + sinθ cosΔθ)
        = (R cosθ) sinΔθ + (R sinθ) cosΔθ
        = x sinΔθ + y cosΔθ
*/
void CircleIterativePolar(HDC hdc, int xc, int yc, int R, COLORREF color)
{
    // First Octant
    double x = R, y = 0;
    double dtheta = 1.0 / R;
    double cdtheta = cos(dtheta), sdtheta = sin(dtheta);
    Draw8Points(hdc, xc, yc, R, 0, color);
    while (x > y)
    {
        double x1 = x * cdtheta - y * sdtheta;
        y = x * sdtheta + y * cdtheta;
        x = x1;
        Draw8Points(hdc, xc, yc, round(x), round(y), color);
    }
}
// 4) MidPoint
void CircleBresenham(HDC hdc, int xc, int yc, int R, COLORREF color)
{
    // First Octant
    int x = 0, y = R;
    int d = 1 - R;
    Draw8Points(hdc, xc, yc, x, y, color);
    while (x < y)
    {
        if (d < 0)
            d += 2 * x + 2;
        else
        {
            d += 2 * (x - y) + 5;
            y--;
        }
        x++;
        Draw8Points(hdc, xc, yc, x, y, color);
    }
}
// 4) ModifiedMidPoint
void CircleModifiedBresenham(HDC hdc, int xc, int yc, int R, COLORREF color)
{
    // First Octant
    int x = 0, y = R;
    int d = 1 - R;
    int c1 = 3, c2 = 5 - 2 * R;
    Draw8Points(hdc, xc, yc, x, y, color);
    while (x < y)
    {
        if (d < 0)
        {
            d += c1;
            c2 += 2;
        }
        else
        {
            d += c2;
            c2 += 4;
            y--;
        }
        c1 += 2;
        x++;
        Draw8Points(hdc, xc, yc, x, y, color);
    }
}




Point circlePoints[2];
int circlePointCount = 0;



LRESULT WINAPI drawCircle(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam , Algorithm currentCircleAlgorithm , COLORREF color , DrawCommand& cmd) {
    HDC hdc;

    switch (msg) {
    case WM_LBUTTONDOWN:
        if (circlePointCount < 2) {
            circlePoints[circlePointCount].x = LOWORD(lParam);
            circlePoints[circlePointCount].y = HIWORD(lParam);
            circlePointCount++;
            hdc = GetDC(hwnd);
            DrawPoint(hdc, circlePoints[circlePointCount - 1].x, circlePoints[circlePointCount - 1].y, color);
            cmd.points.emplace_back(circlePoints[0]);
            if (circlePointCount == 2) {
            cmd.points.emplace_back(circlePoints[1]);
                int dx = circlePoints[1].x - circlePoints[0].x;
                int dy = circlePoints[1].y - circlePoints[0].y;
                int R = (int)sqrt(dx * dx + dy * dy);

                switch (currentCircleAlgorithm) {
                case ALGO_CIRCLE_DIRECT:
                    CircleDirect(hdc, circlePoints[0].x, circlePoints[0].y, R,color); break;
                case ALGO_CIRCLE_POLAR:
                    CirclePolar(hdc, circlePoints[0].x, circlePoints[0].y, R, color); break;
                case ALGO_CIRCLE_ITERATIVE_POLAR:
                    CircleIterativePolar(hdc, circlePoints[0].x, circlePoints[0].y, R, color); break;
                case ALGO_CIRCLE_MIDPOINT:
                    CircleBresenham(hdc, circlePoints[0].x, circlePoints[0].y, R, color); break;
                case ALGO_CIRCLE_MODIFIED_MIDPOINT:
                    CircleModifiedBresenham(hdc, circlePoints[0].x, circlePoints[0].y, R, color); break;
                }

                circlePointCount = 0; // Reset for next circle
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


