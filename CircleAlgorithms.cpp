#include <Windows.h>
#include<sstream>
#include <cmath>
using namespace std;

int Round(double x) {
    return (int)(x + 0.5);
}

struct Point {
    double x, y;
    Point(double x = 0.0, double y = 0.0) : x(x), y(y) {}
};

// Draw a filled circle at the point
void DrawPoint(HDC hdc, int x, int y, COLORREF color) {
    HBRUSH brush = CreateSolidBrush(color);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
    Ellipse(hdc, x - 5, y - 5, x + 5, y + 5);
    SelectObject(hdc, oldBrush);
    DeleteObject(brush);
}

void Draw8Points(HDC hdc, int xc, int yc, int a, int b, COLORREF color)
{
    SetPixel(hdc, xc + a, yc + b, color);
    SetPixel(hdc, xc - a, yc + b, color);
    SetPixel(hdc, xc - a, yc - b, color);
    SetPixel(hdc, xc + a, yc - b, color);
    SetPixel(hdc, xc + b, yc + a, color);
    SetPixel(hdc, xc - b, yc + a, color);
    SetPixel(hdc, xc - b, yc - a, color);
    SetPixel(hdc, xc + b, yc - a, color);
}

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

enum CircleAlgorithm {
    DIRECT,
    POLAR,
    ITERATIVE_POLAR,
    BRESENHAM,
    MODIFIED_BRESENHAM
};

CircleAlgorithm currentCircleAlgorithm = DIRECT;
Point circlePoints[2]; // [0]: center, [1]: point on circumference
int circlePointCount = 0;

void DisplayCircleInstructions(HDC hdc) {
    stringstream ss;
    ss << "Current Circle Algorithm: ";
    switch (currentCircleAlgorithm) {
    case DIRECT: ss << "(Direct)"; break;
    case POLAR: ss << "(Polar)"; break;
    case ITERATIVE_POLAR: ss << "(Iterative Polar)"; break;
    case BRESENHAM: ss << "(Bresenham)"; break;
    case MODIFIED_BRESENHAM: ss << "(Modified Bresenham)"; break;
    }
    ss << " Click center, then radius point,Press keys to switch algorithms.(1) DIRECT,  (2)POLAR , (3)ITERATIVE_POLAR, (4) BRESENHAM ,(5) MODIFIED_BRESENHAM";

    TextOutA(hdc, 10, 10, ss.str().c_str(), (int)ss.str().length());
}

LRESULT WINAPI WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    HDC hdc;

    switch (msg) {
    case WM_LBUTTONDOWN:
        if (circlePointCount < 2) {
            circlePoints[circlePointCount].x = LOWORD(lParam);
            circlePoints[circlePointCount].y = HIWORD(lParam);
            circlePointCount++;

            hdc = GetDC(hwnd);
            DrawPoint(hdc, circlePoints[circlePointCount - 1].x, circlePoints[circlePointCount - 1].y, RGB(255, 165, 0));

            if (circlePointCount == 2) {
                int dx = circlePoints[1].x - circlePoints[0].x;
                int dy = circlePoints[1].y - circlePoints[0].y;
                int R = (int)sqrt(dx * dx + dy * dy);

                switch (currentCircleAlgorithm) {
                case DIRECT:
                    CircleDirect(hdc, circlePoints[0].x, circlePoints[0].y, R, RGB(255, 0, 0)); break;
                case POLAR:
                    CirclePolar(hdc, circlePoints[0].x, circlePoints[0].y, R, RGB(0, 255, 0)); break;
                case ITERATIVE_POLAR:
                    CircleIterativePolar(hdc, circlePoints[0].x, circlePoints[0].y, R, RGB(0, 0, 255)); break;
                case BRESENHAM:
                    CircleBresenham(hdc, circlePoints[0].x, circlePoints[0].y, R, RGB(255, 255, 0)); break;
                case MODIFIED_BRESENHAM:
                    CircleModifiedBresenham(hdc, circlePoints[0].x, circlePoints[0].y, R, RGB(255, 0, 255)); break;
                }

                circlePointCount = 0; // Reset for next circle
            }

            DisplayCircleInstructions(hdc);
            ReleaseDC(hwnd, hdc);
        }
        break;

    case WM_KEYDOWN:
        if (wParam == '1') currentCircleAlgorithm = DIRECT;
        else if (wParam == '2') currentCircleAlgorithm = POLAR;
        else if (wParam == '3') currentCircleAlgorithm = ITERATIVE_POLAR;
        else if (wParam == '4') currentCircleAlgorithm = BRESENHAM;
        else if (wParam == '5') currentCircleAlgorithm = MODIFIED_BRESENHAM;

        hdc = GetDC(hwnd);
        DisplayCircleInstructions(hdc);
        ReleaseDC(hwnd, hdc);
        break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        hdc = BeginPaint(hwnd, &ps);
        DisplayCircleInstructions(hdc);
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

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow) {
    WNDCLASS wc = {};
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hInstance = hInstance;
    wc.lpfnWndProc = WndProc;
    wc.lpszClassName = L"MyClass";
    wc.lpszMenuName = NULL;
    wc.style = CS_HREDRAW | CS_VREDRAW;

    RegisterClass(&wc);

    HWND hwnd = CreateWindow(L"MyClass", L"Circle Algorithms",
        WS_OVERLAPPEDWINDOW, 100, 100, 800, 600, NULL, NULL, hInstance, NULL);
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
