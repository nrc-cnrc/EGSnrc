/*
###############################################################################
#
#  EGSnrc dose visualization tool for DOSXYZnrc
#  Copyright (C) 2015 National Research Council Canada
#
#  This file is part of EGSnrc.
#
#  EGSnrc is free software: you can redistribute it and/or modify it under
#  the terms of the GNU Affero General Public License as published by the
#  Free Software Foundation, either version 3 of the License, or (at your
#  option) any later version.
#
#  EGSnrc is distributed in the hope that it will be useful, but WITHOUT ANY
#  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
#  FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for
#  more details.
#
#  You should have received a copy of the GNU Affero General Public License
#  along with EGSnrc. If not, see <http://www.gnu.org/licenses/>.
#
###############################################################################
#
#  Author:          Iwan Kawrakow, 1998
#
#  Contributors:
#
###############################################################################
*/


/* INCLUDES */

#include <Xm/XmAll.h>
#include <X11/Xcms.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

/* DEFINES */

#define  DrawWindowSizeX 552
#define  DrawWindowSizeY 572
#define  MainWindowSizeX 800
#define  MainWindowSizeY 800
#define  NIsolines       20
#define  MaxGrayShades   128
#define  MinGrayShades    64
#define  NDensityLabels  7
#define  MaxMajorTicks   10
#define  NMinorTicks     5
#define  MajorTicksLength 4
#define  MinorTicksLength 4
#define  MyFontName "6x12"
#define  XStepCW     2
#define  YStepCW     2

#ifdef MY_COLOR
  /* To customize the isoline colors, play around with the arrays below */
static float red[20] =
                      {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                       1.0, 1.0, 0.5, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
static float green[20] =
                      {0.0, 0.0, 0.0, 0.0, 0.0, 0.1, 0.2, 0.3, 0.4, 0.6,
                       0.8, 1.0, 1.0, 0.5, 1.0, 1.0, 0.8, 0.5, 0.25, 0.0};
static float blue[20] =
                      {1.0, 0.75, 0.5, 0.25, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                       0.0, 0.0, 0.0, 0.0, 0.0, 0.5, 1.0, 1.0, 1.0, 1.0};
#define i_gelb   11
#define i_green  14
#else
#define i_gelb   13
#define i_green  16
#endif

#ifdef TRANSFORM_DENSITY

static float h_numbers[13] = {0.0, 24.0, 520.0, 904.0, 1048.0, 1128.0,
                              1528.0, 1976.0, 2488.0, 2824.0, 3224.0,
                               3624.0, 3832.0};
static float rho_array[13] = {0.0, 0.00121, 0.5, 0.95, 1.05, 1.1, 1.334,
                              1.603, 1.85, 2.1, 2.4, 2.7, 2.83};
#endif

static float MaxDensity = 3.072;
static float MinDensity = 0.1;
static float DGpow = 1.5;

static Boolean NoName_status=1;

static int isolines[] = {150,140,130,120,110,105,100,95,90,85,80,
                         75,70,60,50,40,30,20,10,5};

/* To change the set of islonies that are on at start up,
 * modify the following
 */
static Boolean set_isolines[] = {0,0,0,0,0,0,1,0,1,0,1,0,1,1,1,1,1,1,1,0};

static float which_ticks[] = {0.5, 1.0, 2.0, 2.5, 5.0, 10.0, 20.0, 25.0};

/* isoline colors */
static XcmsFloat hue[] = {310.0,320.0,330.0,340.0,350.0,0.0,10.0,15.0,20.0,
                          30.0,40.0,50.0,75.0,85.0,100.0,115.0,140.0,
                          180.0,210.0,245.0};

static Pixel     gray_shades[MaxGrayShades];
static XcmsFloat gray_values[MaxGrayShades];
static int       n_gray_shades;
static XcmsColor isoline_colors[NIsolines];

static float density_labels[] = {0.0, 0.5, 1.0, 1.5, 2.0, 2.5, 3.0};

typedef struct _DrawData {
        GC gc;
        Position drawX;
        Position drawY;
        Dimension drawWidth;
        Dimension drawHeight;
} DrawData;

typedef struct {
        int   i;
        Pixel j;
} Gray;

typedef struct {
  float x;
  float y;
  float z;
} FloatVector;

typedef struct {
  int   x;
  int   y;
  int   z;
} IntVector;

typedef struct {
  FloatVector VoxelSize;
  IntVector   VoxelNumber;
  float       *xplanes;
  float       *yplanes;
  float       *zplanes;
  float       *Data;
  float       *Slice;
} Data3D;

typedef struct {
  float  Dx;
  float  Dy;
  int    max_x;
  int    max_y;
  float  xo;
  float  yo;
  float  x_min;
  float  x_max;
  float  y_min;
  float  y_max;
  float  pixel_per_cm_x;
  float  pixel_per_cm_y;
  Pixel  *gray;
  float  *density;
  float  *dose;
} PlotData;

static DrawData *drawDataLegend = NULL;
static DrawData *drawData = NULL;
static Data3D   *density_data = NULL;
static Data3D   *dose_data = NULL;
static PlotData plot;

static float dose_max, dose_max1;

/* slices */
static int n_slice = 40;
static int present_slice = 0;

/* PROTOTYPES */

typedef unsigned long   DWORD;
void swap_real4(DWORD *b4_var);
float point_density(int x, int y);
float point_dose(int x, int y);

/* density to gray shade conversion function */
static int DensityToGray(XcmsFloat density);

static Widget MainWindow_Create(Widget parent);
static void ConfigureDrawData(Widget w, DrawData *data);
static void DrawAreaNew(Widget parent, XtPointer data, XmDrawingAreaCallbackStruct *cbs);
static void GetDose(Widget parent, XtPointer data, XmDrawingAreaCallbackStruct *cbs);
static void DrawArea(Widget w, int reason);
static void Endshow(Widget parent, XtPointer data, XmPushButtonCallbackStruct *cbs);
static void GetNextSlice(Widget parent, XtPointer data, XmPushButtonCallbackStruct *cbs);
static void GetPreviousSlice(Widget parent, XtPointer data, XmPushButtonCallbackStruct *cbs);
static void SetColorWithName(char *name, Widget wid, GC gc);
static Status SetIsolineColors(Widget w, Colormap cmap, XcmsCCC ccc);
static Status SetGrayShades(Widget w, Colormap cmap, XcmsCCC ccc);
static void DrawGrayScaleLegend(Widget parent, XtPointer data, XmDrawingAreaCallbackStruct *cbs);
static void ChangeSlice(Widget parent, XtPointer data, XmScaleCallbackStruct *cbs);
static void ChangeMaxDensity(Widget parent, XtPointer data, XmScaleCallbackStruct *cbs);
static void ChangeMinDensity(Widget parent, XtPointer data, XmScaleCallbackStruct *cbs);
static void ScaleDose(Widget parent, XtPointer data, XmScaleCallbackStruct *cbs);
static void NextSlice(Widget parent, XtPointer data, XmPushButtonCallbackStruct *cbs);
static void PrevSlice(Widget parent, XtPointer data, XmPushButtonCallbackStruct *cbs);
static void ZoomInDrawArea(Widget parent, XtPointer data, XmPushButtonCallbackStruct *cbs);
static void ZoomOutDrawArea(Widget parent, XtPointer data, XmPushButtonCallbackStruct *cbs);
static void changedToggle(Widget parent, XtPointer data, XmToggleButtonCallbackStruct *cbs);
static void changedColorWash(Widget parent, XtPointer data, XmToggleButtonCallbackStruct *cbs);
static void changedNoName(Widget parent, XtPointer data, XmToggleButtonCallbackStruct *cbs);
static void changedPlane(Widget parent, XtPointer data, XmToggleButtonCallbackStruct *cbs);
static void adjust_scale(int slice);
       void get_data(int argc, char **argv);
       void get_slice();

/* STATIC GLOBAL WIDGETS */

static  Widget drawingArea;
static  Widget GrayScaleLegend;
static  Widget IsolineCheckBox;
static  Widget GoToSlice;
static  Widget IsoToggleButtons[NIsolines];
static  Widget XZPlane,XYPlane,YZPlane;
static  Widget NoName;
static  Widget MainDialog;
static  Widget dial1;
static  Widget ColorWash;

/* FALLBACK RESOURCES */

static String fallback_resources[] = {
"*fontList:    *courier-bold*140*iso8859-1",
/* "dosxyz_show*Quit.fontList: 9x15", */
"*my_label.labelString: The DOSXYZ Dose Visualization Tool       ©Iwan Kawrakow,\
 NRC/CNRC",
 NULL
};

/* FUNCTIONS */

int main(int argc,char **argv)
{
  XtAppContext app_context;
  Widget Toplevel;
  Dimension width,height;
  Arg args[5];
  Cardinal n;

  get_data(argc,argv);

  XtSetLanguageProc(NULL, NULL, NULL);
  Toplevel = XtAppInitialize(&app_context, "Vmc_show", NULL, 0,
             &argc, argv, fallback_resources, NULL, 0);

  MainDialog = MainWindow_Create(Toplevel);

  XtManageChild(MainDialog);
  XtRealizeWidget(Toplevel);
  n = 0;
  XtSetArg (args[n], XmNwidth, &width); n++;
  XtSetArg (args[n], XmNheight, &height); n++;
  XtGetValues (dial1, args, n);
#ifdef DEBUG
  printf(" dial1: width = %d height = %d\n",width,height);
#endif
/*
  n = 0;
  XtSetArg (args[n], XmNwidth, 942); n++;
  XtSetArg (args[n], XmNheight, 852); n++;
  XtUnmanageChild(MainDialog);
  XtSetValues(MainDialog, args, n);
  XtManageChild(MainDialog);
  XtUnrealizeWidget(Toplevel);
  XtRealizeWidget(Toplevel);
*/
#ifdef DEBUG
  printf(" Display: %d %d\n",
   DisplayWidth(XtDisplay(Toplevel),DefaultScreen(XtDisplay(Toplevel))),
   DisplayHeight(XtDisplay(Toplevel),DefaultScreen(XtDisplay(Toplevel))));
#endif
  width += 5;
  if( width > DisplayWidth(XtDisplay(Toplevel),DefaultScreen(XtDisplay(Toplevel))) )
   width = DisplayWidth(XtDisplay(Toplevel),DefaultScreen(XtDisplay(Toplevel)));
  height += 25;
  if( height > DisplayHeight(XtDisplay(Toplevel),DefaultScreen(XtDisplay(Toplevel)))-20 )
   height = DisplayHeight(XtDisplay(Toplevel),DefaultScreen(XtDisplay(Toplevel)))-20;
  XMoveResizeWindow(XtDisplay(Toplevel),XtWindow(Toplevel),10,20,width,height);


  XtAppMainLoop(app_context);
  return 0;
}

static Widget MainWindow_Create(Widget parent)
{
  Arg args[40];
  Cardinal n;
  Dimension width, height;
/*  Widget mainWindow; */
/*  Widget dial1; */
  Widget scrollw;
/*  Widget IsolineCheckBox; */
/*  Widget IsoToggleButtons[NIsolines]; */
  Widget Legends;
  Widget Quit;
  Widget Next;
  Widget Previous;
  Widget ZoomIn;
  Widget ZoomOut;
  Widget PlanesRadioBox;
  Widget SetMaxDensity;
  Widget SetMinDensity;
  Widget MiscStuff;
  Widget Sep1,Sep_left,Sep_right;
/*  Widget DensityScales; */
/*  Widget GoToSlice; */
  Widget ScaleDmax;
  Widget my_label;
  int i;
  static char temp_string[10];
  Status status;
  static Colormap cmap;
  XColor theRGBColor, HwColor;
  int theScreen;
  XmString tmpLabelString = NULL;
  XcmsCCC   ccc;
  int legend_x_position;

  theScreen = DefaultScreen(XtDisplay(parent));
  cmap = DefaultColormap(XtDisplay(parent), theScreen);
  ccc = XcmsCCCOfColormap(XtDisplay(parent),cmap);

  status = XLookupColor(XtDisplay(parent), cmap, "black", &theRGBColor, &HwColor);
  status = XAllocColor( XtDisplay(parent), cmap, &HwColor);

  status = SetIsolineColors(parent,cmap,ccc);
  if(status != XcmsSuccess ) exit(2);
#ifdef DEBUG
  printf(" after SetIsolineColors!\n");
#endif

  status = SetGrayShades(parent,cmap,ccc);
  if(status != XcmsSuccess ) exit(3);

  /* scrollw = XtVaCreateManagedWidget("scrollw", */
  scrollw = XtVaCreateWidget("scrollw",
                              xmScrolledWindowWidgetClass, parent,
                            /*  XmNnavigationType, XmEXCLUSIVE_TAB_GROUP, */
                              XmNscrollingPolicy, XmAUTOMATIC,
                            /*  XmNvisualPolicy, XmVARIABLE, */
                            /*  XmNvisualPolicy, XmCONSTANT, */
                              NULL);

  n = 0;
  dial1 = XmCreateForm(scrollw, "form1", args, n);
  /* dial1 = XmCreateForm(parent, "form1", args, n); */

  n = 0;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNleftWidget, dial1); n++;
  XtSetArg(args[n], XmNleftOffset, 25); n++;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNbottomWidget, dial1); n++;
  XtSetArg(args[n], XmNbottomOffset, 25); n++;
  XtSetArg(args[n], XmNwidth, 126); n++;
  XtSetArg(args[n], XmNheight, 36); n++;
  /* XtSetArg(args[n], XmNfontList, "*courier-bold*180*iso8859-1"); n++; */
