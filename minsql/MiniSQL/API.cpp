
#include "stdafx.h"
#include "API.h"
#include "my_catalog_manager(1).h"
#include "BPlusTree.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#define UNKNOWN_FILE 8                                 //�����궨�壬���ڶ����ļ�����
#define TABLE_FILE 9   
#define INDEX_FILE 10

using namespace std;
class Catalog_Manager;
class RecordManager;


extern Catalog_Manager cm;                             //��ȫ�ֶ�����ⲿ����

//��ӡһ���������еļ�¼�ʹ�ӡ�ļ�¼��
void API::printRecord(string tableName)               
{
	//�����ֵ���Ϣ������ļ����棬ֱ�ӳ�����ʾ
	if(cm.FindFile(tableName)==TABLE_FILE)
	{
		int m;
		m=rm.selectRecord(tableName);
		cout<<m<<" records selected"<<endl;
	}
	else
		cout<<"There is no table "<<tableName<<endl;
}

//����n��where������ӡ���еļ�¼�ʹ�ӡ�ļ�¼��
void API::printRecord(string tableName, vector<COND> x)
{
	//�����ֵ���Ϣ������ļ����棬ֱ�ӳ�����ʾ
	if(cm.FindFile(tableName)==TABLE_FILE)
	{
		int m;
		m=rm.selectRecord(tableName,x);
		cout<<m<<" records selected"<<endl;
	}
	else
		cout<<"There is no table "<<tableName<<endl;
}



//����в������¼
void API::insertRecord(string tableName,vector<string> v)
{
	//�����ֵ���Ϣ������ļ����棬ֱ�ӳ�����ʾ
	if(cm.FindFile(tableName)!=TABLE_FILE)
	{
		cout<<"ERROR:There is no table "<<tableName<<endl;
		return ;
	}
		vector<string> type;
		type=cm.getCollType(tableName);

		//s������Ϊ��¼����ʱ�������
		char s[2000];
		memset(s,0,2000);
		char *p;
		int pos=0;
		for(unsigned i=0;i<v.size();i++)
		{
			//������ͳ�����ʱ����ĳ���
			if(cm.CalLength(type.at(i))+pos>2000)
			{
				cout<<"Failed to insert. The record is too long"<<endl;
				break;
			}
			//��������ͱ�����������תΪchar��,��char���ֽ���ʽ��������
			if(type.at(i)=="int")
			{
				int_m t;
				t.value=atoi(v.at(i).c_str());
				p=(char *)&t;
				for(int j=0;j<sizeof(int);j++,pos++)
				{
					s[pos]=*p;
					p++;
				}
			}
			//�����float�α�������floatתΪchar�ͣ���char���ֽ���ʽ��������
			else if(type.at(i)=="float")
			{
				float tt;
				tt=(float)atof(v.at(i).c_str());
				p=(char *)&tt;
				memcpy(s + pos, p, sizeof(float));
				pos += sizeof(float);
			}
			//������ַ��͵ı�����ֱ�ӽ����ֽڴ洢
			else
			{			
				//���ʵ�������string���ڶ����char���ȣ��򱨴�
				if(v.at(i).length()>(unsigned int)cm.CalLength(type.at(i)))
				{
					cout<<"Insert Failed. The string of "<<v.at(i)<<" is too long"<<endl;
					break;
				}
				const char * cp;
				cp=v.at(i).c_str();
				for(unsigned int j=0;j<v.at(i).length();j++,pos++)
				{
					s[pos]=*cp;
					cp++;
				}
				//�����Ƕ����洢�������ж��峬��������0�洢.
				for(unsigned int j=v.at(i).length();j<(unsigned int)cm.CalLength(type.at(i));j++,pos++)
					;

			}
		}

		//����������в����¼�ɹ�����ͬ�����ڴ��ֵ���Ϣ�в����¼��Ϣ
		if(rm.insertRecord(tableName,s))
		{
			cm.InsertRecord(tableName,1);
		}
		//printRecord(tableName);
}

