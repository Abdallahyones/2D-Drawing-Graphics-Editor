#if defined(UNICODE) && !defined(_UNICODE)
#define _UNICODE
#elif defined(_UNICODE) && !defined(UNICODE)
#define UNICODE
#endif

#include <tchar.h>
#include <windows.h>
#include <math.h>
#include <queue>
#include "Common.h"

using namespace std;

/*  Declare Windows procedure  */
LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);


// Vector4 for polynomial coefficients
class Vector4 {
    double v[4];
public:
    Vector4(double a = 0, double b = 0, double c = 0, double d = 0) {
        v[0] = a;
        v[1] = b;
        v[2] = c;
        v[3] = d;
    }

    Vector4(double a[]) { memcpy(v, a, 4 * sizeof(double)); }

    double &operator[](int i) { return v[i]; }
};

// Matrix4 for Hermite basis
class Matrix4 {
    Vector4 M[4];
public:
    Matrix4(double A[]) {
        for (int i = 0; i < 4; ++i)
            M[i] = Vector4(A + i * 4);
    }

    Vector4 &operator[](int i) { return M[i]; }
};

// Utility: Matrix * Vector
Vector4 operator*(Matrix4 M, Vector4 &b) {
    Vector4 res;
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            res[i] += M[i][j] * b[j];
    return res;
}

// Utility: Dot product
double DotProduct(Vector4 &a, Vector4 &b) {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + a[3] * b[3];
}

// Compute Hermite coefficients
Vector4 GetHermiteCoeff(double x0, double s0, double x1, double s1) {
    static double H[16] = {
            2, -2, 1, 1,
            -3, 3, -2, -1,
            0, 0, 1, 0,
            1, 0, 0, 0
    };
    static Matrix4 basis(H);
    Vector4 v(x0, x1, s0, s1);
    return basis * v;
}

// Hermite curve drawing function
void DrawHermiteCurve(HDC hdc, Point &P0, Point &T0, Point &P1, Point &T1, int numpoints,
                      COLORREF color) {

    // draw by color
    HPEN pen = CreatePen(PS_SOLID, 1, color); // Create pen with given color
    HPEN oldPen = (HPEN)SelectObject(hdc, pen); // Select pen into DC and save old one

    Vector4 xcoeff = GetHermiteCoeff(P0.x, T0.x, P1.x, T1.x);
    Vector4 ycoeff = GetHermiteCoeff(P0.y, T0.y, P1.y, T1.y);
    if (numpoints < 2) return;
    double dt = 1.0 / (numpoints - 1);
    for (double t = 0; t <= 1; t += dt) {
        Vector4 vt;
        vt[3] = 1;
        for (int i = 2; i >= 0; i--)
            vt[i] = vt[i + 1] * t;
        int x = round(DotProduct(xcoeff, vt));
        int y = round(DotProduct(ycoeff, vt));
        if (t == 0)
            MoveToEx(hdc, x, y, NULL);
        else
            LineTo(hdc, x, y);
    }
    // restore old color
    SelectObject(hdc, oldPen); // Restore old pen
    DeleteObject(pen);         // Delete created pen
}

// Bézier curve as a special Hermite curve
void DrawBezierCurve(HDC hdc, Point &P0, Point &P1, Point &P2, Point &P3, int numpoints,
                     COLORREF color) {
    Point T0(3 * (P1.x - P0.x), 3 * (P1.y - P0.y));
    Point T1(3 * (P3.x - P2.x), 3 * (P3.y - P2.y));
    DrawHermiteCurve(hdc, P0, T0, P3, T1, numpoints, color);
}

// Cardinal spline using Hermite curve segments
// Cardinal spline using Hermite curve segments
void DrawCardinalSpline(HDC hdc, Point P[], int n, double c, int numpoints, COLORREF color) {
    if (n < 2) return; // Need at least 2 points to draw anything

    // draw by color
    HPEN pen = CreatePen(PS_SOLID, 1, color);
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);

    if (n == 2) {
        // Just draw a line
        MoveToEx(hdc, (int) P[0].x, (int) P[0].y, NULL);
        LineTo(hdc, (int) P[1].x, (int) P[1].y);

        // restore old color
        SelectObject(hdc, oldPen);
        DeleteObject(pen);
        return;
    }

    double tension = (1.0 - c) / 2.0;

    for (int i = 1; i < n - 1; ++i) {
        Point P0 = P[i - 1];
        Point P1 = P[i];
        Point P2 = P[i + 1];

        Point T0(tension * (P2.x - P0.x), tension * (P2.y - P0.y));
        Point T1;

        if (i + 2 < n) {
            Point P3 = P[i + 2];
            T1 = Point(tension * (P3.x - P1.x), tension * (P3.y - P1.y));
        } else {
            // For the last segment, use the same tangent as previous to prevent indexing past the array
            T1 = Point(tension * (P2.x - P0.x), tension * (P2.y - P0.y));
        }

        DrawHermiteCurve(hdc, P1, T0, P2, T1, numpoints, color);

        // restore old color
        SelectObject(hdc, oldPen);
        DeleteObject(pen);
    }

    // Optional: draw small dots for control points
    /*
    for (int i = 0; i < n; ++i) {
        Ellipse(hdc, P[i].x - 2, P[i].y - 2, P[i].x + 2, P[i].y + 2);
    }
    */
}


