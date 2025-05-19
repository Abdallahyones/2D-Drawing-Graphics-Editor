#include <Windows.h>
#include <cmath>
#include <string>
#include <sstream>

using namespace std;

// --------------------------------------------------------------------- Utilities  -----------------------------------------------------------------
void swap(int& a, int& b) {
    int temp = a;
    a = b;
    b = temp;
}

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
        int d = 2 * dy - dx; 
        int d1 = 2 * (dy - dx); // Change when y increments
        int d2 = 2 * dy;        // Change when y doesn't increment
        SetPixel(hdc, x, y, color);
        while (true) {
            if (x == x2) break; 
            x += xStep;
            if (d < 0) {
                d += d2; // Only move in x direction
            }
            else {
                y += yStep;
                d += d1; // Move in both x and y directions
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

    if (step == 0) {
        SetPixel(hdc, x1, y1, color);
        return;
    }

    double x, y;

    for (double t = 0; t <= 1; t += step) {
        x = dx * t + x1;
        y = dy * t + y1;
        SetPixel(hdc, Round(x), Round(y), color);
    }
}

// Global variables
enum Algorithm { 
    DDA, 
    BRESENHAM,
    PARAMETRIC
};
Algorithm currentAlgorithm = DDA;
Point points[2];
int pointCount = 0;
bool drawing = false;

// Display instructions
void DisplayInstructions(HDC hdc) {
    stringstream ss;
    ss << "Current Algorithm: ";
    switch (currentAlgorithm) {
    case DDA: ss << "DDA"; break;
    case BRESENHAM: ss << "Bresenham"; break;
    case PARAMETRIC: ss << "Parametric"; break;
    }
    ss << "\n Click to set first point, then second point";

    TextOutA(hdc, 10, 10, ss.str().c_str(), (int)ss.str().length());
}

// Window Procedure
LRESULT WINAPI WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    HDC hdc;

    switch (msg) {
    case WM_LBUTTONDOWN:
        if (pointCount < 2) {
            points[pointCount].x = LOWORD(lParam);
            points[pointCount].y = HIWORD(lParam);
            pointCount++;

            hdc = GetDC(hwnd);
            if (pointCount == 1) {
                DrawPoint(hdc, points[0].x, points[0].y, RGB(255, 165, 0));
            }
            else if (pointCount == 2) {
                // Draw line between the two points
                switch (currentAlgorithm) {
                case DDA:
                    DrawLineDDA(hdc, points[0].x, points[0].y, points[1].x, points[1].y, RGB(255, 0, 0));
                    break;
                case BRESENHAM:
                    DrawLineBresenham(hdc, points[0].x, points[0].y, points[1].x, points[1].y, RGB(0, 255, 0));
                    break;
                case PARAMETRIC:
                    DrawLineParametric(hdc, points[0].x, points[0].y, points[1].x, points[1].y, RGB(0, 0, 255));
                    break;
                }
                // Draw second point
                DrawPoint(hdc, points[1].x, points[1].y, RGB(255, 165, 0));
                pointCount = 0; // Reset for next line
            }
            DisplayInstructions(hdc);
            ReleaseDC(hwnd, hdc);
        }
        break;

    case WM_KEYDOWN:
        if (wParam == '1') currentAlgorithm = DDA;
        else if (wParam == '2') currentAlgorithm = BRESENHAM;
        else if (wParam == '3') currentAlgorithm = PARAMETRIC;

        hdc = GetDC(hwnd);
        DisplayInstructions(hdc);
        ReleaseDC(hwnd, hdc);
        break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        hdc = BeginPaint(hwnd, &ps);
        DisplayInstructions(hdc);
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

// Main Entry Point
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Register window class
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"LineDrawingDemo";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    if (!RegisterClass(&wc)) return 0;

    // Create window
    HWND hwnd = CreateWindow(
        L"LineDrawingDemo", L"Line Drawing Algorithms Demo (1=DDA, 2=Bresenham, 3=Parametric)",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        NULL, NULL, hInstance, NULL);

    if (!hwnd) return 0;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // Message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}
