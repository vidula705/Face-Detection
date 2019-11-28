/*******************************************************************************************************
 *File:    FaceDetectCounter.c
 *Desc:    This file Contains definition of routines to detect face and eye using cascade.
 *-----    -----------
 *******************************************************************************************************/

//make includes
#include "FaceCount.h"

/**************************************************************************************************************************
 *Function: main
 *Desc:     main calling routing(works as entry point function)
 **************************************************************************************************************************/
int main()
{
	CvCapture *capture                   = NULL;
	FaceFeatureList* pCurFeatureList     = NULL;
	FaceFeatureList* pFeatureListHistory = NULL;
	CvSeq *pFaces                        = NULL;
	int IsFirstFrame                     = 1;
	int nIndex                           = 0;
	int IsNoFace                         = 0;
	int FaceCounter                      = 0;
	int UpdatCount                       = 0;
	CvFont font;
	char DisplayCounter                    [20];
	void* Hdl                            = NULL;
	int nIndex1;
	int nMatchIndex;
	int temp;
	IplImage* Fg ;
	IplImage* Bg ;
	IplImage* imgMask;
	IplImage* img_gray;
	UNUSEDVAR(UpdatCount);
	cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 1.0, 1.0, 0 , 3 , 8);


/*To capture from CAM*/
#if CAPTURE_LIVE
	capture = cvCaptureFromCAM( 0 );

/*To capture from Video*/
#else
	capture = cvCaptureFromAVI("./girl.avi");