void CircleBresham(HDC hdc, int xc, int yc, int radius, COLORREF c) {
    int x = 0, y = radius, d = 1 - radius;
    Draw8Points(hdc, xc, yc, x, y, c);
    while (x < y) {
        if (d < 0) d += 2 * x + 2;
        else {
            d += (2 * (x - y) + 5);
            y--;
        }
        x++;
        Draw8Points(hdc, xc, yc, x, y, c);
    }

}


void FillQuarterCircle_line(HDC hdc, int xc, int yc, int r, int quarter, COLORREF c1 ,COLORREF c2) {
    for (int y = 0; y <= r; ++y) {
        int xLimit = (int) round(sqrt(r * r - y * y));
        COLORREF currentColor = (y % 2 == 0) ? c1 : c2;

        // Create pen for this line
        HPEN pen = CreatePen(PS_SOLID, 1, currentColor);
        HPEN oldPen = (HPEN) SelectObject(hdc, pen);

        switch (quarter) {
            case 1:
                MoveToEx(hdc, xc, yc - y, NULL);
                LineTo(hdc, xc + xLimit, yc - y);
                break;
            case 2:
                MoveToEx(hdc, xc - xLimit, yc - y, NULL);
                LineTo(hdc, xc, yc - y);
                break;
            case 3:
                MoveToEx(hdc, xc - xLimit, yc + y, NULL);
                LineTo(hdc, xc, yc + y);
                break;
            case 4:
                MoveToEx(hdc, xc, yc + y, NULL);
                LineTo(hdc, xc + xLimit, yc + y);
                break;
        }

        // Restore and delete pen
        SelectObject(hdc, oldPen);
        DeleteObject(pen);
    }
}

void FillQuarterCircle_circle(HDC hdc, int xc, int yc, int R, int quarter, COLORREF c) {
    int step = 8;                 // spacing between small circles
    int smallRadius = 3;         // radius of each small circle

    for (int y = 0; y <= R; y += step) {
        for (int x = 0; x <= R; x += step) {
            if (x * x + y * y <= R * R) {
                int drawX = 0, drawY = 0;

                switch (quarter) {
                    case 1:
                        drawX = xc + x;
                        drawY = yc - y;
                        break; // Top Right
                    case 2:
                        drawX = xc - x;
                        drawY = yc - y;
                        break; // Top Left
                    case 3:
                        drawX = xc - x;
                        drawY = yc + y;
                        break; // Bottom Left
                    case 4:
                        drawX = xc + x;
                        drawY = yc + y;
                        break; // Bottom Right
                    default:
                        return;
                }

                CircleBresham(hdc, drawX, drawY, smallRadius, c);
            }
        }
    }
}

void DrawLineDDAFilll(HDC hdc, int x1, int y1, int x2, int y2, COLORREF c) {
    int dx = x2 - x1;
    int dy = y2 - y1;

    int steps = max(abs(dx), abs(dy));

    // calculate increment in x and y
    float xInc = dx / (float) steps;
    float yInc = dy / (float) steps;

    float x = x1;
    float y = y1;

    for (int i = 0; i <= steps; i++) {
        SetPixel(hdc, Round(x), Round(y), c);
        x += xInc;
        y += yInc;
    }
}

void DrawLineMidpoint(HDC hdc, int x1, int y1, int x2, int y2, COLORREF c) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);

    int x = x1;
    int y = y1;

    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;

    bool steep = dy > dx;

    if (steep) {
        std::swap(dx, dy);
    }

    int d = 2 * dy - dx;
    int dE = 2 * dy;
    int dNE = 2 * (dy - dx);

    SetPixel(hdc, x, y, c);

    for (int i = 0; i < dx; ++i) {
        if (d < 0) {
            d += dE;
            if (steep)
                y += sy;
            else
                x += sx;
        } else {
            d += dNE;
            x += sx;
            y += sy;
        }

        SetPixel(hdc, x, y, c);
    }
}


void FillRectangleWithBezier(HDC hdc, Point topLeft, int width, int height, COLORREF color) {
    int numCurves = height / 5; // Curve density

    for (int i = 0; i <= numCurves; ++i) {
        double t = (double) i / numCurves;
        double y = topLeft.y + t * height;

        // Control points for horizontal Bézier curve
        Point P0(topLeft.x, y);
        Point P1(topLeft.x + width * 0.33, y + sin(i) * 5); // Optional wave effect
        Point P2(topLeft.x + width * 0.66, y - sin(i) * 5);
        Point P3(topLeft.x + width, y);

        DrawBezierCurve(hdc, P0, P1, P2, P3, 100, color);
    }
}


