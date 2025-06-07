#include <iostream>
#include <Windows.h>
#include <vector>
#include "Common.h"
using namespace std;


void makeClipped(HWND hwnd , COLORREF c ,vector<Point> &polygon );
bool isPolygonClosed = false;

vector<Point> userPolygon;
// Linear interpolation for intersection with vertical clipping edge (x = xEdge)
Point IntersectVertical(Point p1, Point p2, double xEdge) {
	double x = xEdge;
	double y = p1.y + (p2.y - p1.y) * (xEdge - p1.x) / (p2.x - p1.x);
	return Point(x, y);
}

// Linear interpolation for intersection with horizontal clipping edge (y = yEdge)
Point IntersectHorizontal(Point p1, Point p2, double yEdge) {
	double y = yEdge;
	double x = p1.x + (p2.x - p1.x) * (yEdge - p1.y) / (p2.y - p1.y);
	return Point(x, y);
}

// Clip against x >= xLeft
vector<Point> clipLeft(const vector<Point>& poly) {
	vector<Point> out;
	int n = poly.size();
	Point v1 = poly[n - 1];
	bool in1 = v1.x >= xLeft;

	for (int i = 0; i < n; i++) {
		Point v2 = poly[i];
		bool in2 = v2.x >= xLeft;

		if (in1 && in2)
			out.push_back(v2);
		else if (in1 && !in2)
			out.push_back(IntersectVertical(v1, v2, xLeft));
		else if (!in1 && in2) {
			out.push_back(IntersectVertical(v1, v2, xLeft));
			out.push_back(v2);
		}

		v1 = v2;
		in1 = in2;
	}
	return out;
}

// Clip against x <= xRight
vector<Point> clipRight(const vector<Point>& poly) {
	vector<Point> out;
	int n = poly.size();
	Point v1 = poly[n - 1];
	bool in1 = v1.x <= xRight;

	for (int i = 0; i < n; i++) {
		Point v2 = poly[i];
		bool in2 = v2.x <= xRight;

		if (in1 && in2)
			out.push_back(v2);
		else if (in1 && !in2)
			out.push_back(IntersectVertical(v1, v2, xRight));
		else if (!in1 && in2) {
			out.push_back(IntersectVertical(v1, v2, xRight));
			out.push_back(v2);
		}

		v1 = v2;
		in1 = in2;
	}
	return out;
}

// Clip against y >= yBottom
vector<Point> clipBottom(const vector<Point>& poly) {
	vector<Point> out;
	int n = poly.size();
	Point v1 = poly[n - 1];
	bool in1 = v1.y >= yBottom;

	for (int i = 0; i < n; i++) {
		Point v2 = poly[i];
		bool in2 = v2.y >= yBottom;

		if (in1 && in2)
			out.push_back(v2);
		else if (in1 && !in2)
			out.push_back(IntersectHorizontal(v1, v2, yBottom));
		else if (!in1 && in2) {
			out.push_back(IntersectHorizontal(v1, v2, yBottom));
			out.push_back(v2);
		}

		v1 = v2;
		in1 = in2;
	}
	return out;
}

// Clip against y <= yTop
vector<Point> clipTop(const vector<Point>& poly) {
	vector<Point> out;
	int n = poly.size();
	Point v1 = poly[n - 1];
	bool in1 = v1.y <= yTop;

	for (int i = 0; i < n; i++) {
		Point v2 = poly[i];
		bool in2 = v2.y <= yTop;

		if (in1 && in2)
			out.push_back(v2);
		else if (in1 && !in2)
			out.push_back(IntersectHorizontal(v1, v2, yTop));
		else if (!in1 && in2) {
			out.push_back(IntersectHorizontal(v1, v2, yTop));
			out.push_back(v2);
		}

		v1 = v2;
		in1 = in2;
	}
	return out;
}

// Main Sutherland-Hodgman clipping pipeline
vector<Point> clipPolygon(const vector<Point>& polygon) {
	vector<Point> p1 = clipLeft(polygon);
	vector<Point> p2 = clipRight(p1);
	vector<Point> p3 = clipBottom(p2);
	return clipTop(p3);
}
void drawPolygon(HDC hdc,  vector<Point>& poly, COLORREF c) {
	int n = poly.size();
	if (n < 2) return;

	for (int i = 0; i < n; i++) {
		Point p1 = poly[i];
		Point p2 = poly[(i + 1) % n];

		int dx = abs(p2.x - p1.x);
		int dy = abs(p2.y - p1.y);
		int steps = max(dx, dy);

		if (steps == 0) continue; // avoid division by zero

		for (int j = 0; j <= steps; j++) {
			double t = (double)j / steps;
			int x = (int)(p1.x + t * (p2.x - p1.x));
			int y = (int)(p1.y + t * (p2.y - p1.y));
			SetPixel(hdc, x, y, c);
		}
	}
}



LRESULT drawRectanglePolygon(HWND hwnd, UINT m, WPARAM wp, LPARAM lp , COLORREF c , DrawCommand&cmd)
{
	HDC hdc;
	PAINTSTRUCT ps;
	switch (m)
	{
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		if (!userPolygon.empty())
			drawPolygon(hdc, userPolygon, c); // original polygon in green
            cmd.points= userPolygon;

		if (isPolygonClosed) {
			vector<Point> clipped = clipPolygon(userPolygon);
			drawPolygon(hdc, clipped,c); // clipped polygon in red
		}

		EndPaint(hwnd, &ps);
		break;

	case WM_LBUTTONDOWN: {
        cout<<"There is click Point in Position (x,y) " <<LOWORD(lp) <<" "<< HIWORD(lp)<<"\n";
		if (isPolygonClosed) break;

		int x = LOWORD(lp);
		int y = HIWORD(lp);
		userPolygon.emplace_back(x, y);

		hdc = GetDC(hwnd);
		SetPixel(hdc, x, y, c);
		ReleaseDC(hwnd, hdc);
		break;
	}

	case WM_RBUTTONDOWN: {
        cmd.points = userPolygon;
        makeClipped( hwnd ,  c , userPolygon);
        drawHistory.emplace_back(cmd);
		break;
	}

	case WM_KEYDOWN: {
		if (wp == VK_ESCAPE) {
			userPolygon.clear();
			isPolygonClosed = false;
			InvalidateRect(hwnd, NULL, TRUE);
		}
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

void makeClipped(HWND hwnd , COLORREF c , vector<Point>&polygon)
{
    if (polygon.size() < 3 || isPolygonClosed) return;
    isPolygonClosed = true;
    HDC hdc = GetDC(hwnd);
    drawPolygon(hdc, polygon, RGB(0,255,0)); // draw original polygon
    vector<Point> clipped = clipPolygon(polygon);
    drawPolygon(hdc, clipped, c); // draw clipped polygon
    polygon.clear();
    isPolygonClosed = false;
    ReleaseDC(hwnd, hdc);
}