#ifdef DEBUG
  printf(" Default font: %s\n",XmFONTLIST_DEFAULT_TAG);
#endif
  tmpLabelString = XmStringCreateLtoR("Quit", XmFONTLIST_DEFAULT_TAG);
  /*tmpLabelString = XmStringCreateLtoR("Quit", "*courier-bold*180*iso8859-1");*/
  XtSetArg(args[n], XmNlabelString, tmpLabelString); n++;
  Quit = XmCreatePushButton(dial1, "Quit", args, n);
  XtManageChild(Quit);

  n = 0;
  XtSetArg(args[n], XmNwidth, 126); n++;
  XtSetArg(args[n], XmNheight, 36); n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
  XtSetArg(args[n], XmNleftWidget, Quit); n++;
  XtSetArg(args[n], XmNleftOffset, 5); n++;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNbottomWidget, dial1); n++;
  XtSetArg(args[n], XmNbottomOffset, 25); n++;
  tmpLabelString = XmStringCreateLtoR("Next", XmFONTLIST_DEFAULT_TAG);
  XtSetArg(args[n], XmNlabelString, tmpLabelString); n++;
  Next = XmCreatePushButton(dial1, "Next", args, n);
  XtManageChild(Next);

  n = 0;
  XtSetArg(args[n], XmNwidth, 126); n++;
  XtSetArg(args[n], XmNheight, 36); n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
  XtSetArg(args[n], XmNleftWidget, Next); n++;
  XtSetArg(args[n], XmNleftOffset, 5); n++;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNbottomWidget, dial1); n++;
  XtSetArg(args[n], XmNbottomOffset, 25); n++;
  tmpLabelString = XmStringCreateLtoR("Previous", XmFONTLIST_DEFAULT_TAG);
  XtSetArg(args[n], XmNlabelString, tmpLabelString); n++;
  Previous = XmCreatePushButton(dial1, "Previous", args, n);
  XtManageChild(Previous);

  n = 0;
  XtSetArg(args[n], XmNwidth, 126); n++;
  XtSetArg(args[n], XmNheight, 36); n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
  XtSetArg(args[n], XmNleftWidget, Previous); n++;
  XtSetArg(args[n], XmNleftOffset, 5); n++;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNbottomWidget, dial1); n++;
  XtSetArg(args[n], XmNbottomOffset, 25); n++;
  tmpLabelString = XmStringCreateLtoR("Zoom In", XmFONTLIST_DEFAULT_TAG);
  XtSetArg(args[n], XmNlabelString, tmpLabelString); n++;
  ZoomIn = XmCreatePushButton(dial1, "ZoomIn", args, n);
  XtManageChild(ZoomIn);

  n = 0;
  XtSetArg(args[n], XmNwidth, 126); n++;
  XtSetArg(args[n], XmNheight, 36); n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
  XtSetArg(args[n], XmNleftWidget, ZoomIn); n++;
  XtSetArg(args[n], XmNleftOffset, 5); n++;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNbottomWidget, dial1); n++;
  XtSetArg(args[n], XmNbottomOffset, 25); n++;
  tmpLabelString = XmStringCreateLtoR("Zoom Out", XmFONTLIST_DEFAULT_TAG);
  XtSetArg(args[n], XmNlabelString, tmpLabelString); n++;
  ZoomOut = XmCreatePushButton(dial1, "ZoomOut", args, n);
  XtManageChild(ZoomOut);

  n = 0;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNrightWidget, dial1); n++;
  XtSetArg(args[n], XmNrightOffset, 10); n++;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNbottomWidget, dial1); n++;
  XtSetArg(args[n], XmNbottomOffset, 25); n++;
/*    sprintf(temp_string,"%3d %%",isolines[i]);   removed Oct 22,98 */
  XtSetArg(args[n], XmNset, 0); n++;
  /*  XtSetArg(args[n], XmNindicatorSize, 20); n++; */
  /*  XtSetArg(args[n], XmNspacing, 1); n++;  */
  ColorWash = XtCreateManagedWidget("Colorwash",
                                     xmToggleButtonWidgetClass,
                                     dial1, args, n);

  n = 0;
/*  XtSetArg(args[n], XmNx,  90); n++; */
/*  XtSetArg(args[n], XmNy, 650); n++; */
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNleftWidget, dial1); n++;
  XtSetArg(args[n], XmNleftOffset, 25); n++;
  XtSetArg(args[n], XmNwidth, 276); n++;
  XtSetArg(args[n], XmNheight,  72); n++;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
  XtSetArg(args[n], XmNbottomWidget, Quit); n++;
  XtSetArg(args[n], XmNbottomOffset, 25); n++;
  tmpLabelString = XmStringCreate("Slice number", XmFONTLIST_DEFAULT_TAG);
  XtSetArg(args[n], XmNtitleString, tmpLabelString); n++;
  XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
  XtSetArg(args[n], XmNdecimalPoints, 0); n++;
  XtSetArg(args[n], XmNminimum, 1); n++;
  XtSetArg(args[n], XmNmaximum, n_slice); n++;
  XtSetArg(args[n], XmNshowValue, TRUE); n++;
  XtSetArg(args[n], XmNvalue, present_slice+1); n++;
  XtSetArg(args[n], XmNscaleMultiple, 1); n++;
  GoToSlice = XmCreateScale(dial1, "SliceNumber", args, n);
  XtManageChild(GoToSlice);

  n = 0;
/*  XtSetArg(args[n], XmNx, 366); n++; */
/*  XtSetArg(args[n], XmNy, 650); n++; */
  XtSetArg(args[n], XmNwidth, 276); n++;
  XtSetArg(args[n], XmNheight,  72); n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
  XtSetArg(args[n], XmNleftWidget, GoToSlice); n++;
  XtSetArg(args[n], XmNleftOffset, 20); n++;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
  XtSetArg(args[n], XmNbottomWidget, Quit); n++;
  XtSetArg(args[n], XmNbottomOffset, 25); n++;
  tmpLabelString = XmStringCreate("Dmax", XmFONTLIST_DEFAULT_TAG);
  XtSetArg(args[n], XmNtitleString, tmpLabelString); n++;
  XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
  XtSetArg(args[n], XmNdecimalPoints, 0); n++;
  XtSetArg(args[n], XmNminimum, 50); n++;
  XtSetArg(args[n], XmNmaximum, 150); n++;
  XtSetArg(args[n], XmNshowValue, TRUE); n++;
  XtSetArg(args[n], XmNvalue, 100); n++;
  XtSetArg(args[n], XmNscaleMultiple, 1); n++;
  ScaleDmax = XmCreateScale(dial1, "Dmax", args, n);
  XtManageChild(ScaleDmax);

  n = 0;
/*
  XtSetArg(args[n], XmNx, 100); n++;
  XtSetArg(args[n], XmNy, 80); n++;
*/
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNleftWidget, dial1); n++;
  XtSetArg(args[n], XmNleftOffset, 25); n++;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
  XtSetArg(args[n], XmNbottomWidget, ScaleDmax); n++;
  XtSetArg(args[n], XmNbottomOffset, 25); n++;
/*
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNtopWidget, dial1); n++;
  XtSetArg(args[n], XmNtopOffset, 25); n++;
*/
  XtSetArg(args[n], XmNwidth, DrawWindowSizeX); n++;
  XtSetArg(args[n], XmNheight, DrawWindowSizeY); n++;
  XtSetArg(args[n], XmNresizePolicy, 0); n++;
  XtSetArg(args[n], XmNbackground, HwColor.pixel); n++;
/*  XtSetArg(args[n], XmNshadowThickness, 3); n++; */
/*  XtSetArg(args[n], XmNshadowType, 0); n++; */
  XtSetArg(args[n], XmNmarginWidth, 30); n++;
  XtSetArg(args[n], XmNmarginHeight,20); n++;
  drawingArea = XmCreateDrawingArea(dial1, "drawingArea", args, n);
  XtManageChild(drawingArea);

  n = 0;
  XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNleftWidget, dial1); n++;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNtopWidget, dial1); n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNrightWidget, dial1); n++;
/*
  tmpLabelString = XmStringCreateLtoR(
    "The DOSXYZ Dose Visualization Tool",
      XmFONTLIST_DEFAULT_TAG);
  XtSetArg(args[n], XmNlabelString, tmpLabelString); n++;
*/
  my_label = XmCreatePushButton(dial1, "my_label", args, n);
  XtManageChild(my_label);

  n = 0;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
  XtSetArg(args[n], XmNleftWidget, drawingArea); n++;
  XtSetArg(args[n], XmNleftOffset, 25); n++;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
  XtSetArg(args[n], XmNbottomWidget, ZoomOut); n++;
  XtSetArg(args[n], XmNbottomOffset, 25); n++;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
  XtSetArg(args[n], XmNtopWidget, my_label); n++;
  XtSetArg(args[n], XmNtopOffset, 25); n++;
  XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
/*  XtSetArg(args[n], XmNpacking, XmPACK_COLUMN); n++; */
/*  XtSetArg(args[n], XmNpacking, XmPACK_TIGHT); n++;  */
/*  XtSetArg(args[n], XmNorientation, XmVERTICAL); n++; */
  Legends = XmCreateRowColumn(dial1, "Legends", args, n);

  n = 0;
  XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
  Sep_left = XmCreateSeparator(Legends,"Sep_left",args,n);
  XtManageChild(Sep_left);

  n = 0;
/*
  XtSetArg(args[n], XmNx, 630); n++;
  XtSetArg(args[n], XmNy, 10); n++;
*/
  XtSetArg(args[n], XmNpacking, XmPACK_COLUMN); n++;
  IsolineCheckBox = XmCreateSimpleCheckBox(Legends, "IsolineCheckBox", args, n);

  for(i = 0; i < NIsolines; i++)
  {
    sprintf(temp_string,"%3d %%",isolines[i]);
    n=0;
    XtSetArg(args[n], XmNset, set_isolines[i]); n++;
    XtSetArg(args[n], XmNindicatorSize, 20); n++;
    XtSetArg(args[n], XmNspacing, 1); n++;
    XtSetArg(args[n], XmNselectColor, isoline_colors[i].pixel); n++;
    IsoToggleButtons[i] = XtCreateManagedWidget(temp_string,
                                                xmToggleButtonWidgetClass,
                                                IsolineCheckBox,
                                                args, n);
  }
  XtManageChild(IsolineCheckBox);

  n = 0;
  XtSetArg(args[n], XmNwidth, 100); n++;
  GrayScaleLegend = XmCreateDrawingArea(Legends, "GrayScaleLegend", args, n);
  XtManageChild(GrayScaleLegend);

  n = 0;
  XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
  Sep1 = XmCreateSeparator(Legends,"Sep1",args,n);
  XtManageChild(Sep1);

  n = 0;
  XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
