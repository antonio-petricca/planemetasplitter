/*
 * planespl.c : Plane splitting and polygon operations in 2D
 * Scanned and reconstructed from original code
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define MAXPOINTS 100
#define MAXEDGES 200

typedef struct {
    double x, y;
} Point;

typedef struct {
    int start, end;
} Edge;

typedef struct {
    Point points[MAXPOINTS];
    int numPoints;
    Edge edges[MAXEDGES];
    int numEdges;
} Polygon;

/* Function Prototypes */
void readPolygon(FILE *file, Polygon *poly);
void writePolygon(FILE *file, const Polygon *poly);
void printPolygon(const Polygon *poly);
double polygonArea(const Polygon *poly);
int isConvex(const Polygon *poly);
void translatePolygon(Polygon *poly, double dx, double dy);
void scalePolygon(Polygon *poly, double sx, double sy);
void rotatePolygon(Polygon *poly, double angleDegrees);
void splitPolygonByLine(const Polygon *poly, double a, double b, double c, Polygon *out1, Polygon *out2);

/* Main program */
int main(int argc, char *argv[]) {
    Polygon poly, polyPos, polyNeg;
    FILE *file;
    double a, b, c;

    if (argc < 2) {
        printf("Usage: %s <polygon_file>\n", argv[0]);
        return 1;
    }

    file = fopen(argv[1], "r");
    if (!file) {
        printf("Could not open file: %s\n", argv[1]);
        return 1;
    }
    readPolygon(file, &poly);
    fclose(file);

    printf("Polygon read from %s:\n", argv[1]);
    printPolygon(&poly);

    printf("Area: %lf\n", polygonArea(&poly));
    printf("Convex: %s\n", isConvex(&poly) ? "Yes" : "No");

    printf("\nEnter line coefficients for splitting (a b c for ax + by + c = 0): ");
    if (scanf("%lf %lf %lf", &a, &b, &c) != 3) {
        printf("Invalid line input.\n");
        return 1;
    }
    splitPolygonByLine(&poly, a, b, c, &polyPos, &polyNeg);
    printf("\nPolygon on positive side of line:\n");
    printPolygon(&polyPos);
    printf("\nPolygon on negative side of line:\n");
    printPolygon(&polyNeg);

    return 0;
}

void readPolygon(FILE *file, Polygon *poly) {
    int i;
    fscanf(file, "%d", &poly->numPoints);
    for (i = 0; i < poly->numPoints; ++i) {
        fscanf(file, "%lf %lf", &poly->points[i].x, &poly->points[i].y);
    }
    fscanf(file, "%d", &poly->numEdges);
    for (i = 0; i < poly->numEdges; ++i) {
        fscanf(file, "%d %d", &poly->edges[i].start, &poly->edges[i].end);
    }
}

void writePolygon(FILE *file, const Polygon *poly) {
    int i;
    fprintf(file, "%d\n", poly->numPoints);
    for (i = 0; i < poly->numPoints; ++i) {
        fprintf(file, "%.8lf %.8lf\n", poly->points[i].x, poly->points[i].y);
    }
    fprintf(file, "%d\n", poly->numEdges);
    for (i = 0; i < poly->numEdges; ++i) {
        fprintf(file, "%d %d\n", poly->edges[i].start, poly->edges[i].end);
    }
}

void printPolygon(const Polygon *poly) {
    int i;
    printf("Points (%d):\n", poly->numPoints);
    for (i = 0; i < poly->numPoints; ++i) {
        printf("  %d: (%.3lf, %.3lf)\n", i, poly->points[i].x, poly->points[i].y);
    }
    printf("Edges (%d):\n", poly->numEdges);
    for (i = 0; i < poly->numEdges; ++i) {
        printf("  %d: %d -> %d\n", i, poly->edges[i].start, poly->edges[i].end);
    }
}