#endif

	if(!capture)
	{
		fprintf(stdout,"Capture failed \n");
		return -1;
	}

	cvSetCaptureProperty(capture, CV_CAP_PROP_FPS, 30.0);
	cvSetCaptureProperty(capture,CV_CAP_PROP_FRAME_WIDTH , IMAGE_WIDTH);
	cvSetCaptureProperty(capture,CV_CAP_PROP_FRAME_HEIGHT , IMAGE_HEIGHT);


	/*face cascade*/
	cascade_f = (CvHaarClassifierCascade*)cvLoad( face_cascade, 0, 0, 0 );
	if(!cascade_f)
	{
		fprintf(stderr, "cascade_f is null\n");
		return -1;
	}

	/*eye cascade*/
	cascade_e = (CvHaarClassifierCascade*)cvLoad( eyes_cascade, 0, 0, 0 );
	if(!cascade_e)
	{
		fprintf(stderr, "cascade_e is null\n");
		return -1;
	}

	//Memory allocation for face features
	pCurFeatureList = (FaceFeatureList*)malloc(sizeof(FaceFeatureList));
	if(!pCurFeatureList)
	{
		fprintf(stderr,"Error Allocating FaceFeatureList\n");
		return -1;
	}

	pCurFeatureList->pFaceFeature = (feature_t*)malloc(sizeof(feature_t)*MAX_FACES_IN_IMAGE);
	if(!pCurFeatureList->pFaceFeature)
	{
		fprintf(stderr,"Error Allocating pCurFeatureList->pFaceFeature\n");
		return -1;
	}

	memset(pCurFeatureList->pFaceFeature, 0x00, sizeof(feature_t)*MAX_FACES_IN_IMAGE);

	pFeatureListHistory = (FaceFeatureList*)malloc(sizeof(FaceFeatureList));
	if(!pFeatureListHistory)
	{
		fprintf(stderr,"Error Allocating FaceFeatureList\n");
		return -1;
	}

	pFeatureListHistory->pFaceFeature = (feature_t*)malloc(sizeof(feature_t)*MAX_FACES_IN_IMAGE);
	if(!pFeatureListHistory->pFaceFeature)
	{
		fprintf(stderr,"Error Allocating pFeatureListHistory->pFaceFeature\n");
		return -1;
	}

	memset(pFeatureListHistory->pFaceFeature, 0x00, sizeof(feature_t)*MAX_FACES_IN_IMAGE);

	//Memory allocation for foreground and background
	Fg   = cvCreateImage(cvSize(IMAGE_WIDTH,IMAGE_HEIGHT), 8, 1);
	if(!Fg)
	{
		fprintf(stdout,"Error Allocating Fg \n");
		exit(0);
	}

	Bg   = cvCreateImage(cvSize(IMAGE_WIDTH,IMAGE_HEIGHT), 8, 1);
	if(!Bg)
	{
		fprintf(stdout,"Error Allocating Bg \n");
		exit(0);
	}

	imgMask   = cvCreateImage(cvSize(IMAGE_WIDTH,IMAGE_HEIGHT), 8, 1);
	if(!imgMask)
	{
		fprintf(stdout,"Error Allocating imgMask \n");
		exit(0);
	}

	memset(Fg->imageData, 0x00, IMAGE_WIDTH*IMAGE_HEIGHT);
	memset(Bg->imageData, 0x00, IMAGE_WIDTH*IMAGE_HEIGHT);


	Hdl = motionDetection( IMAGE_WIDTH, IMAGE_HEIGHT, 1,(unsigned char*)Bg->imageData,(unsigned char*)Fg->imageData);
	if(!Hdl)
	{
		exit(0);
	}

	img_gray   = cvCreateImage(cvSize(IMAGE_WIDTH, IMAGE_HEIGHT), 8, 1);
	if(NULL == img_gray)
	{
		printf("Error \n");
		exit(0);
	}

	while( 1 )
	{

		feature_t* pCurFaceFeature   = pCurFeatureList->pFaceFeature;
		feature_t* pFaceFeatureHist  = pFeatureListHistory->pFaceFeature;

		/*Capture each frame*/
		img                          = cvQueryFrame( capture );
		if(!img)
		{
			fprintf(stderr, "Error : frame not captured \n");
			exit(0);
		}

		/*Convert color image to gray scale*/
		cvCvtColor(img, img_gray, CV_RGB2GRAY);

		/*Functions to separate foreground and background*/
		FgBG(Hdl,(unsigned char*) img_gray->imageData);

		SetMask24x24(img_gray, Fg, imgMask);

		/*Function to  detect face and eyes*/
		detectFaceEyes(img, pCurFeatureList, pFaces, imgMask);

		/*Face counter updation*/
		if(pCurFeatureList->nTotalFaces >= 1 && pCurFaceFeature->Totaleye >= 1)
		{
			if(IsFirstFrame)
			{
				IsFirstFrame = 0;
				pFeatureListHistory->nTotalFaces = 0;
				for(nIndex = 0; nIndex < pCurFeatureList->nTotalFaces ; nIndex++)
				{
					if((TRUE == pCurFaceFeature[nIndex].isValid) )
					{
						/*check whether face is present or not*/
						pFaceFeatureHist[nIndex].isValid    = TRUE;
						pFaceFeatureHist[nIndex].height     = pCurFaceFeature[nIndex].height;
						pFaceFeatureHist[nIndex].width      = pCurFaceFeature[nIndex].width ;
						pFaceFeatureHist[nIndex].area       = pCurFaceFeature[nIndex].area;
						pFaceFeatureHist[nIndex].length     = pCurFaceFeature[nIndex].length;
						pFaceFeatureHist[nIndex].centroid.x = pCurFaceFeature[nIndex].centroid.x;
						pFaceFeatureHist[nIndex].centroid.y = pCurFaceFeature[nIndex].centroid.y;
						pFaceFeatureHist[nIndex].Totaleye   = pCurFaceFeature[nIndex].Totaleye;
						pFeatureListHistory->nTotalFaces++;
						if(pCurFaceFeature[nIndex].Totaleye)
							FaceCounter++;
					}
				}
			}
			else
			{

				temp = pFeatureListHistory->nTotalFaces;
				for(nIndex = 0; nIndex < pCurFeatureList->nTotalFaces && pCurFaceFeature[nIndex].Totaleye; nIndex++)
				{
					nMatchIndex = matchFaceFeatureSet(pFaceFeatureHist, pCurFaceFeature, nIndex, temp);
					if(nMatchIndex != -1)
					{
						updateHistory(pFaceFeatureHist, nMatchIndex,  pCurFaceFeature, nIndex);
					}
					else if(nMatchIndex == -1)
					{
						++FaceCounter;

						for( nIndex1 = 0; nIndex1 < MAX_FACES_IN_IMAGE ;nIndex1++)
						{
							if(FALSE == pFaceFeatureHist[nIndex1].isValid)
							{
								/*check whether face is present or not*/
								pFaceFeatureHist[nIndex1].isValid    = TRUE;
								pFaceFeatureHist[nIndex1].height     = pCurFaceFeature[nIndex].height;
								pFaceFeatureHist[nIndex1].width      = pCurFaceFeature[nIndex].width ;
								pFaceFeatureHist[nIndex1].area       = pCurFaceFeature[nIndex].area;
								pFaceFeatureHist[nIndex1].length     = pCurFaceFeature[nIndex].length;
								pFaceFeatureHist[nIndex1].centroid.x = pCurFaceFeature[nIndex].centroid.x;
								pFaceFeatureHist[nIndex1].centroid.y = pCurFaceFeature[nIndex].centroid.y;
								pFaceFeatureHist[nIndex1].Totaleye   = pCurFaceFeature[nIndex].Totaleye;
								pFeatureListHistory->nTotalFaces++;
								break;
							}
						}
					}
				}

			}
			IsNoFace = 0;
		}
		else
			IsNoFace++ ;


		if(IsNoFace > 30)
		{
			IsNoFace = 0;
			for(nIndex = 0; nIndex < MAX_FACES_IN_IMAGE; nIndex++)
			{
				pFaceFeatureHist[nIndex].isValid    = FALSE;
				pFaceFeatureHist[nIndex].height     = -1;
				pFaceFeatureHist[nIndex].width      = -1;
				pFaceFeatureHist[nIndex].area       = -1;
				pFaceFeatureHist[nIndex].length     = -1;
				pFaceFeatureHist[nIndex].centroid.x = -1;
				pFaceFeatureHist[nIndex].centroid.y = -1;
				pFaceFeatureHist[nIndex].Totaleye   = -1;
			}
			pFeatureListHistory->nTotalFaces        =  0;

		}

		sprintf(DisplayCounter, "FaceCounter  %d",FaceCounter);

		cvPutText(img, DisplayCounter , cvPoint(10,200), &font, cvScalar(0,255,0,0));

		cvShowImage( "Face Detect Counter", img);

		if((cvWaitKey(10) & 255) == 27) break;

	}

	/*free if any cascade is used*/
	if(cascade_e)
		cvReleaseHaarClassifierCascade( &cascade_e );

	if(cascade_f)
		cvReleaseHaarClassifierCascade( &cascade_f );

	if(pFeatureListHistory)
	{
		free(pFeatureListHistory->pFaceFeature);
		pFeatureListHistory->pFaceFeature = NULL;
		free(pFeatureListHistory);
		pFeatureListHistory = NULL;
	}

	if(pCurFeatureList)
	{
		free(pCurFeatureList->pFaceFeature);
		pCurFeatureList->pFaceFeature = NULL;
		free( pCurFeatureList);
		pCurFeatureList = NULL;
	}
	return 0;
}





