
// DatVisualDlg.cpp : implementation file
//
#include "stdafx.h"
#include "DatVisual.h"
#include "DatVisualDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CDatVisualDlg dialog




CDatVisualDlg::CDatVisualDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDatVisualDlg::IDD, pParent)
	,m_RowNum(512)
	,m_ColNum(512)
	,m_ImgNum(1)
	,m_WW(2000)
	,m_WL(200)
	,m_eType(TINT8)
	,m_ImgType(0)
	,m_DataBytes(1)
	,m_minShow(0)
	,m_maxShow(255)
	,m_minValue(0)
	,m_maxValue(0)
	,m_bReadData(false)
	,m_ImgIdx(1)
	,m_iLastShowImgIdx(-1)
	,m_PlayFlag(0)
	,m_sImgIdx(_T(""))
	,m_sMinValue(_T(""))
	,m_sMaxValue(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDatVisualDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX,IDC_RowNum,m_RowNum);
	DDX_Text(pDX,IDC_ColNum,m_ColNum);
	DDX_Text(pDX,IDC_ImgNum,m_ImgNum);
	DDX_Text(pDX,IDC_WinWidth,m_WW);
	DDX_Text(pDX,IDC_WinLocate,m_WL);
	DDX_Control(pDX, IDC_FormatComBox, m_FormatComBox);

	DDX_Control(pDX, IDC_ImgIdxSLIDER, m_slider);
	DDX_Text(pDX, IDC_ImgIdxSTATIC, m_sImgIdx);
	DDX_Text(pDX, IDC_minValueSTATIC, m_sMinValue);
	DDX_Text(pDX, IDC_maxValueSTATIC, m_sMaxValue);
	
}

BEGIN_MESSAGE_MAP(CDatVisualDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_STN_CLICKED(IDC_ImgShow, &CDatVisualDlg::OnStnClickedImgshow)
	ON_BN_CLICKED(ID_ReadImage, &CDatVisualDlg::OnBnClickedReadImage)
	ON_BN_CLICKED(ID_Exit, &CDatVisualDlg::OnBnClickedCancel)
	ON_BN_CLICKED(ID_PlayImage, &CDatVisualDlg::OnBnClickedPlayimage)
	ON_EN_CHANGE(IDC_RowNum, &CDatVisualDlg::OnEnChangeRownum)
	ON_EN_CHANGE(IDC_ColNum, &CDatVisualDlg::OnEnChangeRownum)
	ON_EN_CHANGE(IDC_ImgNum, &CDatVisualDlg::OnEnChangeRownum)
	ON_CBN_SELCHANGE(IDC_FormatComBox, &CDatVisualDlg::OnCbnSelchangeFormatcombox)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_ImgIdxSLIDER, &CDatVisualDlg::OnNMCustomdrawImgidxslider)
	
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_BN_CLICKED(ID_CompareImage, &CDatVisualDlg::OnBnClickedCompareimage)
END_MESSAGE_MAP()


// CDatVisualDlg message handlers

BOOL CDatVisualDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	

	Init();
	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CDatVisualDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CDatVisualDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CDatVisualDlg::Init()
{
	m_FormatComBox.AddString(_T("int8"));   //添加字符串  
	m_FormatComBox.AddString(_T("uint8"));   
	m_FormatComBox.AddString(_T("int16"));   
	m_FormatComBox.AddString(_T("uint16"));    
	m_FormatComBox.AddString(_T("int32"));   
	m_FormatComBox.AddString(_T("float32")); 
	m_FormatComBox.AddString(_T("double")); 
	m_FormatComBox.SetCurSel(0);

	m_slider.SetRange(0,1);//设置滑动范围为1到20
	m_slider.SetTicFreq(1);//每1个单位画一刻度
	m_slider.SetPos(0);//设置滑块初始位置为1 

	
}


void CDatVisualDlg::readImg()
{
	m_pImgBuff.clear();
	int iImgPixel		= m_ColNum*m_RowNum;
	SIZE_T iPixelNum	= iImgPixel*m_ImgNum;

	CString m_strFilePathName = _T("D:\\AxialBPUT.dat") ;
	CFileDialog dlg(TRUE,NULL,NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,_T("All Files(*.dat)|*.dat|所有文件(*.*)|"),NULL);
	if (dlg.DoModal())
	{
		m_strFilePathName = dlg.GetPathName();
	}
	//CDatVisualDlg::readData(pImgData,iPixelNum,m_strFilePathName);

	size_t datalen = iPixelNum*m_DataBytes;
	std::shared_ptr<BYTE>pImgData = FileBase::ReadFile<BYTE>(m_strFilePathName,datalen);
	BYTE* ptrImgData = pImgData.get();
	//cv::namedWindow("srcImg");

	//cv::Mat srcImg(m_RowNum,m_ColNum ,m_ImgType, ptrImgData,m_ColNum*2);
	//cv::imshow("srcImg",srcImg);

	size_t szPerImg = m_DataBytes*iImgPixel;
	if (datalen==iPixelNum*m_DataBytes){
		for(int i=0;i<(int)m_ImgNum;++i){
			
			std::shared_ptr<BYTE>pTemp(new BYTE[szPerImg]);
			memcpy(pTemp.get(),ptrImgData,szPerImg);
			m_pImgBuff.push_back(pTemp);
			ptrImgData += szPerImg;
		}
		if (m_ImgNum)
			m_bReadData = true;
	}else{
		m_ImgNum = 0;
		m_bReadData = false;
	}
}

