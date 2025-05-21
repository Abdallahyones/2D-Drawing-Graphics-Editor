#include <iostream>
#include <Windows.h>
#include <vector>
using namespace std;
/*
// Draw Point in Rectangle 
const int xLeft = 300, xRight = 500, yTop = 400, yBottom = 300;
void DrawPointRectangle(HDC hdc, int x, int y, COLORREF c) {
	if (x >= xLeft && x <= xRight && y >= yBottom && y <= yTop) {
		SetPixel(hdc , x , y , c);
	}
}
LRESULT WndProc(HWND hwnd, UINT m, WPARAM wp, LPARAM lp)
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
		DrawPointRectangle(hdc , x , y , RGB(255 , 0 , 0));
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
}*/






/*
//Draw Point in Square
const int xLeft = 300;
const int yTop = 300;
const int squareSize = 200;
const int xRight = xLeft + squareSize;   
const int yBottom = yTop + squareSize;   

void DrawPointSquare(HDC hdc, int x, int y, COLORREF c) {
	if (x >= xLeft && x <= xRight && y >= yTop && y <= yBottom) {
		SetPixel(hdc, x, y, c);
	}
}

LRESULT WndProc(HWND hwnd, UINT m, WPARAM wp, LPARAM lp)
{
	HDC hdc;
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
		DrawPointSquare(hdc, x, y, RGB(255, 0, 0));
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
*/






/*
//clipping line with rectangle 
const int xLeft = 300, xRight = 500, yTop = 400, yBottom = 300;
const int INSIDE = 0; //0000
const int LEFT = 1; //0001
const int RIGHT = 2; //0010
const int BOTTOM = 4; //0100
const int TOP = 8; //1000
//function to calculate region code 
int ComputeCode(int x, int y) {
	int code = INSIDE;
	if (x < xLeft)
		code |= LEFT;
	else if (x > xRight)
		code |= RIGHT;
	if (y < yBottom)
		code |= BOTTOM;
	else if (y > yTop)
		code |= TOP;
	return code;
}
// Draw Line using DDA
void DrawLineDDA(HDC hdc, int x1, int y1, int x2, int y2, COLORREF c) {
	int dx = x2 - x1;
	int dy = y2 - y1;
	int steps = max(abs(dx), abs(dy));
	float xIncrement = dx / (float)steps;
	float yIncrement = dy / (float)steps;

	float x = x1;
	float y = y1;
	for (int i = 0; i <= steps; ++i) {
		SetPixel(hdc, round(x), round(y), c);
		x += xIncrement;
		y += yIncrement;
	}
}
// Get Slope
float GetSlope(int x1, int y1, int x2, int y2) {
	if (x2 - x1 == 0)
		return INFINITY;
	return (float)(y2 - y1) / ( x2 - x1);
}
// Calculate InterSection of Line with clipping boundry 
void GetIntersection(int CodeOut , int& x , int& y , int x1 , int y1 , int x2 , int y2) {
	float m = GetSlope(x1, y1, x2, y2);
	if (CodeOut & TOP) {
		y = yTop;
		if (m == INFINITY)
			x = x1;
		else
			x = (int)(x1 + (yTop - y1) / m);
	}
	else if (CodeOut & BOTTOM) {
		y = yBottom;
		if (m == INFINITY)
			x = x1;
		else
			x = (int)(x1 + (yBottom - y1) / m);
	}
	else if (CodeOut & RIGHT) {
		x = xRight;
		if (m == INFINITY)
			y = y1;
		else
			y = (int)(y1 + (xRight - x1)* m);
	}
	else if (CodeOut & LEFT) {
		x = xLeft;
		if (m == INFINITY)
			y = y1;
		else
			y =(int) (y1 + (xLeft - x1)* m);
	}
}
// Cohen-sutherland line clipping 
void CohenSutherlandLineRectangle(HDC hdc , int x1 , int y1 , int x2 , int y2 ) {
	int origX1 = x1, origY1 = y1, origX2 = x2, origY2 = y2;
	int code1 = ComputeCode(x1, y1);
	int code2 = ComputeCode(x2, y2);
	bool accept = false;
	while (true) {
		// if my object is on window
		if ((code1 == 0) && (code2 == 0)) {
			accept = true;
			break;
		}
		else if (code1 & code2 ) {// second base case , can't draw line on my screen
			break;
		}
		else {
			int codeOut, x, y;
			codeOut = code1 ? code1 : code2;
			GetIntersection(codeOut , x , y, x1 , y1 , x2 , y2);
			if (codeOut == code1) {
				x1 = x;
				y1 = y;
				code1 = ComputeCode(x1, y1);
			}
			else {
				x2 = x;
				y2 = y;
				code2 = ComputeCode(x2, y2);
			}
		}
	}
	if (accept) {
		// Outside part 1: original to clipped start
		if (x1 != origX1 || y1 != origY1)
			DrawLineDDA(hdc, origX1, origY1, x1, y1, RGB(255, 0, 0)); // red

		// Inside part
		DrawLineDDA(hdc, x1, y1, x2, y2, RGB(0, 0, 255)); // blue

		// Outside part 2: clipped end to original end
		if (x2 != origX2 || y2 != origY2)
			DrawLineDDA(hdc, x2, y2, origX2, origY2, RGB(255, 0, 0)); // red
	}
	else {
		// Line completely outside, draw full line in red
		DrawLineDDA(hdc, origX1, origY1, origX2, origY2, RGB(255, 0, 0));
	}
}
LRESULT WndProc(HWND hwnd, UINT m, WPARAM wp, LPARAM lp)
{
	HDC hdc;
	PAINTSTRUCT ps;
	static int x1, y1, x2, y2;
	static bool isFirstClick = true;

	switch (m)
	{
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		Rectangle(hdc, xLeft, yBottom, xRight, yTop);// Draw clipping window
		EndPaint(hwnd, &ps);
		break;

	case WM_LBUTTONDOWN: {
		hdc = GetDC(hwnd);
		int x = LOWORD(lp);
		int y = HIWORD(lp);

		if (isFirstClick) {
			x1 = x;
			y1 = y;
			isFirstClick = false;
		}
		else {
			x2 = x;
			y2 = y;
			CohenSutherlandLineRectangle(hdc, x1, y1, x2, y2); // Clip and draw
			isFirstClick = true;
		}

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
*/




