/*
 * lwpanel.h -- minimal reconstruction of the 1996 LWPanels API
 *
 * NOTE: this is a RECONSTRUCTION. The original LWPanels header was released
 * separately from the LightWave 3D 5.0 SDK and is not available here, so
 * this file declares only what planespl.c actually uses. Names and shapes
 * follow the real LWPanels API as closely as could be remembered; numeric
 * tag values are placeholders (the plugin never depends on them).
 *
 * The convenience macros below rely on two file-scope variables that the
 * plugin must define, exactly as with the original LWPanels:
 *
 *      static LWPanControlDesc desc;
 *      static DynaValue        ival = { DY_INTEGER };
 */
#ifndef LWPANEL_H
#define LWPANEL_H

#include <plug.h>       /* GlobalFunc */
#include <lwmod.h>      /* DynaValue  */

/* Name used to ask the host for the panel services via GlobalFunc */
#define PANEL_SERVICES_NAME     "LWPanelServices"

/* Opaque handle of a panel window */
typedef void *LWPanelID;

/* Panel tags (used with the panel get/set functions) */
#define PAN_X           1       /* window x position */
#define PAN_Y           2       /* window y position */
#define PAN_W           3       /* window width      */
#define PAN_H           4       /* window height     */
#define PAN_TITLE       5       /* window title      */

/* Control tags (used with the control get/set functions) */
#define CTL_X           101     /* control x position */
#define CTL_Y           102     /* control y position */
#define CTL_W           103     /* control width      */
#define CTL_H           104     /* control height     */
#define CTL_VALUE       105     /* current value      */

/* Flags for the panel open function */
#define PANF_BLOCKING   0x0001  /* wait until the user closes the panel */
#define PANF_CANCEL     0x0002  /* show a Cancel button                 */

/* Control types */
#define LWT_BOOLEAN     1       /* check box                  */
#define LWT_CHOICE      2       /* one-of-many choice control */

/* Choice layout */
#define MX_HCHOICE      0       /* items laid out horizontally */
#define MX_VCHOICE      1       /* items laid out vertically   */

/* A single control living inside a panel */
typedef struct st_LWControl LWControl;
struct st_LWControl {
    void  (*draw)(LWControl *ctl, int flags);
    void  (*get) (LWControl *ctl, int tag, DynaValue *value);
    void  (*set) (LWControl *ctl, int tag, DynaValue *value);
    void   *priv_data;
};

/* Description handed to addControl; only the choice flavour is needed */
typedef struct st_LWPanControlDesc {
    int type;                   /* LWT_xxx */
    struct {
        int    vertical;        /* MX_HCHOICE or MX_VCHOICE         */
        char **items;           /* NULL-terminated list of labels   */
    } choice;
} LWPanControlDesc;

/* Table of panel services returned by GlobalFunc(PANEL_SERVICES_NAME) */
typedef struct st_LWPanelFuncs {
    LWPanelID  (*create)     (char *title, void *funcs);
    void       (*destroy)    (LWPanelID pan);
    int        (*open)       (LWPanelID pan, int flags);
    int        (*handle)     (LWPanelID pan, int flags);
    void       (*draw)       (LWPanelID pan, int flags);
    void       (*close)      (LWPanelID pan);
    void       (*get)        (LWPanelID pan, int tag, void *value);
    void       (*set)        (LWPanelID pan, int tag, void *value);
    LWControl *(*addControl) (LWPanelID pan, char *type,
                              LWPanControlDesc *desc, char *label);
    LWControl *(*nextControl)(LWPanelID pan, LWControl *ctl);
    void        *user_data;
    GlobalFunc  *globalFun;
} LWPanelFuncs;

/* ---- Convenience macros (use the plugin's `desc` and `ival` globals) ---- */

/* Create a panel with the given title */
#define PAN_CREATE(pfunc, title)    (*(pfunc)->create)(title, pfunc)

/* Show the panel and wait; non-zero result means the user pressed Ok */
#define PAN_POST(pfunc, pan)        (*(pfunc)->open)(pan, PANF_BLOCKING | PANF_CANCEL)

/* Destroy the panel */
#define PAN_KILL(pfunc, pan)        (*(pfunc)->destroy)(pan)

/* Add a horizontal choice control */
#define HCHOICE_CTL(pfunc, pan, title, array)                               \
    (desc.type = LWT_CHOICE,                                                \
     desc.choice.vertical = MX_HCHOICE,                                     \
     desc.choice.items = (array),                                           \
     (*(pfunc)->addControl)(pan, "ChoiceControl", &desc, title))

/* Add a check box */
#define BOOL_CTL(pfunc, pan, title)                                         \
    (desc.type = LWT_BOOLEAN,                                               \
     (*(pfunc)->addControl)(pan, "BoolControl", &desc, title))

/* Read an integer control value into variable n */
#define GET_INT(ctl, n)                                                     \
    ((*(ctl)->get)(ctl, CTL_VALUE, &ival), n = ival.intv.value)

#endif /* LWPANEL_H */