void CDatVisualDlg::compareImg(){

	int iImgPixel		= m_ColNum*m_RowNum;
	SIZE_T iPixelNum	= iImgPixel*m_ImgNum;

	CString m_strFilePathName = _T("D:\\AxialBPUT.dat") ;
	CFileDialog dlg(TRUE,NULL,NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,_T("All Files(*.dat)|*.dat|所有文件(*.*)|"),NULL);
	if (dlg.DoModal())
	{
		m_strFilePathName = dlg.GetPathName();
	}
	size_t datalen = iPixelNum*m_DataBytes;
	size_t szPerImg = m_DataBytes*iImgPixel;
	std::shared_ptr<BYTE>pImgData = FileBase::ReadFile<BYTE>(m_strFilePathName,datalen);
	BYTE* ptrImgData = pImgData.get();
	Imaging Img;
	if (datalen==iPixelNum*m_DataBytes){
		for(int i=0;i<(int)m_ImgNum;++i){

			std::shared_ptr<BYTE>pTemp(new BYTE[szPerImg]);
			memcpy(pTemp.get(),ptrImgData,szPerImg);
			Img.ImageDiff(m_pImgBuff[i].get(),pTemp.get(),iImgPixel,m_ImgType);
			//m_pImgBuff.push_back(pTemp);
			ptrImgData += szPerImg;
		}
		if (m_ImgNum)
			m_bReadData = true;
	}else{
		m_ImgNum = 0;
		m_bReadData = false;
	}
}

void  CDatVisualDlg::showImg(int ImgIdx){
	if (m_bReadData){
		figureLayout();
		Imaging m_Img;
		ImagingPara ImgPara;
		ImgPara.pWnd		= GetDlgItem(IDC_ImgShow);
		ImgPara.iHeight		= m_RowNum;
		ImgPara.iWidth		= m_ColNum;


		ImgPara.minPixel	= m_minShow;
		ImgPara.maxPixel	= m_maxShow;
		ImgPara.nByteStep	= m_DataBytes;
		ImgPara.type		= m_ImgType;
		m_Img.Init(ImgPara);
		m_Img.setWinWidth(m_WW);
		m_Img.setWinLocate(m_WL);
		m_Img.showImage(m_pImgBuff[ImgIdx].get());
		m_minValue			= m_Img.getMinValue();
		m_maxValue			= m_Img.getMaxValue();
	}
}

void CDatVisualDlg::figureLayout(){

	//int cx,cy;
	//cx = GetSystemMetrics(SM_CXSCREEN);
	//cy = GetSystemMetrics(SM_CYSCREEN);

	//CRect rcTemp;
	//rcTemp.TopLeft()		= CPoint(cx/20, cy/20);
	//rcTemp.BottomRight()	= CPoint(cx/20+280+m_RowNum, cy/20+100+m_ColNum);
	//MoveWindow(&rcTemp);
	CRect rect;
	HWND hwnd = AfxGetApp()->GetMainWnd()->GetSafeHwnd();   //获取主窗口句柄
	CWnd* pWnd = CWnd::FromHandle(hwnd);
	//pWnd->GetClientRect(&rect);
	//SetWindowPos(pWnd, 0,0,767,570 ,SWP_NOMOVE);
	pWnd =  GetDlgItem(IDC_ImgShow);
	pWnd->SetWindowPos(NULL,0,0,555,492 ,SWP_NOMOVE);
	pWnd->GetClientRect(&rect);
	if (m_RowNum>m_ColNum){
		//rect.BottomRight()	= CPoint(m_ColNum*rect.right/m_RowNum,rect.bottom);
		rect.BottomRight()	= CPoint(m_ColNum*rect.right/m_RowNum,rect.bottom);
	}else if (m_RowNum<m_ColNum){
		rect.BottomRight()	= CPoint(rect.right,m_RowNum*rect.bottom/m_ColNum);
	}
	pWnd->SetWindowPos(NULL,rect.left,rect.top,rect.right,rect.bottom,SWP_NOMOVE);
	//rect.TopLeft()		= CPoint(0,0);
	//rect.BottomRight()	= CPoint(800, 700);

	//pWnd->SetWindowPos(NULL, rect.left,rect.top,rect.right,rect.bottom,SWP_NOMOVE);
	//pWnd = GetDlgItem(IDC_ImgIdxSLIDER);
	//rect.TopLeft()		= CPoint(10,60+m_ColNum);
	//rect.BottomRight()	= CPoint(10+m_RowNum, 60+m_ColNum);
	//pWnd->SetWindowPos(NULL,rect.left,rect.top,rect.right,rect.bottom,SWP_FRAMECHANGED);
}