/************************************************************************************************************
 *Function: motionDetection
 *Desc:     Separates Foreground and background
 ***************************************************************************************************************/
void * motionDetection( int width, int height, int channels, unsigned char* pBg, unsigned char* pFg)
{
	TAMPER_DATA	*handle;
	u8			*data;
	int			image_size = width * height;

	handle = (TAMPER_DATA *)calloc( 1, sizeof( TAMPER_DATA ) );
	if ( handle == NULL ) return NULL;
	handle->width = width, handle->height = height, handle->channels = channels;
	// there are 10 images that are stored + 1 for input, assuming worst case scale = 1
	// so 1 *(w+h) for graying, 10 *(w+h)/MIN_SCALING/MIN_SCALING for the others
	data = (u8 *) calloc( 1, image_size + (image_size * 6)/MIN_SCALING/MIN_SCALING );
	if ( data == NULL ) { free( handle ); return NULL; }
	// assign the pointers to all frame storages from this single block
	handle->frame_full_gray = data; data += image_size;
	handle->cur_frame = data; data += image_size;
	handle->prev_frame = data; data += image_size;
	handle->diff = data; data += image_size;
	handle->cur_bg = pBg; data += image_size;
	handle->prev_bg = data; data += image_size;
	handle->cur_fg = pFg;
	handle->isFirstFrame = 1;
	handle->scale_factor = 1; 	//Keeping scaling factor to 1 as scaled Luma data is received
	return (void *)handle;
}






