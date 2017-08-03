// ArpCheaterDlg.h : header file
//

#include "afxwin.h"
#if !defined(AFX_ARPCHEATERDLG_H__FBF3B6A5_D420_4397_A0B3_C0AFA870203E__INCLUDED_)
#define AFX_ARPCHEATERDLG_H__FBF3B6A5_D420_4397_A0B3_C0AFA870203E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CArpCheaterDlg dialog
UINT SendArpPacket(LPVOID lpParam );
UINT ProtectMy(LPVOID lpParam);
UINT ReboundArpAttack(LPVOID lpParam);
UINT GetCorrectMac(LPVOID lpParam);
UINT StartArpMonitor(LPVOID lpParam);
UINT AlermIconFlash(LPVOID lpParam);
UINT GetMyMac(LPVOID lpParam);
//CString	m_info;
class CArpCheaterDlg : public CDialog
{
    // Construction
public:
    CArpCheaterDlg(CWnd* pParent = NULL);	// standard constructor
    int  CArpCheaterDlg::InitRemoteControl();
    // Dialog Data
    //{{AFX_DATA(CArpCheaterDlg)
    enum { IDD = IDD_ARPCHEATER_DIALOG };
    CListBox	m_listip;
    CButton	m_autodetect;
    CComboBox	m_adapter;
	UINT	m_sendinterval;
	CString	m_oneip;
	CString	m_filepath;
	CString	m_localmac;
	CString	m_gatewaymac;
	CString	m_gatewayip;
	CString	m_forgemac;
	CString	m_localip;
	int		m_radioiplist;
	int		m_radioactiontype;
	CString	m_netmask;
	BOOL	m_wirelessadapter;
	CString	m_info;
	CString	m_reboundip;
	CString	m_reboundmac;
	CString	m_arpmonitor;
	BOOL	m_record_arp_packet;
	//}}AFX_DATA
	
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CArpCheaterDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL
	
	// Implementation
protected:
	HICON m_hIcon;
	
	// Generated message map functions
	void SetAdapter();
	bool InitArp();
	bool LoadIPFileToIPList(bool isall);
	bool LoadDestineComputers(bool isall);
    char *iptos(unsigned long in);
	void SaveIpToFile(bool isAuto);
	void OnDestroy();
	void SetAutoRun(bool isactive);
	void AssertValid() const;
    

	//{{AFX_MSG(CArpCheaterDlg)
	virtual BOOL OnInitDialog();
	afx_msg LRESULT onShowTask(WPARAM wParam,LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButtonauto();
	afx_msg void OnSelchangeComboadapter();
	afx_msg void OnButtonsave();
	afx_msg void OnButtonload();
	afx_msg void OnButtonstart();
	afx_msg void OnButtonbrowsefile();
	afx_msg void OnChangeEdittcip();
	afx_msg void OnButtonprotect();
	afx_msg void OnButtonrebound();
	afx_msg void OnButtongetmac();
	afx_msg void OnButtoncleaninfo();
	afx_msg void OnButtonstopmonitor();
	afx_msg void OnButtonstartmonitor();
	afx_msg void OnWirelessLan();
	afx_msg void OnMenuitemexit();
	afx_msg void OnClose();
	afx_msg void OnMenuitemmore();
	afx_msg void OnButtonlistallip();
	afx_msg void OnButtonhide();
	afx_msg void OnMenuitemtrustip();
	afx_msg void OnMenuitemconfig();
	afx_msg void OnMenuitemhelp();
	afx_msg void OnCheckrecordarp();
	afx_msg void OnButtonseepacket();
	afx_msg void OnMenuitem32777();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedBtntest();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ARPCHEATERDLG_H__FBF3B6A5_D420_4397_A0B3_C0AFA870203E__INCLUDED_)
