#include <string.h>

/*** Messages ***/

void ViewS(char S[])
{
    Msg->info("string", S);
}

void ViewI(int nr)
{
    char str[255];
    sprintf(str, "%d", nr);
    Msg->info("integer", str);
}

void ViewI2(int P[2])
{
    char str[255];
    sprintf(str, "%d %d", P[0], P[1]);
    Msg->info("integer", str);
}

void ViewI2_2(int nr1, int nr2)
{
    char str[255];
    sprintf(str, "%d %d", nr1, nr2);
    Msg->info("integer", str);
}

void ViewL(long nr)
{
    char str[255];
    sprintf(str, "%ld", nr);
    Msg->info("integer", str);
}

void ViewD(double nr)
{
    char str[255];
    sprintf(str, "%f", nr);
    Msg->info("double", str);
}

void ViewP(double P[3])
{
    char str[255];
    sprintf(str, "%f %f %f", P[0], P[1], P[2]);
    Msg->info("point", str);
}

void ViewP2(double P[2])
{
    char str[255];
    sprintf(str, "%f %f", P[0], P[1]);
    Msg->info("point", str);
}

void View2P(double P1[3], double P2[3])
{
    char str[255];
    sprintf(str, "%f %f %f - %f %f %f", P1[0], P1[1], P1[2], P2[0], P2[1], P2[2]);
    Msg->info("points", str);
}

void View4P(double P1[3], double P2[3], double P3[3], double P4[3])
{
    char str[255];
    sprintf(str, "%f %f %f - %f %f %f - %f %f %f - %f %f %f", 
        P1[0], P1[1], P1[2], P2[0], P2[1], P2[2], P3[0], P3[1], P3[2], P4[0], P4[1], P4[2]);
    Msg->info("points", str);
}

void View4P2(double P[4][3])
{
    char str[255];
    sprintf(str, "%f %f %f - %f %f %f - %f %f %f - %f %f %f", 
        P[0][0], P[0][1], P[0][2], P[1][0], P[1][1], P[1][2], 
        P[2][0], P[2][1], P[2][2], P[3][0], P[3][1], P[3][2]);
    Msg->info("points", str);
}

void ViewE(Edge E)
{
    char str[255];
    sprintf(str, "%d", E.eP);
    Msg->info("Edge", str);
}

void ViewEC(char C[], Edge E)
{
    char str[255];
    sprintf(str, "%d", E.eP);
    Msg->info(C, str);
}

void View2E(Edge E1, Edge E2)
{
    char str[255];
    sprintf(str, "%d %d", E1.eP, E2.eP);
    Msg->info("Edges", str);
}

void View2EC(char C[], Edge E1, Edge E2)
{
    char str[255];
    sprintf(str, "%d %d", E1.eP, E2.eP);
    Msg->info(C, str);
}

/*** Log file ***/

static FILE *LogF;

void CreateLogFile(char *BaseName)
{
    LogF = fopen(BaseName, "w");
}

void StoreLog_String(char *Line)
{
    if (localData.CreateLogFile) return;
    fprintf(LogF, "%s\n", Line);
}

void StoreLog_Poly(PolygonInfo *Poly)
{
    int i;
    if (localData.CreateLogFile) return;
    fprintf(LogF, "Poly: %d\n", Poly->spol);
    for (i = 0; i < Poly->numPnts; i++)
        fprintf(LogF, "%d ", Poly->points[i]);
    fprintf(LogF, "\n");
}

void StoreLog_PolyScan(LastPolyScanned Poly, CutDirection OldCutDir, CutDirection NewCutDir, int Idx, int SearchOrder)
{
    int i;
    if (localData.CreateLogFile) return;
    fprintf(LogF, "--- Polygon scan info: idx: %d, Polygon search order: %d\n", Idx, SearchOrder);
    fprintf(LogF, "--- Old cut edges: %d, %d; New cut edges: %d, %d\n", 
        OldCutDir.Ed1.eP, OldCutDir.Ed2.eP, NewCutDir.Ed1.eP, NewCutDir.Ed2.eP);
}

void StoreLog_Cutted(PolyID PolyID, CutDirection OutDir, Edge CreateEdge1, Edge CreateEdge2)
{
    if (localData.CreateLogFile) return;
    fprintf(LogF, "*** Polygon cut: %d\n", PolyID);
    fprintf(LogF, "--- OutDir: %d, %d; Create edges: %d, %d\n", 
        OutDir.Ed1.eP, OutDir.Ed2.eP, CreateEdge1.eP, CreateEdge2.eP);
}

void StoreLog_CreatedPoints(int Idx, double P1[3])
{
    if (localData.CreateLogFile) return;
    fprintf(LogF, "*** Created Point %d at (%f, %f, %f)\n", Idx, P1[0], P1[1], P1[2]);
}

void StoreLog_SidePoints(double SideP1[3], double SideP2[3])
{
    if (localData.CreateLogFile) return;
    fprintf(LogF, "--- Side Point 1 at (%f, %f, %f)\n", SideP1[0], SideP1[1], SideP1[2]);
    fprintf(LogF, "--- Side Point 2 at (%f, %f, %f)\n", SideP2[0], SideP2[1], SideP2[2]);
}

void StoreLog_PointAtPos(double P[3], int APos)
{
    if (localData.CreateLogFile) return;
    if (APos < 0)
        fprintf(LogF, "--- not found: (%f, %f, %f)\n", P[0], P[1], P[2]);
    else
        fprintf(LogF, "--- found: (%f, %f, %f) at node %d\n", P[0], P[1], P[2], APos);
}

void StoreLog_CutEdges(CutDirection CutDir)
{
    if (localData.CreateLogFile) return;
    fprintf(LogF, "--- Cut edges: %d, %d\n", CutDir.Ed1.eP, CutDir.Ed2.eP);
}

void StoreLog_Total(int TotalSplitted, int Passes)
{
    if (localData.CreateLogFile) return;
    fprintf(LogF, "//// Total splitted polygons: %d ////\n", TotalSplitted);
    fprintf(LogF, "//// Total needed passes: %d ////\n", Passes);
}

void StoreLog_UsedPoint(int Idx, double P[3])
{
    if (localData.CreateLogFile) return;
    fprintf(LogF, "--- Used side point %d at (%f, %f, %f)\n", Idx, P[0], P[1], P[2]);
}

void StoreLog_StoreInPoint(int Idx, int InPoint)
{
    if (localData.CreateLogFile) return;
    if (InPoint)
        fprintf(LogF, "--- Stored point %d at side point %d\n", Idx, InPoint);
    else
        fprintf(LogF, "--- Stored point %d at default side point %d\n", Idx, Idx);
}

void DoneLogFile(void)
{
    if (localData.CreateLogFile) return;
    fclose(LogF);
}