/*
//clipping Line with square
const int xLeft = 300 , yTop = 300;
const int squareSize = 200;
const int xRight = xLeft + squareSize;   
const int yBottom = yTop + squareSize;   
const int INSIDE = 0; //0000
const int LEFT = 1; //0001
const int RIGHT = 2; //0010
const int BOTTOM = 4; //0100
const int TOP = 8; //1000
//function to calculate region code 
int ComputeCode(int x, int y) {
	int code = INSIDE;
	if (x < xLeft)
		code |= LEFT;
	else if (x > xRight)
		code |= RIGHT;
	if (y < yTop)
		code |= TOP;
	else if (y > yBottom)
		code |= BOTTOM;

	return code;
}

// Draw Line using DDA
void DrawLineDDA(HDC hdc, int x1, int y1, int x2, int y2, COLORREF c) {
	int dx = x2 - x1;
	int dy = y2 - y1;
	int steps = max(abs(dx), abs(dy));
	float xIncrement = dx / (float)steps;
	float yIncrement = dy / (float)steps;

	float x = x1;
	float y = y1;
	for (int i = 0; i <= steps; ++i) {
		SetPixel(hdc, round(x), round(y), c);
		x += xIncrement;
		y += yIncrement;
	}
}
// Get Slope
float GetSlope(int x1, int y1, int x2, int y2) {
	if (x2 - x1 == 0)
		return INFINITY;
	return (float)(y2 - y1) / (x2 - x1);
}
// Calculate InterSection of Line with clipping boundry 
void GetIntersection(int CodeOut, int& x, int& y, int x1, int y1, int x2, int y2) {
	float m = GetSlope(x1, y1, x2, y2);
	if (CodeOut & TOP) {
		y = yTop;
		if (m == INFINITY)
			x = x1;
		else
			x = (int)(x1 + (yTop - y1) / m);
	}
	else if (CodeOut & BOTTOM) {
		y = yBottom;
		if (m == INFINITY)
			x = x1;
		else
			x = (int)(x1 + (yBottom - y1) / m);
	}
	else if (CodeOut & RIGHT) {
		x = xRight;
		if (m == INFINITY)
			y = y1;
		else
			y = (int)(y1 + (xRight - x1) * m);
	}
	else if (CodeOut & LEFT) {
		x = xLeft;
		if (m == INFINITY)
			y = y1;
		else
			y = (int)(y1 + (xLeft - x1) * m);
	}
}
// Cohen-sutherland line clipping 
void CohenSutherlandLineSquare(HDC hdc, int x1, int y1, int x2, int y2) {
	int origX1 = x1, origY1 = y1, origX2 = x2, origY2 = y2;
	int code1 = ComputeCode(x1, y1);
	int code2 = ComputeCode(x2, y2);
	bool accept = false;
	while (true) {
		// if my object is on window
		if ((code1 == 0) && (code2 == 0)) {
			accept = true;
			break;
		}
		else if (code1 & code2) {// second base case , can't draw line on my screen
			break;
		}
		else {
			int codeOut, x, y;
			codeOut = code1 ? code1 : code2;
			GetIntersection(codeOut, x, y, x1, y1, x2, y2);
			if (codeOut == code1) {
				x1 = x;
				y1 = y;
				code1 = ComputeCode(x1, y1);
			}
			else {
				x2 = x;
				y2 = y;
				code2 = ComputeCode(x2, y2);
			}
		}
	}
	if (accept) {
		// Outside part 1: original to clipped start
		if (x1 != origX1 || y1 != origY1)
			DrawLineDDA(hdc, origX1, origY1, x1, y1, RGB(255, 0, 0)); // red

		// Inside part
		DrawLineDDA(hdc, x1, y1, x2, y2, RGB(0, 0, 255)); // blue

		// Outside part 2: clipped end to original end
		if (x2 != origX2 || y2 != origY2)
			DrawLineDDA(hdc, x2, y2, origX2, origY2, RGB(255, 0, 0)); // red
	}
	else {
		// Line completely outside, draw full line in red
		DrawLineDDA(hdc, origX1, origY1, origX2, origY2, RGB(255, 0, 0));
	}
}
LRESULT WndProc(HWND hwnd, UINT m, WPARAM wp, LPARAM lp)
{
	HDC hdc;
	PAINTSTRUCT ps;
	static int x1, y1, x2, y2;
	static bool isFirstClick = true;

	switch (m)
	{
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		Rectangle(hdc, xLeft, yBottom, xRight, yTop);// Draw clipping window
		EndPaint(hwnd, &ps);
		break;

	case WM_LBUTTONDOWN: {
		hdc = GetDC(hwnd);
		int x = LOWORD(lp);
		int y = HIWORD(lp);

		if (isFirstClick) {
			x1 = x;
			y1 = y;
			isFirstClick = false;
		}
		else {
			x2 = x;
			y2 = y;
			CohenSutherlandLineSquare(hdc, x1, y1, x2, y2); // Clip and draw
			isFirstClick = true;
		}

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
*/

