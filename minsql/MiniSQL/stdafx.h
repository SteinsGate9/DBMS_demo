// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#include <algorithm>
#include <iostream>
#include <vector>
typedef struct COND
{
	std::string colName;
	std::string cond;
	std::string operater;
	COND(std::string a, std::string b, std::string c) :colName(a), cond(b),operater(c) {}
	COND(){}
}COND;

// TODO: �ڴ˴����ó�����Ҫ������ͷ�ļ�
