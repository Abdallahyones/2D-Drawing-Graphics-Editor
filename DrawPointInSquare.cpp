#include <Windows.h>
#include <vector>
#include "Common.h"
using namespace std;

const int Xright = xLeft + squareSize;
const int Ybottom = yTop + squareSize;

void DrawPointSquare(HDC hdc, int x, int y, COLORREF c) {
    if (x >= xLeft && x <= Xright && y >= yTop && y <= yBottom) {
        SetPixel(hdc, x, y, c);
    }
}

LRESULT drawPointSquare(HWND hwnd, UINT m, WPARAM wp, LPARAM lp , COLORREF c  ,DrawCommand& cmd)
{
    HDC hdc;
    PAINTSTRUCT ps;
    switch (m)
    {
        case WM_PAINT:
            hdc = BeginPaint(hwnd, &ps);
            Rectangle(hdc, xLeft, yTop, Xright, yBottom); // draw only once
            EndPaint(hwnd, &ps);
            break;
        case WM_LBUTTONDOWN: {
            hdc = GetDC(hwnd);
            int x = LOWORD(lp);
            int y = HIWORD(lp);
            cmd.points.emplace_back(x,y);
            DrawPointSquare(hdc, x, y,c);
            drawHistory.emplace_back(cmd);
            ReleaseDC(hwnd, hdc);
            break;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hwnd, m, wp, lp);
    }

    return 0;
}