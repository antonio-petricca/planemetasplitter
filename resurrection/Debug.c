#include <string.h>

//*** Messages

    static void
ViewS(char S[])
{
    (*Msg->info)("string:", S);
}

    static void
ViewI(int I)
{
    char         str[255];

    sprintf (str, "%d", I);
    (*Msg->info)("integer:", str);
}

    static void
ViewI2(int P[2])
{
    char         str[255];

    sprintf (str, "%d : %d", P[0], P[1]);
    (*Msg->info)("point:", str);
}

    static void
View2I(int I1, int I2)
{
    char         str[255];

    sprintf (str, "%d # %d", I1, I2);
    (*Msg->info)("integer:", str);
}

    static void
ViewL(long L)
{
    char         str[255];

    sprintf (str, "%d", L);
    (*Msg->info)("integer:", str);
}

    static void
ViewD(double D)
{
    char         str[255];

    sprintf (str, "%f", D);
    (*Msg->info)("double:", str);
}

    static void
ViewP(double P[3])
{
    char         str[255];

    sprintf (str, "%f : %f : %f", P[0], P[1], P[2]);
    (*Msg->info)("point:", str);
}

    static void
ViewP2(double P[2])
{
    char         str[255];

    sprintf (str, "%f : %f", P[0], P[1]);
    (*Msg->info)("point:", str);
}

    static void
View2P(double P1[3], double P2[3])
{
    char         str1[255], str2[255];

    sprintf (str1, "%f : %f : %f", P1[0], P1[1], P1[2]);
    sprintf (str2, "%f : %f : %f", P2[0], P2[1], P2[2]);
    (*Msg->info)(str1, str2);
}

    static void
View4P(double P1[3], double P2[3], double P3[3], double P4[3])
{
    char         str1[255], str2[255];

    sprintf (str1, "%f : %f : %f - %f : %f : %f", P1[0], P1[1], P1[2], P2[0], P2[1], P2[2]);
    sprintf (str2, "%f : %f : %f - %f : %f : %f", P3[0], P3[1], P3[2], P4[0], P4[1], P4[2]);
    (*Msg->info)(str1, str2);
}

    static void
View4P2(double P[4][3])
{
    char         str1[255], str2[255];

    sprintf (str1, "%f : %f : %f - %f : %f : %f", P[0][0], P[0][1], P[0][2], P[1][0], P[1][1], P[1][2]);
    sprintf (str2, "%f : %f : %f - %f : %f : %f", P[2][0], P[2][1], P[2][2], P[3][0], P[3][1], P[3][2]);
    (*Msg->info)(str1, str2);
}

    static void
ViewN(PntID P[3])
{
    char         str[255];

    sprintf (str, "%i : %i : %i", P[0], P[1], P[2]);
    (*Msg->info)("point:", str);
}


    static void
ViewE(Edge E)
{
    char         str[255];

    sprintf (str, "%d : %d", E.sP, E.eP);
    (*Msg->info)("Edge:", str);
}

    static void
ViewEC(Edge E, char C[])
{
    char         str[255];

    sprintf (str, "%d : %d", E.sP, E.eP);
    (*Msg->info)(C, str);
}

    static void
View2E(Edge E1, Edge E2)
{
    char         str1[255], str2[255];

    sprintf (str1, "%d : %d", E1.sP, E1.eP);
    sprintf (str2, "%d : %d", E2.sP, E2.eP);
    (*Msg->info)(str1, str2);
}

    static void
View2EC(char C[], Edge E1, Edge E2)
{
    char         str[255];

    sprintf (str, "%d : %d / %d : %d", E1.sP, E1.eP, E2.sP, E2.eP);
    (*Msg->info)(C, str);
}


//*** Log file
static FILE *LogF;

    static void
CreateLogFile(char *BaseName)
{
    if (!localData.CreateLogFile) return;

    LogF= fopen(BaseName, "w");
}
    static void
_StoreLog_String(char *Line)
{
    if (!localData.CreateLogFile) return;

    fprintf(LogF, "%s \n", Line);
}

    static void
_StoreLog_Poly(PolygonInfo *Poly)
{
    PointInfo *PI;
    int       I;

    if (!localData.CreateLogFile) return;

    fprintf(LogF, "%s: %i \n", "Polygon info", Poly->pol);

    for (I=0; I<Poly->numPnts; I++) {
        PI = (*MEO->pointInfo)(MEO->state, Poly->points[I]);
        fprintf(LogF, "    %s %i: (%f ; %f ; %f) \n", "Point", I, PI->position[0], PI->position[1], PI->position[2]);
    }

    fprintf(LogF, "\n");
}

    static void
