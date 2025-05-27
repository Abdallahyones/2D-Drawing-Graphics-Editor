#include <windows.h>
#include <list>
#include <vector>
#include <cmath>
#include <algorithm>
#include <sstream>
#include "Common.h"

using namespace std;

#define MAX_SCANLINES 600

/*
    Defines the maximum number of scanlines (y-coordinates) the program can handle for polygon filling.
    This assumes the window height is at most 600 pixels
*/
// -------------------------------------------- Utility Functions --------------------------------------------------


void DrawLineDDa(HDC hdc, int startX, int startY, int endX, int endY, COLORREF color) {
    int dx = endX - startX;
    int dy = endY - startY;
    SetPixel(hdc, startX, startY, color);

    if (abs(dx) >= abs(dy)) { // X-dominant case
        int currentX = startX;
        int xIncrement = dx > 0 ? 1 : -1;
        double currentY = startY;
        double yIncrement = dx != 0 ? (double) dy / dx * xIncrement : 0;
        while (currentX != endX) {
            currentX += xIncrement;
            currentY += yIncrement;
            SetPixel(hdc, currentX, Round(currentY), color);
        }
    } else {
        // Y-dominant case (not used for horizontal scanlines)
        int currentY = startY;
        int yIncrement = dy > 0 ? 1 : -1;
        double currentX = startX;
        double xIncrement = dy != 0 ? (double) dx / dy * yIncrement : 0;
        while (currentY != endY) {
            currentY += yIncrement;
            currentX += xIncrement;
            SetPixel(hdc, Round(currentX), currentY, color);
        }
    }
}

// -------------------------------------------------- Structures -------------------------------------------------

//for Convex Fill
struct ScanLineEntry {
    int minXCoord, maxXCoord;
};

// for General Fill
struct PolygonEdge {
    double currentX;
    double inverseSlope;
    int maxYCoord;

    bool operator<(const PolygonEdge &r) const { return currentX < r.currentX; }
};

typedef list <PolygonEdge> EdgeList;

// ----------------------------------------------- Convex Polygon Fill Functions -------------------------------------
void InitializeScanlineTable(ScanLineEntry table[]) {
    for (int i = 0; i < MAX_SCANLINES; i++) {
        table[i].minXCoord = INT_MAX;
        table[i].maxXCoord = INT_MIN;
    }
}

void ScanEdge(POINT vertexStart, POINT vertexEnd, ScanLineEntry table[]) {
    if (vertexStart.y == vertexEnd.y) {
        return;
    }
    if (vertexStart.y > vertexEnd.y) {
        swap(vertexStart, vertexEnd);
    }
    double edgeInverseSlope = (double) (vertexEnd.x - vertexStart.x) / (vertexEnd.y - vertexStart.y);
    double currentEdgeX = vertexStart.x;
    int currentScanLineY = vertexStart.y;
    while (currentScanLineY < vertexEnd.y) {
        if (currentEdgeX < table[currentScanLineY].minXCoord) {
            table[currentScanLineY].minXCoord = (int) (currentEdgeX);
        }
        if (currentEdgeX > table[currentScanLineY].maxXCoord) {
            table[currentScanLineY].maxXCoord = (int) floor(currentEdgeX);
        }
        currentScanLineY++;
        currentEdgeX += edgeInverseSlope;
    }
}

void DrawScanLines(HDC hdc, ScanLineEntry table[], COLORREF fillColor) {
    for (int scanlineY = 0; scanlineY < MAX_SCANLINES; scanlineY++) {
        if (table[scanlineY].minXCoord < table[scanlineY].maxXCoord) {
            int xStart = table[scanlineY].minXCoord;
            int xEnd = table[scanlineY].maxXCoord;
            DrawLineDDa(hdc, xStart, scanlineY, xEnd, scanlineY, fillColor);

        }
    }
}

void ConvertPolyonToTable(POINT vertices[], int vertexCount, ScanLineEntry table[]) {
    POINT previousVertex = vertices[vertexCount - 1];
    for (int i = 0; i < vertexCount; i++) {
        POINT currentVertex = vertices[i];
        ScanEdge(previousVertex, currentVertex, table);
        previousVertex = vertices[i];
    }
}

void ConvexFill(HDC hdc, POINT vertices[], int vertexCount, COLORREF fillColor) {
    ScanLineEntry* table = new ScanLineEntry[MAX_SCANLINES];
    InitializeScanlineTable(table);
    ConvertPolyonToTable(vertices, vertexCount, table);
    DrawScanLines(hdc, table, fillColor);
    delete[] table;
}

// ----------------------------------------------- General Polygon Fill Functions -------------------------------------
PolygonEdge InitEdgeRec(POINT &vertexStart, POINT &vertexEnd) {
    if (vertexStart.y > vertexEnd.y) swap(vertexStart, vertexEnd);
    PolygonEdge rec;
    rec.currentX = vertexStart.x;
    rec.maxYCoord = vertexEnd.y;
    rec.inverseSlope = (double) (vertexEnd.x - vertexStart.x) / (vertexEnd.y - vertexStart.y);
    return rec;
}