/**************************************************************************************************************************
 *Function: BackgroundUpdate
 *Desc:    Update the backgroung
 **************************************************************************************************************************/
void BackgroundUpdate(TAMPER_DATA *context)
{
	u8 *pdiff, *p_bg, *p_bg_prev, *p_fg;
	int i,j,w,h;

	w = context->width/context->scale_factor, h = context->height/context->scale_factor;
	p_bg = context->cur_bg;
	p_bg_prev = context->prev_bg;
	pdiff = context->cur_frame;
	p_fg = context->cur_fg;
	// update background
	for ( i = 0; i != h; i++ )
	{
		for ( j = 0; j != w; j++, p_bg++, p_bg_prev++, pdiff++, p_fg++ )
		{
			*p_fg = (abs(*p_bg_prev - *pdiff) < ABSOLUTE_ALLOWED_VARIATION) ? 0 : 255;
			if(!*p_fg)
				*p_bg = ( ALPHA * *p_bg_prev + BETA * *pdiff ) >> 16;
		}
	}
}



/**************************************************************************************************************************
 *Function: FgBG
 *Desc:     main calling routing(works as entry point function)
 **************************************************************************************************************************/
int FgBG( void *tHandle, unsigned char *inimg)
{
	TAMPER_DATA		*context = (TAMPER_DATA	*)tHandle;
	int				w, h;

	if (context == NULL) {
		fprintf(stderr,"Invalid handle \n");
		return -1;
	}

	w = context->width/context->scale_factor;
	h = context->height/context->scale_factor;

	memcpy((unsigned char*)context->cur_frame, (unsigned char*)inimg, context->width * context->height );


	if(context->isFirstFrame)
	{
		memcpy( context->cur_bg, inimg, w * h );
		memcpy( context->prev_bg, inimg, w * h );
		context->isFirstFrame = 0;
	}

	memcpy( context->prev_frame, context->cur_frame, w * h );

	BackgroundUpdate(context);

	memcpy( context->prev_bg, context->cur_bg, w * h );

	return context->tamper_status;
}


/**************************************************************************************************************************
 *Function: SetMask24x24
 *Desc:     Set mask of 24x24 to set src image to dest iamge if motion detected
 **************************************************************************************************************************/

void SetMask24x24(IplImage *img_gray, IplImage *Fg, IplImage* imgMask)
{

	int index1               = 0;
	int index2               = 0;
	int index3               = 0;
	int index4               = 0;
	unsigned char* pImgMask = NULL;
	int k1                   = 24;
	int k2                   = 24;
	int INDEX;
	int i,j;
	int a                    = 0;
	int b                    = 0;
	int countmask            = 0;


	memset(imgMask->imageData, 0,img_gray->height*img_gray->width );

	for(index1 = 0; index1 < img_gray->height; index1+=24)
	{
		pImgMask = (unsigned char*) Fg->imageData     + index1*Fg->widthStep;

		a = img_gray->height - index1;
		if(a < 24) k1 = k1 + a; else k1 = 24;

		for(index2 = 0; index2 < img_gray->width; index2+=24)
		{
			countmask = 0;
			b = img_gray->width - index1;
			if(b < 24) k2 = k2 + b; else k2 = 24;

			for(index3 = 0; index3 < k1; index3++)
			{
				pImgMask =(unsigned char*) Fg->imageData  + (index1 + index3)*Fg->widthStep       + index2;

				for(index4 = 0; index4 < k2; index4++)
				{
					countmask += pImgMask[index4] ;
				}
			}

			if(countmask)
			{
				for(i = 0; i < k1; i++)
				{
					for(j = 0; j < k2; j++)
					{
						INDEX = (index1+i)*img_gray->widthStep + (index2+j);
						*( imgMask->imageData + INDEX)  =  *( img_gray->imageData + INDEX);
					}
				}
			}
		}
	}
}




/*******************************************************************************************************
 *Function: detectFaceEyes
 *Desc    : To detect face and eyes from live videos
 *******************************************************************************************************/
