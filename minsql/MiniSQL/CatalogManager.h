
#ifndef CATALOGMANAGER_H
#define CATALOGMANAGER_H
#include "stdafx.h"
#include <string>
#include <vector>
using namespace std;
class CatalogManager{
public:
	struct Index
	{
		string indexName;              //���������
		string searchTable;            //���������Ӧ�ı���
		string searchKey;              //���������Ӧ�Ĺؼ�����
	};
	struct Table
	{
		string tableName;               //����
		int recordNum;                  //��ż�¼��Ŀ
		int collNum;                    //�����ֶ���Ŀ
		int recordLen;                  //�����¼����
		string primKey;                 //�������
		vector<string> collName;        //��Ŷ�Ӧ���ֶ���
		vector<string> collType;        //��Ŷ�Ӧ���ֶ�����
		vector<int> collUniq;           //��Ŷ�Ӧ���ֶ��Ƿ�Ωһ
		vector<string> collIndex;       //��Ŷ�Ӧ�ֶ��ϵ�������Ϣ
	};
	 
	int tableNum;                       //����Ŀ
	int indexNum;                       //������Ŀ
	vector<Table> tv;                   //���б�
	vector<Index> iv;                   //�����б�


	CatalogManager();
	~CatalogManager() { }

   

	//�������غ����������ڴ�����ʱ����,���ֵ��м����´��������Ϣ
	int addTable(string tableName,vector<string> col,vector<string> type,vector<int> uniq);
	int addTable(string tableName,vector<string> col,vector<string> type,vector<int> uniq,string primKey);
	
	//������������ʱ���ã����ֵ��м����´�����������Ϣ
	int addIndex(string indexName,string tableName,string colName);
	

	//���ֵ��ļ��еļ�¼����recordNum������insert values
	int insertRecord(string tableName,int recordNum);
	
	//ɾ��һ������ֵ���Ϣ
	int dropTable(string tableName);

	//ɾ��һ���������ֵ���Ϣ
	int dropIndex(string index);

	//���ֵ��ļ���ɾ����¼,��һ�����غ���ɾ��ȫ����¼��recordNum��Ϊ0��
	//�ڶ������غ���ɾ��deleteNum����¼����deleteNum����
	int deleteValue(string tableName);
	int deleteValue(string tableName,int deleteNum);

	//�����ļ���Ϣ���Ƿ�Ϊ�������߱�������ߣ���������Ⱥͼ�¼��
	 void getFileInfo(string fileName,int * n);
	
	//�����ļ��������ļ����ͻ����Ƿ�������ļ�
	int findFile(string fileName);

	//�ж��ڶ�Ӧ���Ӧ�ֶ����Ƿ�������
	int existIndex(string tableName,string col);

	//���ڴ��е��ֵ���Ϣ�����ļ�
	int writeBack();

	//�������غ������ڲ��Ҽ�¼�ĳ���
	int calcuteLenth(string tableName);
	int calcuteLenth(vector<string> v);
	int calcuteLenth2(string type);
	
	//���ر�ļ�¼��Ŀ
	int getRecordNum(string tableName);

	//�����ֶ����б�
	vector<string> getCollName(string tableName);

	//�����ֶ������б�
	vector<string> getCollType(string tableName);

	//�����ֶ��Ƿ�Ωһ�б�
	vector<int> getCollUniq(string tableName);

	//�����ֶ������б�
	vector<string> getCollIndex(string tableName);

	//��������
	string getPrimaryKey(string tableName);
};
#endif
