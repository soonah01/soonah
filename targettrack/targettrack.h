// targettrack.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�" 
#endif

#include "resource.h"		// ������ 


// CtargettrackApp:
// �йش����ʵ�֣������ targettrack.cpp
//

class CtargettrackApp : public CWinApp
{
public:
	CtargettrackApp();

// ��д
	public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CtargettrackApp theApp;