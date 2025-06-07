#include <Windows.h>
#include <vector>
#include "Common.h"
using namespace std;



void DrawPointSquare(HDC hdc, int x, int y, COLORREF c) {
    if (x >= xLeft && x <= Xright && y >= yTop && y <= Ybottom) {
        DrawPoint(hdc, x, y, c);
    }
}

LRESULT drawPointSquare(HWND hwnd, UINT m, WPARAM wp, LPARAM lp , COLORREF c  ,DrawCommand& cmd)
{
    HDC hdc;
    PAINTSTRUCT ps;
    switch (m)
    {
        case WM_PAINT:
            break;
        case WM_LBUTTONDOWN: {
            cout<<"There is click Point in Position (x,y) " <<LOWORD(lp) <<" "<< HIWORD(lp)<<"\n";
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