//�����еļ�¼ȫ��ɾ��,ͬʱ���ɾ���ļ�¼��Ŀ
void API::deleteValue(string tableName)
{
	//�����ֵ���Ϣ������ļ����棬ֱ�ӳ�����ʾ
	if(cm.FindFile(tableName)!=TABLE_FILE)
	{
		cout<<"ERROR:There is no table "<<tableName<<endl;
		return ;
	}
	//������RecordManager��ɾ����¼��Ȼ���������ֵ��н���¼����Ϊ0
	int num=rm.deleteValue(tableName);
	if(cm.DeleteRecord(tableName))
		cout<<"Delete "<<num<<" records "<<"in "<<tableName<<endl;
	else
		cout<<"ERROR:Fail to delete in table "<<tableName<<endl;
	printRecord(tableName);
}

//����n��where����ɾ�����еļ�¼��ͬʱ���ɾ����¼����Ŀ
void API::deleteValue(string tableName, vector<COND> x)
{
	//�����ֵ���Ϣ������ļ����棬ֱ�ӳ�����ʾ
	if(cm.FindFile(tableName)!=TABLE_FILE)
	{
		cout<<"ERROR:There is no table "<<tableName<<endl;
		return ;
	}
	int num=rm.deleteValue(tableName,x);
	if(cm.DeleteRecord(tableName,num))
		cout<<"Delete "<<num<<" records "<<"in "<<tableName<<endl;
	else
		cout<<"ERROR:Fail to delete in table "<<tableName<<endl;
	printRecord(tableName);
}




//�����ļ�������������ļ��ļ�¼��Ŀ
int API::getRecordNum(string tableName)
{
	return cm.getRecordNum(tableName);
}

//�����ļ������б�
vector<string> API::getCollName(string tableName)
{
	return cm.getCollName(tableName);
}

//�����ļ����������б�
vector<string> API::getCollType(string tableName)
{
	return cm.getCollType(tableName);
}

//���ݱ����������ļ��еļ�¼����
int API::calcuteLenth(string tableName)
{
	return cm.CalLength2(tableName);
}

//����ĳ���������ͣ���������������͵ĳ���
int API::calcuteLenth2(string type)
{
	return cm.CalLength(type);
}

//ɾ��һ����
void API::dropTable(string tableName)
{
	//�����ֵ���Ϣ������ļ����棬ֱ�ӳ�����ʾ
	if(cm.FindFile(tableName)!=TABLE_FILE)
	{
		cout<<"ERROR:There is no table "<<tableName<<endl;
		return ;
	}
	//ֱ�����ֵ���Ϣ�н���ɾ��
	if(cm.DropTable(tableName))
		cout<<"Drop table "<<tableName<<" successfully"<<endl;
}

//ɾ��һ������
void API::dropIndex(string indexName)
{
	//�����ֵ���Ϣ������������棬ֱ�ӳ�����ʾ
	if(cm.FindFile(indexName)!=INDEX_FILE)
	{
		cout<<"ERROR:There is no index "<<indexName<<endl;
		return ;
	}
	if(cm.DropIndex(indexName))
		cout<<"Drop index "<<indexName<<" successfully"<<endl;
}

int SizeOf(string s)
{
	if (s == "int") return sizeof(int);
	if (s == "float") return sizeof(float);

	int i = 5;
	string num;
	int number;
	while (s[i] != ')')
	{
		num.push_back(s[i]);
		i++;
	}
	number = atoi(num.c_str());
	return number;
}

