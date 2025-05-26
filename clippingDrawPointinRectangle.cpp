#include <Windows.h>
#include <vector>
#include "Common.h"

using namespace std;

// Draw Point in Rectangle

void DrawPointRectangle(HDC hdc, int x, int y, COLORREF c) {
	if (x >= xLeft && x <= xRight && y >= yBottom && y <= yTop) {
		SetPixel(hdc , x , y , c);
	}
}
LRESULT drawPointRectangle(HWND hwnd, UINT m, WPARAM wp, LPARAM lp , COLORREF color , DrawCommand& cmd)
{   HDC hdc;
	PAINTSTRUCT ps;
	switch (m)
	{
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		Rectangle(hdc, xLeft, yTop, xRight, yBottom); // draw only once
		EndPaint(hwnd, &ps);
		break;
	case WM_LBUTTONDOWN: {
		hdc = GetDC(hwnd);
		int x = LOWORD(lp);
		int y = HIWORD(lp);
        cmd.points.emplace_back(x,y);
		DrawPointRectangle(hdc , x , y ,color);
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