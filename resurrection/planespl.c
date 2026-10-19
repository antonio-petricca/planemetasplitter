/*

 Plane meta splitter 1.0    by Delirio(November 1996)

*/

#include <stdio.h>
#include <splug.h>
#include <lwmod.h>
#include "lwpanel.h"

#define false   0
#define true    1
#define EDERR_FOUNDPOL 10;

typedef struct st_Edge {                             //Edge data strcut
            int sP, eP;
        } Edge;

typedef struct st_CutDirection {                     //Polygon cut direction
            Edge Ed1, Ed2;
        } CutDirection;

typedef struct st_LastPolyScanned {                  //Last scanned polygons infos
            double       Points[4][3];
            double       SidePosition[3];
            PntID        SidePoint;
            CutDirection CutDir;
        } LastPolyScanned;

typedef struct st_LastScanData {                     //Last two scanned polygons infos
            LastPolyScanned Poly1, Poly2;
        } LastScanData;

typedef struct st_MyData {                           //General local server data
           PolID        FirstPol, FoundPol;
           long         SelPolsCount;
           int          LocalAxis, CreateLogFile, RevNormals;

           PntID        *PtID1, *PtID2;
           double       Pt1[3], Pt2[3];
           CutDirection *CurrCutDir;
        } MyData;

//Global variables
static LWPanControlDesc desc;
static  DynaValue   ival={DY_INTEGER},
                    ivecval={DY_VINT},
                    fval={DY_FLOAT},
                    fvecval={DY_VFLOAT},
                    sval={DY_STRING};
static MyData       localData;
static LastScanData lastData;
static MeshEditOp   *MEO;
static MessageFuncs *Msg;

/**********************/
/*** Debug routines ***/
/**********************/

#include "Debug.c"

/************************/
/*** Support routines ***/
/************************/

    static void
CopyPoints(const double S[3], double T[3])
{
    T[0] = S[0];
    T[1] = S[1];
    T[2] = S[2];
}

    static void
CopyPoints2(const double S[2], double T[2])
{
    T[0] = S[0];
    T[1] = S[1];
}

    static void
CopyPolyPoints(const double S[4][3], double T[4][3])
{
    CopyPoints(S[0], T[0]);
    CopyPoints(S[1], T[1]);
    CopyPoints(S[2], T[2]);
    CopyPoints(S[3], T[3]);
}

    static int
CmpPoints(const double P1[3], const double P2[3])
{
    return ((P1[0]==P2[0]) && (P1[1]==P2[1]) && (P1[2]==P2[2]));
}

    static int
CmpEdges(const Edge Ed1, const Edge Ed2)
{
    return (((Ed1.sP == Ed2.sP) && (Ed1.eP == Ed2.eP)) || ((Ed1.sP == Ed2.eP) && (Ed1.eP == Ed2.sP)));
}

CmpCutDirection(const CutDirection CutD1, const CutDirection CutD2)
{
    return (
        (CmpEdges(CutD1.Ed1, CutD2.Ed1) && CmpEdges(CutD1.Ed2, CutD2.Ed2)) ||
        (CmpEdges(CutD1.Ed2, CutD2.Ed1) && CmpEdges(CutD1.Ed1, CutD2.Ed2))
            );
}

    static PntID
RecognizeVoidPoint(const double P[3], int *FoundAt)
{
    if (CmpPoints(lastData.Poly1.SidePosition, P)) {
        *FoundAt = 1;
        return lastData.Poly1.SidePoint;
    } else
    if (CmpPoints(lastData.Poly2.SidePosition, P)) {
        *FoundAt = 2;
        return lastData.Poly2.SidePoint;
    } else {
        *FoundAt = 0;
        return 0;
    }
}

    static void
SwapInt(int *I1, int *I2)
{
    int Aux = *I1;
    *I1 = *I2;
    *I2 = Aux;
}

    static int
GetPrevNode(const int N)
{
    int Node = N;
    Node--;
    if (Node<0) Node = 3;
    return Node;
}
    static int
GetNextNode(const int N)
{
    int Node = N;
    Node++;
    if (Node>3) Node = 0;
    return Node;
}
    static void
SortCutDir(CutDirection *Edges)
{
    if (Edges->Ed1.sP > Edges->Ed1.eP) SwapInt(&(Edges->Ed1.sP), &(Edges->Ed1.eP));
    if (Edges->Ed2.sP < Edges->Ed2.eP) SwapInt(&(Edges->Ed2.sP), &(Edges->Ed2.eP));
}
    static void