void BuildEdgeTable(POINT *polygonVertices, int vertexCount, EdgeList table[]) {
    POINT previousVertex = polygonVertices[vertexCount - 1];
    for (int i = 0; i < vertexCount; i++) {
        POINT currentVertex = polygonVertices[i];
        if (previousVertex.y == currentVertex.y) {
            previousVertex = currentVertex;
            continue; // Skip horizontal edges
        }
        PolygonEdge rec = InitEdgeRec(previousVertex, currentVertex);
        table[previousVertex.y].push_back(rec);
        previousVertex = polygonVertices[i];
    }
}

void GeneralPolygonFill(HDC hdc, POINT *polygonVertices, int vertexCount, COLORREF fillColor) {
    EdgeList *edgeTable = new EdgeList[MAX_SCANLINES];
    BuildEdgeTable(polygonVertices, vertexCount, edgeTable);

    // Find the first scanline that contains edges
    int currentScanline = 0;
    while (currentScanline < MAX_SCANLINES && edgeTable[currentScanline].empty()) {
        currentScanline++;
    }

    // If no edges found in the entire table, return
    if (currentScanline == MAX_SCANLINES) {
        delete[] edgeTable;
        return;
    }

    // Initialize the active edge list with edges from the first non-empty scanline
    EdgeList activeEdges = edgeTable[currentScanline];

    // Process each scanline from bottom to top
    while (!activeEdges.empty()) {
        // Sort active edges by their current x position (left to right)
        activeEdges.sort();

        // Fill between pairs of edges (edge pairs define horizontal spans to fill)
        for (auto currentEdge = activeEdges.begin(); currentEdge != activeEdges.end();) {
            // Get left edge x position (rounded up)
            int leftX = (int) ceil(currentEdge->currentX);
            ++currentEdge;

            // If no matching right edge, break
            if (currentEdge == activeEdges.end()) break;

            // Get right edge x position (rounded down)
            int rightX = (int) floor(currentEdge->currentX);

            // Draw horizontal line between edges
            if (leftX <= rightX) {  // Only draw if valid span
                DrawLineDDa(hdc, leftX, currentScanline, rightX, currentScanline, fillColor);
            }

            ++currentEdge;
        }

        // Move to next scanline
        currentScanline++;

        // Remove edges that end at this scanline
        for (auto currentEdge = activeEdges.begin(); currentEdge != activeEdges.end();) {
            if (currentScanline == currentEdge->maxYCoord) {
                currentEdge = activeEdges.erase(currentEdge);
            } else {
                ++currentEdge;
            }
        }

        // Update x positions of remaining edges using their slope
        for (auto &edge: activeEdges) {
            edge.currentX += edge.inverseSlope;
        }

        // Add new edges that start at this scanline
        activeEdges.insert(activeEdges.end(),
                           edgeTable[currentScanline].begin(),
                           edgeTable[currentScanline].end());
    }
    delete[] edgeTable;
}


// -------------------------------------------- Window Procedure --------------------------------------------------
LRESULT CALLBACK
drawConvex(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, Algorithm algo, COLORREF color, DrawCommand &cmd) {
    static vector<POINT> vertices;
    HDC hdc;
    int x, y;

    switch (msg) {
        case WM_LBUTTONDOWN:
            hdc = GetDC(hwnd);
            x = LOWORD(lParam);
            y = HIWORD(lParam);
            cmd.points.emplace_back(x, y);
            // Add vertex
            vertices.push_back({x, y});
            // Draw vertex point
            SetPixel(hdc, x, y, color);
            // Draw line to previous vertex if exists using DDA
            if (vertices.size() > 1) {
                POINT prev = vertices[vertices.size() - 2];
                POINT curr = vertices[vertices.size() - 1];
                cmd.points.emplace_back(curr.x, curr.y);
                DrawLineDDa(hdc, prev.x, prev.y, curr.x, curr.y, color);
            }
            ReleaseDC(hwnd, hdc);
            break;
        case WM_RBUTTONDOWN:
            if (vertices.size() >= 3) {
                hdc = GetDC(hwnd);
                // Draw the closing edge using DDA
                POINT first = vertices[0];
                POINT last = vertices[vertices.size() - 1];
                for (int i = 2; i < vertices.size(); i++) {
                    cmd.points.emplace_back(vertices[i].x, vertices[i].y);
                }
                DrawLineDDa(hdc, last.x, last.y, first.x, first.y, color);
                if (algo == ALGO_FILL_CONVEX) {
                    ConvexFill(hdc, vertices.data(), vertices.size(), color);
                } else {
                    GeneralPolygonFill(hdc, vertices.data(), vertices.size(), color);
                }
                ReleaseDC(hwnd, hdc);
                vertices.clear(); // Clear vertices after filling
            }
            break;

        case WM_KEYDOWN:

            break;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            hdc = BeginPaint(hwnd, &ps);
            EndPaint(hwnd, &ps);
        }
            break;

        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

