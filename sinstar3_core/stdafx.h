// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // �� Windows ͷ���ų�����ʹ�õ�����
// Windows ͷ�ļ�:
#include <windows.h>
#include <BaseTyps.h>
#include <Unknwn.h>
#include <tchar.h>


// TODO: �ڴ˴����ó�����Ҫ������ͷ�ļ�
#include "resource.h"

#include "../include/sinstar-i.h"
#include "../include/TextService-i.h"
#include "../include/unknownimpl.h"

#include "sinstar3_core.h"

#define	 DLL_SOUI
#include <souistd.h>
#include <core/SHostDialog.h>
#include <control/SMessageBox.h>
#include <control/souictrls.h>
#include <com-cfg.h>
#include "resource.h"
#define R_IN_CPP	//�������������
#include "res\resource.h"
using namespace SOUI;