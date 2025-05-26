#include <windows.h>
#include <commdlg.h>
#include "LineAlgorithms.cpp"
#include "CircleAlgorithms.cpp"
#include "ConvexAndNonConvexFill.cpp"
#include "Ellipses_3_algorithms.cpp"
#include "Flood Fill.cpp"
#include "ProjectGraphics.cpp"
#include "clippingLinewithsquare.cpp"
#include "clippinglinewithrectangle.cpp"
#include "clippingDrawPointinRectangle.cpp"
#include "DrawPointInSquare.cpp"
#include "Common.h"
#include<iostream>
#include <fstream>
#include<vector>

using namespace std;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

void DrawAlgo(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

COLORREF currentBackgroundColor = RGB(255, 255, 255);
COLORREF currentShapeColor = RGB(0, 0, 0);
Shape currentShape = SHAPE_LINE;
Algorithm currentAlgorithm = ALGO_LINE_DDA;
HBRUSH backgroundBrush = CreateSolidBrush(currentBackgroundColor);


vector<DrawCommand> drawHistory;
void saveToFile(const string& filename) {
    ofstream out(filename);
    if (!out) {
        cerr << "Failed to open file for writing.\n";
        return;
    }
    out << drawHistory.size() << '\n';
    for (const auto& cmd : drawHistory) {
        out << static_cast<int>(cmd.shape) << ' '
            << static_cast<int>(cmd.algorithm) << ' '
            << cmd.shapeColor << ' ' << cmd.fillColor << ' '
            << static_cast<int>(cmd.quarter) << ' '
            << cmd.radius << ' ' << cmd.width << ' ' << cmd.height << ' '
            << cmd.thickness << ' ' << cmd.curveTension << '\n';

        out << cmd.points.size() << '\n';
        for (const auto& pt : cmd.points)
            out << pt << '\n';

        out << cmd.controlPoints.size() << '\n';
        for (const auto& pt : cmd.controlPoints)
            out << pt << '\n';
    }
}

void loadFromFile(const string& filename) {
   ifstream in(filename);
    if (!in) {
        throw std::runtime_error("Failed to open file for reading.");
    }
    int commandCount;
    in >> commandCount;

    for (int i = 0; i < commandCount; ++i) {
        DrawCommand cmd;
        int shapeInt, algorithmInt, quarterInt;
        in >> shapeInt >> algorithmInt >> cmd.shapeColor >> cmd.fillColor >> quarterInt
           >> cmd.radius >> cmd.width >> cmd.height >> cmd.thickness >> cmd.curveTension;

        cmd.shape = static_cast<Shape>(shapeInt);
        cmd.algorithm = static_cast<Algorithm>(algorithmInt);
        cmd.quarter = static_cast<FillQuarter>(quarterInt);

        int cnt;
        in >> cnt;
        cmd.points.resize(cnt);
        for (auto& pt : cmd.points)
            in >> pt;

        int controlCount;
        in >> controlCount;
        cmd.controlPoints.resize(controlCount);
        for (auto& pt : cmd.controlPoints)
            in >> pt;

        drawHistory.push_back(cmd);
    }
    cout<<drawHistory.size();
}

void ClearScreen(HWND hwnd) {
    HDC hdc = GetDC(hwnd);
    RECT rect;
    GetClientRect(hwnd, &rect);
    FillRect(hdc, &rect, backgroundBrush);
    ReleaseDC(hwnd, hdc);
}

COLORREF OpenColorDialog(HWND hwnd, COLORREF initialColor) {
    CHOOSECOLOR cc = {sizeof(CHOOSECOLOR)};
    static COLORREF acrCustClr[16];
    cc.hwndOwner = hwnd;
    cc.lpCustColors = acrCustClr;
    cc.rgbResult = initialColor;
    cc.Flags = CC_FULLOPEN | CC_RGBINIT;

    if (ChooseColor(&cc)) {
        return cc.rgbResult;
    }
    return initialColor;
}

void AddMenus(HWND hwnd) {
    HMENU hMenubar = CreateMenu();
    HMENU fileMenu = CreateMenu();
    HMENU drawMenu = CreateMenu();
    HMENU colorMenu = CreateMenu();
    HMENU algoMenu = CreateMenu();

    // File Menu
    AppendMenu(fileMenu, MF_STRING, 1, "Save");
    AppendMenu(fileMenu, MF_STRING, 2, "Load");
    AppendMenu(fileMenu, MF_STRING, 3, "Clear");

    // Draw Menu
    AppendMenu(drawMenu, MF_STRING, 4, "Draw DDA Line");
    AppendMenu(drawMenu, MF_STRING, 5, "Draw Midpoint Line");
    AppendMenu(drawMenu, MF_STRING, 6, "Draw BRESENHAM Line");

    // Color Menu
    AppendMenu(colorMenu, MF_STRING, 7, "Change Background Color");
    AppendMenu(colorMenu, MF_STRING, 8, "Change Shape Color");

    // Circle Submenu
    HMENU circleMenu = CreateMenu();
    AppendMenu(circleMenu, MF_STRING, 100, "Direct");
    AppendMenu(circleMenu, MF_STRING, 101, "Polar");
    AppendMenu(circleMenu, MF_STRING, 102, "Iterative Polar");
    AppendMenu(circleMenu, MF_STRING, 103, "Midpoint");
    AppendMenu(circleMenu, MF_STRING, 104, "Modified Midpoint");

    // Ellipse Submenu
    HMENU ellipseMenu = CreateMenu();
    AppendMenu(ellipseMenu, MF_STRING, 105, "Direct");
    AppendMenu(ellipseMenu, MF_STRING, 106, "Polar");
    AppendMenu(ellipseMenu, MF_STRING, 107, "Midpoint");

    // Fill Submenu
    HMENU fillMenu = CreateMenu();
    AppendMenu(fillMenu, MF_STRING, 13, "Fill Circle with Lines");
    AppendMenu(fillMenu, MF_STRING, 14, "Fill Circle with Circles");
    AppendMenu(fillMenu, MF_STRING, 15, "Fill Square with Hermit Curve");
    AppendMenu(fillMenu, MF_STRING, 16, "Fill Rectangle with Bezier Curve");
    AppendMenu(fillMenu, MF_STRING, 17, "Convex  Filling");
    AppendMenu(fillMenu, MF_STRING, 25, "Non-Convex Filling");
    AppendMenu(fillMenu, MF_STRING, 18, "Recursive Flood Fill");
    AppendMenu(fillMenu, MF_STRING, 19, "Non-Recursive Flood Fill");

// Clipping Submenu
    HMENU clipMenu = CreateMenu();

    HMENU clipRectangleMenu = CreateMenu();
    AppendMenu(clipRectangleMenu, MF_STRING, 220, "Point");
    AppendMenu(clipRectangleMenu, MF_STRING, 221, "Line");
    AppendMenu(clipRectangleMenu, MF_STRING, 222, "Polygon");
    AppendMenu(clipMenu, MF_POPUP, (UINT_PTR) clipRectangleMenu, "Clipping with Rectangle");

    HMENU clipSquareMenu = CreateMenu();
    AppendMenu(clipSquareMenu, MF_STRING, 230, "Point");
    AppendMenu(clipSquareMenu, MF_STRING, 231, "Line");
    AppendMenu(clipMenu, MF_POPUP, (UINT_PTR) clipSquareMenu, "Clipping with Square");

    // Main Algo Menu
    AppendMenu(algoMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(algoMenu, MF_POPUP, (UINT_PTR) drawMenu, "Line Algorithms");
    AppendMenu(algoMenu, MF_POPUP, (UINT_PTR) circleMenu, "Circle Algorithms");
    AppendMenu(algoMenu, MF_POPUP, (UINT_PTR) ellipseMenu, "Ellipse Algorithms");
    AppendMenu(algoMenu, MF_POPUP, (UINT_PTR) fillMenu, "Fill Algorithms");
    AppendMenu(algoMenu, MF_STRING, 20, "Cardinal Spline Curve");
    AppendMenu(algoMenu, MF_POPUP, (UINT_PTR) clipMenu, "Clipping Algorithms");

    // Add Menus to MenuBar
    AppendMenu(hMenubar, MF_POPUP, (UINT_PTR) fileMenu, "File");
    AppendMenu(hMenubar, MF_POPUP, (UINT_PTR) colorMenu, "Color");
    AppendMenu(hMenubar, MF_POPUP, (UINT_PTR) algoMenu, "Algorithms");

    SetMenu(hwnd, hMenubar);
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "DrawingApp";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_CROSS);
    wc.hbrBackground = backgroundBrush;

    RegisterClass(&wc);

    HWND hwnd = CreateWindow(CLASS_NAME, "2D Drawing Program",
                             WS_OVERLAPPEDWINDOW,
                             CW_USEDEFAULT, CW_USEDEFAULT, 900, 700,
                             NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    AllocConsole();
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        if (msg.message == WM_KEYDOWN || msg.message == WM_CHAR)
            continue;

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

bool drawingMode = false;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            AddMenus(hwnd);
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case 1:
                    saveToFile("in.txt");
                    break;
                case 2:
                    loadFromFile("in.txt");
                    break;
                case 3:
                    ClearScreen(hwnd);
                    break;
                case 4:
                    currentAlgorithm = ALGO_LINE_DDA;
                    currentShape = SHAPE_LINE;
                    drawingMode = true;
                    break;
                case 5:
                    currentAlgorithm = ALGO_LINE_MIDPOINT;
                    currentShape = SHAPE_LINE;
                    drawingMode = true;
                    break;
                case 6:
                    currentAlgorithm = ALGO_LINE_BRESENHAM;
                    currentShape = SHAPE_LINE;
                    drawingMode = true;
                    break;
                case 7:
                    currentBackgroundColor = OpenColorDialog(hwnd, currentBackgroundColor);
                    DeleteObject(backgroundBrush);
                    backgroundBrush = CreateSolidBrush(currentBackgroundColor);
                    SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR) backgroundBrush);
                    InvalidateRect(hwnd, NULL, TRUE);
                    break;
                case 8:
                    currentShapeColor = OpenColorDialog(hwnd, currentShapeColor);
                    break;

                case 9:
                    ClearScreen(hwnd);
                    drawingMode = false;
                    break;
                case 10:
                    saveToFile("in.txt");
                    drawingMode = false;
                    break;
                case 11:
                    loadFromFile("in.txt");
                    break;
                case 12:
                    currentAlgorithm = ALGO_CIRCLE_DIRECT;
                    currentShape = SHAPE_CIRCLE;
                    drawingMode = true;
                    break;
                case 13:
                    currentAlgorithm = ALGO_FILL_CIRCLE_LINES;
                    currentShape = SHAPE_FILLING;
                    drawingMode = true;
                    break;
                case 14:
                    currentAlgorithm = ALGO_FILL_CIRCLE_CIRCLES;
                    currentShape = SHAPE_FILLING;
                    drawingMode = true;
                    break;

                case 15:
                    currentAlgorithm = ALGO_FILL_SQUARE_HERMIT_VERTICAL;
                    currentShape = SHAPE_FILLING;
                    drawingMode = true;

                    break;
                case 16:
                    currentAlgorithm = ALGO_FILL_RECTANGLE_BEZIER_HORIZONTAL;
                    currentShape = SHAPE_FILLING;
                    drawingMode = true;
                    break;

                case 17:
                    currentAlgorithm = ALGO_FILL_CONVEX;
                    currentShape = SHAPE_Convex;
                    drawingMode = true;
                    break;
                case 18:
                    currentAlgorithm = ALGO_Recursive;
                    currentShape = SHAPE_Recursive_AND_NOT;
                    drawingMode = true;
                    break;
                case 19:
                    currentAlgorithm = ALGO_NON_Recursive;
                    currentShape = SHAPE_Recursive_AND_NOT;
                    drawingMode = true;
                    break;

                case 20:
                    currentAlgorithm = ALGO_CARDINAL_SPLINE;
                    currentShape = Spline_Curve;
                    drawingMode = true;
                    break;

                case 21:
                    currentAlgorithm = ALGO_ELLIPSE_DIRECT;
                    currentShape = SHAPE_ELLIPSE;
                    drawingMode = true;
                    break;
                case 25 :
                    currentAlgorithm = ALGO_FILL_NONCONVEX;
                    currentShape = SHAPE_Convex;
                    drawingMode = true;
                case 100:
                    currentAlgorithm = ALGO_CIRCLE_DIRECT;
                    currentShape = SHAPE_CIRCLE;
                    drawingMode = true;
                    break;
                case 101:
                    currentAlgorithm = ALGO_CIRCLE_POLAR;
                    currentShape = SHAPE_CIRCLE;
                    drawingMode = true;
                    break;
                case 102:
                    currentAlgorithm = ALGO_CIRCLE_ITERATIVE_POLAR;
                    currentShape = SHAPE_CIRCLE;
                    drawingMode = true;
                    break;
                case 103:
                    currentAlgorithm = ALGO_CIRCLE_MIDPOINT;
                    currentShape = SHAPE_CIRCLE;
                    drawingMode = true;
                    break;
                case 104:
                    currentAlgorithm = ALGO_CIRCLE_MODIFIED_MIDPOINT;
                    currentShape = SHAPE_CIRCLE;
                    drawingMode = true;
                    break;
                case 105:
                    currentAlgorithm = ALGO_ELLIPSE_DIRECT;
                    currentShape = SHAPE_ELLIPSE;
                    drawingMode = true;
                    break;
                case 106:
                    currentAlgorithm = ALGO_ELLIPSE_POLAR;
                    currentShape = SHAPE_ELLIPSE;
                    drawingMode = true;
                    break;
                case 107:
                    currentAlgorithm = ALGO_ELLIPSE_MIDPOINT;
                    currentShape = SHAPE_ELLIPSE;
                    drawingMode = true;
                    break;
                case 220:
                    currentShape = SHAPE_CLIPING;
                    currentAlgorithm = ALGO_CLIP_RECTANGLE_POINT;
                    drawingMode = true;
                    break;
                case 221:
                    currentShape = SHAPE_CLIPING;
                    currentAlgorithm = ALGO_CLIP_RECTANGLE_LINE;
                    drawingMode = true;
                    break;
                case 222:
                    currentShape = SHAPE_CLIPING;
                    currentAlgorithm = ALGO_CLIP_RECTANGLE_POLYGON;
                    drawingMode = true;
                    break;
                case 230:
                    currentShape = SHAPE_CLIPING;
                    currentAlgorithm = ALGO_CLIP_SQUARE_POINT;
                    drawingMode = true;
                    break;
                case 231:
                    currentShape = SHAPE_CLIPING;
                    currentAlgorithm = ALGO_CLIP_SQUARE_LINE;
                    drawingMode = true;
                    break;


            }
            break;

        case WM_LBUTTONDOWN:
            if (drawingMode) {
                DrawAlgo(hwnd, msg, wParam, lParam);
            }
            break;

        case WM_DESTROY:
            DeleteObject(backgroundBrush);
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return 0;
}

