#if !defined(AFX_DIALOGCONFIG_H__3E8B412D_0D73_4118_93B1_CDB3D3E6B8A4__INCLUDED_)
#define AFX_DIALOGCONFIG_H__3E8B412D_0D73_4118_93B1_CDB3D3E6B8A4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DialogConfig.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDialogConfig dialog

class CDialogConfig : public CDialog
{
// Construction
public:
	CDialogConfig(CWnd* pParent = NULL);   // standard constructor
	void Reset();

// Dialog Data
	//{{AFX_DATA(CDialogConfig)
	enum { IDD = IDD_DIALOGCONFIG };
	BOOL	m_autoprotect;
	BOOL	m_autorun;
	BOOL	m_startmini;
	CString	m_path;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDialogConfig)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDialogConfig)
	virtual void OnOK();
	afx_msg void OnCheckautorun();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	bool saveoption[3];
	CString configfilePath;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DIALOGCONFIG_H__3E8B412D_0D73_4118_93B1_CDB3D3E6B8A4__INCLUDED_)
