#include "Common.h"

// Constructor implementation
Point::Point(double x, double y) : x(x), y(y) {}

// Round function
int Round(double x) {
    return static_cast<int>(x + 0.5);
}

// Draw a filled circle at the point
void DrawPoint(HDC hdc, int x, int y, COLORREF color) {
    HBRUSH brush = CreateSolidBrush(color);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
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
