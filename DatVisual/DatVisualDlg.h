#include<iostream>
#include<Windows.h>
#include<cstdlib>
#include <fstream>
#include <tchar.h>
#include "afxwin.h"
#include "DataBase.h"		// Img
#include "Image/Imaging.h"
#include "afxcmn.h"
// DatVisualDlg.h : header file
//

#pragma once



// CDatVisualDlg dialog
class CDatVisualDlg : public CDialogEx
{
// Construction
public:
	CDatVisualDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_DATVISUAL_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedReadImage();
	afx_msg void OnStnClickedImgshow();
	afx_msg void OnBnClickedCancel();

private:
	void Init();

	void readImg();

	void compareImg();

	void showImg(int ImgIdx=0);

	void plotFigure();

	void figureLayout();

private:
	DWORD m_RowNum;
	DWORD m_ColNum;
	DWORD m_ImgNum;
	DWORD m_WW;
	DWORD m_WL;
	READDATATYPE m_eType;
	int		m_ImgType;
	int		m_DataBytes;
	CComboBox m_FormatComBox;
	double m_minShow;
	double m_maxShow;
	double m_minValue;
	double m_maxValue;
	std::vector<std::shared_ptr<BYTE>>m_pImgBuff;
public:
	afx_msg void OnEnChangeRownum();
	afx_msg void OnCbnSelchangeFormatcombox();
	afx_msg void OnNMCustomdrawImgidxslider(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedPlayimage();
public:
	
	CSliderCtrl m_slider;
	bool m_bReadData;
	int m_ImgIdx;
	int m_iLastShowImgIdx;
	int m_PlayFlag;
	CString m_sImgIdx;
	CString m_sMinValue;
	CString m_sMaxValue;

	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnBnClickedCompareimage();
};
