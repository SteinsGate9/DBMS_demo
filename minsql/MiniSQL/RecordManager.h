/************************Щ�ļ�����MiniSQL��RecordManager��Ķ���********************************************/
/************************����:��ʿ��  ʱ�䣺2018��6��*******************************************************/

#pragma once
#include "stdafx.h"
#include <string>
#include <vector>
using namespace std;
class RecordManager{
public:
	RecordManager(){}
	~RecordManager(){}
	//�������м�¼
	int selectRecord(string tableName);
	//������˳���ţ����Ե�һ��where������ʱ�򣬿����Ż�
	int selectRecord(string tableName,string colName1,string cond1,string operater1);
	//���where����ʱ���޷��Ż�
	int selectRecord(string tableName,vector<COND> x);
	//ɾ�����м�¼
	int deleteValue(string tableName);
	//������˳���ţ����Ե�һ��where������ʱ�򣬿����Ż�
	int deleteValue(string tableName, string colName1,string cond1,string operater1);
	//���where�����޷��Ż�
	int deleteValue(string tableName, vector<COND> x);
	//�����¼
	int insertRecord(string tableName,char * s);
	//���������
	int condCheck(string type,char * value,string scond,string operater);
	//���ڷ���1
	int compare(string type, const char * value, const char * value2);
};
