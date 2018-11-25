
#ifndef API_H
#define API_H
#include "RecordManager.h"
#include <string>
#include <vector>
using namespace std;
class IndexManager;
class RecordManager;
class API{
public:
	RecordManager rm;
	API(){}
	~API(){}
	//删除表
	void dropTable(string tableName);
	//删除index
	void dropIndex(string indexName);
	//建立索引
	void createIndex(string fileName,string tableName,string colName);
	//建立表
	void createTable(string tableName,vector<string> col,vector<string> type,vector<int> uniq,string primKey);
	//打印数据
	void printRecord(string tableName);
	//打印条件数据
	void printRecord(string tableName,vector<COND>);
    //插入数据
	void insertRecord(string tableName,vector<string> v);
	//删除所有数据
	void deleteValue(string tableName);
	//删除条件数据
	void deleteValue(string tableName, vector<COND>);
	//查找文件块
	int FindFile(string tableName);
	//查找文件信息
	void GetFileInfo(string fileName, int * n);
	//返回文件中数据多少
	int getRecordNum(string tableName);
	//计算记录长度
	int calcuteLenth(string tableName);
	//计算字段长度
	int calcuteLenth2(string type);
	//得到字段数组
	vector<string> getCollName(string tableName);
	//得到字段类型数组
	vector<string> getCollType(string tableName);
	//得到主键
	string         getPrimaryKey(string tableName);
	//得到Unique字段
	vector<string> getUniq(string tableName);
	//插入
	int insertIndexItem(string fileName, string colName, string value, int block, int index);
	int getIndexItem(string fileName, string colName, string value, int * block, int * index);
};

//这两个数据结构用于把int型，float型数据向字节转换，方便将其定长存储
struct int_m{
	int value;
};
struct float_m{
	float value;
};
#endif