/*
  XtSetArg(args[n], XmNpacking, XmPACK_COLUMN); n++;
*/
  MiscStuff = XtCreateWidget("DensityScales",xmRowColumnWidgetClass,
                              Legends, args, n);

  n = 0;
  XtSetArg(args[n], XmNpacking, XmPACK_COLUMN); n++;
  PlanesRadioBox = XmCreateSimpleRadioBox(MiscStuff, "PlanesRadioBox", args, n);
  n=0;
  XtSetArg(args[n], XmNset, 1); n++;
  /* XtSetArg(args[n], XmNindicatorSize, 15); n++; */
  XZPlane = XtCreateManagedWidget("XZ-Plane",xmToggleButtonWidgetClass,
                                  PlanesRadioBox, args, n);
  n=0;
  XtSetArg(args[n], XmNset, 0); n++;
  /* XtSetArg(args[n], XmNindicatorSize, 15); n++; */
  XYPlane = XtCreateManagedWidget("XY-Plane",xmToggleButtonWidgetClass,
                                  PlanesRadioBox, args, n);
  n=0;
  XtSetArg(args[n], XmNset, 0); n++;
  /* XtSetArg(args[n], XmNindicatorSize, 15); n++; */
  YZPlane = XtCreateManagedWidget("YZ-Plane",xmToggleButtonWidgetClass,
                                  PlanesRadioBox, args, n);
  XtManageChild(PlanesRadioBox);

  n = 0;
  XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
  Sep1 = XmCreateSeparator(MiscStuff,"Sep1",args,n);
  XtManageChild(Sep1);

  n=0;
  XtSetArg(args[n], XmNset, NoName_status); n++;
  NoName = XtCreateManagedWidget("Expand",xmToggleButtonWidgetClass,
                                  MiscStuff, args, n);

  n = 0;
  XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
  Sep1 = XmCreateSeparator(MiscStuff,"Sep1",args,n);
  XtManageChild(Sep1);

  n = 0;
  XtSetArg(args[n], XmNwidth,  99); n++;
  XtSetArg(args[n], XmNheight, 256); n++;
/*  tmpLabelString = XmStringCreate("RHOmin", XmFONTLIST_DEFAULT_TAG); */
  tmpLabelString = XmStringCreateLtoR(
    "m\ni\nn\ni\nm\nu\nm\n\nd\ne\nn\ns\ni\nt\ny", XmFONTLIST_DEFAULT_TAG);
  XtSetArg(args[n], XmNtitleString, tmpLabelString); n++;
  XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
  XtSetArg(args[n], XmNdecimalPoints, 3); n++;
  XtSetArg(args[n], XmNminimum, 0); n++;
  XtSetArg(args[n], XmNmaximum, 1000); n++;
  XtSetArg(args[n], XmNshowValue, TRUE); n++;
/*  XtSetArg(args[n], XmNshowValue, FALSE); n++; */
  XtSetArg(args[n], XmNvalue, ((int) (1000.*MinDensity))); n++;
  XtSetArg(args[n], XmNscaleMultiple, 1); n++;
  SetMinDensity = XmCreateScale(MiscStuff, "Rho_min", args, n);
  XtManageChild(SetMinDensity);

  n = 0;
  XtSetArg(args[n], XmNwidth,  99); n++;
  XtSetArg(args[n], XmNheight, 256); n++;
  XtSetArg(args[n], XmNstringDirection, XmVERTICAL); n++;
/*  tmpLabelString = XmStringCreate("RHOmax", XmFONTLIST_DEFAULT_TAG);  */
  tmpLabelString = XmStringCreateLtoR(
    "m\na\nx\ni\nm\nu\nm\n\nd\ne\nn\ns\ni\nt\ny", XmFONTLIST_DEFAULT_TAG);
  XtSetArg(args[n], XmNtitleString, tmpLabelString); n++;
  XtSetArg(args[n], XmNdecimalPoints, 3); n++;
  XtSetArg(args[n], XmNminimum, 1500); n++;
  XtSetArg(args[n], XmNmaximum, (int) (1000.0*MaxDensity)); n++;
  XtSetArg(args[n], XmNshowValue, TRUE); n++;
/*  XtSetArg(args[n], XmNshowValue, FALSE); n++; */
  XtSetArg(args[n], XmNvalue, (int) (1000.0*MaxDensity)); n++;
  XtSetArg(args[n], XmNscaleMultiple, 1); n++;
  XtSetArg(args[n], XmNprocessingDirection, XmMAX_ON_BOTTOM); n++;
  SetMaxDensity = XmCreateScale(MiscStuff, "Rho_max", args, n);
  XtManageChild(SetMaxDensity);

  XtManageChild(MiscStuff);

  n = 0;
  XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
  Sep_right = XmCreateSeparator(Legends,"Sep_right",args,n);
  XtManageChild(Sep_right);

  XtManageChild(Legends);

  n = 0;
  XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
  XtSetArg(args[n], XmNleftOffset, 25); n++;
  XtSetArg(args[n], XmNleftWidget, drawingArea); n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNrightWidget, dial1); n++;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
  XtSetArg(args[n], XmNtopWidget, Legends); n++;
  Sep1 = XmCreateSeparator(dial1,"Sep1",args,n);
  XtManageChild(Sep1);

  n = 0;
  XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
  XtSetArg(args[n], XmNleftOffset, 25); n++;
  XtSetArg(args[n], XmNleftWidget, drawingArea); n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNrightWidget, dial1); n++;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
  XtSetArg(args[n], XmNbottomWidget, Legends); n++;
  Sep1 = XmCreateSeparator(dial1,"Sep1",args,n);
  XtManageChild(Sep1);

/*
  n = 0;
  XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNleftWidget, dial1); n++;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNtopWidget, dial1); n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNrightWidget, dial1); n++;
  tmpLabelString = XmStringCreateLtoR(
    "The DOSXYZ Dose Visualization Tool",
      XmFONTLIST_DEFAULT_TAG);
  XtSetArg(args[n], XmNlabelString, tmpLabelString); n++;
  my_label = XmCreatePushButton(dial1, "my_label", args, n);
  XtManageChild(my_label);
*/


/* Callbacks */
  XtAddCallback(Quit, XmNactivateCallback,
       (XtCallbackProc) Endshow, (XtPointer)NULL);
  XtAddCallback(GrayScaleLegend, XmNexposeCallback,
       (XtCallbackProc) DrawGrayScaleLegend, (XtPointer)NULL);
  XtAddCallback(GoToSlice, XmNvalueChangedCallback,
       (XtCallbackProc) ChangeSlice, (XtPointer)NULL);
  XtAddCallback(ScaleDmax, XmNvalueChangedCallback,
       (XtCallbackProc) ScaleDose, (XtPointer)NULL);
  XtAddCallback(SetMaxDensity, XmNvalueChangedCallback,
       (XtCallbackProc) ChangeMaxDensity, (XtPointer)NULL);
  XtAddCallback(SetMaxDensity, XmNdragCallback,
       (XtCallbackProc) ChangeMaxDensity, (XtPointer)NULL);
  XtAddCallback(SetMinDensity, XmNvalueChangedCallback,
       (XtCallbackProc) ChangeMinDensity, (XtPointer)NULL);
  XtAddCallback(SetMinDensity, XmNdragCallback,
       (XtCallbackProc) ChangeMinDensity, (XtPointer)NULL);
  XtAddCallback(Next, XmNactivateCallback,
       (XtCallbackProc) NextSlice, (XtPointer)NULL);
  XtAddCallback(Previous, XmNactivateCallback,
       (XtCallbackProc) PrevSlice, (XtPointer)NULL);
  XtAddCallback(drawingArea, XmNexposeCallback,
       (XtCallbackProc) DrawAreaNew, (XtPointer)NULL);
  XtAddCallback(drawingArea, XmNinputCallback,
       (XtCallbackProc) GetDose, (XtPointer)NULL);
  XtAddCallback(ZoomIn, XmNactivateCallback,
       (XtCallbackProc) ZoomInDrawArea, (XtPointer)NULL);
  XtAddCallback(ZoomOut, XmNactivateCallback,
       (XtCallbackProc) ZoomOutDrawArea, (XtPointer)NULL);
  XtAddCallback(ColorWash, XmNvalueChangedCallback,
       (XtCallbackProc) changedColorWash, (XtPointer)NULL);

  for(i = 0; i < NIsolines; i++)
  {
    XtAddCallback(IsoToggleButtons[i], XmNvalueChangedCallback,
       (XtCallbackProc) changedToggle, (XtPointer)NULL);
  }
  XtAddCallback(XZPlane, XmNvalueChangedCallback,
       (XtCallbackProc) changedPlane, (XtPointer)NULL);
  XtAddCallback(XYPlane, XmNvalueChangedCallback,
       (XtCallbackProc) changedPlane, (XtPointer)NULL);
  XtAddCallback(YZPlane, XmNvalueChangedCallback,
       (XtCallbackProc) changedPlane, (XtPointer)NULL);
  XtAddCallback(NoName, XmNvalueChangedCallback,
       (XtCallbackProc) changedNoName, (XtPointer)NULL);

  /* return dial1; */
  XtManageChild(dial1);
  /* XtRealizeWidget(dial1); */
  n = 0;
  XtSetArg (args[n], XmNwidth, &width); n++;
  XtSetArg (args[n], XmNheight, &height); n++;
  XtGetValues (dial1, args, n);
#ifdef DEBUG
  printf(" dial1: width = %d height = %d\n",width,height);
#endif
  /* XtUnrealizeWidget(dial1); */
  /* XtManageChild(scrollw); */
  XtGetValues (dial1, args, n);
#ifdef DEBUG
  printf(" dial1: width = %d height = %d\n",width,height);
#endif

  return scrollw;
}

#ifdef MY_COLOR
Status SetIsolineColors(Widget w, Colormap cmap, XcmsCCC ccc)
{
  Status status;
  int i;
  XColor theRGBColor, HwColor;

  for(i=0; i<NIsolines; i++)
  {
    isoline_colors[i].spec.RGBi.red   = (XcmsFloat) red[i];
    isoline_colors[i].spec.RGBi.green = (XcmsFloat) green[i];
    isoline_colors[i].spec.RGBi.blue  = (XcmsFloat) blue[i];
    isoline_colors[i].format          = XcmsRGBiFormat;
    if(XcmsAllocColor(XtDisplay(w),cmap,&isoline_colors[i],XcmsRGBFormat) ==
       XcmsFailure)
    {
      printf(" Can not allocate desired Isoline colors! \n");
      printf(" failed at i = %d RGB = %g %g %g\n",i,red[i],green[i],blue[i]);
      return XcmsFailure;
    }
  }
  return XcmsSuccess;
}

#else
Status SetIsolineColors(Widget w, Colormap cmap, XcmsCCC ccc)
{
  Status status;
  int i;

  for(i=0; i<NIsolines; i++)
  {
    status = XcmsTekHVCQueryMaxVC(ccc, hue[i], &isoline_colors[i]);
    if( status != XcmsSuccess)
    {
      printf(" SetIsolineColors: screen can not display desired color!\n");
      return status;
    }
    if(XcmsAllocColor(XtDisplay(w),cmap,&isoline_colors[i],XcmsRGBFormat) ==
       XcmsFailure)
    {
      printf(" Can not allocate desired Isoline colors! \n");
      return XcmsFailure;
    }
  }
  return status;
}
#endif

Status SetGrayShades(Widget w, Colormap cmap, XcmsCCC ccc)
{
  XcmsColor color;
  int i,depth;
  long n_collor_cells;
  XcmsFloat minv,maxv,deltav;
  XcmsFloat hue = 0.0, chroma = 0.0;
  Status status;

/* Query minimum and maximum value */
  if( XcmsTekHVCQueryMinV(ccc,hue,chroma,&color) == XcmsFailure)
  {
    printf(" Failed at XcmsTekHVCQueryMinV! \n");
    return XcmsFailure;
  }
  else minv = color.spec.TekHVC.V;
  if( XcmsTekHVCQueryMaxV(ccc,hue,chroma,&color) == XcmsFailure)
  {
    printf(" Failed at XcmsTekHVCQueryMaxV! \n");
    return XcmsFailure;
  }
  else maxv = color.spec.TekHVC.V;

#ifdef DEBUG
  printf(" Entries in the default colormap: %d\n",
    DisplayCells(XtDisplay(w), DefaultScreen(XtDisplay(w))));
#endif
  depth = DefaultDepth(XtDisplay(w), DefaultScreen(XtDisplay(w)));
  if( depth < 8 )
  {
    printf(" Screen depth of 8 or higher required!\n Present depth: %d\n",
             depth);
    return XcmsFailure;
  }
  n_collor_cells = 1 << depth;
  n_gray_shades = (int) (n_collor_cells - isoline_colors[NIsolines-1].pixel
                           - 5);
  if( n_gray_shades > MaxGrayShades) n_gray_shades = MaxGrayShades;
  if( n_gray_shades < MinGrayShades)
  {
    printf(" Not enough color cells to allocate %d gray shades!\n",MinGrayShades);
    return XcmsFailure;
  }
   printf(" gray shades: %d\n",n_gray_shades);
  deltav = (maxv - minv)/(n_gray_shades-1);
  for (i=0; i < n_gray_shades-1; i++)
  {
    color.format = XcmsTekHVCFormat;
    color.spec.TekHVC.H = hue;
    color.spec.TekHVC.C = chroma;
    color.spec.TekHVC.V = minv + i*deltav;
    gray_values[i] = color.spec.TekHVC.V;
    status = XcmsAllocColor(XtDisplay(w),cmap,&color,XcmsRGBFormat);
    gray_shades[i] = color.pixel;
  }
  gray_shades[n_gray_shades-1] = gray_shades[n_gray_shades-2];
  if(status == XcmsSuccess)
  {
    printf(" %d gray shades allocated!\n",n_gray_shades);
    return status;
  }
  else
  {
    /*return XcmsFailure;*/
    return XcmsSuccess;
  }
}

