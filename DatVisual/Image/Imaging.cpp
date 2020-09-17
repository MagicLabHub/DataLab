#include "stdafx.h"
#include "Imaging.h"

Imaging::Imaging(void)
	:m_iWW(2000)
	,m_iWL(200)
	,m_minValue(0.0)
	,m_maxValue(0.0)
	,m_iPixelNum(0)
	,m_minShowPixel(0)
	,m_maxShowPixel(255)
{
	
}

Imaging::~Imaging(void)
{
	
}

void Imaging::Init(ImagingPara Para){
	m_Para = Para;
}


template<typename T>
void Imaging::UpdateArray(T*img){
	double minT		= m_iWL - 0.5*m_iWW;
	double maxT		= m_iWL + 0.5*m_iWW;
	m_minValue = 0.0;
	m_maxValue = 0.0;
	findMaxMinValue<T>(img,m_minValue,m_maxValue);
	double dThreadShow = (m_maxValue - m_minValue)/(maxT-minT);
	if (dThreadShow<0.1|m_iWW<=0)
	{
		maxT = m_maxValue;
		minT = m_minValue;
	}
	double fScalar	= (m_maxShowPixel - m_minShowPixel)/(maxT-minT);
	for (int i=0;i<m_iPixelNum;++i)
	{
		img[i]	= (img[i] - minT)*fScalar+m_minShowPixel;
	}
}

template<typename T>
void Imaging::Subtract(T*imgInOut,T*imgsrc,size_t nDataNum){
	for (int i=0;i<nDataNum;++i){
		imgInOut[i] -=  imgsrc[i];
	}
}

template<typename T>
void Imaging::findMaxMinValue(T*img,double &minV,double &maxV){
	if (m_iPixelNum>0)
	{
		minV		= 10000000.0;
		maxV		= -minV;
	}
	
	for (int i=0;i<m_iPixelNum;++i)
	{
		if (img[i]>maxV)
		{
			maxV = img[i];
		}
		if (img[i]<minV)
		{
			minV = img[i];
		}	
	}
}

void Imaging::showImage(BYTE*pImg){
	m_iPixelNum		= m_Para.iHeight * m_Para.iWidth;
	m_maxShowPixel	= m_Para.maxPixel;
	m_minShowPixel	= m_Para.minPixel;
	int nImglen = m_iPixelNum*m_Para.nByteStep;

	// 数据调整
	std::shared_ptr<BYTE>pImgbuff(new BYTE[nImglen]);
	memcpy(pImgbuff.get(),pImg,nImglen);
	BYTE*img = pImgbuff.get();
	int type = m_Para.type;
	if (type==0){
		UpdateArray((INT8*)img);
	}else if (type==1){
		UpdateArray((UINT8*)img);
	}else if (type==2){
		UpdateArray((INT16*)img);
	}else if (type==3){
		UpdateArray((UINT16*)img);
	}else if (type==4){
		UpdateArray((INT32*)img);
	}else if (type==5){
		UpdateArray((FLOAT*)img);
	}else if (type==6){
		UpdateArray((DOUBLE*)img);
	}
	// 图像显示
	CRect  rect;
	CDC *pDC = m_Para.pWnd->GetDC();
	m_Para.pWnd->GetClientRect(&rect);
	IplImage* m_CVImg = 0;
	m_CVImg =  cvCreateImage(cvSize(m_Para.iWidth,m_Para.iHeight),m_Para.nByteStep*8,1);
	cvSetData(m_CVImg, img, m_CVImg->widthStep); 
	DrawToHDC( pDC->m_hDC,m_CVImg, &rect); 
	//Sleep(10);
}

