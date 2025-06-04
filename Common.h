#ifndef ALGONAMES_H
#define ALGONAMES_H

#include <windows.h>
#include <vector>
#include <iostream>
using namespace std;

// Enumerations for shapes
enum Shape {
    SHAPE_LINE,
    SHAPE_CIRCLE,
    SHAPE_ELLIPSE,
    SHAPE_CLIPING,
    SHAPE_FILLING,
    Spline_Curve,
    SHAPE_Convex,
    SHAPE_Recursive_AND_NOT,
};

// Enumerations for algorithms
enum Algorithm {
    // Line Drawing Algorithms
    ALGO_LINE_DDA,
    ALGO_LINE_MIDPOINT,
    ALGO_LINE_BRESENHAM,

    // Circle Drawing Algorithms
    ALGO_CIRCLE_DIRECT,
    ALGO_CIRCLE_POLAR,
    ALGO_CIRCLE_ITERATIVE_POLAR,
    ALGO_CIRCLE_MIDPOINT,
    ALGO_CIRCLE_MODIFIED_MIDPOINT,

    // Ellipse Drawing Algorithms
    ALGO_ELLIPSE_DIRECT,
    ALGO_ELLIPSE_POLAR,
    ALGO_ELLIPSE_BRESENHAM,
    ALGO_ELLIPSE_MIDPOINT,
    ALGO_ELLIPSE_ITERATIVE,

    // Recursive 
    ALGO_Recursive,
    ALGO_NON_Recursive,

    // Filling Algorithms
    ALGO_FILL_CIRCLE_LINES,
    ALGO_FILL_CIRCLE_CIRCLES,
    ALGO_FILL_SQUARE_HERMIT_VERTICAL,
    ALGO_FILL_RECTANGLE_BEZIER_HORIZONTAL,
    ALGO_FILL_CONVEX,

    ALGO_FILL_NONCONVEX,


    // Curve Algorithms
    ALGO_CARDINAL_SPLINE,

    // Clipping Algorithms
    ALGO_CLIP_RECTANGLE_LINE,
    ALGO_CLIP_RECTANGLE_POINT,
    ALGO_CLIP_RECTANGLE_POLYGON,
    ALGO_CLIP_SQUARE_POINT,
    ALGO_CLIP_SQUARE_LINE
};
enum class FillQuarter {
    FIRST = 1, SECOND, THIRD, FOURTH, ALL
};

// Struct for point
struct Point {
    double x, y;

    Point(double x = 0.0, double y = 0.0);
    friend ostream& operator<<(ostream& os, const Point& p) {
        return os << p.x << ' ' << p.y;
    }
    friend istream& operator>>(istream& is, Point& p) {
        return is >> p.x >> p.y;
    }
};


const int xLeft = 300, xRight = 500, yTop = 400, yBottom = 300;
const int INSIDE = 0; //0000
const int LEFT = 1; //0001
const int RIGHT = 2; //0010
const int BOTTOM = 4; //0100
const int TOP = 8; //1000
const int squareSize = 200;
struct DrawCommand {
    Shape shape;
    Algorithm algorithm;
    COLORREF shapeColor;
    COLORREF fillColor;
    std::vector<Point> points;
    std::vector<Point> controlPoints;
    FillQuarter quarter = FillQuarter::ALL;
    int radius = 0;
    int width = 0, height = 0;
    int thickness = 1;
    int curveTension = 0;

    DrawCommand(Shape shape,Algorithm algorithm,COLORREF color) : shape(shape), algorithm(algorithm), shapeColor(color) {}
    DrawCommand(){}
};


// Function declarations
int Round(double x);

void DrawPoint(HDC hdc, int x, int y, COLORREF color);

void Draw8Points(HDC hdc, int xc, int yc, int x, int y, COLORREF color);

#endif // ALGONAMES_H