GetOppositeEdge(const Edge Source, Edge *Opposite)
{
    Opposite->sP = Source.sP;
    Opposite->eP = Source.eP;

    if (Opposite->sP > Opposite->eP) SwapInt(&(Opposite->sP), &(Opposite->eP)); //Make points to be near

    if ((Opposite->eP - Opposite->sP) == 1) {        //External & internal intervals
        Opposite->sP = GetPrevNode(Opposite->sP);
        Opposite->eP = GetNextNode(Opposite->eP);
    } else {
        Opposite->sP = GetNextNode(Opposite->sP);
        Opposite->eP = GetPrevNode(Opposite->eP);
    }
}

    static int
Min(const int V1, const int V2)
{
    if (V1<V2) return V1;
    else       return V2;
}

    static int
Max(const int V1, const int V2)
{
    if (V1>V2) return V1;
    else       return V2;
}

    static void //unused: for future applications
ShiftPoints(PntID P[4], int By)
{
    PntID Aux;

    while (By) {
        Aux = P[0];
        P[0] = P[1];
        P[1] = P[2];
        P[2] = P[3];
        P[3] = Aux;

        By--;
    }
}

    static int
IsDirOrario(Edge E)
{
    if (!localData.RevNormals) return (E.eP == GetNextNode(E.sP));
    else                       return (E.eP != GetNextNode(E.sP));
}

/********************/
/*** Main program ***/
/********************/

    static void
GetMedianPoints(PolygonInfo *Poly)
{
    PointInfo *P1, *P2;
    double    Pos1[3], Pos2[3];

    //First median point
    P1 = (*MEO->pointInfo)(MEO->state, Poly->points[localData.CurrCutDir->Ed1.sP]);
    CopyPoints(P1->position, Pos1);

    P2 = (*MEO->pointInfo)(MEO->state, Poly->points[localData.CurrCutDir->Ed1.eP]);
    CopyPoints(P2->position, Pos2);

    localData.Pt1[0] = ((Pos1[0] + Pos2[0]) / 2);
    localData.Pt1[1] = ((Pos1[1] + Pos2[1]) / 2);
    localData.Pt1[2] = ((Pos1[2] + Pos2[2]) / 2);

    //Second median point
    P1 = (*MEO->pointInfo)(MEO->state, Poly->points[localData.CurrCutDir->Ed2.sP]);
    CopyPoints(P1->position, Pos1);

    P2 = (*MEO->pointInfo)(MEO->state, Poly->points[localData.CurrCutDir->Ed2.eP]);
    CopyPoints(P2->position, Pos2);

    localData.Pt2[0] = ((Pos1[0] + Pos2[0]) / 2);
    localData.Pt2[1] = ((Pos1[1] + Pos2[1]) / 2);
    localData.Pt2[2] = ((Pos1[2] + Pos2[2]) / 2);
}

    static EDError
