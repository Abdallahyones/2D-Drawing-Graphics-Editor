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

using namespace std ;

/*  Declare Windows procedure  */
LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);

/*  Make the class name into a global variable  */
TCHAR szClassName[ ] = _T("CodeBlocksWindowsApp");

int WINAPI WinMain (HINSTANCE hThisInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR lpszArgument,
                    int nCmdShow)
{
    HWND hwnd;               /* This is the handle for our window */
    MSG messages;            /* Here messages to the application are saved */
    WNDCLASSEX wincl;        /* Data structure for the windowclass */

    /* The Window structure */
    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure;      /* This function is called by windows */
    wincl.style = CS_DBLCLKS;                 /* Catch double-clicks */
    wincl.cbSize = sizeof (WNDCLASSEX);

    /* Use default icon and mouse-pointer */
    wincl.hIcon = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;                 /* No menu */
    wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
    wincl.cbWndExtra = 0;                      /* structure or the window instance */
    /* Use Windows's default colour as the background of the window */
    wincl.hbrBackground = (HBRUSH) COLOR_BACKGROUND;

    /* Register the window class, and if it fails quit the program */
    if (!RegisterClassEx (&wincl))
        return 0;

    /* The class is registered, let's create the program*/
    hwnd = CreateWindowEx (
            0,                   /* Extended possibilites for variation */
            szClassName,         /* Classname */
            _T("Code::Blocks Template Windows App"),       /* Title Text */
            WS_OVERLAPPEDWINDOW, /* default window */
            CW_USEDEFAULT,       /* Windows decides the position */
            CW_USEDEFAULT,       /* where the window ends up on the screen */
            544,                 /* The programs width */
            375,                 /* and height in pixels */
            HWND_DESKTOP,        /* The window is a child-window to desktop */
            NULL,                /* No menu */
            hThisInstance,       /* Program Instance handler */
            NULL                 /* No Window Creation data */
    );

    /* Make the window visible on the screen */
    ShowWindow (hwnd, nCmdShow);

    /* Run the message loop. It will run until GetMessage() returns 0 */
    while (GetMessage (&messages, NULL, 0, 0))
    {
        /* Translate virtual-key messages into character messages */
        TranslateMessage(&messages);
        /* Send message to WindowProcedure */
        DispatchMessage(&messages);
    }

    /* The program return-value is 0 - The value that PostQuitMessage() gave */
    return messages.wParam;
}

int Round(double a) {
    return (int) a;
}

// Vector2 for 2D point representation
struct Vector2 {
    double x, y;
    Vector2(double a = 0, double b = 0) : x(a), y(b) {}
};

// Vector4 for polynomial coefficients
class Vector4 {
    double v[4];
public:
    Vector4(double a = 0, double b = 0, double c = 0, double d = 0) {
        v[0] = a; v[1] = b; v[2] = c; v[3] = d;
    }
    Vector4(double a[]) { memcpy(v, a, 4 * sizeof(double)); }
    double& operator[](int i) { return v[i]; }
};

// Matrix4 for Hermite basis
class Matrix4 {
    Vector4 M[4];
public:
    Matrix4(double A[]) {
        for (int i = 0; i < 4; ++i)
            M[i] = Vector4(A + i * 4);
    }
    Vector4& operator[](int i) { return M[i]; }
};

// Utility: Matrix * Vector
Vector4 operator*(Matrix4 M, Vector4& b) {
    Vector4 res;
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            res[i] += M[i][j] * b[j];
    return res;
}

// Utility: Dot product
double DotProduct(Vector4& a, Vector4& b) {
    return a[0]*b[0] + a[1]*b[1] + a[2]*b[2] + a[3]*b[3];
}

// Compute Hermite coefficients
Vector4 GetHermiteCoeff(double x0, double s0, double x1, double s1) {
    static double H[16] = {
            2, -2,  1,  1,
            -3,  3, -2, -1,
            0,  0,  1,  0,
            1,  0,  0,  0
    };
    static Matrix4 basis(H);
    Vector4 v(x0, x1, s0, s1);
    return basis * v;
}

// Hermite curve drawing function
void DrawHermiteCurve(HDC hdc, Vector2& P0, Vector2& T0, Vector2& P1, Vector2& T1, int numpoints, COLORREF color = RGB(0, 0, 0)) {
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
}

// Bézier curve as a special Hermite curve
void DrawBezierCurve(HDC hdc, Vector2& P0, Vector2& P1, Vector2& P2, Vector2& P3, int numpoints, COLORREF color = RGB(0, 0, 0)) {
    Vector2 T0(3 * (P1.x - P0.x), 3 * (P1.y - P0.y));
    Vector2 T1(3 * (P3.x - P2.x), 3 * (P3.y - P2.y));
    DrawHermiteCurve(hdc, P0, T0, P3, T1, numpoints, color);
}

