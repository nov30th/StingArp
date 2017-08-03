// SickIP.cpp : implementation file
//

#include "stdafx.h"
#include "ArpCheater.h"
#include "SickIP.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSickIP

unsigned long ips[1000];
unsigned long requests[1000];
long currectIpMax = 0;

CSickIP::CSickIP()
{
}

CSickIP::~CSickIP()
{
}


BEGIN_MESSAGE_MAP(CSickIP, CWnd)
	//{{AFX_MSG_MAP(CSickIP)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CSickIP message handlers

int CSickIP::AddIP(unsigned long ip)
{
    int i = 0;
	for (; i <currectIpMax ;i++)
	{
		//相同IP
		if (ips[i] == ip)
		{
			requests[i]++;
		}
	}
	//没有记录则添加
	if (i == currectIpMax)
	{
		ips[i] = ip;
		requests[i] = 0;
		currectIpMax++;
	}
	return 1;
}