void Imaging::creatImage(BYTE*img){

	CRect  rect;
	//cv::namedWindow("srcImg");
	CDC *pDC = m_Para.pWnd->GetDC();
	m_Para.pWnd->GetClientRect(&rect);


	BITMAPINFO* bmi = (BITMAPINFO*)new BYTE[sizeof(BITMAPINFOHEADER) + 256*sizeof(RGBQUAD)];  
	//ZeroMemory(&bmi, sizeof(BITMAPINFO));  

	bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);   
	bmi->bmiHeader.biWidth = m_Para.iWidth;   
	bmi->bmiHeader.biHeight = 0 - m_Para.iHeight;   
	bmi->bmiHeader.biPlanes = 1;   
	bmi->bmiHeader.biBitCount = 8*m_Para.nByteStep;   
	bmi->bmiHeader.biSizeImage = (m_Para.iWidth*bmi->bmiHeader.biBitCount+31)/32*4*m_Para.iHeight; 
	bmi->bmiHeader.biCompression = BI_RGB;  
	// Set Palette
	for(int i=0;i<256;i++)
	{
		bmi->bmiColors[i].rgbRed = (BYTE)i;
		bmi->bmiColors[i].rgbGreen = (BYTE)i;
		bmi->bmiColors[i].rgbBlue = (BYTE)i;
		bmi->bmiColors[i].rgbReserved = (BYTE)0;
	}

	::SetStretchBltMode(pDC->GetSafeHdc(), COLORONCOLOR);
	StretchDIBits(pDC->GetSafeHdc(), 0, 0, rect.Width(), rect.Height(), 0, 0, m_Para.iWidth, m_Para.iHeight, img, bmi, DIB_RGB_COLORS, SRCCOPY); 


	//CImage image;
	//image.Create(m_Para.iWidth, m_Para.iHeight, 8*m_Para.nByteStep);
	//BYTE *pDestData = (BYTE*)image.GetBits();
	//int iDestPitch = image.GetPitch();
	//int iSrcPitch = m_Para.nByteStep*m_Para.iWidth;
	//for(int i = 0; i < m_Para.iHeight; i++)
	//{
	//	memcpy(pDestData + i * iDestPitch, img + i * iSrcPitch, iSrcPitch);
	//} 

	//image.Draw(pDC->m_hDC,rect);
	//image.Destroy();
}

void Imaging::ImageDiff(BYTE*imgInOut,BYTE*imgsrc,size_t nDataNum,int type){
	if (nDataNum==0){
		nDataNum = m_iPixelNum;
		type = m_Para.type;
	}
	if (type==0){
		Subtract((INT8*)imgInOut,(INT8*)imgsrc,nDataNum);
	}else if (type==1){
		Subtract((UINT8*)imgInOut,(UINT8*)imgsrc,nDataNum);
	}else if (type==2){
		Subtract((INT16*)imgInOut,(INT16*)imgsrc,nDataNum);
	}else if (type==3){
		Subtract((UINT16*)imgInOut,(UINT16*)imgsrc,nDataNum);
	}else if (type==4){
		Subtract((INT32*)imgInOut,(INT32*)imgsrc,nDataNum);
	}else if (type==5){
		Subtract((FLOAT*)imgInOut,(FLOAT*)imgsrc,nDataNum);
	}else if (type==6){
		Subtract((DOUBLE*)imgInOut,(DOUBLE*)imgsrc,nDataNum);
	}
}



void Imaging::setWinWidth(int nWinWidth){
	m_iWW = nWinWidth;
}

void Imaging::setWinLocate(int nWinLocate){
	m_iWL = nWinLocate;
}

double Imaging::getMinValue(){
	return m_minValue;
}

double Imaging::getMaxValue(){
	return m_maxValue;
}