SplitPoly(PolID Polygon)
{
    PolID        NewPol;
    PntID        V1[4], V2[4];
    PolygonInfo  *Poly;
    Edge         CutEdge1, CutEdge2;
    int          createdP1, createdP2, localFoundAt, localFoundAt1, localFoundAt2;

    _StoreLog_SidePoints(lastData.Poly1.SidePosition, lastData.Poly2.SidePosition); //LOGGING
    Poly = (*MEO->polyInfo)(MEO->state, Polygon);
    if (!Poly)
        return EDERR_USERABORT;

    GetMedianPoints(Poly);

    //Init recreation values
    createdP1 = false;
    createdP2 = false;
    localFoundAt  = 0;
    localFoundAt1 = 0;
    localFoundAt2 = 0;

    //Create median point 1
    if ((!(localData.PtID1 = RecognizeVoidPoint(localData.Pt1, &localFoundAt1))) || (localData.FirstPol == Polygon)) {
        localData.PtID1 = (*MEO->addPoint)(MEO->state, localData.Pt1);
        if (!localData.PtID1)
            return EDERR_USERABORT;

        createdP1 = true;
        _StoreLog_CreatedPoints(1, localData.Pt1); //LOGGING
    } else _StoreLog_UsedPoint(localFoundAt1, localData.Pt1); //LOGGING

    //Create median point 2
    if ((!(localData.PtID2 = RecognizeVoidPoint(localData.Pt2, &localFoundAt2))) || (localData.FirstPol == Polygon)) {
        localData.PtID2 = (*MEO->addPoint)(MEO->state, localData.Pt2);
        if (!localData.PtID2)
            return EDERR_USERABORT;

        createdP2 = true;
        _StoreLog_CreatedPoints(2, localData.Pt2); //LOGGING
    } else _StoreLog_UsedPoint(localFoundAt2, localData.Pt2); //LOGGING


    //Set new last values for point 1
    if (localFoundAt1)
        localFoundAt = localFoundAt1;
    else
    if (localFoundAt2)
        localFoundAt = localFoundAt2;
    else
        localFoundAt = 0;

    if (createdP1)
        switch (localFoundAt) {
            case 1:
                lastData.Poly1.SidePoint = localData.PtID1;
                CopyPoints(localData.Pt1, lastData.Poly1.SidePosition);
                _StoreLog_StoreInPoint(1, 1); //LOGGING
                break;
            case 2:
                lastData.Poly2.SidePoint = localData.PtID1;
                CopyPoints(localData.Pt1, lastData.Poly2.SidePosition);
                _StoreLog_StoreInPoint(1, 2); //LOGGING
                break;
            default:
                lastData.Poly1.SidePoint = localData.PtID1;
                CopyPoints(localData.Pt1, lastData.Poly1.SidePosition);
                _StoreLog_StoreInPoint(1, 0); //LOGGING
        }

    //Set new last values for point 2
    if (createdP2)
        switch (localFoundAt) {
            case 1:
                lastData.Poly1.SidePoint = localData.PtID2;
                CopyPoints(localData.Pt2, lastData.Poly1.SidePosition);
                _StoreLog_StoreInPoint(2, 1); //LOGGING
                break;
            case 2:
                lastData.Poly2.SidePoint = localData.PtID2;
                CopyPoints(localData.Pt2, lastData.Poly2.SidePosition);
                _StoreLog_StoreInPoint(2, 2); //LOGGING
                break;
            default:
                lastData.Poly2.SidePoint = localData.PtID2;
                CopyPoints(localData.Pt2, lastData.Poly2.SidePosition);
                _StoreLog_StoreInPoint(2, 0); //LOGGING
        }

    //Create two new polygons
    SortCutDir(localData.CurrCutDir);
    CutEdge1.sP = localData.CurrCutDir->Ed1.sP;
    CutEdge1.eP = localData.CurrCutDir->Ed1.eP;
    GetOppositeEdge(CutEdge1, &CutEdge2);
    _StoreLog_Cutted(Polygon, *localData.CurrCutDir, CutEdge1, CutEdge2); //LOGGING

    if (!IsDirOrario(CutEdge1)) {  //Direct new polygons normal
        V1[0]=localData.PtID1;
        V1[1]=Poly->points[CutEdge1.sP];
        V1[2]=Poly->points[CutEdge2.sP];
        V1[3]=localData.PtID2;

        V2[0]=Poly->points[CutEdge1.eP];
        V2[1]=localData.PtID1;
        V2[2]=localData.PtID2;
        V2[3]=Poly->points[CutEdge2.eP];
    } else {
        V1[0]=localData.PtID1;
        V1[1]=Poly->points[CutEdge1.eP];
        V1[2]=Poly->points[CutEdge2.eP];
        V1[3]=localData.PtID2;

        V2[0]=Poly->points[CutEdge1.sP];
        V2[1]=localData.PtID1;
        V2[2]=localData.PtID2;
        V2[3]=Poly->points[CutEdge2.sP];
    }

        //Create first polygon
        NewPol = (*MEO->addPoly)(MEO->state, Poly->surface, 4, V1);
        if (!NewPol)
            return EDERR_USERABORT;

        //Create second polygon
        NewPol = (*MEO->addPoly)(MEO->state, Poly->surface, 4, V2);
        if (!NewPol)
            return EDERR_USERABORT;

    //Removing original polygon
    if ((*MEO->remPoly)(MEO->state, Polygon))
        return EDERR_USERABORT;

    return EDERR_NONE;
}

/**********************/
/*** Plane creating ***/
/**********************/

    static int