// Cardinal spline using Hermite curve segments
// Cardinal spline using Hermite curve segments
void DrawCardinalSpline(HDC hdc, Vector2 P[], int n, double c, int numpoints, COLORREF color = RGB(0, 0, 0)) {
    if (n < 2) return; // Need at least 2 points to draw anything
    if (n == 2) {
        // Just draw a line
        MoveToEx(hdc, (int)P[0].x, (int)P[0].y, NULL);
        LineTo(hdc, (int)P[1].x, (int)P[1].y);
        return;
    }

    double tension = (1.0 - c) / 2.0;

    for (int i = 1; i < n - 1; ++i) {
        Vector2 P0 = P[i - 1];
        Vector2 P1 = P[i];
        Vector2 P2 = P[i + 1];

        Vector2 T0(tension * (P2.x - P0.x), tension * (P2.y - P0.y));
        Vector2 T1;

        if (i + 2 < n) {
            Vector2 P3 = P[i + 2];
            T1 = Vector2(tension * (P3.x - P1.x), tension * (P3.y - P1.y));
        } else {
            // For the last segment, use the same tangent as previous to prevent indexing past the array
            T1 = Vector2(tension * (P2.x - P0.x), tension * (P2.y - P0.y));
        }

        DrawHermiteCurve(hdc, P1, T0, P2, T1, numpoints, color);
    }

    // Optional: draw small dots for control points
    /*
    for (int i = 0; i < n; ++i) {
        Ellipse(hdc, P[i].x - 2, P[i].y - 2, P[i].x + 2, P[i].y + 2);
    }
    */
}


void Draw8Points(HDC hdc,int xc, int yc, int x, int y, COLORREF c){
    SetPixel(hdc,xc + x,yc + y,c);
    SetPixel(hdc,xc + -x,yc + y,c);
    SetPixel(hdc,xc + -x,yc + -y,c);
    SetPixel(hdc,xc + x,yc + -y,c);
    SetPixel(hdc,xc + y, yc + x,c);
    SetPixel(hdc,xc + -y, yc + x,c);
    SetPixel(hdc,xc + -y, yc + -x,c);
    SetPixel(hdc,xc + y, yc + -x,c);
}

void CircleBresham(HDC hdc, int xc, int yc, int radius, COLORREF c){
    int x = 0, y = radius, d = 1 - radius;
    Draw8Points(hdc,xc,yc,x,y,c);
    while(x < y){
        if(d < 0) d += 2 * x + 2;
        else{
            d += (2 *(x - y)+5);
            y--;
        }
        x++;
        Draw8Points(hdc,xc,yc,x,y,c);
    }

}


void fill(HDC hdc, int x, int y, int xc, int yc, int r, COLORREF c) {
    const int N = r + 1;
    bool vis[N][N] = {};
    queue<pair<int, int>> ls;
    ls.push({0, 0});
    vis[r][r] = true;

    int dx[] = {1, 0, -1, 0};
    int dy[] = {0, 1, 0, -1};

    while (!ls.empty()) {
        x = ls.front().first ;
        y = ls.front().second ;
        ls.pop();

        if (y > x || x*x + y*y > r*r) continue;

        SetPixel(hdc, xc + x, yc + y, c);
        SetPixel(hdc, xc - x, yc - y, c);

        for (int i = 0; i < 4; ++i) {
            int nx = x + dx[i];
            int ny = y + dy[i];

            if (nx < 0 || ny < 0 || nx >= N || ny >= N || nx < ny ) continue;
            if (!vis[nx][ny]) {
                vis[nx][ny] = true;
                ls.push({nx, ny});
            }
        }
    }
}

void FillQuarterCircle(HDC hdc, int xc, int yc, int r, int quarter, COLORREF c) {
    for (int y = 0; y <= r; ++y) {
        int xLimit = (int) sqrt(r * r - y * y);
        for (int x = 0; x <= xLimit; ++x) {
            switch (quarter) {
                case 1: SetPixel(hdc, xc + x, yc - y, c); break;
                case 2: SetPixel(hdc, xc - x, yc - y, c); break;
                case 3: SetPixel(hdc, xc - x, yc + y, c); break;
                case 4: SetPixel(hdc, xc + x, yc + y, c); break;
            }
        }
    }
}
void FillQuarterCircle_line(HDC hdc, int xc, int yc, int r, int quarter, COLORREF c1 = RGB(0, 0, 0), COLORREF c2 = RGB(255, 255, 255)) {
    for (int y = 0; y <= r; ++y) {
        int xLimit = (int)round(sqrt(r * r - y * y));
        COLORREF currentColor = (y % 2 == 0) ? c1 : c2;

        // Create pen for this line
        HPEN pen = CreatePen(PS_SOLID, 1, currentColor);
        HPEN oldPen = (HPEN)SelectObject(hdc, pen);

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
                    case 1: drawX = xc + x; drawY = yc - y; break; // Top Right
                    case 2: drawX = xc - x; drawY = yc - y; break; // Top Left
                    case 3: drawX = xc - x; drawY = yc + y; break; // Bottom Left
                    case 4: drawX = xc + x; drawY = yc + y; break; // Bottom Right
                    default: return;
                }

                CircleBresham(hdc, drawX, drawY, smallRadius, c);
            }
        }
    }
}