void FillSquareWithHermite(HDC hdc, Point topLeft, int sideLength, COLORREF color) {
    Point P0, P1, T0, T1;
    int numCurves = sideLength / 5; // Density of curves

    for (int i = 0; i <= numCurves; ++i) {
        double t = (double) i / numCurves;
        double x = topLeft.x + t * sideLength;

        // Start and end points (vertical lines)
        P0 = Point(x, topLeft.y);
        P1 = Point(x, topLeft.y + sideLength);

        // Add curvature in X-direction using a sine function
        double dx = 20 * sin(t * 3.14159);  // 20 = curvature amplitude

        T0 = Point(dx, sideLength / 2);  // Curved tangent at top
        T1 = Point(-dx, sideLength / 2); // Opposite curve at bottom

        DrawHermiteCurve(hdc, P0, T0, P1, T1, 100, color);
    }
}


void DrawRectangle(HDC hdc, int x1, int y1, int x2, int y2 , COLORREF color_shape , COLORREF color_filling) {
    // Determine the actual corners of the rectangle
    int left = min(x1, x2);
    int right = max(x1, x2);
    int top = min(y1, y2);
    int bottom = max(y1, y2);

    // Draw top edge (DDA) - Red
    DrawLineDDAFilll(hdc, left, top, right, top, color_shape);

    // Draw bottom edge (DDA) - Green
    DrawLineDDAFilll(hdc, left, bottom, right, bottom, color_shape);

    // Draw left edge (Midpoint) - Blue
    DrawLineMidpoint(hdc, left, top, left, bottom, color_shape);

    // Draw right edge (Midpoint) - Cyan
    DrawLineMidpoint(hdc, right, top, right, bottom, color_shape);

    int height = bottom - top;
    int width = right - left;
    Point topLeft(left, top);
    FillRectangleWithBezier(hdc, topLeft, width, height, color_filling);

}

void DrawSquare(HDC hdc, int x1, int y1, int x2, int y2 , COLORREF color_shape , COLORREF color_filling) {
    // Calculate the difference in x and y
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int side = min(dx, dy);  // Choose the smaller side to form a square

    // Determine the direction
    int xDir = (x2 >= x1) ? 1 : -1;
    int yDir = (y2 >= y1) ? 1 : -1;

    // Adjust x2 and y2 to form a square
    x2 = x1 + side * xDir;
    y2 = y1 + side * yDir;

    // Draw as a rectangle with equal sides
    int left = min(x1, x2);
    int right = max(x1, x2);
    int top = min(y1, y2);
    int bottom = max(y1, y2);

    // Draw top edge (DDA) - Red
    DrawLineDDAFilll(hdc, left, top, right, top, color_shape);

    // Draw bottom edge (DDA) - Green
    DrawLineDDAFilll(hdc, left, bottom, right, bottom, color_shape);

    // Draw left edge (Midpoint) - Blue
    DrawLineMidpoint(hdc, left, top, left, bottom, color_shape);

    // Draw right edge (Midpoint) - Cyan
    DrawLineMidpoint(hdc, right, top, right, bottom, color_shape);

    Point topLeft(left, top);
    FillSquareWithHermite(hdc, topLeft, side, color_filling);
}