ComparePolygons(const double sP[3], const double eP[3], const Edge CurrEdge, LastPolyScanned *Poly, const double CurrPoly[4][3])
{
    int  I, Found1 = false, Found2 = false;
    Edge FoundEdge, OppositeEdge;

    _StoreLog_CutEdges(Poly->CutDir); //LOGGING

    for (I=0; I<=3; I++) {
        if (CmpPoints(sP, CurrPoly[I])) {
            FoundEdge.sP = I;
            Found1 = true;
        }
            else

        if (CmpPoints(eP, CurrPoly[I])) {
            FoundEdge.eP = I;
            Found2 = true;
        }
    }

    //LOGGING
    if (Found1) _StoreLog_PointAtPos(sP, FoundEdge.sP);
    else        _StoreLog_PointAtPos(sP, -1);
    if (Found2) _StoreLog_PointAtPos(eP, FoundEdge.eP);
    else        _StoreLog_PointAtPos(eP, -1);

    if (Found1 && Found2 && !CmpEdges(CurrEdge, FoundEdge)) {
        GetOppositeEdge(FoundEdge, &OppositeEdge);
        Poly->CutDir.Ed1.sP = FoundEdge.sP;
        Poly->CutDir.Ed1.eP = FoundEdge.eP;
        Poly->CutDir.Ed2.sP = OppositeEdge.sP;
        Poly->CutDir.Ed2.eP = OppositeEdge.eP;
        SortCutDir(&(Poly->CutDir));
    }

    return (Found1 && Found2);
}

    static int
IsPolyAdiacent(const PolygonInfo *CurrPoly)
{
    PointInfo    *PI;
    int          I;
    double       Pts[4][3];

    //Retrieve polygon points
    for (I=0; I<=3; I++) {
        PI = (*MEO->pointInfo)(MEO->state, CurrPoly->points[I]);
        CopyPoints(PI->position, Pts[I]);
    }

    //Comparing points
    _StoreLog_String("    --- at poly 1:"); //LOGGING
    if (ComparePolygons(lastData.Poly1.Points[lastData.Poly1.CutDir.Ed1.sP], lastData.Poly1.Points[lastData.Poly1.CutDir.Ed1.eP], lastData.Poly1.CutDir.Ed1, &(lastData.Poly1), Pts) ||
        ComparePolygons(lastData.Poly1.Points[lastData.Poly1.CutDir.Ed2.sP], lastData.Poly1.Points[lastData.Poly1.CutDir.Ed2.eP], lastData.Poly1.CutDir.Ed2, &(lastData.Poly1), Pts))
    {
        _StoreLog_String("");    //LOGGING
        _StoreLog_PolyScan(lastData.Poly1, *(localData.CurrCutDir), lastData.Poly1.CutDir, 1, localData.SelPolsCount); //LOGGING

        localData.CurrCutDir = &(lastData.Poly1.CutDir);
        CopyPolyPoints(Pts, lastData.Poly1.Points);
        return true;
    }
          else {
    _StoreLog_String("    --- at poly 2:"); //LOGGING

    if (ComparePolygons(lastData.Poly2.Points[lastData.Poly2.CutDir.Ed1.sP], lastData.Poly2.Points[lastData.Poly2.CutDir.Ed1.eP], lastData.Poly2.CutDir.Ed1, &(lastData.Poly2), Pts) ||
        ComparePolygons(lastData.Poly2.Points[lastData.Poly2.CutDir.Ed2.sP], lastData.Poly2.Points[lastData.Poly2.CutDir.Ed2.eP], lastData.Poly2.CutDir.Ed2, &(lastData.Poly2), Pts))
    {
        _StoreLog_String("");    //LOGGING
        _StoreLog_PolyScan(lastData.Poly2, *(localData.CurrCutDir), lastData.Poly2.CutDir, 2, localData.SelPolsCount); //LOGGING

        localData.CurrCutDir = &(lastData.Poly2.CutDir);
        CopyPolyPoints(Pts, lastData.Poly2.Points);
        return true;
    }}

    return false;
}

    static EDError
ScanAdiacentPoly(MyData *localData, const PolygonInfo *PI)
{
    int NotDeleted;

    NotDeleted = ((PI->flags & (PPDF_DELETE | PPDF_CURVE)) != PPDF_DELETE) && (PI->numPnts == 4);

    if ((PI->pol == localData->FirstPol) && NotDeleted) {

        //Found first polygon
        localData->FoundPol = PI->pol;
        localData->SelPolsCount++;

        return EDERR_FOUNDPOL;
    }
    else
    if (NotDeleted) {

        //Find new adiacent polygons
        if (IsPolyAdiacent(PI)) {
            localData->FoundPol = PI->pol;
            localData->SelPolsCount++;
            return EDERR_FOUNDPOL;
        }
    }

    return EDERR_NONE;
}

    static EDError
