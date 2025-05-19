#include <windows.h>
#include <queue>
using namespace std;

// --- Struct for non-recursive flood fill ---
struct Point {
    double x, y;
    Point(double x = 0.0, double y = 0.0) : x(x), y(y) {}
};

// --- Draw 8 symmetric points for circle ---
void Draw8Points(HDC hdc, int xc, int yc, int x, int y, COLORREF c) {
    SetPixel(hdc, xc + x, yc + y, c);
    SetPixel(hdc, xc - x, yc + y, c);
    SetPixel(hdc, xc - x, yc - y, c);
    SetPixel(hdc, xc + x, yc - y, c);
    SetPixel(hdc, xc + y, yc + x, c);
    SetPixel(hdc, xc - y, yc + x, c);
    SetPixel(hdc, xc - y, yc - x, c);
    SetPixel(hdc, xc + y, yc - x, c);
}


// --- Bresenham Circle Drawing ---
void CircleBresenham(HDC hdc, int xc, int yc, int radius, COLORREF c) {
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
int method = 0; // 0 = Recursive, 1 = Non-recursive

// --- Window Procedure ---
LRESULT WINAPI WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_LBUTTONDOWN: {
        HDC hdc = GetDC(hwnd);
        int x = LOWORD(lp);
        int y = HIWORD(lp);

        COLORREF boundaryColor = RGB(0, 0, 0); // black boundary

        // Draw the circle centered at (x, y) with radius 150
        CircleBresenham(hdc, x, y, 150, RGB(0, 0, 0));

        // Call the selected flood fill method
        if (method == 0) {
            FloodFillOneOctantRecursive(hdc, x, y, 0, 0, boundaryColor, RGB(255, 0, 0));
        }
        else {
            FloodFillNonRecursive(hdc, x, y, boundaryColor, RGB(0, 0, 255));
        }

        ReleaseDC(hwnd, hdc);
        break;
    }
    case WM_RBUTTONDOWN: {
        method = !method;
        MessageBox(hwnd, method ? L"Non-Recursive Mode" : L"Recursive Mode", L"Mode Switched", MB_OK);
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

// --- WinMain Entry Point ---
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

    HWND hwnd = CreateWindow(L"MyClass", L"Flood Fill (Left: Fill, Right: Toggle Mode)",
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
