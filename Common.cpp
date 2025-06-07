#include "Common.h"

std::vector<DrawCommand> drawHistory;
bool isRectangleDrawn = false;
bool isSquareDrawn = false;

// Constructor implementation
Point::Point(double x, double y) : x(x), y(y) {}

// Round function
int Round(double x) {
    return static_cast<int>(x + 0.5);
}

// Draw a filled circle at the point
void DrawPoint(HDC hdc, int x, int y, COLORREF color) {
    HBRUSH brush = CreateSolidBrush(color);
    HBRUSH oldBrush = (HBRUSH) SelectObject(hdc, brush);
    Ellipse(hdc, x - 5, y - 5, x + 5, y + 5);
    SelectObject(hdc, oldBrush);
    DeleteObject(brush);
}

// Draw 8 symmetric points for circle
void Draw8Points(HDC hdc, int xc, int yc, int x, int y, COLORREF color) {
    SetPixel(hdc, xc + x, yc + y, color);
    SetPixel(hdc, xc - x, yc + y, color);
    SetPixel(hdc, xc - x, yc - y, color);
    SetPixel(hdc, xc + x, yc - y, color);
    SetPixel(hdc, xc + y, yc + x, color);
    SetPixel(hdc, xc - y, yc + x, color);
    SetPixel(hdc, xc - y, yc - x, color);
    SetPixel(hdc, xc + y, yc - x, color);
}

void drawRectangle(HWND hwnd) {
    if (isRectangleDrawn)return;
    changeRectangleWindow();
    HDC hdc = GetDC(hwnd);
    Rectangle(hdc, xLeft, yTop, xRight, yBottom);
    ReleaseDC(hwnd, hdc);
}

void drawRectangleSquare(HWND hwnd) {
    if (isSquareDrawn)return;
    changeSquareWindow();
    HDC hdc = GetDC(hwnd);
    Rectangle(hdc, xLeft, yTop, Xright, Ybottom);
    ReleaseDC(hwnd, hdc);
}

void changeRectangleWindow() {
    isRectangleDrawn = true;
}

void changeSquareWindow() {
    isSquareDrawn = true;
}