CreateMetaSplitPlane()
{
    CutDirection CutDir;
    char             str1[100], str2[100];
    int              Passes = 0;

    _StoreLog_String("Polygons cutting"); //LOGGING
    _StoreLog_String("----------------"); //LOGGING
    _StoreLog_String("");                    //LOGGING

    //Init runtime parameters
    if (!localData.LocalAxis)
    {   //initial EVEN CUT
        CutDir.Ed1.sP = 0;
        CutDir.Ed1.eP = 1;
        CutDir.Ed2.sP = 3;
        CutDir.Ed2.eP = 2;
    }
    else
    {   //initial ODD CUT
        CutDir.Ed1.sP = 0;
        CutDir.Ed1.eP = 3;
        CutDir.Ed2.sP = 1;
        CutDir.Ed2.eP = 2;
    }
    localData.CurrCutDir = &CutDir;

    _StoreLog_PolyScan(lastData.Poly1, *(localData.CurrCutDir), lastData.Poly1.CutDir, 0, 0); //LOGGING

    lastData.Poly1.CutDir.Ed1.sP = CutDir.Ed1.sP;
    lastData.Poly1.CutDir.Ed1.eP = CutDir.Ed1.eP;
    lastData.Poly1.CutDir.Ed2.sP = CutDir.Ed2.sP;
    lastData.Poly1.CutDir.Ed2.eP = CutDir.Ed2.eP;

    lastData.Poly2.CutDir.Ed1.sP = CutDir.Ed1.sP;
    lastData.Poly2.CutDir.Ed1.eP = CutDir.Ed1.eP;
    lastData.Poly2.CutDir.Ed2.sP = CutDir.Ed2.sP;
    lastData.Poly2.CutDir.Ed2.eP = CutDir.Ed2.eP;

    localData.SelPolsCount = 0;

    //Init scanning
    do {
        //Reset found (last) polygon
        localData.FoundPol = 0;

        //Scanning polygons
        if ((*MEO->polyScan)(MEO->state, ScanAdiacentPoly, &localData, OPLYR_PRIMARY) && !localData.FoundPol) goto Error;

            else

        //Split found polygon
        if (localData.FoundPol)
            if (SplitPoly(localData.FoundPol)) goto Error;

        Passes++;

    } while (localData.FoundPol);

    _StoreLog_Total(localData.SelPolsCount, Passes); //LOGGING
    sprintf(str1, "Number of polygons splitted: %d", localData.SelPolsCount);
    sprintf(str2, "Number of passes needed: %d",  Passes);
    (*Msg->info)(str1, str2);

    return EDERR_NONE;
Error:
    (*Msg->error)("I cannot terminate polygons subdividing.", NULL);
    return EDERR_USERABORT;
}

/*****************************/
/*** Initialize parameters ***/
/*****************************/

    static EDError
FindFirstPoly(MyData *localData, const PolygonInfo *PI)
{
    PointInfo *P1, *P2, *P3, *P4;

    if  (((PI->flags & (PPDF_SELECT | PPDF_CURVE)) == PPDF_SELECT) && (PI->numPnts == 4)) {
        if (!localData->FirstPol) {
            //Setting first polygon
            localData->FirstPol = PI->pol;

            //Init LastPrev polygon struct
            P1 = (*MEO->pointInfo)(MEO->state, PI->points[0]);
            CopyPoints(P1->position, lastData.Poly1.Points[0]);
            P2 = (*MEO->pointInfo)(MEO->state, PI->points[1]);
            CopyPoints(P2->position, lastData.Poly1.Points[1]);
            P3 = (*MEO->pointInfo)(MEO->state, PI->points[2]);
            CopyPoints(P3->position, lastData.Poly1.Points[2]);
            P4 = (*MEO->pointInfo)(MEO->state, PI->points[3]);
            CopyPoints(P4->position, lastData.Poly1.Points[3]);

            CopyPolyPoints(lastData.Poly1.Points, lastData.Poly2.Points);
        }
        localData->SelPolsCount++;
    }


    _StoreLog_Poly(PI); //LOGGING
    return EDERR_NONE;
}

    static EDError
