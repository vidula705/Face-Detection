
//include the files
#include   "cv.h"
#include   "highgui.h"
#include   <stdio.h>
#include   <stdlib.h>
#include   <string.h>
#include   <ctype.h>
#include   <errno.h>

#ifndef FALSE
#  define  FALSE         0
#endif
#ifndef TRUE
#  define TRUE           1
#endif

#define CAPTURE_LIVE     1
#define GRAYLEVEL        255

#define IMAGE_HEIGHT     480
#define IMAGE_WIDTH      640

//#define IMAGE_HEIGHT   (1080)
//#define IMAGE_WIDTH    (1920)


#define  MAX_FACES_IN_IMAGE   50

#define UNUSEDVAR(ab) (ab = ab)
#define FACESIZE 90
#define MAXSIZE 1000
#define MINSIZE 20
#define RESIZE 100

/********* CONFIG PARAMETERS *********/
#define     BG_LEARNING                 (0.9)
#define     DENOMINATOR                 (256 * 256)
#define     ALPHA                       ((int)(DENOMINATOR * BG_LEARNING) )
#define     BETA                        (DENOMINATOR-ALPHA)
#define     MIN_SCALING                 (1)
#define     THRESHOLD                   (50)

#define     SENSITIVITY                 (60)
#define     TAMPER_TRIGGER_THRESHOLD    (30)
#define     NO_TAMPER_MAX               (10)
#define     ABSOLUTE_ALLOWED_VARIATION  (20)

/****** ALERT ENUMS ******/
#define     NO_TAMPER_ALERT              (0)
#define     TAMPER_ATTEMPT_ALERT         (1)
#define     TAMPER_ALERT                 (2)
#define     NO_SIGNAL_ALERT              (3)

typedef unsigned char	u8;


//structure declarations
/*******************************************************************************************************/
//Structure for Centroid points of ellipse 
typedef struct Centroid  
{
    int    x;                         /* center_x */
    int    y;                         /* center_y */
}Centroid_t;


//this structure contains the properties of a face being detected
typedef struct FaceFeature 
{
    int        height;                  /*face hieght*/
    int        width;                   /*face width */
    int        area;                    /*face area*/
    int        length;                  /*face area*/
    Centroid_t centroid;                /*faceCentroid*/
    char       isValid;                 /*Is data valid*/
    int        Totaleye;                /*number of eyes in a face*/
                                         /*if a face does not have two eyes then dont increment the counter*/
}feature_t; 


typedef struct FaceFeatureList
{
    feature_t*    pFaceFeature;        /* face features */
    int           nTotalFaces;
}FaceFeatureList;



typedef struct tag_tamper_data {
	int		width, height, channels;
	int		scale_factor;
	int		status;
	u8		*frame_full, *frame_full_gray;
	u8		*cur_frame, *prev_frame, *diff;
	u8		*cur_bg, *prev_bg;
	u8		*cur_fg;
	int		tamper_count;
	int		tamper_status;
	int		frame_count;
	int		cur_frame_mean;
	int     isFirstFrame;
} TAMPER_DATA;

/*******************************************************************************************************/
//global declaration
/*******************************************************************************************************/

IplImage      *img;
IplImage      *img_in;
IplImage      *Re_img;
int           frame  = 0;
CvRect        *face  = 0;
CvSeq         *contours = 0;
CvMemStorage*            storage   = 0;
CvHaarClassifierCascade* cascade_f = 0;
CvHaarClassifierCascade* cascade_e = 0;



//CasCades path
const char* face_cascade    = "./haarcascade_frontalface_default.xml";
const char* eyes_cascade    = "./haarcascade_eye_tree_eyeglasses.xml";


//Function Declaration

void * motionDetection( int width, int height, int channels, unsigned char* pBg, unsigned char* pFg);

void BackgroundUpdate(TAMPER_DATA *context);

int eiFgBG( void *tHandle, unsigned char *inimg);

void SetMask24x24(IplImage *img_gray, IplImage *Fg, IplImage* imgMask);

void detectFaceEyes(IplImage *img ,FaceFeatureList *pFeatureList, CvSeq *faces ,IplImage *fg);

int matchFaceFeatureSet(feature_t* pFaceFeatureHist, feature_t* pCurFaceFeature, int nRefIndex, int nTotalFaces );

int updateHistory(feature_t *pFaceFeatureHist, int nMatchIndex, feature_t *pCurFaceFeature, int nIndex);