static void  Endshow(Widget parent,  XtPointer data,
                     XmPushButtonCallbackStruct *cbs)
{
  exit(0);
}

static void DrawGrayScaleLegend(Widget parent, XtPointer data,
                                XmDrawingAreaCallbackStruct *cbs)
{
  static int first_draw = 1;
  static float xxx;
  int i,ii,y_r,y_r_new;
  int width;
  static char string[10];
  XcmsFloat density;
  float dens;
  XGCValues values;
  long value_mask;

  if (first_draw)
  {
    GC gc=XCreateGC(XtDisplay(parent), cbs->window, 0, NULL);
    if (drawDataLegend == NULL)
    {
      drawDataLegend = (DrawData *)XtMalloc (sizeof(DrawData));
      drawDataLegend->gc = gc;
      value_mask = GCForeground | GCBackground;
      XGetGCValues(XtDisplay(parent), gc, value_mask, &values);
#ifdef DEBUG
      printf(" Foreground: %d Background: %d\n",values.foreground,values.background);
#endif
      ConfigureDrawData (parent, drawDataLegend);
    }
    xxx = (float) drawDataLegend->drawHeight / (float) (n_gray_shades-1);
    first_draw = 0;
  }
/*
  XtUnmanageChild(parent);
  XtManageChild(parent);
*/
  width = drawDataLegend->drawWidth/2;
  y_r = drawDataLegend->drawY + drawDataLegend->drawHeight;
  for (i=0; i<n_gray_shades-1; i++)
  {
    y_r_new = y_r;
    y_r = drawDataLegend->drawY + drawDataLegend->drawHeight
           - (int) ( xxx * (float) (i+1) );
    XSetForeground(XtDisplay(parent), drawDataLegend->gc, gray_shades[i]);
    XFillRectangle(XtDisplay(parent), cbs->window, drawDataLegend->gc,
                   drawDataLegend->drawX, y_r,
                   width, y_r_new - y_r);
  }
/*
  XSetForeground(XtDisplay(parent), drawDataLegend->gc,
                 isoline_colors[i_green].pixel);
  XSetForeground(XtDisplay(parent), drawDataLegend->gc,
                 gray_shades[n_gray_shades-2]);
  XSetFunction(XtDisplay(parent), drawDataLegend->gc, GXinvert);
*/
  SetColorWithName("black", parent, drawDataLegend->gc);
  for (i=0; i<NDensityLabels; i++)
  {
    dens = density_labels[i];
    sprintf(string,"%.1f",dens);
    density = (XcmsFloat) dens;
    ii = DensityToGray(density);
    y_r = drawDataLegend->drawY + drawDataLegend->drawHeight -
          (int) ( xxx * (float) ii);
    if(y_r > drawDataLegend->drawY + 10 &
       y_r < drawDataLegend->drawY + drawDataLegend->drawHeight - 10)
    {
/*
    XSetForeground(XtDisplay(parent), drawDataLegend->gc,
                 gray_shades[n_gray_shades - ii]);
*/
    ii = drawDataLegend->drawX + width/2 - 4*strlen(string);
    ii = drawDataLegend->drawX + width + 3;
/*
    XDrawString (XtDisplay(parent), cbs->window, drawDataLegend->gc,
                 drawDataLegend->drawX + width + 3, y_r,
                 string, strlen(string));
*/
    XDrawString (XtDisplay(parent), cbs->window, drawDataLegend->gc,
                 ii, y_r, string, strlen(string));
    }
  }
  XSetFunction(XtDisplay(parent), drawDataLegend->gc, GXcopy);
}

static void ConfigureDrawData(Widget w, DrawData *data)
{
    Arg args[6];
    Dimension width, height, st, ht, mw, mh;
    Dimension totalMarginWidth;
    Dimension totalMarginHeight;

    width = height = st = ht = mw = mh = 0;
    XtSetArg (args[0], XmNwidth, &width);
    XtSetArg (args[1], XmNheight, &height);
    XtSetArg (args[2], XmNshadowThickness, &st);
    XtSetArg (args[3], XmNhighlightThickness, &ht);
    XtSetArg (args[4], XmNmarginWidth, &mw);
    XtSetArg (args[5], XmNmarginHeight, &mh);
    XtGetValues (w, args, 6);

    totalMarginWidth = st + ht + mw;
    totalMarginHeight = st + ht + mh;

    if (2 * totalMarginWidth < width && 2 * totalMarginHeight < height) {
        data->drawX = totalMarginWidth;
        data->drawY = totalMarginHeight;
        data->drawWidth = width - 2 * totalMarginWidth;
        data->drawHeight = height - 2 * totalMarginHeight;
    }
    else {
        data->drawWidth = 0;
        data->drawHeight = 0;
    }
}

#ifdef DG_POWER

static int DensityToGray(XcmsFloat density)
{
  XcmsFloat temp,temp1;
  int i;

  if (density < MinDensity) i = 0;
  else if(density >= MaxDensity) i = n_gray_shades-1;
  else
  {
    temp=(XcmsFloat) (pow(MaxDensity,DGpow) - pow(MinDensity, DGpow))/
            ((XcmsFloat) (n_gray_shades-1));
    temp1 = (pow(density,DGpow) - pow(MinDensity, DGpow))/temp;
    i = (int) temp1;
  }
  return i;
}

#else

#ifdef DG_SQRT

static int DensityToGray(XcmsFloat density)
{
  XcmsFloat temp,temp1;
  int i;

  if (density < MinDensity) i = 0;
  else if(density >= MaxDensity) i = n_gray_shades-1;
  else
  {
    temp=(XcmsFloat) (sqrt(MaxDensity) - sqrt(MinDensity))/
            ((XcmsFloat) (n_gray_shades-1));
    temp1 = (sqrt(density) - sqrt(MinDensity))/temp;
    i = (int) temp1;
  }
  return i;
}

#else

static int DensityToGray(XcmsFloat density)
{
  XcmsFloat temp,temp1;
  int i;

  if (density < MinDensity) i = 0;
  else if(density >= MaxDensity) i = n_gray_shades-1;
  else
  {
    temp=(XcmsFloat) (MaxDensity - MinDensity)/((XcmsFloat) (n_gray_shades-1));
    temp1 = (density - MinDensity)/temp;
    i = (int) temp1;
  }
  return i;
}

#endif

#endif

static void SetColorWithName(char *name, Widget wid, GC gc)
{
  XColor theRGBColor, HwColor;
  int status;
  Colormap cmap;
  int theScreen;

  theScreen = DefaultScreen(XtDisplay(wid));
  cmap = DefaultColormap(XtDisplay(wid), theScreen);

  status = XLookupColor(XtDisplay(wid), cmap, name, &theRGBColor, &HwColor);
  if (status)
  {
        status = XAllocColor( XtDisplay(wid), cmap, &HwColor);
        if (status)
        {
                XSetForeground(XtDisplay(wid), gc, HwColor.pixel);
                XFlush(XtDisplay(wid));
        }
  }
}

static void ChangeSlice(Widget parent, XtPointer data,
                        XmScaleCallbackStruct *cbs)
{
  present_slice = cbs->value - 1;
#ifdef DEBUG
  printf(" Present slice: %d\n",present_slice);
#endif
  get_slice();
  DrawArea(drawingArea,1);
}

static void ScaleDose(Widget parent, XtPointer data,
                      XmScaleCallbackStruct *cbs)
{
  float scale,dmax,dens;
  int i,j,k,address,i_max,j_max,k_max,ij_max;

  dmax = cbs->value;
#ifdef DEBUG
  printf(" New Dmax = %g\n",dmax);
#endif
  scale = dmax/dose_max1; dose_max1 = dmax;
  i_max = density_data->VoxelNumber.x;
  j_max = density_data->VoxelNumber.y;
  k_max = density_data->VoxelNumber.z;
  ij_max = i_max * j_max;

  for(k=0; k<k_max; ++k)
  {
    for(j=0; j<j_max; ++j)
    {
      for(i=0; i<i_max; ++i)
      {
        address = i + j * i_max + k * ij_max;
        dens = *(dose_data->Data + address) * scale;
        *(dose_data->Data + address) = dens;
      }
    }
  }
  get_slice();

  DrawArea(drawingArea,1);

}

static void NextSlice(Widget parent, XtPointer data,
                      XmPushButtonCallbackStruct *cbs)
{

  present_slice++;
  if( present_slice > n_slice-1) present_slice = 0;
  adjust_scale(present_slice);
  get_slice();
  DrawArea(drawingArea,1);
}

static void PrevSlice(Widget parent, XtPointer data,
                      XmPushButtonCallbackStruct *cbs)
{

  present_slice--;
  if( present_slice < 0) present_slice = n_slice - 1;
  adjust_scale(present_slice);
  get_slice();
  DrawArea(drawingArea,1);
}

static void adjust_scale(int slice)
{
  Arg args[1];

  XtSetArg(args[0], XmNvalue, slice+1);
  XtSetValues(GoToSlice,args,1);
}