const int xLeft = 300, xRight = 500, yTop = 400, yBottom = 300;
bool isPolygonClosed = false;
struct Point {
	int x, y;
	Point(int x , int y):x(x) , y(y){}
};
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



LRESULT drawClipping(HWND hwnd, UINT m, WPARAM wp, LPARAM lp)
{
	HDC hdc;
	PAINTSTRUCT ps;
	switch (m)
	{
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		Rectangle(hdc, xLeft, yBottom, xRight, yTop); // Draw clipping rectangle

		if (!userPolygon.empty())
			drawPolygon(hdc, userPolygon, RGB(0, 0, 255)); // original polygon in green

		if (isPolygonClosed) {
			vector<Point> clipped = clipPolygon(userPolygon);
			drawPolygon(hdc, clipped, RGB(255, 0, 0)); // clipped polygon in red
		}

		EndPaint(hwnd, &ps);
		break;

	case WM_LBUTTONDOWN: {
		if (isPolygonClosed) break;

		int x = LOWORD(lp);
		int y = HIWORD(lp);
		userPolygon.push_back(Point(x, y));

		hdc = GetDC(hwnd);
		SetPixel(hdc, x, y, RGB(0, 0, 255)); // visual feedback for point
		ReleaseDC(hwnd, hdc);
		break;
	}

	case WM_RBUTTONDOWN: {
		if (userPolygon.size() < 3 || isPolygonClosed) break;

		isPolygonClosed = true;

		hdc = GetDC(hwnd);
		drawPolygon(hdc, userPolygon, RGB(0, 255, 0)); // draw original polygon

		vector<Point> clipped = clipPolygon(userPolygon);
		drawPolygon(hdc, clipped, RGB(255, 0, 0)); // draw clipped polygon
		ReleaseDC(hwnd, hdc);
		break;
	}

	case WM_KEYDOWN: {
		if (wp == VK_ESCAPE) {
			userPolygon.clear();
			isPolygonClosed = false;
			InvalidateRect(hwnd, NULL, TRUE); // force redraw
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

