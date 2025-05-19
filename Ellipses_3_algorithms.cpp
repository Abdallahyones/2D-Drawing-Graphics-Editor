#include <iostream>
#include <Windows.h>
#include <cmath>

#define ID_BACKGROUND_WHITE  1
#define ID_BACKGROUND_ORIGINAL  2
#define ID_ELLIPSE_POLAR       3
#define ID_ELLIPSE_CARTESIAN   4
#define ID_ELLIPSE_BRESENHAM   5
HBRUSH hBackgroundBrush = CreateSolidBrush(RGB(192, 192, 192));
int selectedAlgorithm = ID_ELLIPSE_POLAR;

int Round(double x)
{
    return (int)(x + 0.5);
}

/**---------------------Draw 4 points for similarity------------------------------**/
// for drawing point and 3 siblings for ellipse (similarity) 
void Draw4Points(HDC hdc, int xc, int yc, int x, int y, COLORREF c) {
    // original
    SetPixel(hdc, xc + x, yc + y, c);
    // 3 siblings
    SetPixel(hdc, xc - x, yc + y, c);
    SetPixel(hdc, xc + x, yc - y, c);
    SetPixel(hdc, xc - x, yc - y, c);
}

/**-------------------------------Polar-----------------------------------------**/
// draw ellipse by substitution in 2 polar equations to calculate x and y 
void Drawellipsepolar(HDC hdc, int xc, int yc, int a, int b, COLORREF c) {
    const double PI = 3.141592653589793 / 2; // loop on only 1 quad on ellipse & by similarity we will draw whole ellipse
    int x = a, y = 0;
    double theta = 0, step = 1.0/ max(a,b);
    Draw4Points(hdc, xc, yc, x, y, c);
    // till theta reach first quad angle 
    while (theta <= PI) {
        // increment theta
        theta += step;
        // calculate new x & y
        x = Round(a * cos(theta));
        y = Round(b * sin(theta));
        Draw4Points(hdc,xc,yc,x,y,c);
    } 
}

/**----------------------------Cartesian----------------------------------------**/
// use cartesian equation in calculating new dimension of point that will be drawn 
// we have 2 conditions when slope < 1 & slope >= 1
void Drawellipsecartesian(HDC hdc, int xc, int yc, int a, int b, COLORREF c) {
    int x = a, y = 0;
    Draw4Points(hdc, xc, yc, x, y, c);
    // slope >= 1
    // so always increment y and calculate x by equation 
    while (y < b) {
        y++;
        x = Round(sqrt(pow(a,2)*(1.0-(pow(y,2)/(double)pow(b,2)))));
        Draw4Points(hdc, xc, yc, x, y, c);
    }
    x = 0, y = b;
    Draw4Points(hdc, xc, yc, x, y, c);
    // slope < 1
    // so always increment x and calculate y by equation
    while (x < a) {
        x++;
        y = Round(sqrt(pow(b, 2) * (1.0 - (pow(x, 2) /(double) pow(a, 2)))));
        Draw4Points(hdc, xc, yc, x, y, c);
    }
}

/**------------------Bresenham without (midpoint)-------------------------------**/
// draw ellipse by using cartesian equation gat all terms on 1 side and store it in decision variable
void Drawellipsebresenham(HDC hdc, int xc, int yc, int a, int b, COLORREF c) {
    int x = a, y = 0, b2= b*b, a2= a*a;
    Draw4Points(hdc, xc, yc, x, y, c);
    // slope >= 1
    // increment y with 0.5 in d
    // whan d become < 0 only increment y else increment y and decrement x
    while (a2*y < b2*x) {
        double d = (x * x) / (double)a2 + ((y + 0.5) * (y + 0.5)) / (double)(b2)-1;
        if (d >= 0) x--;
        y++;
        Draw4Points(hdc, xc, yc, x, y, c);
    }
    x = 0;y = b;
    Draw4Points(hdc, xc, yc, x, y, c);
    // slope < 1
    // increment x with 0.5 in d
    // whan d become < 0 only increment x else increment x and decrement y
    while (b2*x < a2*y) {
        double d = ((x + 0.5) * (x + 0.5)) / (double)a2 + (y * y) / (double)(b2) - 1;
        if (d >= 0) y--;
        x++;
        Draw4Points(hdc, xc, yc, x, y, c);
    }
}