void get_data(int argc, char **argv)
{
  char *progname;
  char *dm_name, *dose_name;
  char *home="";
  char  dose_dir[80]="";
  char  dm_file[80]="";
  char  dose_file[80]="";
  char  material[24]="";
  char  buf[1024];
  int   ind1,ind2;
  FILE  *fp;
  int   read_error,ijk_max,dm_size;
  char  line[4]="";
  float dens, dens_new, dens_min = 1000.0, dens_max = -1.0;
  int   i,j,k,i_max,j_max,k_max,ij_max,address,ii,len;
#ifdef SWAP
  int swap_bytes = 1;
#else
  int swap_bytes = 0;
#endif
  float scale,aux,ct_number;
  int   n_material;

  progname = argv[0];
  if( argc < 2)
  {
    printf(" Usage: %s density_file [dose_file]\n",progname);
    exit(1);
  }
  dm_name   = argv[1];
  strcpy(dm_file,dm_name);
  if( strstr(dm_file,".egs4phant") == NULL &&
      strstr(dm_file,".egsphant") == NULL ) {
       strcat(dm_file,".egsphant");
       printf(" Density matrix: %s\n",dm_file);
       fp = fopen(dm_file,"r");
       if (fp == NULL) {
          printf(" Can't open density matrix file %s\n",dm_file);
          strcpy(dm_file,dm_name);
          strcat(dm_file,".egs4phant");
          printf(" Will try opening %s\n",dm_file);
          fp = fopen(dm_file,"r");
       }
  }
  else {
    printf(" Density matrix: %s\n",dm_file);
    fp = fopen(dm_file,"r");
  }
  if( fp == NULL )
  {
    printf(" Error opening density matrix file %s\n",dm_file);
    exit(1);
  }
  if(argc >= 3) {
    dose_name = argv[2];
    strcpy(dose_file,dose_name);
    len = strlen(dose_file);
    if( len > 6 ) {
      if( strcmp(&dose_file[len-7],".3ddose") != 0 )
        strcat(dose_file,".3ddose");
    }
    else strcat(dose_file,".3ddose");
    home = getenv("HOME");
    strcpy(dose_dir,home);
    ind1 = strlen(home);
    if( strcmp(&dose_dir[ind1-1],"/") ){strcpy(&dose_dir[ind1],"/"); ++ind1;}
    strcpy(&dose_dir[ind1],"egs4/dosxyz/"); ind1 = strlen(dose_dir);
    strcpy(&dose_dir[ind1],dose_file); ind1 = strlen(dose_dir);
/*    strcpy(&dose_dir[ind1],".3ddose"); ind1 = strlen(dose_dir); */
    /* Now dose_dir should contain the dose file */
    /* printf("    Dose matrix: %s\n",dose_dir); */
  }
  else printf(" No dose matrix specified!\n");

  /* allocate density matrix data */
  density_data = malloc( sizeof(Data3D) );
  if( density_data == NULL)
  {
    printf(" Can not allocate memory for density matrix data!\n");
    exit(1);
  }
  /* read density data */
  fscanf(fp,"%d\n",&n_material);
  printf("\n Number of materials: %d\n",n_material);
  for(i=0; i<n_material; i++) {
    fscanf(fp,"%s\n",material);
    printf("    Material %d is %s\n",i+1,material);
  }
  for(i=0; i<n_material; i++) fscanf(fp,"%f",&aux);
  fscanf(fp,"\n");
  fscanf(fp,"%d %d %d\n",&density_data->VoxelNumber.x,
    &density_data->VoxelNumber.y,&density_data->VoxelNumber.z);
  printf("\n Phantom dimensions: x = %d y = %d z = %d\n",
   density_data->VoxelNumber.x,
   density_data->VoxelNumber.y,density_data->VoxelNumber.z);
  density_data->xplanes = (float*)
     malloc( (density_data->VoxelNumber.x+1) * sizeof(float) );
  density_data->yplanes = (float*)
     malloc( (density_data->VoxelNumber.y+1) * sizeof(float) );
  density_data->zplanes = (float*)
     malloc( (density_data->VoxelNumber.z+1) * sizeof(float) );
  for(i=0; i<density_data->VoxelNumber.x+1; i++)
    fscanf(fp,"%f",&density_data->xplanes[i]);
  fscanf(fp,"\n");
  for(i=0; i<density_data->VoxelNumber.y+1; i++)
    fscanf(fp,"%f",&density_data->yplanes[i]);
  fscanf(fp,"\n");
  for(i=0; i<density_data->VoxelNumber.z+1; i++)
    fscanf(fp,"%f",&density_data->zplanes[i]);
  fscanf(fp,"\n");
#ifdef DEBUG
  printf(" x-planes: ");
  for(i=0; i<density_data->VoxelNumber.x+1; i++)
    printf("%g ",density_data->xplanes[i]); printf("\n\n");
  printf(" y-planes: ");
  for(i=0; i<density_data->VoxelNumber.y+1; i++)
    printf("%g ",density_data->yplanes[i]); printf("\n\n");
  printf(" z-planes: ");
  for(i=0; i<density_data->VoxelNumber.z+1; i++)
    printf("%g ",density_data->zplanes[i]); printf("\n\n");
#endif
  /* skip material information */
  for(k=0; k<density_data->VoxelNumber.z; k++) {
    for(i=0; i<density_data->VoxelNumber.y; i++) fscanf(fp,"%s\n",buf);
    fscanf(fp,"\n");
  }
  i_max = density_data->VoxelNumber.x;
  j_max = density_data->VoxelNumber.y;
  k_max = density_data->VoxelNumber.z;
  ij_max = i_max * j_max;
  ijk_max = ij_max * k_max;
  dm_size = ijk_max * sizeof( float );
  density_data->Data = (float*) malloc(dm_size);
  if( density_data->Data == NULL)
  {
    printf(" Can not allocate %d bytes for density matrix!\n",dm_size);
    exit(2);
  }
  printf(" %d bytes allocated for density matrix\n",dm_size);
  for(k=0; k<k_max; ++k)
  {
    for(j=0; j<j_max; ++j)
    {
      for(i=0; i<i_max; ++i)
      {
        address = i + j * i_max + k * ij_max;
        fscanf(fp,"%f",&aux);
#ifdef TRANSFORM_DENSITY
        /* transform the density */
        /* first go back to CT numbers */
        if( aux < 0.083 )    { ct_number = (aux - 0.001)/0.082*50.; }
        else if( aux < 0.5 ) { ct_number = (aux - 0.083)/0.417*250. + 50.; }
        else if( aux < 1.5 ) { ct_number = (aux - 0.5)/1.*825. + 300.; }
        else                 { ct_number = (aux - 1.5)/0.9*1475. + 1125.; }
        if(ct_number <= 0.0) { aux = 0.0; }
        else if(ct_number >= h_numbers[12]) { aux = rho_array[12]; }
        else
        {
          for(ii=0; ii<12; ii++) {
            if( (ct_number > h_numbers[ii]) && (ct_number <= h_numbers[ii+1]) )
            {
              aux = rho_array[ii] + (ct_number - h_numbers[ii])/
              (h_numbers[ii+1] - h_numbers[ii])*(rho_array[ii+1]-rho_array[ii]);
              break;
            }
          }
        }
#endif
        *(density_data->Data + address) = aux;
        if( aux > dens_max) dens_max = aux;
        if( aux < dens_min) dens_min = aux;
      }
      fscanf(fp,"\n");
    }
    fscanf(fp,"\n");
  }
  density_data->VoxelSize.x =
     density_data->xplanes[1] - density_data->xplanes[0];
  density_data->VoxelSize.y =
     density_data->yplanes[1] - density_data->yplanes[0];
  density_data->VoxelSize.z =
     density_data->zplanes[1] - density_data->zplanes[0];
  printf("\n Dx Dy Dz: %g %g %g\n",density_data->VoxelSize.x,
    density_data->VoxelSize.y,density_data->VoxelSize.z);
  fclose(fp);
  printf("\n Density matrix reading completed succesful\n");
  printf("\n Maximum density: %g\n",dens_max);
  printf(" Minimum density: %g\n\n",dens_min);

  n_slice = density_data->VoxelNumber.y;

  dose_data = malloc( sizeof(Data3D) );
  if( dose_data == NULL)
  {
    printf(" Can not allocate memory for dose matrix data!\n");
    exit(1);
  }
  dose_data->Data = malloc(dm_size);
  if( dose_data->Data == NULL)
  {
    printf(" Can not allocate %d bytes for dose matrix!\n",dm_size);
    exit(2);
  }
/*  printf(" %d bytes allocated for dose matrix\n",dm_size); */

  if( argc >= 3) {
    fp = fopen(dose_file,"r");
    if( fp == NULL ) {
      printf(" %s does not exist,\n will try %s\n",dose_file,dose_dir);
      fp = fopen(dose_dir,"r");
      if( fp == NULL )
      {
        printf(" Dose matrix file %s or %s not found\n",dose_dir,dose_file);
        printf(" Starting without dose file!\n");
        return;
      }
      printf(" Using dose distribution from %s\n",dose_dir);
    }
    else printf(" Using dose distribution from %s\n",dose_file);
    fscanf(fp,"%d %d %d\n",&dose_data->VoxelNumber.x,
     &dose_data->VoxelNumber.y,&dose_data->VoxelNumber.z);
    if( (dose_data->VoxelNumber.x != density_data->VoxelNumber.x) ||
        (dose_data->VoxelNumber.y != density_data->VoxelNumber.y) ||
        (dose_data->VoxelNumber.z != density_data->VoxelNumber.z) )
    {
      printf(" Inconsistent dose and density data files!\n");
      exit(6);
    }
    if( dose_data->Data == NULL)
    {
      printf(" Can not allocate %d bytes for dose matrix!\n",dm_size);
      exit(2);
    }
    dose_data->xplanes = (float*) malloc( (i_max+1) * sizeof(float) );
    dose_data->yplanes = (float*) malloc( (j_max+1) * sizeof(float) );
    dose_data->zplanes = (float*) malloc( (k_max+1) * sizeof(float) );
    for(i=0; i<i_max+1; i++) fscanf(fp,"%f",&dose_data->xplanes[i]);
    fscanf(fp,"\n");
    for(i=0; i<j_max+1; i++) fscanf(fp,"%f",&dose_data->yplanes[i]);
    fscanf(fp,"\n");
    for(i=0; i<k_max+1; i++) fscanf(fp,"%f",&dose_data->zplanes[i]);
    fscanf(fp,"\n");

    for(k=0; k<k_max; ++k)
    {
      for(j=0; j<j_max; ++j)
      {
        for(i=0; i<i_max; ++i)
        {
          address = i + j * i_max + k * ij_max;
          fscanf(fp,"%f",&aux);
          *(dose_data->Data + address) = aux;
          if( aux > dose_max) dose_max = aux;
        }
      }
    }
    printf("\n Dose maximum found: %g\n",dose_max);

    dose_max1 = 100.0;
    printf(" Scaling to %g %%\n\n",dose_max1);
    scale = 1; if(dose_max > 0) scale = dose_max1/dose_max;
    for(k=0; k<k_max; ++k)
    {
      for(j=0; j<j_max; ++j)
      {
        for(i=0; i<i_max; ++i)
        {
          address = i + j * i_max + k * ij_max;
          dens = *(dose_data->Data + address) * scale;
          *(dose_data->Data + address) = dens;
        }
      }
    }
  }
}

static void DrawAreaNew(Widget parent, XtPointer data,
                        XmDrawingAreaCallbackStruct *cbs)
{
  Boolean xz_button_is_set,xy_button_is_set,yz_button_is_set;
  Arg    args[1];
  static int first_draw = 1;
  float dens,aux;
  XWindowAttributes window_attributes;
  long event_mask;
  Font test_font;

  if (first_draw)
  {
    GC gc=XCreateGC(XtDisplay(parent), cbs->window, 0, NULL);
    test_font = XLoadFont(XtDisplay(parent), MyFontName);
    XSetFont(XtDisplay(parent), gc, test_font);

    if (drawData == NULL) {
      drawData = (DrawData *)XtMalloc (sizeof(DrawData));
      drawData->gc = gc;
      ConfigureDrawData (parent, drawData);
    }

    XtUnmanageChild(GoToSlice);
    XtManageChild(GoToSlice);

    XGetWindowAttributes(XtDisplay(drawingArea), XtWindow(drawingArea),
                         &window_attributes);
    event_mask = window_attributes.your_event_mask | PointerMotionMask;
#ifdef DEBUG
    printf(" event_mask = %d\n",event_mask);
#endif
    XSelectInput(XtDisplay(drawingArea), XtWindow(drawingArea), event_mask);

    XtSetArg(args[0], XmNset, &xz_button_is_set);
    XtGetValues (XZPlane, args, 1);
    XtSetArg(args[0], XmNset, &xy_button_is_set);
    XtGetValues (XYPlane, args, 1);
    XtSetArg(args[0], XmNset, &yz_button_is_set);
    XtGetValues (YZPlane, args, 1);
    if ( xz_button_is_set )
    {
      plot.Dx = density_data->VoxelSize.x;
      plot.Dy = density_data->VoxelSize.z;
      plot.max_x = density_data->VoxelNumber.x;
      plot.max_y = density_data->VoxelNumber.z;
      plot.x_min = density_data->xplanes[0];
      plot.y_min = density_data->zplanes[0];
    }
    else if( xy_button_is_set )
    {
      plot.Dx = density_data->VoxelSize.x;
      plot.Dy = density_data->VoxelSize.y;
      plot.max_x = density_data->VoxelNumber.x;
      plot.max_y = density_data->VoxelNumber.y;
      plot.x_min = density_data->xplanes[0];
      plot.y_min = density_data->yplanes[0];
    }
    else
    {
      plot.Dx = density_data->VoxelSize.y;
      plot.Dy = density_data->VoxelSize.z;
      plot.max_x = density_data->VoxelNumber.y;
      plot.max_y = density_data->VoxelNumber.z;
      plot.x_min = density_data->yplanes[0];
      plot.y_min = density_data->zplanes[0];
    }
    plot.xo = plot.x_min; plot.yo = plot.y_min;
    plot.x_max = plot.x_min + plot.Dx * plot.max_x;
    plot.y_max = plot.y_min + plot.Dy * plot.max_y;
    if(!NoName_status)
    {
      aux = plot.x_max - plot.x_min;
      if ( plot.y_max - plot.y_min > aux ) aux = plot.y_max - plot.y_min;
      plot.x_max = plot.x_min + aux;
      plot.y_max = plot.y_min + aux;
    }
    plot.pixel_per_cm_x = (float) drawData->drawWidth/(plot.x_max - plot.x_min);
    plot.pixel_per_cm_y = (float) drawData->drawHeight/(plot.y_max - plot.y_min);
    if( plot.density != NULL) free(plot.density);
    plot.density = malloc( plot.max_x * plot.max_y * sizeof(float) );
    if( plot.dose != NULL) free(plot.dose);
    plot.dose = malloc( plot.max_x * plot.max_y * sizeof(float) );
    if( plot.gray != NULL) free(plot.gray);
    plot.gray = malloc( plot.max_x * plot.max_y * sizeof(long) );
    get_slice();
    first_draw  = 0;
  }
  DrawArea(parent,1);
}

