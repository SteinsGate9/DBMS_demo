
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
	//ɾ����
	void dropTable(string tableName);
	//ɾ��index
	void dropIndex(string indexName);
	//��������
	void createIndex(string fileName,string tableName,string colName);
	//������
	void createTable(string tableName,vector<string> col,vector<string> type,vector<int> uniq,string primKey);
	//��ӡ����
	void printRecord(string tableName);
	//��ӡ��������
	void printRecord(string tableName,vector<COND>);
    //��������
	void insertRecord(string tableName,vector<string> v);
	//ɾ����������
	void deleteValue(string tableName);
	//ɾ����������
	void deleteValue(string tableName, vector<COND>);
	//�����ļ���
	int FindFile(string tableName);
	//�����ļ���Ϣ
	void GetFileInfo(string fileName, int * n);
	//�����ļ������ݶ���
	int getRecordNum(string tableName);
	//�����¼����
	int calcuteLenth(string tableName);
	//�����ֶγ���
	int calcuteLenth2(string type);
	//�õ��ֶ�����
	vector<string> getCollName(string tableName);
	//�õ��ֶ���������
	vector<string> getCollType(string tableName);
	//�õ�����
	string         getPrimaryKey(string tableName);
	//�õ�Unique�ֶ�
	vector<string> getUniq(string tableName);
	//����
	int insertIndexItem(string fileName, string colName, string value, int block, int index);
	int getIndexItem(string fileName, string colName, string value, int * block, int * index);
};

//���������ݽṹ���ڰ�int�ͣ�float���������ֽ�ת�������㽫�䶨���洢
struct int_m{
	int value;
};
struct float_m{
	float value;
};
#endif