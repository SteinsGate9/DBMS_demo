/************************些文件包括MiniSQL中RecordManager类的定义********************************************/
/************************作者:黄士诚  时间：2018年6月*******************************************************/

#pragma once
#include "stdafx.h"
#include <string>
#include <vector>
using namespace std;
class RecordManager{
public:
	RecordManager(){}
	~RecordManager(){}
	//查找所有记录
	int selectRecord(string tableName);
	//由于是顺序存放，所以当一条where条件的时候，可以优化
	int selectRecord(string tableName,string colName1,string cond1,string operater1);
	//多个where条件时候无法优化
	int selectRecord(string tableName,vector<COND> x);
	//删除所有记录
	int deleteValue(string tableName);
	//由于是顺序存放，所以当一条where条件的时候，可以优化
	int deleteValue(string tableName, string colName1,string cond1,string operater1);
	//多个where条件无法优化
	int deleteValue(string tableName, vector<COND> x);
	//插入记录
	int insertRecord(string tableName,char * s);
	//条件检查器
	int condCheck(string type,char * value,string scond,string operater);
	//大于返回1
	int compare(string type, const char * value, const char * value2);
};