void DrawLineDDA(HDC hdc, int x1, int y1, int x2, int y2, COLORREF c)
{
    int dx = x2 - x1;
    int dy = y2 - y1;

    int steps = max(abs(dx), abs(dy));

    // calculate increment in x and y
    float xInc = dx / (float)steps;
    float yInc = dy / (float)steps;

    float x = x1;
    float y = y1;

    for (int i = 0; i <= steps; i++)
    {
        SetPixel(hdc, Round(x), Round(y), c);
        x += xInc;
        y += yInc;
    }
}

void DrawLineMidpoint(HDC hdc, int x1, int y1, int x2, int y2, COLORREF c)
{
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);

    int x = x1;
    int y = y1;

    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;

    bool steep = dy > dx;

    if (steep)
    {
        std::swap(dx, dy);
    }

    int d = 2 * dy - dx;
    int dE = 2 * dy;
    int dNE = 2 * (dy - dx);

    SetPixel(hdc, x, y, c);

    for (int i = 0; i < dx; ++i)
    {
        if (d < 0)
        {
            d += dE;
            if (steep)
                y += sy;
            else
                x += sx;
        }
        else
        {
            d += dNE;
            x += sx;
            y += sy;
        }

        SetPixel(hdc, x, y, c);
    }
}


void FillRectangleWithBezier(HDC hdc, Vector2 topLeft, int width, int height, COLORREF color = RGB(0, 0, 0)) {
    int numCurves = height / 5; // Curve density

    for (int i = 0; i <= numCurves; ++i) {
        double t = (double)i / numCurves;
        double y = topLeft.y + t * height;

        // Control points for horizontal Bézier curve
        Vector2 P0(topLeft.x, y);
        Vector2 P1(topLeft.x + width * 0.33, y + sin(i) * 5); // Optional wave effect
        Vector2 P2(topLeft.x + width * 0.66, y - sin(i) * 5);
        Vector2 P3(topLeft.x + width, y);

        DrawBezierCurve(hdc, P0, P1, P2, P3, 100, color);
    }
}


void FillSquareWithHermite(HDC hdc, Vector2 topLeft, int sideLength, COLORREF color = RGB(0, 0, 0)) {
    Vector2 P0, P1, T0, T1;
    int numCurves = sideLength / 5; // Density of curves

    for (int i = 0; i <= numCurves; ++i) {
        double t = (double)i / numCurves;
        double x = topLeft.x + t * sideLength;

        // Start and end points (vertical lines)
        P0 = Vector2(x, topLeft.y);
        P1 = Vector2(x, topLeft.y + sideLength);

        // Add curvature in X-direction using a sine function
        double dx = 20 * sin(t * 3.14159);  // 20 = curvature amplitude

        T0 = Vector2(dx, sideLength / 2);  // Curved tangent at top
        T1 = Vector2(-dx, sideLength / 2); // Opposite curve at bottom

        DrawHermiteCurve(hdc, P0, T0, P1, T1, 100, color);
    }
}



void DrawRectangle(HDC hdc, int x1, int y1, int x2, int y2)
{
    // Determine the actual corners of the rectangle
    int left = min(x1, x2);
    int right = max(x1, x2);
    int top = min(y1, y2);
    int bottom = max(y1, y2);

    // Draw top edge (DDA) - Red
    DrawLineDDA(hdc, left, top, right, top, RGB(255, 0, 0));

    // Draw bottom edge (DDA) - Green
    DrawLineDDA(hdc, left, bottom, right, bottom, RGB(0, 255, 0));

    // Draw left edge (Midpoint) - Blue
    DrawLineMidpoint(hdc, left, top, left, bottom, RGB(0, 0, 255));

    // Draw right edge (Midpoint) - Cyan
    DrawLineMidpoint(hdc, right, top, right, bottom, RGB(0, 255, 255));

    int height = bottom - top;
    int width = right - left;
    Vector2 topLeft(left, top);
    FillRectangleWithBezier(hdc, topLeft, width, height, RGB(0, 0, 0));

}