double polygonArea(const Polygon *poly) {
    double area = 0.0;
    int i, j;
    for (i = 0; i < poly->numPoints; ++i) {
        j = (i + 1) % poly->numPoints;
        area += poly->points[i].x * poly->points[j].y;
        area -= poly->points[j].x * poly->points[i].y;
    }
    return fabs(area) * 0.5;
}

int isConvex(const Polygon *poly) {
    int i, n = poly->numPoints;
    int got_negative = 0, got_positive = 0;
    for (i = 0; i < n; ++i) {
        double dx1 = poly->points[(i+2)%n].x - poly->points[(i+1)%n].x;
        double dy1 = poly->points[(i+2)%n].y - poly->points[(i+1)%n].y;
        double dx2 = poly->points[i].x - poly->points[(i+1)%n].x;
        double dy2 = poly->points[i].y - poly->points[(i+1)%n].y;
        double cross = dx1 * dy2 - dy1 * dx2;
        if (cross < 0)
            got_negative = 1;
        else if (cross > 0)
            got_positive = 1;
        if (got_negative && got_positive)
            return 0;
    }
    return 1;
}

void translatePolygon(Polygon *poly, double dx, double dy) {
    int i;
    for (i = 0; i < poly->numPoints; ++i) {
        poly->points[i].x += dx;
        poly->points[i].y += dy;
    }
}

void scalePolygon(Polygon *poly, double sx, double sy) {
    int i;
    for (i = 0; i < poly->numPoints; ++i) {
        poly->points[i].x *= sx;
        poly->points[i].y *= sy;
    }
}

void rotatePolygon(Polygon *poly, double angleDegrees) {
    int i;
    double angleRad = angleDegrees * M_PI / 180.0;
    double cosA = cos(angleRad);
    double sinA = sin(angleRad);
    for (i = 0; i < poly->numPoints; ++i) {
        double x = poly->points[i].x;
        double y = poly->points[i].y;
        poly->points[i].x = x * cosA - y * sinA;
        poly->points[i].y = x * sinA + y * cosA;
    }
}

/* Split polygon by line: ax + by + c = 0
   Points on the positive side go to out1, negative to out2.
   Edges are reconstructed for each respective polygon.
*/
void splitPolygonByLine(const Polygon *poly, double a, double b, double c, Polygon *out1, Polygon *out2) {
    int i, j, n = poly->numPoints;
    int p1_idx = 0, p2_idx = 0;
    double d1, d2;
    Point p1, p2, inter;
    // Temporary arrays for point storage
    Point points1[MAXPOINTS], points2[MAXPOINTS];
    int num1 = 0, num2 = 0;

    for (i = 0; i < n; ++i) {
        p1 = poly->points[i];
        p2 = poly->points[(i+1)%n];
        d1 = a*p1.x + b*p1.y + c;
        d2 = a*p2.x + b*p2.y + c;

        // If p1 on positive/zero side, add to out1
        if (d1 >= 0) {
            points1[num1++] = p1;
        }
        // If p1 on negative/zero side, add to out2
        if (d1 <= 0) {
            points2[num2++] = p1;
        }
        // If edge crosses the line, compute intersection
        if ((d1 > 0 && d2 < 0) || (d1 < 0 && d2 > 0)) {
            double t = d1 / (d1 - d2);
            inter.x = p1.x + t * (p2.x - p1.x);
            inter.y = p1.y + t * (p2.y - p1.y);
            points1[num1++] = inter;
            points2[num2++] = inter;
        }
    }
    // Fill out1
    out1->numPoints = num1;
    for (i = 0; i < num1; ++i) out1->points[i] = points1[i];
    out1->numEdges = num1;
    for (i = 0; i < num1; ++i) {
        out1->edges[i].start = i;
        out1->edges[i].end = (i+1)%num1;
    }
    // Fill out2
    out2->numPoints = num2;
    for (i = 0; i < num2; ++i) out2->points[i] = points2[i];
    out2->numEdges = num2;
    for (i = 0; i < num2; ++i) {
        out2->edges[i].start = i;
        out2->edges[i].end = (i+1)%num2;
    }
}