void Imaging::DrawToHDC( HDC hDCDst,IplImage* pImg, RECT* pDstRect )
{
	IplImage* IplImg = 0;
	IplImg = cvCreateImage(cvSize(pImg->width,pImg->height),IPL_DEPTH_8U,1);
	IplImg->origin = pImg->origin == 0 ? IPL_ORIGIN_TL : IPL_ORIGIN_BL;
	cvConvertImage( pImg, IplImg, 0 );
	if( pDstRect && IplImg && IplImg->depth == IPL_DEPTH_8U && IplImg->imageData )
	{
		uchar buffer[sizeof(BITMAPINFOHEADER) + 1024];
		BITMAPINFO* bmi = (BITMAPINFO*)buffer;
		int bmp_w = IplImg->width, bmp_h = IplImg->height;
		CvRect roi = cvGetImageROI( IplImg );
		CvRect dst = RectToCvRect( *pDstRect );
		if( roi.width == dst.width && roi.height == dst.height )
		{
			Show( hDCDst,IplImg, dst.x, dst.y, dst.width, dst.height, roi.x, roi.y );
			cvReleaseImage( &IplImg );
			return;
		}
		if( roi.width > dst.width )
		{
			SetStretchBltMode(
				hDCDst,   // handle to device context
				HALFTONE );
		}
		else
		{
			SetStretchBltMode(
				hDCDst,   // handle to device context
				COLORONCOLOR );
		}
		FillBitmapInfo( bmi, bmp_w, bmp_h, Bpp(IplImg), IplImg->origin );
		::StretchDIBits(
			hDCDst,
			dst.x, dst.y, dst.width, dst.height,
			roi.x, roi.y, roi.width, roi.height,
			IplImg->imageData, bmi, DIB_RGB_COLORS, SRCCOPY );
		cvReleaseImage( &IplImg );
	}
}

void Imaging::Show( HDC dc,IplImage* pImg, int x, int y, int w, int h, int from_x, int from_y )
{
	if( pImg && pImg->depth == IPL_DEPTH_8U )
	{
		uchar buffer[sizeof(BITMAPINFOHEADER) + 1024];
		BITMAPINFO* bmi = (BITMAPINFO*)buffer;
		int bmp_w = pImg->width, bmp_h = pImg->height;
		FillBitmapInfo( bmi, bmp_w, bmp_h, Bpp(pImg), pImg->origin );
		from_x = MIN( MAX( from_x, 0 ), bmp_w - 1 );
		from_y = MIN( MAX( from_y, 0 ), bmp_h - 1 );
		int sw = MAX( MIN( bmp_w - from_x, w ), 0 );
		int sh = MAX( MIN( bmp_h - from_y, h ), 0 );
		SetDIBitsToDevice(
			dc, x, y, sw, sh, from_x, from_y, from_y, sh,
			pImg->imageData + from_y*pImg->widthStep,
			bmi, DIB_RGB_COLORS );
	}
}

void Imaging::FillBitmapInfo( BITMAPINFO* bmi, int width, int height, int bpp, int origin )
{
	assert( bmi && width >= 0 && height >= 0 && (bpp == 8 || bpp == 24 || bpp == 32));

	BITMAPINFOHEADER* bmih = &(bmi->bmiHeader);

	memset( bmih, 0, sizeof(*bmih));
	bmih->biSize = sizeof(BITMAPINFOHEADER);
	bmih->biWidth = width;
	bmih->biHeight = origin ? abs(height) : -abs(height);
	bmih->biPlanes = 1;
	bmih->biBitCount = (unsigned short)bpp;
	bmih->biCompression = BI_RGB;
	if( bpp == 8 )
	{
		RGBQUAD* palette = bmi->bmiColors;
		int i;
		for( i = 0; i < 256; i++ )
		{
			palette[i].rgbBlue = palette[i].rgbGreen = palette[i].rgbRed = (BYTE)i;
			palette[i].rgbReserved = 0;
		}
	}
}


CV_INLINE RECT Imaging::NormalizeRect( RECT r )
{
	int t;
	if( r.left > r.right )
	{
		t = r.left;
		r.left = r.right;
		r.right = t;
	}
	if( r.top > r.bottom )
	{
		t = r.top;
		r.top = r.bottom;
		r.bottom = t;
	}

	return r;
}

CV_INLINE CvRect Imaging::RectToCvRect( RECT sr )
{
	sr = NormalizeRect( sr );
	return cvRect( sr.left, sr.top, sr.right - sr.left, sr.bottom - sr.top );
}