static void DrawArea(Widget w, int reason)
{
  int i,j,address;
  float dens;
  float x,x1,y,y1;
  int   ix,ix1,iy,iy1,width,height,i_gray;
  float dose,dose_max_in_slice=0.0;
  int l;
  float Dx,Dy,f1,f2,f3,f4,x2,x3,x4,y2,y3,y4,xa,xb,ya,yb,fMax,fMin,c;
  int   nPoints,ifMax;
  float Epsilon=1e-30, Infinity = 1e10;
  float xxx[4],yyy[4];
  Boolean button_is_set;
  Arg    args[1];
  void DrawTicks(Widget w);

/*
  XSetForeground(XtDisplay(w), drawData->gc, gray_shades[0]);
  XSetLineAttributes(XtDisplay(w), drawData->gc, 5, LineSolid, CapRound,
                     JoinRound);
  XDrawLine (XtDisplay(w), XtWindow(w), drawData->gc,
                           0, 0, 0, DrawWindowSizeY);
  XDrawLine (XtDisplay(w), XtWindow(w), drawData->gc,
                           0, 0, DrawWindowSizeX, 0);
  XSetForeground(XtDisplay(w), drawData->gc, gray_shades[n_gray_shades - 25]);
  XDrawLine (XtDisplay(w), XtWindow(w), drawData->gc,
                          0, DrawWindowSizeY, DrawWindowSizeX, DrawWindowSizeY);
  XDrawLine (XtDisplay(w), XtWindow(w), drawData->gc,
                          DrawWindowSizeX, 0, DrawWindowSizeX, DrawWindowSizeY);
  XSetLineAttributes(XtDisplay(w), drawData->gc, 0, LineSolid, CapRound,
                     JoinRound);
  printf(" Drawing slice number %d\n",present_slice);
*/

  if(reason == 1)
  {
    XSetForeground(XtDisplay(w), drawData->gc, gray_shades[0]);
    XFillRectangle(XtDisplay(w), XtWindow(w), drawData->gc,
                 0, 0, DrawWindowSizeX+1, DrawWindowSizeY+1);
  }

  for (j=0; j<plot.max_y; j++)
  {
    y = plot.yo + plot.Dy * j;
    if(y < plot.y_min) y=plot.y_min; if(y > plot.y_max) y=plot.y_max;
    y1 = y + plot.Dy;
    if(y1 < plot.y_min) y1=plot.y_min; if(y1 > plot.y_max) y1=plot.y_max;
    iy = (y - plot.y_min) * plot.pixel_per_cm_y;
    iy1 = (y1 - plot.y_min) * plot.pixel_per_cm_y;
    height = iy1 - iy + 1;
    iy = drawData->drawY + iy;
    for (i=0; i<plot.max_x; i++)
    {
      x = plot.xo + plot.Dx * i;
      if(x < plot.x_min) x=plot.x_min; if(x > plot.x_max) x=plot.x_max;
      x1 = x + plot.Dx;
      if(x1 < plot.x_min) x1=plot.x_min; if(x1 > plot.x_max) x1=plot.x_max;
      ix = (x - plot.x_min) * plot.pixel_per_cm_x;
      ix1 = (x1 - plot.x_min) * plot.pixel_per_cm_x;
      width = ix1 - ix + 1;
      ix = drawData->drawX + ix;
      address = i + plot.max_x * j;
      XSetForeground(XtDisplay(w), drawData->gc, plot.gray[address]);
      XFillRectangle(XtDisplay(w), XtWindow(w), drawData->gc,
                     ix,iy,width,height);
      dose = plot.dose[address];
      if( dose > dose_max_in_slice) dose_max_in_slice = dose;
    }
  }
/*  printf(" Max dose in slice %d is %g\n",present_slice,dose_max_in_slice); */

  Dx = plot.Dx;
  Dy = plot.Dy;
  XtSetArg(args[0], XmNset, &button_is_set);
  for (l = 0; l < NIsolines; l++)
  {
    c = (float) isolines[l];
    if( c < dose_max_in_slice)
    {
      XtGetValues (IsoToggleButtons[l], args, 1);
      if( button_is_set )
      {
        XSetForeground(XtDisplay(w), drawData->gc, isoline_colors[l].pixel);
 /*       printf(" Ploting contour level %g\n",c); */
        for (j = 0; j < plot.max_y-1; j++)
        {
          for (i = 0; i < plot.max_x-1; i++)
          {
            address = i + plot.max_x * j;
            f1 = plot.dose[address + plot.max_x + 1];
            f2 = plot.dose[address + plot.max_x];
            f3 = plot.dose[address];
            f4 = plot.dose[address + 1];
            fMax = f1; ifMax = 1;
            if (f2 > fMax) { fMax = f2; ifMax = 2; }
            if (f3 > fMax) { fMax = f3; ifMax = 3; }
            if (f4 > fMax) { fMax = f4; ifMax = 4; }
            fMin = f1;
            if (f2 < fMin) fMin = f2;
            if (f3 < fMin) fMin = f3;
            if (f4 < fMin) fMin = f4;
            if (c < fMax && c > fMin)
            {
              xa = (i + 0.5) * Dx; xb = xa + Dx;
              ya = (j + 0.5) * Dy; yb = ya + Dy;
              if (fabs (f1 - f2) > Epsilon) {
                x1 = xa + Dx * (c - f2)/(f1 - f2); y1 = yb;
              } else { x1 = Infinity; y1 = Infinity; }

              if (fabs (f2 - f3) > Epsilon) {
                x2 = xa; y2 = ya + Dy * (c - f3)/(f2 - f3);
              } else { x2 = Infinity; y2 = Infinity; }

              if (fabs (f4 - f3) > Epsilon) {
                x3 = xa + Dx * (c - f3)/(f4 - f3); y3 = ya;
              } else { x3 = Infinity; y3 = Infinity; }

              if (fabs (f1 - f4) > Epsilon) {
                x4 = xb; y4 = ya + Dy * (c - f4)/(f1 - f4);
              } else { x4 = Infinity; y4 = Infinity; }
              nPoints = 0;
              if (ifMax == 1) {
                if (x1 > xa && x1 < xb) {
                  xxx[nPoints]=x1; yyy[nPoints]=y1; nPoints++; }
                if (y4 > ya && y4 < yb) {
                  xxx[nPoints]=x4; yyy[nPoints]=y4; nPoints++; }
                if (x3 > xa && x3 < xb) {
                  xxx[nPoints]=x3; yyy[nPoints]=y3; nPoints++; }
                if (y2 > ya && y2 < yb) {
                  xxx[nPoints]=x2; yyy[nPoints]=y2; nPoints++; }
              }
              else if (ifMax == 2) {
                if (x1 > xa && x1 < xb) {
                  xxx[nPoints]=x1; yyy[nPoints]=y1; nPoints++; }
                if (y2 > ya && y2 < yb) {
                  xxx[nPoints]=x2; yyy[nPoints]=y2; nPoints++; }
                if (y4 > ya && y4 < yb) {
                  xxx[nPoints]=x4; yyy[nPoints]=y4; nPoints++; }
                if (x3 > xa && x3 < xb) {
                  xxx[nPoints]=x3; yyy[nPoints]=y3; nPoints++; }
              }
              else if (ifMax == 3) {
                if (x3 > xa && x3 < xb) {
                  xxx[nPoints]=x3; yyy[nPoints]=y3; nPoints++; }
                if (y2 > ya && y2 < yb) {
                  xxx[nPoints]=x2; yyy[nPoints]=y2; nPoints++; }
                if (x1 > xa && x1 < xb) {
                  xxx[nPoints]=x1; yyy[nPoints]=y1; nPoints++; }
                if (y4 > ya && y4 < yb) {
                  xxx[nPoints]=x4; yyy[nPoints]=y4; nPoints++; }
              }
              else {
                if (y4 > ya && y4 < yb) {
                  xxx[nPoints]=x4; yyy[nPoints]=y4; nPoints++; }
                if (x3 > xa && x3 < xb) {
                  xxx[nPoints]=x3; yyy[nPoints]=y3; nPoints++; }
                if (y2 > ya && y2 < yb) {
                  xxx[nPoints]=x2; yyy[nPoints]=y2; nPoints++; }
                if (x1 > xa && x1 < xb) {
                  xxx[nPoints]=x1; yyy[nPoints]=y1; nPoints++; }
              }
              if (nPoints == 2)
              {
                xxx[0] += plot.xo; xxx[1] += plot.xo;
                yyy[0] += plot.yo; yyy[1] += plot.yo;
                if(xxx[0] < plot.x_min) xxx[0] = plot.x_min;
                if(xxx[0] > plot.x_max) xxx[0] = plot.x_max;
                if(xxx[1] < plot.x_min) xxx[1] = plot.x_min;
                if(xxx[1] > plot.x_max) xxx[1] = plot.x_max;
                if(yyy[0] < plot.y_min) yyy[0] = plot.y_min;
                if(yyy[0] > plot.y_max) yyy[0] = plot.y_max;
                if(yyy[1] < plot.y_min) yyy[1] = plot.y_min;
                if(yyy[1] > plot.y_max) yyy[1] = plot.y_max;
                ix  = (xxx[0] - plot.x_min) * plot.pixel_per_cm_x +
                        drawData->drawX;
                ix1 = (xxx[1] - plot.x_min) * plot.pixel_per_cm_x +
                        drawData->drawX;
                iy  = (yyy[0] - plot.y_min) * plot.pixel_per_cm_y +
                        drawData->drawY;
                iy1 = (yyy[1] - plot.y_min) * plot.pixel_per_cm_y +
                        drawData->drawY;
                XDrawLine (XtDisplay(w), XtWindow(w), drawData->gc,
                           ix, iy, ix1, iy1);
              }
              if (nPoints == 4)
              {
                xxx[0] += plot.xo; xxx[1] += plot.xo;
                xxx[2] += plot.xo; xxx[3] += plot.xo;
                yyy[0] += plot.yo; yyy[1] += plot.yo;
                yyy[2] += plot.yo; yyy[3] += plot.yo;
                if(xxx[0] < plot.x_min) xxx[0] = plot.x_min;
                if(xxx[0] > plot.x_max) xxx[0] = plot.x_max;
                if(xxx[1] < plot.x_min) xxx[1] = plot.x_min;
                if(xxx[1] > plot.x_max) xxx[1] = plot.x_max;
                if(yyy[0] < plot.y_min) yyy[0] = plot.y_min;
                if(yyy[0] > plot.y_max) yyy[0] = plot.y_max;
                if(yyy[1] < plot.y_min) yyy[1] = plot.y_min;
                if(yyy[1] > plot.y_max) yyy[1] = plot.y_max;
                if(xxx[2] < plot.x_min) xxx[2] = plot.x_min;
                if(xxx[2] > plot.x_max) xxx[2] = plot.x_max;
                if(xxx[3] < plot.x_min) xxx[3] = plot.x_min;
                if(xxx[3] > plot.x_max) xxx[3] = plot.x_max;
                if(yyy[2] < plot.y_min) yyy[2] = plot.y_min;
                if(yyy[2] > plot.y_max) yyy[2] = plot.y_max;
                if(yyy[3] < plot.y_min) yyy[3] = plot.y_min;
                if(yyy[3] > plot.y_max) yyy[3] = plot.y_max;
                ix  = (xxx[0] - plot.x_min) * plot.pixel_per_cm_x +
                        drawData->drawX;
                ix1 = (xxx[1] - plot.x_min) * plot.pixel_per_cm_x +
                        drawData->drawX;
                iy  = (yyy[0] - plot.y_min) * plot.pixel_per_cm_y +
                        drawData->drawY;
                iy1 = (yyy[1] - plot.y_min) * plot.pixel_per_cm_y +
                        drawData->drawY;
                XDrawLine (XtDisplay(w), XtWindow(w), drawData->gc,
                           ix, iy, ix1, iy1);
                ix  = (xxx[2] - plot.x_min) * plot.pixel_per_cm_x +
                        drawData->drawX;
                ix1 = (xxx[3] - plot.x_min) * plot.pixel_per_cm_x +
                        drawData->drawX;
                iy  = (yyy[2] - plot.y_min) * plot.pixel_per_cm_y +
                        drawData->drawY;
                iy1 = (yyy[3] - plot.y_min) * plot.pixel_per_cm_y +
                        drawData->drawY;
                XDrawLine (XtDisplay(w), XtWindow(w), drawData->gc,
                           ix, iy, ix1, iy1);
              }
            }
          }
        }
      }
    }
  }

  XtSetArg(args[0], XmNset, &button_is_set);
  XtGetValues (ColorWash, args, 1);
  if( button_is_set ) {
    for(ix=drawData->drawX; ix<drawData->drawX+drawData->drawWidth; ix+=XStepCW) {
      for(iy=drawData->drawY; iy<drawData->drawY+drawData->drawHeight; iy+=YStepCW) {
        dose = point_dose(ix,iy);
        if( dose > isolines[NIsolines-1] ) {
          for(l=NIsolines-2; l>=0; l--) {
            if( dose < isolines[l] ) break;
          }
          XSetForeground(XtDisplay(w), drawData->gc, isoline_colors[l+1].pixel);
          XDrawPoint(XtDisplay(w), XtWindow(w), drawData->gc, ix, iy);
        }
      }
    }
  }

  if(reason == 1) DrawTicks(w);

}