void DrawSquare(HDC hdc, int x1, int y1, int x2, int y2){
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
    DrawLineDDA(hdc, left, top, right, top, RGB(255, 0, 0));

    // Draw bottom edge (DDA) - Green
    DrawLineDDA(hdc, left, bottom, right, bottom, RGB(0, 255, 0));

    // Draw left edge (Midpoint) - Blue
    DrawLineMidpoint(hdc, left, top, left, bottom, RGB(0, 0, 255));

    // Draw right edge (Midpoint) - Cyan
    DrawLineMidpoint(hdc, right, top, right, bottom, RGB(0, 255, 255));

    Vector2 topLeft(left, top);
    FillSquareWithHermite(hdc, topLeft, side, RGB(0, 0, 0));
}




LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam , Algorithm algo )
{
    HDC hdc;
    static int clickCount = 0;
    static int x1, x2, y1, y2, r1;
    static int selectedQuarter = 4; // Default to first quarter
    static char Shape = 'C';

    static int xStart, yStart;         // For rectangle
    static bool isFirstClick = true;   // For rectangle
    static bool drawCircleMode = false; // Toggle between modes
    static bool Rectingle = false ;

    static vector<Vector2> controlPoints;
    const int numpoints = 1000;      // Spline smoothness
    const double tension = 0.0;      // Cardinal spline tension (0 < c < 1)

    // 4 Right Click for DrawCardinalSpline
    // 2 Left Click for Rectangle or Square or Circle
    // click char 'R' -> Rectangle
    // click char 'C' -> Circle
    // click char 'S' -> Square
    // 1 , 2 , 3 , 4 for selectedQuarter


    switch (message)  // handle the messages
    {
        case WM_LBUTTONDOWN:
            hdc = GetDC(hwnd);
            if (Shape != 'C'){ // Rectangle or Square mode
                if (Shape == 'R'){
                    if (isFirstClick) {
                        xStart = LOWORD(lParam);
                        yStart = HIWORD(lParam);
                        isFirstClick = false;
                    }
                    else {
                        int xEnd = LOWORD(lParam);
                        int yEnd = HIWORD(lParam);
                        DrawRectangle(hdc, xStart, yStart, xEnd, yEnd);
                        Rectingle ^= 1 ;
                        isFirstClick = true;
                    }
                }else{
                    // Rectangle input mode
                    if (isFirstClick) {
                        xStart = LOWORD(lParam);
                        yStart = HIWORD(lParam);
                        isFirstClick = false;
                    }
                    else {
                        int xEnd = LOWORD(lParam);
                        int yEnd = HIWORD(lParam);
                        DrawSquare(hdc, xStart, yStart, xEnd, yEnd);
                        Rectingle ^= 1 ;
                        isFirstClick = true;
                    }
                }
                ReleaseDC(hwnd, hdc);
                break;
            }
            else{
                if (clickCount == 0) {
                    x1 = LOWORD(lParam); // center of the circle
                    y1 = HIWORD(lParam);
                    clickCount++;
                }
                else if (clickCount == 1) {
                    x2 = LOWORD(lParam); // point on the perimeter
                    y2 = HIWORD(lParam);
                    r1 = Round(sqrt(pow((x1 - x2), 2) + pow((y1 - y2), 2)));
                    clickCount++;
                    hdc = GetDC(hwnd);
                    CircleBresham(hdc, x1, y1, r1, RGB(0, 0, 0));
//                    fill(hdc , 0 , 0 , x1 , y1 , r1 , RGB(0,0,0));
//                FillQuarterCircle(hdc, x1, y1, r1, selectedQuarter, RGB(0, 0, 0));
//                    FillQuarterCircle_line(hdc, x1, y1, r1, selectedQuarter, RGB(0, 0, 0));
                    FillQuarterCircle_circle(hdc, x1, y1, r1, selectedQuarter, RGB(0, 0, 255));
                    ReleaseDC(hwnd, hdc);
                    clickCount = 0; // reset for new input
                }
                break;
            }

        case WM_RBUTTONDOWN:
            // test
//            controlPoints = {
//                    Vector2(100, 200),  // P0 - top-left
//                    Vector2(100, 100),  // P1 - bottom-left
//                    Vector2(200, 200),  // P2 - top-right
//                    Vector2(200, 100)   // P3 - bottom-right
//            };
            if (controlPoints.size() < 4){
                int x = LOWORD(lParam);
                int y = HIWORD(lParam);
                controlPoints.push_back(Vector2(x, y));
            }
            if (controlPoints.size() >= 4) {
                hdc = GetDC(hwnd);
                DrawCardinalSpline(hdc, controlPoints.data(), controlPoints.size(), tension, numpoints, RGB(0, 150, 255));
                ReleaseDC(hwnd, hdc);
                // Optional: clear points after drawing
                controlPoints.clear();
            }
            break;

        case WM_KEYDOWN:
            if (wParam >= '1' && wParam <= '4') {
                selectedQuarter = wParam - '0';
            }
            if (wParam == 'C' || wParam == 'R' || wParam == 'S'){
                Shape = wParam;
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