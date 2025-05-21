#ifndef ALGONAMES_H
#define ALGONAMES_H

#include <windows.h>

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
    ALGO_ELLIPSE_MIDPOINT,

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
    ALGO_CLIP_RECTANGLE_POINT_LINE_POLYGON,
    ALGO_CLIP_SQUARE_POINT_LINE
};

// Struct for point
struct Point {
    double x, y;
    Point(double x = 0.0, double y = 0.0);
};

// Function declarations
int Round(double x);
void DrawPoint(HDC hdc, int x, int y, COLORREF color);
void Draw8Points(HDC hdc, int xc, int yc, int x, int y, COLORREF color);

#endif // ALGONAMES_H