void DrawAlgo(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    DrawCommand cmd(currentShape, currentAlgorithm, currentShapeColor);
    if (currentShape == SHAPE_LINE) {
        drawLine(hwnd, msg, wParam, lParam, currentAlgorithm, currentShapeColor, cmd);
    } else if (currentShape == SHAPE_CIRCLE) {
        drawCircle(hwnd, msg, wParam, lParam, currentAlgorithm, currentShapeColor, cmd);
    } else if (currentShape == SHAPE_ELLIPSE) {
        drawEllipses(hwnd, msg, wParam, lParam, currentAlgorithm, currentShapeColor, cmd);
    } else if (currentShape == SHAPE_Convex) {
        drawConvex(hwnd, msg, wParam, lParam, currentAlgorithm, currentShapeColor, cmd);
    } else if (currentShape == SHAPE_Recursive_AND_NOT) {
        drawRecursive(hwnd, msg, wParam, lParam, currentAlgorithm, currentShapeColor, cmd);
    } else if (currentShape == SHAPE_FILLING) {


    } else if (currentShape == SHAPE_CLIPING) {
        if (currentAlgorithm == ALGO_CLIP_RECTANGLE_POINT) {
            drawPointRectangle(hwnd, msg, wParam, lParam, currentShapeColor, cmd);
        } else if (currentAlgorithm == ALGO_CLIP_RECTANGLE_LINE) {
            drawLineRectangle(hwnd, msg, wParam, lParam, currentShapeColor, cmd);
        } else if (currentAlgorithm == ALGO_CLIP_RECTANGLE_POLYGON) {
            drawRectanglePolygon(hwnd, msg, wParam, lParam, currentShapeColor, cmd);
        } else if (currentAlgorithm == ALGO_CLIP_SQUARE_POINT) {
            drawPointSquare(hwnd, msg, wParam, lParam, currentShapeColor, cmd);
        } else if (currentAlgorithm == ALGO_CLIP_SQUARE_LINE) {
            drawLineSquare(hwnd, msg, wParam, lParam, currentShapeColor ,cmd);
        }
    } else if (currentShape == Spline_Curve) {
    }
    drawHistory.emplace_back(cmd);
}