GetFirstSelPoly()
{
    _StoreLog_String("First polygons scanning"); //LOGGING
    _StoreLog_String("-----------------------"); //LOGGING
    _StoreLog_String("");                            //LOGGING

    //Init first polygon search
    localData.FirstPol = 0;
    localData.SelPolsCount = 0;

    //Scan first polygon
    if ((*MEO->polyScan)(MEO->state, FindFirstPoly, &localData, OPLYR_PRIMARY)) {
        (*Msg->error)("I cannot terminate first polygons scanning.", NULL);
        return EDERR_USERABORT;
    }

    //Check first polygon found
    if (!localData.FirstPol || localData.SelPolsCount>1) {
        (*Msg->error)("Invalid start polygon.", NULL);
        return EDERR_USERABORT;
    }

    return EDERR_NONE;
}

/************************/
/*** Init application ***/
/************************/

    XCALL_(int);
Activate (long version, GlobalFunc *global, MeshEditBegin *local, void *serverData)
{
    MessageFuncs    *OwnMsg;
    LWPanelFuncs    *Form;
    LWPanelID       FormID = 0;
    LWControl       *AxisCtl, *LogCtl, *RevCtl;
    MeshEditOp      *OwnMEO;
    EDError         Err;
    char            *CItems[4];
    int             Dim = 300;

    XCALL_INIT;
    if (version != 1)
        return AFUNC_BADVERSION;

    //Allocate info messages
    OwnMsg = (*global) ("Info Messages", GFUSE_TRANSIENT);
    if (!OwnMsg)
        return AFUNC_BADGLOBAL;
    Msg = OwnMsg;

    //Make window
    Form = (*global)(PANEL_SERVICES_NAME, GFUSE_TRANSIENT);
    if (!Form)
        return AFUNC_BADVERSION;

    FormID = PAN_CREATE(Form, "Plane meta splitter 1.0 by Delirio");
    if (FormID) {
        //Setup window
        (*Form->set)(FormID, PAN_X, &Dim);
        (*Form->set)(FormID, PAN_Y, &Dim);

        CItems[0] = "     Even    ";
        CItems[1] = "     Odd     ";
        CItems[2] = NULL;
        AxisCtl  = HCHOICE_CTL(Form, FormID, "Edges to cut", CItems);
        RevCtl   = BOOL_CTL(Form, FormID, "Reverse polygons normal");
        LogCtl   = BOOL_CTL(Form, FormID, "Create log file (PlaneMetaSplitter.log)");
    }

    if (PAN_POST(Form, FormID)) {
        GET_INT(AxisCtl, localData.LocalAxis);
        GET_INT(RevCtl, localData.RevNormals);
        GET_INT(LogCtl, localData.CreateLogFile);

        CreateLogFile("PlaneMetaSplitter.log"); //LOGGING
        _StoreLog_String("----------------------------------------------------------------"); //LOGGING
        if (localData.LocalAxis) _StoreLog_String("Plane meta splitter by Delirio(November 1996). Search mode: ODD");
        else                     _StoreLog_String("Plane meta splitter by Delirio(November 1996). Search mode: EVEN");
        _StoreLog_String("----------------------------------------------------------------"); //LOGGING
        _StoreLog_String("");
                                                 //LOGGING
        //Init mesh editing
        OwnMEO = (*local)(0, 0, OPSEL_USER);
        if (!OwnMEO) {
            (*OwnMsg->error)("I cannot start mesh editing.", NULL);
            PAN_KILL(Form, FormID);
            DoneLogFile; //LOGGING
            return AFUNC_OK;
        }
        MEO = OwnMEO;

        // Control existence of start plane polygon
        Err = GetFirstSelPoly();
        if (Err) {
            (*OwnMEO->done)(OwnMEO->state, Err, 0);
            PAN_KILL(Form, FormID);
            DoneLogFile; //LOGGING
            return AFUNC_OK;
        }

        //Polygon splitting test
        Err = CreateMetaSplitPlane();
        if (Err) {
            (*OwnMEO->done)(OwnMEO->state, Err, 0);
            PAN_KILL(Form, FormID);
            DoneLogFile; //LOGGING
            return AFUNC_OK;
        }

        //Done mesh editing
        (*OwnMEO->done)(OwnMEO->state, Err, EDSELM_SELECTNEW);

        DoneLogFile; //LOGGING
    }

    PAN_KILL(Form, FormID);

    return AFUNC_OK;
}

char    ServerClass[] = "MeshDataEdit";
char    ServerName[]  = "Plane_meta_splitter";
