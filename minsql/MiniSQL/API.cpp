
#include "stdafx.h"
#include "API.h"
#include "my_catalog_manager(1).h"
#include "BPlusTree.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#define UNKNOWN_FILE 8                                 //三个宏定义，用于定义文件类型
#define TABLE_FILE 9   
#define INDEX_FILE 10

using namespace std;
class Catalog_Manager;
class RecordManager;


extern Catalog_Manager cm;                             //对全局对象的外部声明

//打印一个表中所有的记录和打印的记录数
void API::printRecord(string tableName)               
{
	//查找字典信息，如果文件不存，直接出错提示
	if(cm.FindFile(tableName)==TABLE_FILE)
	{
		int m;
		m=rm.selectRecord(tableName);
		cout<<m<<" records selected"<<endl;
	}
	else
		cout<<"There is no table "<<tableName<<endl;
}

//根据n个where条件打印表中的记录和打印的记录数
void API::printRecord(string tableName, vector<COND> x)
{
	//查找字典信息，如果文件不存，直接出错提示
	if(cm.FindFile(tableName)==TABLE_FILE)
	{
		int m;
		m=rm.selectRecord(tableName,x);
		cout<<m<<" records selected"<<endl;
	}
	else
		cout<<"There is no table "<<tableName<<endl;
}



//向表中插入入记录
void API::insertRecord(string tableName,vector<string> v)
{
	//查找字典信息，如果文件不存，直接出错提示
	if(cm.FindFile(tableName)!=TABLE_FILE)
	{
		cout<<"ERROR:There is no table "<<tableName<<endl;
		return ;
	}
		vector<string> type;
		type=cm.getCollType(tableName);

		//s用来作为记录的暂时存放数组
		char s[2000];
		memset(s,0,2000);
		char *p;
		int pos=0;
		for(unsigned i=0;i<v.size();i++)
		{
			//如果类型超出临时数组的长度
			if(cm.CalLength(type.at(i))+pos>2000)
			{
				cout<<"Failed to insert. The record is too long"<<endl;
				break;
			}
			//如果是整型变量，则将整型转为char型,以char的字节形式存入数组
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
			//如果是float形变量，则将float转为char型，以char的字节形式存入数组
			else if(type.at(i)=="float")
			{
				float tt;
				tt=(float)atof(v.at(i).c_str());
				p=(char *)&tt;
				memcpy(s + pos, p, sizeof(float));
				pos += sizeof(float);
			}
			//如果是字符型的变量，直接进行字节存储
			else
			{			
				//如果实际输入的string长于定义的char长度，则报错。
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
				//由于是定长存储，所以列定义超出部分以0存储.
				for(unsigned int j=v.at(i).length();j<(unsigned int)cm.CalLength(type.at(i));j++,pos++)
					;

			}
		}

		//如果在数据中插入记录成功，则同样在内存字典信息中插入记录信息
		if(rm.insertRecord(tableName,s))
		{
			cm.InsertRecord(tableName,1);
		}
		//printRecord(tableName);
}

//将表中的记录全部删除,同时输出删除的记录数目
void API::deleteValue(string tableName)
{
	//查找字典信息，如果文件不存，直接出错提示
	if(cm.FindFile(tableName)!=TABLE_FILE)
	{
		cout<<"ERROR:There is no table "<<tableName<<endl;
		return ;
	}
	//首先在RecordManager中删除记录，然后在数据字典中将记录数改为0
	int num=rm.deleteValue(tableName);
	if(cm.DeleteRecord(tableName))
		cout<<"Delete "<<num<<" records "<<"in "<<tableName<<endl;
	else
		cout<<"ERROR:Fail to delete in table "<<tableName<<endl;
	printRecord(tableName);
}

//根据n个where条件删除表中的记录，同时输出删除记录的数目
void API::deleteValue(string tableName, vector<COND> x)
{
	//查找字典信息，如果文件不存，直接出错提示
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




//查找文件，返回在这个文件的记录数目
int API::getRecordNum(string tableName)
{
	return cm.getRecordNum(tableName);
}

//返回文件属性列表
vector<string> API::getCollName(string tableName)
{
	return cm.getCollName(tableName);
}

//返回文件属性类型列表
vector<string> API::getCollType(string tableName)
{
	return cm.getCollType(tableName);
}

//根据表名，返回文件中的记录长度
int API::calcuteLenth(string tableName)
{
	return cm.CalLength2(tableName);
}

//根据某个属性类型，返回这个属性类型的长度
int API::calcuteLenth2(string type)
{
	return cm.CalLength(type);
}

//删除一个表
void API::dropTable(string tableName)
{
	//查找字典信息，如果文件不存，直接出错提示
	if(cm.FindFile(tableName)!=TABLE_FILE)
	{
		cout<<"ERROR:There is no table "<<tableName<<endl;
		return ;
	}
	//直接在字典信息中将表删除
	if(cm.DropTable(tableName))
		cout<<"Drop table "<<tableName<<" successfully"<<endl;
}

//删除一个索引
void API::dropIndex(string indexName)
{
	//查找字典信息，如果索引不存，直接出错提示
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

//根据给定的表、属性，创建索引
void API::createIndex(string fileName, string tableName, string colName)
{
	//查找字典信息，如果索引存在，直接出错提示
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
	//在IndexManager中增加索引
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

	//在字典信息中增加索引
	cm.AddIndex(fileName, tableName, colName);
	cout << "Create index " << fileName << " successfully" << endl;
}
//根据列、主键等信息创建表
void API::createTable(string tableName,vector<string> col,vector<string> type,vector<int> uniq,string primKey)
{
	//查找字典信息，如果表已经存在，直接出错提示
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


//给定表名，列名，这个例的一个值以及这条记录在数据文件中的储位置，将这个值插入到索引引文件中
//其中的位置是块偏移和记录在块中的位置
int API::insertIndexItem(string fileName,string colName,string value,int block,int index)
{

	return 1;
}

//给定表名、列名，以及这个列上的一个值，返回这条记录在数据文件中对应的位置
int API::getIndexItem(string fileName,string colName,string value,int * block,int * index)
{
	return 1;
}

string API::getPrimaryKey(string tableName)
{
	//查找字典信息，如果表已经存在，直接出错提示
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