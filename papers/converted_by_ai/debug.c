#include <string.h>
#include <stdio.h>

// --- Messages ---

static void ViewS(char S[])
{
    (*Msg->info)("string", S);
}

static void ViewI(int I)
{
    char str[255];
    sprintf(str, "%d", I);
    (*Msg->info)("integer", str);
}

static void ViewI2(int P[2])
{
    char str[255];
    sprintf(str, "%d, %d", P[0], P[1]);
    (*Msg->info)("point", str);
}

static void ViewI3(int I[2])
{
    char str[255];
    sprintf(str, "%d, %d", I[1], I[2]);
    (*Msg->info)("integer", str);
}

static void ViewL(long L)
{
    char str[255];
    sprintf(str, "%ld", L);
    (*Msg->info)("integer", str);
}

static void ViewD(double D)
{
    char str[255];
    sprintf(str, "%e", D);
    (*Msg->info)("float", str);
}

static void ViewD2(double P[2])
{
    char str[255];
    sprintf(str, "%e, %e", P[0], P[1]);
    (*Msg->info)("point", str);
}

static void ViewD3(double P[3])
{
    char str[255];
    sprintf(str, "%e, %e, %e", P[0], P[1], P[2]);
    (*Msg->info)("point", str);
}

static void ViewD2x3(double P[2][3])
{
    char str1[255], str2[255];
    sprintf(str1, "%e, %e, %e", P[0][0], P[0][1], P[0][2]);
    sprintf(str2, "%e, %e, %e", P[1][0], P[1][1], P[1][2]);
    (*Msg->info)("point1", str1);
    (*Msg->info)("point2", str2);
}

static void ViewD3x4(double P1[3], double P2[3], double P3[3], double P4[3])
{
    char str1[255], str2[255], str3[255], str4[255];
    sprintf(str1, "%e, %e, %e", P1[0], P1[1], P1[2]);
    sprintf(str2, "%e, %e, %e", P2[0], P2[1], P2[2]);
    sprintf(str3, "%e, %e, %e", P3[0], P3[1], P3[2]);
    sprintf(str4, "%e, %e, %e", P4[0], P4[1], P4[2]);
    (*Msg->info)("1", str1);
    (*Msg->info)("2", str2);
    (*Msg->info)("3", str3);
    (*Msg->info)("4", str4);
}

static void ViewI2x4(int P[4][2])
{
    char str1[255], str2[255], str3[255], str4[255];
    sprintf(str1, "%d, %d", P[0][0], P[0][1]);
    sprintf(str2, "%d, %d", P[1][0], P[1][1]);
    sprintf(str3, "%d, %d", P[2][0], P[2][1]);
    sprintf(str4, "%d, %d", P[3][0], P[3][1]);
    (*Msg->info)("1", str1);
    (*Msg->info)("2", str2);
    (*Msg->info)("3", str3);
    (*Msg->info)("4", str4);
}

static void ViewI2x3(int P[3][2])
{
    char str1[255], str2[255], str3[255];
    sprintf(str1, "%d, %d", P[0][0], P[0][1]);
    sprintf(str2, "%d, %d", P[1][0], P[1][1]);
    sprintf(str3, "%d, %d", P[2][0], P[2][1]);
    (*Msg->info)("1", str1);
    (*Msg->info)("2", str2);
    (*Msg->info)("3", str3);
}

static void ViewEdge(Edge E)
{
    char str[255];
    sprintf(str, "%d, %d", E.sP, E.eP);
    (*Msg->info)("edge", str);
}

static void ViewEdge2(Edge E1, Edge E2)
{
    char str[255];
    sprintf(str, "E1: %d, %d  E2: %d, %d", E1.sP, E1.eP, E2.sP, E2.eP);
    (*Msg->info)("edges", str);
}

static void ViewE(char Cl, Edge E1, Edge E2)
{
    char str[255];
    sprintf(str, "%c: %d, %d / %d, %d", Cl, E1.sP, E1.eP, E2.sP, E2.eP);
    (*Msg->info)("e", str);
}

// --- Log file ---
static FILE *LogF;

static void CreateLogFile(char *BaseName)
{
    if (!localData.CreateLogFile) return;
    LogF = fopen(BaseName, "w+");
}

static void StoreLog_String(char *Line)
{
    if (!localData.CreateLogFile) return;
    fprintf(LogF, "%s\n", Line);
}