void detectFaceEyes(IplImage *img ,FaceFeatureList *pFeatureList, CvSeq *faces ,IplImage *fg)
{

	int i                    = 0;
	int j                    = 0;
	pFeatureList->nTotalFaces = 0;
	CvRect *eye;

	/*Memory Allocation*/
	img_in  = cvCreateImage(cvSize(img->width, img->height), 8, 1);
	Re_img  = cvCreateImage(cvSize(RESIZE,RESIZE), 8, 1);

	cvCvtColor(img, img_in, CV_BGR2GRAY);

	cvEqualizeHist(img_in,img_in);
	cvEqualizeHist(fg,fg);

	storage = cvCreateMemStorage(0) ;

	/*Detect face in the frame*/
	faces = cvHaarDetectObjects(fg, cascade_f, storage, 1.4,5, CV_HAAR_DO_CANNY_PRUNING, cvSize(MINSIZE,MINSIZE),cvSize(MAXSIZE, MAXSIZE));
	if(!faces)    return ;

	/*save face properties from detected face*/
	pFeatureList->nTotalFaces = faces->total;


	for(j = 0; j < faces->total; j++)
	{
		CvSeq *eyes  = NULL;
		CvRect *face = (CvRect*)cvGetSeqElem(faces, j);
		feature_t* pFaceFeature      = NULL;

		if(!face)    return ;

		cvRectangle( img, cvPoint(face->x, face->y),cvPoint( face->x + face->width, face->y + face->height),CV_RGB(255, 0, 0), 2, 8, 0 );

		cvSetImageROI(fg, cvRect (face->x, face->y + (face->height/5.5), face->width, face->height/3.0));

		//Resize the image if face is less than FACESIZE and detect eyes in rescaled image
		if((face->height < FACESIZE) || (face->width < FACESIZE))
		{

			cvResize(img_in, Re_img, CV_INTER_LINEAR );

			cvEqualizeHist(Re_img,Re_img);

			cvResetImageROI(Re_img);

			eyes = cvHaarDetectObjects(Re_img, cascade_e, storage, 1.1,2,CV_HAAR_DO_CANNY_PRUNING ,cvSize(MINSIZE,MINSIZE), cvSize(MAXSIZE,MAXSIZE));

			pFaceFeature                 = pFeatureList->pFaceFeature;
			pFaceFeature[j].isValid      = TRUE;
			pFaceFeature[j].height       = face->height;
			pFaceFeature[j].width        = face->width;
			pFaceFeature[j].area         = face->width * face->height;
			pFaceFeature[j].length       = (face->width*2) + (face->height*2);
			pFaceFeature[j].centroid.x   = (face->x) + (face->width  / 2.0);
			pFaceFeature[j].centroid.y   = (face->y) + (face->height / 2.0);
			pFaceFeature[j].Totaleye     = eyes->total;   /* count number of eyes in each face */

			cvResetImageROI(Re_img);

			for( i = 0; i < (eyes ? eyes->total : 0); i++ )
			{
				CvRect *eyeResize = (CvRect*)cvGetSeqElem(eyes, i);
				if(!eyeResize)    return ;
				cvRectangle(Re_img,cvPoint(eyeResize->x, eyeResize->y), cvPoint(eyeResize->x + eyeResize->width, eyeResize->y + eyeResize->height), CV_RGB(0, 255, 0), 2, 8, 0 );

				eye = eyeResize;
				cvRectangle(img,cvPoint(eye->x, eye->y), cvPoint(eye->x + eye->width/3, eye->y + eye->height/3), CV_RGB(0, 255, 0), 2, 8, 0 );

			}
			cvResetImageROI(img);

		}

		else
		{

			eyes = cvHaarDetectObjects(fg, cascade_e, storage, 1.1, 2,CV_HAAR_DO_CANNY_PRUNING ,cvSize(MINSIZE,MINSIZE), cvSize(MAXSIZE,MAXSIZE));

			pFaceFeature                 = pFeatureList->pFaceFeature;
			pFaceFeature[j].isValid      = TRUE;
			pFaceFeature[j].height       = face->height;
			pFaceFeature[j].width        = face->width;
			pFaceFeature[j].area         = face->width * face->height;
			pFaceFeature[j].length       = (face->width*2) + (face->height*2);
			pFaceFeature[j].centroid.x   = (face->x) + (face->width  / 2.0);
			pFaceFeature[j].centroid.y   = (face->y) + (face->height / 2.0);
			pFaceFeature[j].Totaleye     = eyes->total;   /* count number of eyes in each face */


			cvSetImageROI( img, cvRect( face->x, face->y + (face->height/5.5), face->width, face->height/3.0));

			for( i = 0; i < (eyes ? eyes->total : 0); i++ )
			{
				CvRect *eye = (CvRect*)cvGetSeqElem(eyes, i);
				if(!eye)    return ;
				cvRectangle(img,cvPoint(eye->x, eye->y), cvPoint(eye->x + eye->width, eye->y + eye->height), CV_RGB(0, 255, 0), 2, 8, 0 );

			}
		}
		cvResetImageROI(img);
		cvResetImageROI(fg);
	}

	cvReleaseImage(&Re_img);
	cvReleaseImage(&img_in);
	cvReleaseMemStorage(&storage  );

}



