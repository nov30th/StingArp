// ConfigInfo.h: interface for the CConfigInfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CONFIGINFO_H__433E2420_DA37_4486_96FE_02E7DBC86B59__INCLUDED_)
#define AFX_CONFIGINFO_H__433E2420_DA37_4486_96FE_02E7DBC86B59__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CConfigInfo  
{
public:
	CConfigInfo();
	virtual ~CConfigInfo();
	bool LoadFile();
	bool SaveFile();
	bool CreateFile();

public:
	int saveoption[3];
	CString configfilePath;
	unsigned long trustip;
};

#endif // !defined(AFX_CONFIGINFO_H__433E2420_DA37_4486_96FE_02E7DBC86B59__INCLUDED_)