void CDatVisualDlg::OnBnClickedReadImage()
{
	
	// TODO: Add your control notification handler code here
	UpdateData(true);
	readImg();
	

	showImg();
	m_sMinValue.Format(_T("min = %.6f "),m_minValue);
	m_sMaxValue.Format(_T("max = %.6f "),m_maxValue);
	UpdateData(false);
	//CDialogEx::OnOK();
}


void CDatVisualDlg::OnStnClickedImgshow()
{
	// TODO: Add your control notification handler code here
}


void CDatVisualDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnCancel();

}


void CDatVisualDlg::OnEnChangeRownum()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}


void CDatVisualDlg::OnCbnSelchangeFormatcombox()
{
	UpdateData(TRUE);                      
	m_eType = (READDATATYPE)m_FormatComBox.GetCurSel(); 
	int nchannel =1;
	switch(m_eType)
	{
		case TINT8:
			m_ImgType = CV_8SC(nchannel);
			m_DataBytes = sizeof(uchar);
			m_minShow	= 0;
			m_maxShow	= 255;
			break;
		case TINT8U:
			m_ImgType = CV_8UC(nchannel);
			m_DataBytes = sizeof(char);
			m_minShow	= -127;
			m_maxShow	= 127;
			break;
		case TINT16:
			m_ImgType = CV_16SC(nchannel);
			m_DataBytes = sizeof(UINT16);
			m_minShow	= -32768;
			m_maxShow	= 32768;
			break;
		case TINT16U:
			m_ImgType = CV_16UC(nchannel);
			m_DataBytes = sizeof(INT16);
			m_minShow	= 0;
			m_maxShow	= 65535;
			break;
		case TINT32:
			m_ImgType = CV_32SC(nchannel);
			m_DataBytes = sizeof(INT32);
			m_minShow	= 0;
			m_maxShow	= 65535;
			break;
		case TFLOAT32:
			m_ImgType	= CV_32FC(nchannel);
			m_DataBytes = sizeof(FLOAT);
			m_minShow	= 0;
			m_maxShow	= 1.0;
			break;
		case TFLOAT64:
			m_ImgType	= CV_64FC(nchannel);
			m_DataBytes = sizeof(DOUBLE);
			m_minShow	= 0;
			m_maxShow	= 1.0;
			break;
		default:
			break;
	}
	UpdateData(false);
}


void CDatVisualDlg::OnNMCustomdrawImgidxslider(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	*pResult = 0;
	UpdateData(TRUE);
	m_slider.SetRange(1,m_ImgNum);//设置滑动范围为1到20
	m_ImgIdx = m_slider.GetPos()-1;
	m_sImgIdx.Format(_T("ImgIdx = %d "),m_ImgIdx+1);
	m_iLastShowImgIdx = m_ImgIdx;
	showImg(m_ImgIdx);
	m_sMinValue.Format(_T("min = %.6f "),m_minValue);
	m_sMaxValue.Format(_T("max = %.6f "),m_maxValue);
	UpdateData(FALSE);
	
}


void CDatVisualDlg::OnBnClickedPlayimage()
{
	UpdateData(TRUE);
	if (m_bReadData){
		m_slider.SetRange(1,m_ImgNum);
		if (m_PlayFlag){
			m_PlayFlag = m_PlayFlag-1;
		}else{
			m_PlayFlag = m_PlayFlag+1; 
		}
		Sleep(10);
		for (int idx = 0;idx<m_ImgNum;idx++){
			int i = idx%m_ImgNum;
			m_slider.SetPos(i);//设置滑块初始位置为1 
			m_sImgIdx.Format(_T("ImgIdx = %d "),i+1);
			showImg(i);
			m_sMinValue.Format(_T("min = %.6f "),m_minValue);
			m_sMaxValue.Format(_T("max = %.6f "),m_maxValue);
			Sleep(100);
		}
	}
	UpdateData(FALSE);
}


void CDatVisualDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	CDialogEx::OnLButtonDown(nFlags, point);
	CString msg;
	//msg.Format(_T("nFlags:%d\n"), nFlags);
	//MessageBox(msg, _T("Left"), MB_OK);
	//msg.Format(_T("ponit:%d %d\n"), point.x,point.y);
	//MessageBox(msg, _T("Left"), MB_OK);
}




void CDatVisualDlg::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CDialogEx::OnRButtonDown(nFlags, point);
	//CString msg;
	//msg.Format(_T("nFlags:%d\n"), nFlags);
	//MessageBox(msg, _T("Right"), MB_OK);
}


void CDatVisualDlg::OnBnClickedCompareimage()
{
	// TODO: Add your control notification handler code here
	UpdateData(true);

	//readImage();
	compareImg();

	showImg();
	m_sMinValue.Format(_T("min = %.6f "),m_minValue);
	m_sMaxValue.Format(_T("max = %.6f "),m_maxValue);
	UpdateData(false);
	//CDialogEx::OnOK();
}
