// ConfigInfo.cpp: implementation of the CConfigInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ArpCheater.h"
#include "ConfigInfo.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define CONFIGFILENAME "QzjConfigArp.cfg" //≈‰÷√Œƒº˛√˚
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CConfigInfo::CConfigInfo()
{

}

CConfigInfo::~CConfigInfo()
{

}

bool CConfigInfo::LoadFile()
{
	char temp[101];
	FILE *fp;
	fp = fopen(CONFIGFILENAME,"r");
	if (fp==NULL)
		return false;
	fgets(temp,100,fp);
	fclose(fp);
	sscanf(temp,"%d;%d;%d;%ld;%s",&saveoption[0],&saveoption[1],&saveoption[2],&trustip);
	return true;
}

bool CConfigInfo::CreateFile()
{
	FILE *fp;
	fp = fopen(CONFIGFILENAME,"w");
	if (fp==NULL)
		return false;
	fputs("0;0;0;0;",fp);
	fclose(fp);
	return true;
}


bool CConfigInfo::SaveFile()
{
	char temp[101];
	FILE *fp;
	fp = fopen(CONFIGFILENAME,"w");
	if (fp==NULL)
		return false;
	sprintf(temp,"%d;%d;%d;%ld;",saveoption[0],saveoption[1],saveoption[2],trustip);
	fputs(temp,fp);
	fclose(fp);
	return true;
}