LRESULT CALLBACK FillingAlgo(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, Algorithm algo, DrawCommand &cmd) {
    HDC hdc;
    static int clickCount = 0;
    static int x1, x2, y1, y2, r1;
    static int selectedQuarter = 4;

    static int xStart, yStart;
    static bool isFirstClick = true;
    static bool waitingForQuarterClick = false;
    static std::vector<Point> controlPoints;
    const int numpoints = 1000;
    const double tension = 0.0;

    switch (message) {
        case WM_LBUTTONDOWN: {
            cout<<"There is click Point in Position (x,y) " <<LOWORD(lParam) <<" "<< HIWORD(lParam)<<"\n";
            hdc = GetDC(hwnd);
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);

            if (algo == ALGO_FILL_RECTANGLE_BEZIER_HORIZONTAL || algo == ALGO_FILL_SQUARE_HERMIT_VERTICAL) {
                if (isFirstClick) {
                    xStart = x;
                    yStart = y;
                    isFirstClick = false;
                } else {
                    int xEnd = x;
                    int yEnd = y;
                    cmd.points.emplace_back(xStart, yStart);
                    cmd.points.emplace_back(xEnd, yEnd);
                    cmd.quarter =selectedQuarter;
                    if (algo == ALGO_FILL_RECTANGLE_BEZIER_HORIZONTAL) {
                        DrawRectangle(hdc, xStart, yStart, xEnd, yEnd , cmd.shapeColor , cmd.fillColor);
                    } else {
                        DrawSquare(hdc, xStart, yStart, xEnd, yEnd , cmd.shapeColor , cmd.fillColor);
                    }
                    drawHistory.emplace_back(cmd);

                    isFirstClick = true;
                }
            }
            else if (algo == ALGO_FILL_CIRCLE_LINES || algo == ALGO_FILL_CIRCLE_CIRCLES) {
                if (clickCount == 0) {
                    x1 = x;
                    y1 = y;
                    clickCount++;
                } else if (clickCount == 1) {
                    x2 = x;
                    y2 = y;

                    r1 = Round(sqrt(pow((x1 - x2), 2) + pow((y1 - y2), 2)));
                    CircleBresham(hdc, x1, y1, r1, cmd.shapeColor);
                    waitingForQuarterClick = true;
                    clickCount++;
                } else if (clickCount == 2 && waitingForQuarterClick) {

                    int dx = x - x1;
                    int dy = y1 - y;

                    if (dx >= 0 && dy >= 0)
                        selectedQuarter = 1;
                    else if (dx < 0 && dy >= 0)
                        selectedQuarter = 2;
                    else if (dx < 0 && dy < 0)
                        selectedQuarter = 3;
                    else if (dx >= 0 && dy < 0)
                        selectedQuarter = 4;

                    cmd.quarter = selectedQuarter;
                    cmd.points.emplace_back(x1, y1);
                    cmd.points.emplace_back(x2, y2);
                    if (algo == ALGO_FILL_CIRCLE_LINES)
                        FillQuarterCircle_line(hdc, x1, y1, r1, selectedQuarter, cmd.fillColor , RGB(255 , 255 , 255));
                    else
                        FillQuarterCircle_circle(hdc, x1, y1, r1, selectedQuarter, cmd.fillColor);

                    drawHistory.emplace_back(cmd);
                    clickCount = 0;
                    waitingForQuarterClick = false;
                }
            }

            else if (algo == ALGO_CARDINAL_SPLINE) {
                controlPoints.emplace_back(x, y);
                cmd.controlPoints = controlPoints;
                if (controlPoints.size() >= 4) {
                    cmd.curveTension = tension;
                    DrawCardinalSpline(hdc, controlPoints.data(), controlPoints.size(), tension, numpoints,
                                       cmd.shapeColor);
                    drawHistory.emplace_back(cmd);
                    controlPoints.clear();
                }
            }

            ReleaseDC(hwnd, hdc);
            break;
        }

        case WM_KEYDOWN:
            if (wParam >= '1' && wParam <= '4') {
                selectedQuarter = wParam - '0';
            }
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hwnd, message, wParam, lParam);
    }


    return 0;
}

void ChoosedrawFilling(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, DrawCommand &cmd) {
    HDC hdc;
    hdc = GetDC(hwnd);
    if (cmd.algorithm == ALGO_FILL_RECTANGLE_BEZIER_HORIZONTAL || cmd.algorithm == ALGO_FILL_SQUARE_HERMIT_VERTICAL) {

        if (cmd.algorithm == ALGO_FILL_RECTANGLE_BEZIER_HORIZONTAL) {
            DrawRectangle(hdc, cmd.points[0].x, cmd.points[0].y, cmd.points[1].x, cmd.points[1].y , cmd.shapeColor , cmd.fillColor);
        } else {
            DrawSquare(hdc, cmd.points[0].x, cmd.points[0].y, cmd.points[1].x, cmd.points[1].y , cmd.shapeColor , cmd.fillColor);
        }
    } else if (cmd.algorithm == ALGO_FILL_CIRCLE_LINES || cmd.algorithm == ALGO_FILL_CIRCLE_CIRCLES) {

        auto [x1, y1] = cmd.points[0];
        auto [x2, y2] = cmd.points[1];

        int r1 = Round(sqrt(pow((x1 - x2), 2) + pow((y1 - y2), 2)));
        CircleBresham(hdc, x1, y1, r1, cmd.shapeColor);
        if (cmd.algorithm == ALGO_FILL_CIRCLE_LINES)
            FillQuarterCircle_line(hdc, x1, y1, r1, cmd.quarter, cmd.fillColor , RGB(255,255,255));
        else if (cmd.algorithm == ALGO_FILL_CIRCLE_CIRCLES)
            FillQuarterCircle_circle(hdc, x1, y1, r1, cmd.quarter, cmd.fillColor);
    } else {

        DrawCardinalSpline(hdc, cmd.controlPoints.data(), cmd.controlPoints.size(), cmd.curveTension, 100,
                           cmd.shapeColor);
    }
    ReleaseDC(hwnd, hdc);

}