//���ݸ����ı����ԣ���������
void API::createIndex(string fileName, string tableName, string colName)
{
	//�����ֵ���Ϣ������������ڣ�ֱ�ӳ�����ʾ
	if (cm.FindFile(fileName) == INDEX_FILE)
	{
		cout << "ERROR:There is index " << fileName << " already" << endl;
		return;
	}
	if (getPrimaryKey(tableName) == "no" || getPrimaryKey(tableName) != colName)
	{
		cout << "ERROR:not primary key" << endl;
		return;
	}
	//��IndexManager����������
	vector<string> column = cm.getCollType(tableName);
	vector<string> columnfuck = getCollName(tableName);

	int preSize = 0;
	int TotalSize = 3 * sizeof(char) + sizeof(int);
	int i = 0;
	for (unsigned i = 0; i<column.size(); i++)
	{
		if (column[i] == cm.getPrimaryKey(tableName))
			break;
		else
		{
			preSize += SizeOf(column[i]);
			TotalSize += SizeOf(column[i]);
		}
	}
	for (unsigned int j = i; j<column.size(); j++)
		TotalSize += SizeOf(column[j]);

	int RecordNum = cm.getRecordNum(tableName);
	char data[1000][100];
	if (cm.getPrimaryKey(tableName) == "int")
	{
		int key[1000];
		ifstream fin(tableName, std::ios::binary);
		if (fin.is_open())
		{
				fin.seekg(0, ios::beg);
			for (int i = 0; i<RecordNum; i++)
			{
				fin.read((char*)&data[i], preSize);
				fin.read((char*)&key[i], SizeOf(getPrimaryKey(tableName)));
				fin.read((char*)((&data[i]) + preSize), TotalSize - preSize - SizeOf(cm.getPrimaryKey(tableName)));
			}
			fin.close();
		}
		Tree<int, char*> index_tree;
		for (int i = 0; i<RecordNum; i++)
			index_tree.insert(key[i], *(data + i));
	}
	else if (cm.getPrimaryKey(tableName) == "float")
	{
		float key[1000];
		ifstream fin(tableName, std::ios::binary);
		if (fin.is_open())
		{
			fin.seekg(0, ios::beg);
			for (int i = 0; i<RecordNum; i++)
			{
				fin.read((char*)&data[i], preSize);
				fin.read((char*)&key[i], SizeOf(getPrimaryKey(tableName)));
				fin.read((char*)((&data[i]) + preSize), TotalSize - preSize - SizeOf(cm.getPrimaryKey(tableName)));
			}
			fin.close();
		}
		Tree<float, char*> index_tree;
		for (int i = 0; i<RecordNum; i++)
			index_tree.insert(key[i], *(data + i));
	}
	/*else
	{
		string key[1000];
		ifstream fin(tableName, std::ios::binary);
		if (fin.is_open())
		{
			fin.seekg(0, ios::beg);
			for (int i = 0; i<RecordNum; i++)
			{
				fin.read((char*)&data[i], preSize);
				fin.read((char*)&key[i], SizeOf(getPrimaryKey(tableName)));
				fin.read((char*)((&data[i]) + preSize), TotalSize - preSize - SizeOf(cm.getPrimaryKey(tableName)));
			}
			fin.close();
		}
		Tree<string,char*> index_tree;
		for (int i = 0; i<RecordNum; i++)
			index_tree.insert(key[i], *(data + i));
	}
*/

	//���ֵ���Ϣ����������
	cm.AddIndex(fileName, tableName, colName);
	cout << "Create index " << fileName << " successfully" << endl;
}
//�����С���������Ϣ������
void API::createTable(string tableName,vector<string> col,vector<string> type,vector<int> uniq,string primKey)
{
	//�����ֵ���Ϣ��������Ѿ����ڣ�ֱ�ӳ�����ʾ
	if(cm.FindFile(tableName)==TABLE_FILE)
	{
		cout<<"ERROR:There is table "<<tableName<<" already"<<endl;
		return ;
	}
	if (cm.FindFile(tableName) != UNKNOWN_FILE)
	{
		cout << "ERROR:Fail to add table, for the file is UNKNOWN" << endl;
		return ;
	}
	if(cm.AddTable(tableName,col,type,uniq,primKey))
		cout<<"Create table "<<tableName<<" successfully"<<endl;
}


//�����������������������һ��ֵ�Լ�������¼�������ļ��еĴ�λ�ã������ֵ���뵽�������ļ���
//���е�λ���ǿ�ƫ�ƺͼ�¼�ڿ��е�λ��
int API::insertIndexItem(string fileName,string colName,string value,int block,int index)
{

	return 1;
}

//�����������������Լ�������ϵ�һ��ֵ������������¼�������ļ��ж�Ӧ��λ��
int API::getIndexItem(string fileName,string colName,string value,int * block,int * index)
{
	return 1;
}

string API::getPrimaryKey(string tableName)
{
	//�����ֵ���Ϣ��������Ѿ����ڣ�ֱ�ӳ�����ʾ
	if (cm.FindFile(tableName) != TABLE_FILE)
	{
		cout << "ERROR:There is table " << tableName << " already" << endl;
		return "FUCK";
	}
	else
	{
		return cm.getPrimaryKey(tableName);
	}
}

int API::FindFile(string tableName)
{
	return cm.FindFile(tableName);
}

void API::GetFileInfo(string fileName, int * n)
{
	return cm.getFileInfo(fileName,n);
}
vector<string> API:: getUniq(string tableName)
{
	return cm.getUniq(tableName);
}