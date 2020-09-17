
// DatVisual.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#include "opencv2/opencv.hpp"		// Img

// CDatVisualApp:
// See DatVisual.cpp for the implementation of this class
//

class CDatVisualApp : public CWinApp
{
public:
	CDatVisualApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CDatVisualApp theApp;