static void changedToggle(Widget parent, XtPointer data,
                          XmToggleButtonCallbackStruct *cbs)
{
  DrawArea(drawingArea,1);
}

static void changedColorWash(Widget parent, XtPointer data,
                             XmToggleButtonCallbackStruct *cbs)
{
#ifdef DEBUG
  printf(" changedColorWash! \n");
#endif
  DrawArea(drawingArea,1);
}

static void ZoomInDrawArea(Widget parent, XtPointer data,
                         XmPushButtonCallbackStruct *cbs)
{
  XEvent event;
  int x,y,x_old=0,y_old=0;
  Boolean button_pressed = 1;
  GC gc;
  unsigned long valuemask = 0;
  XGCValues values;
  int xr=362, yr=362;
  Boolean draw=0;
  XWindowAttributes window_attributes;
  long event_mask;
  Boolean start_rectangle=0;

#ifdef DEBUG
  printf(" ZoomIn function activated!\n");
#endif
  gc = XCreateGC(XtDisplay(drawingArea), XtWindow(drawingArea), valuemask, &values);
  XSetFunction(XtDisplay(drawingArea), gc, GXxor);
/*
  XSetForeground(XtDisplay(drawingArea), gc,
    WhitePixel(XtDisplay(drawingArea),DefaultScreen(XtDisplay(drawingArea))));
*/
  XSetForeground(XtDisplay(drawingArea), gc,
    isoline_colors[i_green].pixel);

/*
  XGetWindowAttributes(XtDisplay(drawingArea), XtWindow(drawingArea),
                       &window_attributes);
#ifdef DEBUG
  printf("%d ",event_mask);
#endif
  event_mask = window_attributes.your_event_mask | PointerMotionMask;
#ifdef DEBUG
  printf("%d\n",event_mask);
#endif
  XSelectInput(XtDisplay(drawingArea), XtWindow(drawingArea), event_mask);
*/

  while(button_pressed)
  {
    XNextEvent(XtDisplay(parent), &event);
    if(event.xany.window == XtWindow(drawingArea) )
    {
      switch(event.type)
      {
        case ButtonPress:
          if(event.xbutton.button == Button1)
          {
            if(!start_rectangle)
            {
              x = event.xbutton.x; y = event.xbutton.y;
              if (x < drawData->drawX) x = drawData->drawX;
              if (y < drawData->drawY) y = drawData->drawY;
              start_rectangle = 1;
            }
            else
            {
              XDrawRectangle(event.xmotion.display, event.xmotion.window, gc,
              x,y,x_old,y_old);
              start_rectangle = 0;
              x_old = 0; y_old = 0;
              xr = event.xbutton.x - x; yr = event.xbutton.y - y;
              if(!NoName_status)
              {
                if(xr < yr) yr=xr;
                if(yr < xr) xr=yr;
              }
              xr = xr + x; yr = yr + y;
#ifdef DEBUG
              printf(" x y xr yr: %d %d %d %d\n",x,y,xr,yr);
#endif
              plot.x_max = plot.x_min + ((float) (xr- drawData->drawX))/
                           plot.pixel_per_cm_x;
              plot.x_min = plot.x_min + ((float) (x - drawData->drawX))/
                           plot.pixel_per_cm_x;
              plot.y_max = plot.y_min + ((float) (yr- drawData->drawY))/
                           plot.pixel_per_cm_y;
              plot.y_min = plot.y_min + ((float) (y - drawData->drawY))/
                           plot.pixel_per_cm_y;
              plot.pixel_per_cm_x = (float) drawData->drawWidth/
                                     (plot.x_max - plot.x_min);
              plot.pixel_per_cm_y = (float) drawData->drawHeight/
                                     (plot.y_max - plot.y_min);
              /* DrawArea(drawingArea); */
              button_pressed = 0;
            }
          }
          else
          {
            if(start_rectangle)
            {
              XDrawRectangle(event.xmotion.display, event.xmotion.window, gc,
                x,y,x_old,y_old);
              start_rectangle = 0;
              x_old = 0; y_old = 0;
            }
          }
          break;
        case MotionNotify:
          if(start_rectangle)
          {
            xr = event.xmotion.x - x; yr = event.xmotion.y - y;
            if( xr < 0 ) xr = 0; if( yr < 0 ) yr = 0;
            if(!NoName_status)
            {
              if(xr < yr) yr=xr;
              if(yr < xr) xr=yr;
            }
            XDrawRectangle(event.xmotion.display, event.xmotion.window, gc,
              x,y,x_old,y_old);
            XDrawRectangle(event.xmotion.display, event.xmotion.window, gc,
              x,y,xr,yr);
            x_old = xr; y_old = yr;
            break;
          }
        default:
          break;
      }
    }
    else
    {
      switch(event.type)
      {
        case ButtonPress:
          button_pressed = 0;
          break;
        case Expose:
          XmUpdateDisplay(MainDialog);
        default:
          break;
      }
    }
  }
  DrawArea(drawingArea,1);
}

static void ZoomOutDrawArea(Widget parent, XtPointer data,
                         XmPushButtonCallbackStruct *cbs)
{
  float aux;

  plot.x_min = plot.xo;
  plot.x_max = plot.x_min + plot.Dx * plot.max_x;
  plot.y_min = plot.yo;
  plot.y_max = plot.y_min + plot.Dy * plot.max_y;
  if(!NoName_status)
  {
    aux = plot.Dx * plot.max_x;
    if ( plot.Dy * plot.max_y > aux ) aux = plot.Dy * plot.max_y;
    plot.x_max = plot.x_min + aux;
    plot.y_max = plot.y_min + aux;
  }
  plot.pixel_per_cm_x = (float) drawData->drawWidth/
                                     (plot.x_max - plot.x_min);
  plot.pixel_per_cm_y = (float) drawData->drawHeight/
                                     (plot.y_max - plot.y_min);
  DrawArea(drawingArea,1);
}

void swap_real4( DWORD *b4_var )
{
  BYTE c1,c2,c3,c4;

  c4 = (BYTE)   *b4_var;
  c3 = (BYTE) ( *b4_var >> 8);
  c2 = (BYTE) ( *b4_var >> 16);
  c1 = (BYTE) ( *b4_var >> 24);

  *b4_var =  ((DWORD) c1)        +
            (((DWORD) c2) <<  8) +
            (((DWORD) c3) << 16) +
            (((DWORD) c4) << 24);
}

void get_slice()
{
  Boolean xz_button_is_set,xy_button_is_set,yz_button_is_set;
  Arg    args[1];
  int i,j,address,i_gray,new_address;
  float dens;

  XtSetArg(args[0], XmNset, &xz_button_is_set);
  XtGetValues (XZPlane, args, 1);
  XtSetArg(args[0], XmNset, &xy_button_is_set);
  XtGetValues (XYPlane, args, 1);
  XtSetArg(args[0], XmNset, &yz_button_is_set);
  XtGetValues (YZPlane, args, 1);

  for (i=0; i<plot.max_x; i++)
  {
    for (j=0; j<plot.max_y; j++)
    {
        if( xz_button_is_set )
          address = i + plot.max_x * (present_slice +
              j * density_data->VoxelNumber.y);
        else if(xy_button_is_set )
          address = i + plot.max_x * (j +
              present_slice * plot.max_y);
        else
          address = present_slice + density_data->VoxelNumber.x * (i +
              j * plot.max_x);
        dens = *(density_data->Data + address);
        i_gray = DensityToGray( (XcmsFloat) dens);
        new_address = i + plot.max_x * j;
        *(plot.gray + new_address) = gray_shades[i_gray];
        *(plot.dose + new_address) = *(dose_data->Data + address);
        *(plot.density + new_address) = *(density_data->Data + address);
    }
  }
}

static void changedPlane(Widget parent, XtPointer data,
                         XmToggleButtonCallbackStruct *cbs)
{
  Boolean xz_button_is_set,xy_button_is_set,yz_button_is_set;
  Arg    args[2];
  float  aux;

#ifdef DEBUG
  printf(" changing plane!\n");
#endif
  XtSetArg(args[0], XmNset, &xz_button_is_set);
  XtGetValues (XZPlane, args, 1);
  XtSetArg(args[0], XmNset, &xy_button_is_set);
  XtGetValues (XYPlane, args, 1);
  XtSetArg(args[0], XmNset, &yz_button_is_set);
  XtGetValues (YZPlane, args, 1);
  if ( xz_button_is_set )
  {
    plot.Dx = density_data->VoxelSize.x;
    plot.Dy = density_data->VoxelSize.z;
    plot.max_x = density_data->VoxelNumber.x;
    plot.max_y = density_data->VoxelNumber.z;
    plot.x_min = density_data->xplanes[0];
    plot.y_min = density_data->zplanes[0];
    n_slice = density_data->VoxelNumber.y;
  }
  else if( xy_button_is_set )
  {
    plot.Dx = density_data->VoxelSize.x;
    plot.Dy = density_data->VoxelSize.y;
    plot.max_x = density_data->VoxelNumber.x;
    plot.max_y = density_data->VoxelNumber.y;
    plot.x_min = density_data->xplanes[0];
    plot.y_min = density_data->yplanes[0];
    n_slice = density_data->VoxelNumber.z;
  }
  else
  {
    plot.Dx = density_data->VoxelSize.y;
    plot.Dy = density_data->VoxelSize.z;
    plot.max_x = density_data->VoxelNumber.y;
    plot.max_y = density_data->VoxelNumber.z;
    plot.x_min = density_data->yplanes[0];
    plot.y_min = density_data->zplanes[0];
    n_slice = density_data->VoxelNumber.x;
  }
  if( present_slice > n_slice-1 ) present_slice = 0;
  plot.xo = plot.x_min; plot.yo = plot.y_min;
  plot.x_max = plot.x_min + plot.Dx * plot.max_x;
  plot.y_max = plot.y_min + plot.Dy * plot.max_y;

  XtUnmanageChild(GoToSlice);
  XtSetArg(args[0], XmNmaximum, n_slice);
  XtSetArg(args[1], XmNvalue, present_slice+1);
  XtSetValues(GoToSlice,args,2);
  XtManageChild(GoToSlice);

  if(!NoName_status)
  {
    aux = plot.x_max - plot.x_min;
    if ( plot.y_max - plot.y_min > aux ) aux = plot.y_max - plot.y_min;
    plot.x_max = plot.x_min + aux;
    plot.y_max = plot.y_min + aux;
  }
  plot.pixel_per_cm_x = (float) drawData->drawWidth/(plot.x_max - plot.x_min);
  plot.pixel_per_cm_y = (float) drawData->drawHeight/(plot.y_max - plot.y_min);
  if( plot.density != NULL) free(plot.density);
  plot.density = malloc( plot.max_x * plot.max_y * sizeof(float) );
  if( plot.dose != NULL) free(plot.dose);
  plot.dose = malloc( plot.max_x * plot.max_y * sizeof(float) );
  if( plot.gray != NULL) free(plot.gray);
  plot.gray = malloc( plot.max_x * plot.max_y * sizeof(long) );
  get_slice();
  DrawArea(drawingArea,1);
}