/**--------------------------------Menu----------------------------------------**/
void AddMenus(HWND hwnd) {
    HMENU hMenubar = CreateMenu();
    HMENU hBackgroundMenu = CreateMenu();
    HMENU hEllipseMenu = CreateMenu();

    AppendMenuW(hBackgroundMenu, MF_STRING, ID_BACKGROUND_WHITE, L"White");
    AppendMenuW(hBackgroundMenu, MF_STRING, ID_BACKGROUND_ORIGINAL, L"Original");
    AppendMenuW(hEllipseMenu, MF_STRING, ID_ELLIPSE_POLAR, L"Polar");
    AppendMenuW(hEllipseMenu, MF_STRING, ID_ELLIPSE_CARTESIAN, L"Cartesian");
    AppendMenuW(hEllipseMenu, MF_STRING, ID_ELLIPSE_BRESENHAM, L"Bresenham");

    AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR)hBackgroundMenu, L"Background");
    AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR)hEllipseMenu, L"Ellipse Algorithm");

    SetMenu(hwnd, hMenubar);
}

LRESULT WndProc(HWND hwnd, UINT m, WPARAM wp, LPARAM lp)
{
    HDC hdc;
    static int xc, yc, x, y, count=0, xc1, yc1, count1 = 0, x1,y1;
    switch (m)
    {
    case WM_ERASEBKGND:
    {
        HDC hdc = (HDC)wp;
        RECT rect;
        GetClientRect(hwnd, &rect);
        FillRect(hdc, &rect, hBackgroundBrush);
        return 1; // background erased
        break;
    }
    // for background & choosing ellipse algorithm
    case WM_COMMAND:
        switch (LOWORD(wp)) {
        case ID_BACKGROUND_WHITE:
            DeleteObject(hBackgroundBrush);
            hBackgroundBrush = CreateSolidBrush(RGB(255, 255, 255));
            count = 0;
            InvalidateRect(hwnd, NULL, TRUE);
            break;

        case ID_BACKGROUND_ORIGINAL:
            DeleteObject(hBackgroundBrush);
            hBackgroundBrush = CreateSolidBrush(RGB(192, 192, 192));
            count = 0;
            InvalidateRect(hwnd, NULL, TRUE);
            break;

        case ID_ELLIPSE_POLAR:
        case ID_ELLIPSE_CARTESIAN:
        case ID_ELLIPSE_BRESENHAM:
            selectedAlgorithm = LOWORD(wp);
            count = 0; // Reset click counter when changing algorithm
            break;
        }
        break;
    case WM_LBUTTONDOWN:
        hdc = GetDC(hwnd);
        count++;
        if (count == 1) {
            xc = LOWORD(lp);
            yc = HIWORD(lp);
            SetPixel(hdc, xc, yc, RGB(255, 0, 0));
        }
        else if (count == 2) {
            x = LOWORD(lp);
            y = HIWORD(lp);
            int a = abs(x - xc), b = abs(y - yc);

            switch (selectedAlgorithm) {
            case ID_ELLIPSE_POLAR:
                Drawellipsepolar(hdc, xc, yc, a, b, RGB(0, 0, 255));
                break;
            case ID_ELLIPSE_CARTESIAN:
                Drawellipsecartesian(hdc, xc, yc, a, b, RGB(0, 0, 255));
                break;
            case ID_ELLIPSE_BRESENHAM:
                Drawellipsebresenham(hdc, xc, yc, a, b, RGB(0, 0, 255));
                break;
            }
            count = 0;
        }
        ReleaseDC(hwnd, hdc);
        break;
        break;
    case WM_CLOSE:
        DestroyWindow(hwnd); break;
    case WM_DESTROY:
        PostQuitMessage(0); break;
    default:return DefWindowProc(hwnd, m, wp, lp);
    }
    return 0;
}

int APIENTRY WinMain(HINSTANCE hi, HINSTANCE pi, LPSTR cmd, int nsh)
{
    WNDCLASS wc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
    wc.lpszClassName = L"MyClass";
    wc.lpszMenuName = NULL;
    wc.lpfnWndProc = WndProc;
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.hInstance = hi;
    wc.hbrBackground = hBackgroundBrush;
    RegisterClass(&wc);
    HWND hwnd = CreateWindow(L"MyClass", L"2D drawing", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hi, 0);
    AddMenus(hwnd);
    ShowWindow(hwnd, nsh);
    UpdateWindow(hwnd);
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