static void StoreLog_PolyPolygonInfo(Poly *poly)
{
    int i;
    if (!localData.CreateLogFile) return;
    fprintf(LogF, "++\t\tPolygon info\n");
    for (i = 0; i < poly->NumPoints; i++)
        fprintf(LogF, "\t\t%d\tPoint: \t%f, %f\n", i, PI->position[0], PI->position[1]);
    fprintf(LogF, "\n");
}

static void StoreLog_PolyScan(LastPolyScanInfo Poly, CutDirection OldCutDir, CutDirection NewCutDir, int Idx, int SearchOrder)
{
    int i;
    if (!localData.CreateLogFile) return;
    fprintf(LogF, "\t\t%i - %i %i\n", "Polygon scan info", Idx, SearchOrder);
    for (i = 0; i < 2; i++)
        fprintf(LogF, "\t\t%i: (%f, %f)\t", "Point", I, Poly.Points[i][0], Poly.Points[i][1]);
    fprintf(LogF, "\n");
    fprintf(LogF, "\t\t%i: (%f,%f) - (%f,%f)\n", "initial cut dir", OldCutDir.Ed.sP, OldCutDir.Ed.eP, OldCutDir.Ed.sP, OldCutDir.Ed.eP);
    fprintf(LogF, "\t\t%i: (%f,%f) - (%f,%f)\n", "old cut edges", OldCutDir.Ed.sP, OldCutDir.Ed.eP, OldCutDir.Ed.sP, OldCutDir.Ed.eP);
    fprintf(LogF, "\t\t%i: (%f,%f) - (%f,%f)\n", "new cut dir", NewCutDir.Ed.sP, NewCutDir.Ed.eP, NewCutDir.Ed.sP, NewCutDir.Ed.eP);
    fprintf(LogF, "\n");
}

static void StoreLog_VistedPoly(Poly PolyD, CutDirection CutDir, Edge CreatedEdge)
{
    if (!localData.CreateLogFile) return;
    fprintf(LogF, "\t\t%i: %i\n", "Visited poly", PolyD);
    fprintf(LogF, "\t\t%i: (%f, %f)\n", "Cut edge", CutDir.Ed.sP, CutDir.Ed.eP);
    fprintf(LogF, "\t\t%i: (%f, %f)\n", "Created edge", CreatedEdge.sP, CreatedEdge.eP);
    fprintf(LogF, "\n");
}

static void StoreLog_CreatedPoint(int Idx, double P[3])
{
    if (!localData.CreateLogFile) return;
    fprintf(LogF, "\t\t-- Created Point at idx (%f, %f, %f)\n", Idx, P[0], P[1], P[2]);
}

static void StoreLog_SidePoints(double SideP1[3], double SideP2[3])
{
    if (!localData.CreateLogFile) return;
    fprintf(LogF, "\t\t-- Side Point 1 at (%f, %f, %f)\n", SideP1[0], SideP1[1], SideP1[2]);
    fprintf(LogF, "\t\t-- Side Point 2 at (%f, %f, %f)\n", SideP2[0], SideP2[1], SideP2[2]);
}

static void StoreLog_PointAtPos(double P[3], int AtPos)
{
    if (!localData.CreateLogFile) return;
    if (AtPos < 0)
        fprintf(LogF, "-- not found: (%f, %f, %f) at node %i.\n", P[0], P[1], P[2], AtPos);
    else
        fprintf(LogF, "-- found: (%f, %f, %f) at node %i.\n", P[0], P[1], P[2], AtPos);
}

static void StoreLog_CutEdges(CutDirection CutDir)
{
    if (!localData.CreateLogFile) return;
    fprintf(LogF, "\t\t%i: (%i, %i)\n", "cut edges", CutDir.Ed.sP, CutDir.Ed.eP);
}

static void StoreLog_Total(int TotalSplitted, int Passes)
{
    if (!localData.CreateLogFile) return;
    fprintf(LogF, "// Total splitted polygons: %i /////\n", TotalSplitted);
    fprintf(LogF, "// Total needed passes: %i /////\n", Passes);
}

static void StoreLog_UsedPoint(int Idx, double P[3])
{
    if (!localData.CreateLogFile) return;
    fprintf(LogF, "-- Used side point at Idx at (%f, %f, %f)\n", Idx, P[0], P[1], P[2]);
}

static void StoreLog_StoredPoint(int Idx, int InPoint)
{
    if (!localData.CreateLogFile) return;
    if (InPoint)
        fprintf(LogF, "-- Stored point at %i side point %i \n", Idx, InPoint);
    else
        fprintf(LogF, "-- Stored point at Idx at default side point %i \n", Idx);
}

static void DoneLogFile(void)
{
    if (!localData.CreateLogFile) return;
    fclose(LogF);
}