_StoreLog_PolyScan(LastPolyScanned Poly, CutDirection OldCutDir, CutDirection NewCutDir, int Idx, int SearchOrder)
{
    int I;

    if (!localData.CreateLogFile) return;

    fprintf(LogF, "%s: %i - %s: %i\n", "Polygon scan info", Idx, "Polygon search order", SearchOrder);

    for (I=0; I<=3; I++)
        fprintf(LogF, "    %s %i: (%f ; %f ; %f)\n", "Point", I, Poly.Points[I][0], Poly.Points[I][1], Poly.Points[I][2]);

    if (!Idx)
        fprintf(LogF, "    %s: (%i;%i) - (%i;%i) \n", "Initial cut edges", OldCutDir.Ed1.sP, OldCutDir.Ed1.eP, OldCutDir.Ed2.sP, OldCutDir.Ed2.eP);
    else {
        fprintf(LogF, "    %s: (%i;%i) - (%i;%i) \n", "Old cut edges", OldCutDir.Ed1.sP, OldCutDir.Ed1.eP, OldCutDir.Ed2.sP, OldCutDir.Ed2.eP);
        fprintf(LogF, "    %s: (%i;%i) - (%i;%i) \n", "New cut edges", NewCutDir.Ed1.sP, NewCutDir.Ed1.eP, NewCutDir.Ed2.sP, NewCutDir.Ed2.eP);
    }

    fprintf(LogF, "\n");
}

    static void
_StoreLog_Cutted(PolID PolyID, CutDirection CutDir, Edge CreateEdge1, Edge CreateEdge2)
{
    if (!localData.CreateLogFile) return;

    fprintf(LogF, "     *** %s: %i \n", "Polygon cut", PolyID);
    fprintf(LogF, "     *** %s: (%i;%i) - (%i;%i) \n", "Cut edges", CutDir.Ed1.sP, CutDir.Ed1.eP, CutDir.Ed2.sP, CutDir.Ed2.eP);
    fprintf(LogF, "     *** %s: (P1; %i; %i; P2) \n", "Polygon 1 points creation order", CreateEdge1.sP, CreateEdge2.sP);
    fprintf(LogF, "     *** %s: (%i; P1; P2; %i) \n", "Polygon 2 points creation order", CreateEdge1.eP, CreateEdge2.eP);

    fprintf(LogF, "\n");
}
    static void
_StoreLog_CreatedPoints(int Idx, double P1[3])
{
    if (!localData.CreateLogFile) return;

    fprintf(LogF, "     *** Created Point %i at (%f; %f; %f) \n", Idx, P1[0], P1[1], P1[2]);
}

    static void
_StoreLog_SidePoints(double SideP1[3], double SideP2[3])
{
    if (!localData.CreateLogFile) return;

    fprintf(LogF, "     *** Side Point 1 at (%f; %f; %f) \n", SideP1[0], SideP1[1], SideP1[2]);
    fprintf(LogF, "     *** Side Point 2 at (%f; %f; %f) \n", SideP2[0], SideP2[1], SideP2[2]);
}

    static void
_StoreLog_PointAtPos(double P[3], int APos)
{
    if (!localData.CreateLogFile) return;

    if (APos<0)
        fprintf(LogF, "     not found: (%f ; %f ; %f).\n", P[0], P[1], P[2]);
    else
        fprintf(LogF, "     found:     (%f ; %f ; %f) at node %i.\n", P[0], P[1], P[2], APos);

}
    static void
_StoreLog_CutEdges(CutDirection CutDir)
{
    if (!localData.CreateLogFile) return;

    fprintf(LogF, "     %s: (%i;%i) - (%i;%i) \n", "Cut edges", CutDir.Ed1.sP, CutDir.Ed1.eP, CutDir.Ed2.sP, CutDir.Ed2.eP);
}

    static void
_StoreLog_Total(int TotalSplitted, int Passes)
{
    if (!localData.CreateLogFile) return;

    fprintf(LogF, "\n");
    fprintf(LogF, "///// Total splitted polygons: %i ///// \n", TotalSplitted);
    fprintf(LogF, "///// Total needed passes    : %i ///// \n", Passes);
    fprintf(LogF, "\n");
}

    static void
_StoreLog_UsedPoint(int Idx, double P[3])
{
    if (!localData.CreateLogFile) return;

    fprintf(LogF, "     *** Used side point %i at (%f ; %f ; %f) \n", Idx, P[0], P[1], P[2]);
}

    static void
_StoreLog_StoreInPoint(int Idx, int InPoint)
{
    if (!localData.CreateLogFile) return;

    if (InPoint) fprintf(LogF, "     *** Stored point %i at side point %i \n", Idx, InPoint);
    else         fprintf(LogF, "     *** Stored point %i at default side point %i \n", Idx, Idx);
}

    static void
DoneLogFile()
{
    if (!localData.CreateLogFile) return;

    fclose(LogF);
}
