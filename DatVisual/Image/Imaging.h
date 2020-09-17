#pragma once
#ifndef _IMGAE_H__
#define _IMGAE_H__
#include<memory>
#include<Windows.h>
#include "opencv2/opencv.hpp"
#include<atlimage.h>
#include "../DataBase.h"

struct ImagingPara
{
	CWnd *pWnd;
	int iWidth;
	int iHeight;
	int nByteStep;
	int type;
	double minPixel;
	double maxPixel;
	ImagingPara()
		:iWidth(0)
		,iHeight(0)
		,nByteStep(0)
		,type(0)
		,minPixel(0)
		,maxPixel(0)
	{

	}
};

class Imaging
{
public:
	Imaging(void);
	~Imaging(void);

	void Init(ImagingPara Para);

	void showImage(BYTE*img);

	void creatImage(BYTE*img);

	void setWinWidth(int nWinWidth);

	void setWinLocate(int nWinLocate);

	double getMinValue();
	
	double getMaxValue();

	void showImageNew(
		CWnd *pWnd,BYTE*img,
		int iWidth,int iHeight,
		double minPixel,double maxPixel,int type);

	void ImageDiff(BYTE*imgInOut,BYTE*imgsrc,size_t nDataNum=0,int type=0);

private:
	void UpdateMat();

	void DrawToHDC( HDC hDCDst,IplImage* pImg, RECT* pDstRect );

	void Show( HDC dc,IplImage* pImg, int x, int y, int w, int h, int from_x, int from_y );
	
	void FillBitmapInfo( BITMAPINFO* bmi, int width, int height, int bpp, int origin );

	CvRect RectToCvRect( RECT sr );
	CV_INLINE RECT NormalizeRect( RECT r );
	CV_INLINE int Bpp(IplImage* pImg) { return pImg ? (pImg->depth & 255)*pImg->nChannels : 0; };

private:
	template<typename T>
	void UpdateArray(T*img);
	template<typename T>
	void findMaxMinValue(T*img,double &minV,double &maxV);
	template<typename T>
	void Subtract(T*imgInOut,T*imgsrc,size_t nDataNum);
private:
	ImagingPara m_Para;
	double m_iWW;
	double m_iWL;
	double m_minValue;
	double m_maxValue;
	int m_iPixelNum;
	double m_minShowPixel;
	double m_maxShowPixel;
	cv::Mat m_srcImg;
	
};

#endif	_IMGAE_H__

