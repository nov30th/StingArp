#if !defined(AFX_RECORDDIALOG_H__D774FC6D_F6F9_4867_A07F_50AD8A8D8315__INCLUDED_)
#define AFX_RECORDDIALOG_H__D774FC6D_F6F9_4867_A07F_50AD8A8D8315__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RecordDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRecordDialog dialog

class CRecordDialog : public CDialog
{
// Construction
public:
	CRecordDialog(CWnd* pParent = NULL);   // standard constructor
	void CRecordDialog::LocalUpdate();
// Dialog Data
	//{{AFX_DATA(CRecordDialog)
	enum { IDD = IDD_DIALOGRECORD };
	CString	m_recstring;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRecordDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CRecordDialog)
	afx_msg void OnClose();
	afx_msg void OnButtonclean();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RECORDDIALOG_H__D774FC6D_F6F9_4867_A07F_50AD8A8D8315__INCLUDED_)