static void GetDose(Widget w, XtPointer data,
                    XmDrawingAreaCallbackStruct *cbs)
{
  int x,y,i,j;
  float dose,xx,yy,density;
  char string[100];
  Boolean show=1;
  XEvent event;
  GC gc;
  unsigned long valuemask = 0;
  XGCValues values;

  switch( cbs->event->type)
  {
    case ButtonPress:
      if(cbs->event->xbutton.button == Button1)
      {
        x = cbs->event->xbutton.x; y = cbs->event->xbutton.y;
        dose = point_dose(x,y);
#ifdef DEBUG
        printf(" x = %d y = %d dose = %g\n",x,y,dose);
#endif
        j = NIsolines - 1;
        for (i=0; i < NIsolines & dose < isolines[i]; i++) j = i;
        XSetForeground(XtDisplay(w), drawData->gc, isoline_colors[j].pixel);
        XFillArc(XtDisplay(w), XtWindow(w), drawData->gc, x, y, 4, 4, 0, 23040);
        sprintf(string,"%.1f",dose);
        XDrawString (XtDisplay(w), cbs->window, drawData->gc, x+3, y,
                     string, strlen(string));
      }
      else if(cbs->event->xbutton.button == Button3)
      {
        x = cbs->event->xbutton.x; y = cbs->event->xbutton.y;
        gc = XCreateGC(XtDisplay(w), XtWindow(w), valuemask, &values);
        XSetFunction(XtDisplay(drawingArea), gc, GXxor);
        XSetForeground(XtDisplay(w), gc, isoline_colors[i_green].pixel);
        XDrawLine(XtDisplay(w), cbs->window, gc,
            x, drawData->drawY, x, drawData->drawY+drawData->drawHeight);
        XDrawLine(XtDisplay(w), cbs->window, gc,
            drawData->drawX, y, drawData->drawX+drawData->drawWidth, y);
        dose = point_dose(x,y); density = point_density(x,y);
        xx = plot.x_min +
               ((float) (x - drawData->drawX))/plot.pixel_per_cm_x;
        yy = plot.y_min +
               ((float) (y - drawData->drawY))/plot.pixel_per_cm_y;
        sprintf(string,"x = %.2f y = %.2f dose = %.1f density = %.3f",
           xx,yy,dose,density);
        XDrawString (XtDisplay(w), cbs->window, gc,
                     drawData->drawX+10, drawData->drawY-4,
                     string, strlen(string));
        while(show)
        {
          XNextEvent(XtDisplay(w), &event);
          switch(event.type)
          {
            case ButtonPress:
#ifdef DEBUG
              printf(" button %d pressed!\n",event.xbutton.button);
#endif
              if(event.xbutton.button == Button2)
              {
                show=0;
                XDrawLine(XtDisplay(w), cbs->window, gc,
                  x, drawData->drawY, x, drawData->drawY+drawData->drawHeight);
                XDrawLine(XtDisplay(w), cbs->window, gc,
                  drawData->drawX, y, drawData->drawX+drawData->drawWidth, y);
                XDrawString (XtDisplay(w), cbs->window, gc,
                     drawData->drawX+10, drawData->drawY-4,
                     string, strlen(string));
              }
              break;
            case MotionNotify:
              XDrawString (XtDisplay(w), cbs->window, gc,
                     drawData->drawX+10, drawData->drawY-4,
                     string, strlen(string));
              XDrawLine(XtDisplay(w), cbs->window, gc,
                  x, drawData->drawY, x, drawData->drawY+drawData->drawHeight);
              XDrawLine(XtDisplay(w), cbs->window, gc,
                  drawData->drawX, y, drawData->drawX+drawData->drawWidth, y);
              x = event.xmotion.x; y = event.xmotion.y;
              if(x<drawData->drawX || x>drawData->drawX+drawData->drawWidth ||
                 y<drawData->drawY || y>drawData->drawY+drawData->drawHeight)
              {
                show=0;
              }
              else
              {
                dose = point_dose(x,y); density = point_density(x,y);
                xx = plot.x_min +
                       ((float) (x - drawData->drawX))/plot.pixel_per_cm_x;
                yy = plot.y_min +
                       ((float) (y - drawData->drawY))/plot.pixel_per_cm_y;
                sprintf(string,"x = %.2f y = %.2f dose = %.1f density = %.3f",
                   xx,yy,dose,density);
                XDrawString (XtDisplay(w), cbs->window, gc,
                     drawData->drawX+10, drawData->drawY-4,
                     string, strlen(string));
                XDrawLine(XtDisplay(w), cbs->window, gc,
                  x, drawData->drawY, x, drawData->drawY+drawData->drawHeight);
                XDrawLine(XtDisplay(w), cbs->window, gc,
                  drawData->drawX, y, drawData->drawX+drawData->drawWidth, y);
              }
            default:
              break;
          }
        }

      }
      break;
    default:
      break;
  }
}

void DrawTicks(Widget w)
{
  int n_check_ticks, i, ix_ticks=0, iy_ticks=0;
  int nx_ticks=0, ny_ticks=0;
  float dx,dy,x_tick_min,y_tick_min;
  int ix,iy;
  float x,y;
  char string[10];
  XmString tick_label = NULL;
  Dimension tick_label_width;
  Font test_font;
  XFontStruct *test_font_struct;
  int label_width;

  n_check_ticks = sizeof( which_ticks ) / sizeof( float );
  dx = plot.x_max - plot.x_min;
  dy = plot.y_max - plot.y_min;
  for (i=0; i<n_check_ticks & which_ticks[i] * MaxMajorTicks < dx; i++)
     ix_ticks = i+1;
  if(ix_ticks >= n_check_ticks) ix_ticks = n_check_ticks-1;
  for (i=0; i<n_check_ticks & which_ticks[i] * MaxMajorTicks < dy; i++)
     iy_ticks = i+1;
  if(iy_ticks >= n_check_ticks) iy_ticks = n_check_ticks-1;
  nx_ticks = (int) (dx / which_ticks[ix_ticks]);
  ny_ticks = (int) (dy / which_ticks[iy_ticks]);
  i = (int) (plot.x_min / which_ticks[ix_ticks]);
  x_tick_min = i * which_ticks[ix_ticks];
  if( x_tick_min < plot.x_min) x_tick_min = x_tick_min + which_ticks[ix_ticks];
  i = (int) (plot.y_min / which_ticks[iy_ticks]);
  y_tick_min = i * which_ticks[iy_ticks];
  if( y_tick_min < plot.y_min) y_tick_min = y_tick_min + which_ticks[iy_ticks];

  XSetForeground(XtDisplay(w), drawData->gc, isoline_colors[i_gelb].pixel);
/*
  test_font = XLoadFont(XtDisplay(w), "*courier-bold*120*iso8859-1");
  test_font_struct = XLoadQueryFont(XtDisplay(w), "*courier-bold*120*iso8859-1");
  test_font = XLoadFont(XtDisplay(w), MyFontName);
  test_font_struct = XLoadQueryFont(XtDisplay(w), MyFontName);
  XSetFont(XtDisplay(w), drawData->gc, test_font);
*/

  XDrawRectangle(XtDisplay(w), XtWindow(w), drawData->gc,
                 drawData->drawX-1, drawData->drawY-1,
                 drawData->drawWidth+1, drawData->drawHeight+1);

  for (i=0; i<nx_ticks+1; i++)
  {
    x = x_tick_min + i * which_ticks[ix_ticks];
    if( x <= plot.x_max )
    {
      sprintf(string,"%.1f",x);
      ix = (int) ((x - plot.x_min) * plot.pixel_per_cm_x );
      ix = ix + drawData->drawX;
      iy = drawData->drawY;
      XDrawLine(XtDisplay(w), XtWindow(w), drawData->gc,
            ix, iy, ix, iy - MajorTicksLength);
      iy = drawData->drawY + drawData->drawHeight;
      XDrawLine(XtDisplay(w), XtWindow(w), drawData->gc,
            ix, iy, ix, iy + MajorTicksLength);
      iy = drawData->drawY + drawData->drawHeight + 17;
      ix = ix - 3 * strlen(string);
/*
      label_width = XTextWidth(test_font_struct,string,strlen(string));
      printf(" Label: %s width = %d\n",string,label_width);
*/
      XDrawString (XtDisplay(w), XtWindow(w), drawData->gc,
                   ix,iy,string,strlen(string));
    }
  }

  for (i=0; i<ny_ticks+1; i++)
  {
    y = y_tick_min + i * which_ticks[iy_ticks];
    if( y <= plot.y_max )
    {
      sprintf(string,"%.1f",y);
      iy = (int) ((y - plot.y_min) * plot.pixel_per_cm_y );
      iy = iy + drawData->drawY;
      ix = drawData->drawX;
      XDrawLine(XtDisplay(w), XtWindow(w), drawData->gc,
            ix, iy, ix - MajorTicksLength, iy);
      ix = ix - 6 * strlen(string) - MajorTicksLength;
      XDrawString (XtDisplay(w), XtWindow(w), drawData->gc,
                   ix,iy+4,string,strlen(string));
      ix = drawData->drawX + drawData->drawWidth;
      XDrawLine(XtDisplay(w), XtWindow(w), drawData->gc,
            ix, iy, ix + MajorTicksLength, iy);
      ix = ix + MajorTicksLength + 2;
      XDrawString (XtDisplay(w), XtWindow(w), drawData->gc,
                   ix,iy+4,string,strlen(string));
    }
  }

}

float point_density(int x, int y)
{
  float xx,yy,density;
  int   i,j,address;

  xx = plot.x_min - plot.xo +
         ((float) (x - drawData->drawX))/plot.pixel_per_cm_x;
  if( xx > plot.Dx * plot.max_x ) return (float) 0.0;
  yy = plot.y_min - plot.yo +
         ((float) (y - drawData->drawY))/plot.pixel_per_cm_y;
  if( yy > plot.Dy * plot.max_y ) return (float) 0.0;
  i = xx/plot.Dx; j = yy/plot.Dy;
  address = i + j * plot.max_x;
  density = plot.density[address];
  return density;
}

float point_dose(int x, int y)
{
  float xx,yy,ai,aj,d1,d2,d3,d4,temp,dose;
  int   i,j,address;

  xx = plot.x_min - plot.xo +
         ((float) (x - drawData->drawX))/plot.pixel_per_cm_x;
  if( xx > plot.Dx * plot.max_x ) return (float) 0.0;
  if( xx < 0 ) return (float) 0.0;
  yy = plot.y_min - plot.yo +
         ((float) (y - drawData->drawY))/plot.pixel_per_cm_y;
  if( yy > plot.Dy * plot.max_y ) return (float) 0.0;
  if( yy < 0 ) return (float) 0.0;
  ai = xx/plot.Dx - 0.5; aj = yy/plot.Dy - 0.5; i = (int) ai; j = (int) aj;
  address = i + j * plot.max_x;
  d1 = plot.dose[address];
  d2 = plot.dose[address + 1];
  d3 = plot.dose[address + plot.max_x];
  d4 = plot.dose[address + plot.max_x + 1];
  temp = ai - (float) i;
  d1 = d1 + (d2 - d1) * temp;
  d2 = d3 + (d4 - d3) * temp;
  dose = d1 + (aj - (float) j) * (d2 - d1);
  if( dose < 0.0 ) dose = 0.0;
  return dose;
}

static void ChangeMaxDensity(Widget parent, XtPointer data,
                             XmScaleCallbackStruct *cbs)
{
  MaxDensity = (float) cbs->value/1000.0;
  get_slice();
  if(cbs->reason == XmCR_VALUE_CHANGED)
  {
    XtUnmapWidget(GrayScaleLegend);
    XtMapWidget(GrayScaleLegend);
    DrawArea(drawingArea,1);
  }
  else DrawArea(drawingArea,0);
}

static void ChangeMinDensity(Widget parent, XtPointer data,
                             XmScaleCallbackStruct *cbs)
{
  MinDensity = (float) cbs->value/1000.0;
  get_slice();
  if(cbs->reason == XmCR_VALUE_CHANGED)
  {
    XtUnmapWidget(GrayScaleLegend);
    XtMapWidget(GrayScaleLegend);
    DrawArea(drawingArea,1);
  } else DrawArea(drawingArea,0);
}

static void changedNoName(Widget parent, XtPointer data,
                          XmToggleButtonCallbackStruct *cbs)
{
  float aux;
  plot.x_min = plot.xo;
  plot.y_min = plot.yo;
  plot.x_max = plot.x_min + plot.Dx * plot.max_x;
  plot.y_max = plot.y_min + plot.Dy * plot.max_y;
  if(NoName_status)
  {
    aux = plot.Dx * plot.max_x;
    if ( plot.Dy * plot.max_y > aux ) aux = plot.Dy * plot.max_y;
    plot.x_max = plot.x_min + aux;
    plot.y_max = plot.y_min + aux;
    NoName_status = 0;
  } else NoName_status = 1;
  plot.pixel_per_cm_x = (float) drawData->drawWidth/
                                     (plot.x_max - plot.x_min);
  plot.pixel_per_cm_y = (float) drawData->drawHeight/
                                     (plot.y_max - plot.y_min);

  DrawArea(drawingArea,1);
}