/*******************************************************************************************************
 *Function: matchFaceFeatureSet
 *Desc: To match the features of the two face in a same frame
 *******************************************************************************************************/
int matchFaceFeatureSet(feature_t* pFaceFeatureHist, feature_t* pCurFaceFeature, int nRefIndex, int nTotalFaces )
{
	int nIndex        = 0;
	char MatchFound   = 0;
	int nMatchIndex    = -1;
	UNUSEDVAR(MatchFound);

	for(nIndex = 0; nIndex < nTotalFaces; nIndex++)
	{
		if(abs(pFaceFeatureHist[nIndex].height - pCurFaceFeature[nRefIndex].height) < 5)
		{
			MatchFound  = 1;
			nMatchIndex = nIndex;
			break;
		}
		else if(abs(pFaceFeatureHist[nIndex].width - pCurFaceFeature[nRefIndex].width) < 5 )
		{
			MatchFound  = 1;
			nMatchIndex = nIndex;
			break;
		}
		else if((pFaceFeatureHist[nIndex].area - pCurFaceFeature[nRefIndex].area) < 5)
		{
			MatchFound  = 1;
			nMatchIndex = nIndex;
			break;
		}
		else if(abs(pFaceFeatureHist[nIndex].length  - pCurFaceFeature[nRefIndex].length) < 5)
		{
			MatchFound  = 1;
			nMatchIndex = nIndex;
			break;
		}
		else if(abs(pFaceFeatureHist[nIndex].centroid.x - pCurFaceFeature[nRefIndex].centroid.x) < 20)
		{
			MatchFound  = 1;
			nMatchIndex = nIndex;
			break;
		}
		else if(abs(pFaceFeatureHist[nIndex].centroid.y - pCurFaceFeature[nRefIndex].centroid.y) < 20)
		{
			MatchFound  = 1;
			nMatchIndex = nIndex;
			break;
		}
	}
	return nMatchIndex;
}


/**************************************************************************************************************************
 *Function: updateHistory
 *Desc:    To update the face features history
 **************************************************************************************************************************/

int updateHistory(feature_t *pFaceFeatureHist, int nMatchIndex, feature_t *pCurFaceFeature, int nIndex)
{
	pFaceFeatureHist[nMatchIndex].isValid    = TRUE;
	pFaceFeatureHist[nMatchIndex].height     = pCurFaceFeature[nIndex].height;
	pFaceFeatureHist[nMatchIndex].width      = pCurFaceFeature[nIndex].width ;
	pFaceFeatureHist[nMatchIndex].area       = pCurFaceFeature[nIndex].area;
	pFaceFeatureHist[nMatchIndex].length     = pCurFaceFeature[nIndex].length;
	pFaceFeatureHist[nMatchIndex].centroid.x = pCurFaceFeature[nIndex].centroid.x;
	pFaceFeatureHist[nMatchIndex].centroid.y = pCurFaceFeature[nIndex].centroid.y;
	pFaceFeatureHist[nMatchIndex].Totaleye   = pCurFaceFeature[nIndex].Totaleye;
	return 0;
}

