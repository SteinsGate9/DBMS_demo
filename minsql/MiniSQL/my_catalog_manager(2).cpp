#include <iostream>
#include <fstream>
#include <cstring>
#include <stdlib.h>
#include "my_catalog_manager(1).h"
//#include <io.h>
#define UNKNOWN_FILE 8                                 //三个宏定义，用于定义文件类型
#define TABLE_FILE 9   
#define INDEX_FILE 10
using namespace std;
Catalog_Manager::Catalog_Manager()
{
	string tmp;
	string s;
	bool judge;
	Index now_index;
	int length;
	ifstream file("catalog.info");                          //创建字典文件对象
	if (!file)
	{
		cout << "Fail to open catalog.info when read dictionary information" << endl;
		exit(0);
	}
	else
	{
		/*
		文件格式：
		1						table_num
		@
		student					table_name
		3						col_num
		0						record_num
		0						index_num
		id name address 		col_name
		int char(10) char(10) 	col_type
		1 0 0					col_unique
		no no no				index_information
		NO						primary_key
		$end
		*/
		file >> table_num;
		file >> tmp;
		while (tmp != "$end")                      //判断是否读到文件尾
		{
			//cout << "tmp: " << tmp << endl;
			if (tmp == "@")                               //读到表,接下去输入表的各种信息
			{
				//cout << "ok" << endl;
				Table now;                //创建表结构实体
				file >> tmp;
				now.table_name.assign(tmp, 0, strlen(tmp.c_str()));  //读入表名称 
				file >> now.col_num;
				file >> now.record_num;
				file >> now.index_num;
				for (int i = 0; i<now.col_num; i++)            //读入字段名
				{
					file >> tmp;
					s.assign(tmp, 0, strlen(tmp.c_str()));
					now.col_name.push_back(s);
				}
				for (int i = 0; i<now.col_num; i++)            //读入字段类型
				{
					file >> tmp;
			    	//cout << "type_name: " << tmp << endl;
					s.assign(tmp, 0, strlen(tmp.c_str()));
					//cout << s << endl;
					length = CalLength(s);
					now.col_length.push_back(length);
					now.col_type.push_back(s);
				}
				for (int i = 0; i<now.col_num; i++)            //读入字段是否惟一
				{
					file >> judge;
					now.col_unique.push_back(judge);
				}

				for (int i = 0; i<now.col_num; i++)            //读入字段的索引信息
				{
					file >> tmp;
					if (tmp == "no")
					{
						file >> tmp;
						break;
					}
					s.assign(tmp, 0, strlen(tmp.c_str()));
					now.index_num++;
					now_index.index_name = s;
					now_index.key_name = now.col_name[i];
					now.indexes.push_back(now_index);
				}

				file >> tmp;
				now.pri_key.assign(tmp, 0, strlen(tmp.c_str()));     //读入关键字信息
				tables.push_back(now);                           //将表结构放入表列表中
				file >> tmp;
			}
			else                                           //错误提示
			{
				cout << "Fail to read catalog.info,storing error found" << endl;
			}
		}
	}
}

int Catalog_Manager::CalLength(string s)
{
	if (s == "int") return sizeof(int);
	if (s == "float") return sizeof(float);
	if (s[0] == 'c')
	{
	//	cout << "ok" << endl;
		int i = 5;
		string num;
		int number;
		while (s[i] != ')')
		{
			//cout << s[i] << endl;
			num.push_back(s[i]);
			i++;
		}
		//cout << num << endl;
		number = atoi(num.c_str());
		return number;
	}
	return 0;
}



////将内存中的字典信息按照固定的格式写回文件
/*
			文件格式：
				1						table_num	
				@
				student					table_name
				3						col_num
				0						record_num
				0						index_num
				id name address 		col_name
				int char(10) char(10) 	col_type
				1 0 0					col_unique
				no no no				index_information
				(0 10 10)				col_length
				NO						primary_key
				$end
		*/ 
bool Catalog_Manager::WriteBack()
{
	const char * fileName = "catalog.info";
	ofstream file(fileName);
	if (!file)
	{
		cout << "Fail to open catalog.info when write";
		return 0;
	}
	else
	{
		file << table_num << endl;
		for (int i = 0; i<table_num; i++)
		{
			file << "@" << endl << tables[i].table_name << endl;
			file << tables[i].col_num << endl;
			file << tables[i].record_num << endl;
			file << tables[i].index_num << endl;
			for (int j = 0; j<tables[i].col_num; j++)
				file << tables[i].col_name[j] << " ";
			file << endl;
			for (int j = 0; j<tables[i].col_num; j++)
				file << tables[i].col_type[j] << " ";
			file << endl;
			for (int j = 0; j<tables[i].col_num; j++)
				file << tables[i].col_unique[j] << " ";
			file << endl;
			int flag = 0;
			for (unsigned int j = 0; j<tables[i].indexes.size(); j++)
			{
				flag = 1;
				file << tables[i].indexes[j].index_name << " ";
			}
			if (flag == 1) file << endl;
			else file << "no indexes" << endl;
			file << tables[i].pri_key << endl;
		}
	}file << "$end";
	return 1;
}


int Catalog_Manager::CalLength2(string table_name)
{
	int length = 0;
	vector<string> sv;
	vector<Table>::const_iterator it;
	for(it=tables.begin();it!=tables.end();++it)                  //查找字典信息中的列type的列表
	{
		if((*it).table_name==table_name)
			sv=(*it).col_type;
	}

	vector<string>::const_iterator pos;
	for(pos=sv.begin();pos!=sv.end();++pos)            //对字段列表中的每一个type，计算其长度，计算总和
		length = length + CalLength(*pos);
	return length+7;
}


//删除一个表中固定数目的记录信息
bool Catalog_Manager::DeleteRecord(string table_name,int delete_num)
{
	//在字典信息中找到对应的文件，并将recordNum修改
	for(unsigned int i = 0;i < tables.size();i++)
	{
		if(tables[i].table_name == table_name)
		{
			tables[i].record_num = tables[i].record_num - delete_num;
			return 1;
		}
	}
	return 0;
}

bool Catalog_Manager::DeleteRecord(string table_name)
{
	return DeleteRecord(table_name,1);
}

//在字典信息中删除一个表
bool Catalog_Manager::DropTable(string table_name)
{
	int flag=0;
	table_num--;                                   //将表数目减一
	vector<Table>::iterator pos;
	for(pos=tables.begin();pos!=tables.end();++pos)
	{
		if((*pos).table_name==table_name)
		{
			//删除tv中对就的项
			pos=tables.erase(pos);
			flag=1;
			break;
		}
	}
	if(flag==0)                //如果文件不存在，则提示出错信息
	{
		cout<<"Table "<<table_name<<" does not exists"<<endl;
		return 0;
	}

	remove(table_name.c_str());
	return 1;
}

//在字典文件的表中删除一个索引文件
bool Catalog_Manager::DropIndex(string index_name)
{
	int flag=0;
	vector<Table>::iterator pos;
	vector<Index>::iterator rmv;
	for(pos=tables.begin();pos!=tables.end();++pos)
	{		
			for(rmv=(*pos).indexes.begin();rmv!=(*pos).indexes.end();++rmv)
			{
				if((*rmv).index_name == index_name)
				{
					flag = 1;
					(*pos).index_num--;
					rmv = (*pos).indexes.erase(rmv);
					break;
				}
			}
	}
	if(flag==0)
	{
		cout<<"ERROR:Index "<<index_name<<" does not exists"<<endl;
		return 0;
	}
	return 1;
}


//在字典信息中插入记录
bool Catalog_Manager::InsertRecord(string table_name,int record_num)
{
	int flag=0;
	vector<Table>::iterator pos;
	for(pos=tables.begin();pos!=tables.end();++pos)
	{
		//找到对应的表，并将记录数增加一个
		if((*pos).table_name==table_name)
		{
			flag=1;
			(*pos).record_num=(*pos).record_num+record_num;
		}
	}
	if(flag==0)
	{
		cout<<"Fail to insert records,no such table found"<<endl;
		return 0;
	}
	return 1;
}

//在字典信息中为表中的字段增加索引信息
bool Catalog_Manager::AddIndex(string index_name,string table_name,string col_name)
{
	vector<Table>::iterator pos;
	vector<Index>::iterator nid;
	int flag = 0;
	for(pos=tables.begin();pos!=tables.end();++pos)
	{
		if((*pos).table_name==table_name)
		{
			for(nid=(*pos).indexes.begin();nid!=(*pos).indexes.end();++nid)
		
				if((*nid).index_name == index_name)
				{       
					cout << "this index already exists" << endl;
					return 0;
				}
			}
			for(unsigned int i=0;i<(*pos).col_name.size();i++)
			{
				if((*pos).col_name[i]==col_name)
				{
					flag = 1;       
					Index it;
					it.index_name = index_name;
					it.key_name = col_name;
					(*pos).indexes.push_back(it);
					(*pos).index_num++;
					break;
				}
			}                              
	}

	if(flag == 0)
	{
		cout<<"Fail to create index, index "<<table_name<<" does not exist"<<endl;
		return 0;
	}
	//创建索引文件
	ofstream outfile(index_name.c_str());
	if(!outfile)
	{
		cout<<"Fail to create file"<<endl;
		return 0;
	}
	outfile.close();
	
	return 1;
}

//在创建表时，向字典信息中增加一个表的信息
bool Catalog_Manager::AddTable(string table_name,vector<string> col_name,vector<string> col_type,vector<int> col_unique)
{
	string pri("no");
	if(AddTable(table_name,col_name,col_type,col_unique,pri))
		return 1;
	else 
		return 0;
	
}

//在创建表时，向字典信息中增加一个表的信息
bool Catalog_Manager::AddTable(string table_name,vector<string> col_name,vector<string> col_type,vector<int> col_unique,string pri)
{
	vector<Table>::iterator pos;
	for(pos=tables.begin();pos!=tables.end();++pos)
	{
		if((*pos).table_name==table_name)
		{
			cout << "the table already exists" << endl;
			return 0;
		}
	}
	
	table_num++;
	Table t;       //创建一个表的struct
	t.table_name=table_name;         //把提供的表信息放入这个struct
	t.col_num=col_name.size();
	t.record_num=0;
	t.index_num=0;
	t.pri_key=pri.empty()?"no":pri;
	t.col_name=col_name;
	t.col_type=col_type;
	t.col_unique=col_unique;
	
	string s; 
	int length;
	//索引是在CatalogManager模块中增加的，不是在Interpreter模块中增加的
	for(unsigned int i=0;i<col_type.size();i++)
	{
		s = col_type[i];
		length = CalLength(s);
		t.col_length.push_back(length);
	} 
	tables.push_back(t);


	//为建立的表创建相应的文件
	ofstream outfile(table_name.c_str());
	if(!outfile)
	{
		cout<<"Fail to create file"<<endl;
		return 0;
	}
	outfile.close();


	return 1;
}


//加的 
int Catalog_Manager::FindFile(string fileName)
{
	for(int i=0;i<table_num;i++)
	{
		if(tables[i].table_name==fileName)
			return TABLE_FILE;
	}
	for(int i=0;i<table_num;i++)
	{
		for(int j = 0;j < tables[i].index_num;j++)
		{ 
			if(tables[i].indexes[j].index_name==fileName)
				return INDEX_FILE;
		} 
	}
	return UNKNOWN_FILE;
}

void Catalog_Manager::getFileInfo(string fileName,int * n)
{
	if(FindFile(fileName)==UNKNOWN_FILE)                    //如果没有这个文件存在，则在数组中返回UNKNOWN_FILE
		n[0]=UNKNOWN_FILE;
	else if (FindFile(fileName)==INDEX_FILE)                //如果是个索引文件，则在数组中返回INDEX_FILE
		n[0]=INDEX_FILE;
	else                                                    //如果是个表文件，则在数组中返回TABLE_FILE，
	{                                                       //和表的记录长度，记录数目
		for(unsigned int i=0;i<tables.size();i++)
		{
			if(tables[i].table_name==fileName)
			{
				n[0]=TABLE_FILE;
				n[1]=tables[i].record_num;
				n[2]=CalLength2(tables[i].table_name);
			}
		}
	}
}


//根据给定的表名返回字段名列表
vector<string> Catalog_Manager::getCollName(string tableName)
{
	vector<string> v;
	vector<Table>::const_iterator it;

	//对表列表遍历，对文件名匹配查找
	for(it=tables.begin();it!=tables.end();++it)
	{
		if((*it).table_name==tableName)
			return (*it).col_name;
	}
	return v;
}

//根据给定的表名返回字段类型列表
vector<string> Catalog_Manager::getCollType(string tableName)
{
	vector<string> v;
	vector<Table>::const_iterator it;

	//对表列表遍历，对文件名匹配查找
	for(it=tables.begin();it!=tables.end();++it)
	{
		if((*it).table_name==tableName)
			return (*it).col_type;
	}
	return v;
}

//根据给定的表名返回字段是否惟一列表
vector<int> Catalog_Manager::getCollUniq(string tableName)
{
	vector<Table>::const_iterator it;
	vector<int>fuck;
	//对表列表遍历，对文件名匹配查找
	for(it=tables.begin();it!=tables.end();++it)
	{
		if((*it).table_name==tableName)
			return (*it).col_unique;
	}
	return fuck;
}

//根据给定的表名返回字段索引信息列表
vector<string> Catalog_Manager::getCollIndex(string tableName)
{
	vector<string> v;
	vector<Table>::const_iterator it;

	//对表列表遍历，对文件名匹配查找
	for(it=tables.begin();it!=tables.end();++it)
	{
		if((*it).table_name==tableName)
		{
			for(int i = 0;i < (*it).index_num;i++)
			{
				v.push_back((*it).indexes[i].index_name);
			}
		}
	}
	return v;
}

//返回一个表中的记录数目
int Catalog_Manager::getRecordNum(string tableName)
{
	vector<Table>::const_iterator it;
	int recordnum = 0;
	//对表列表遍历，对文件名匹配查找
	for(it=tables.begin();it!=tables.end();++it)
	{
		if((*it).table_name==tableName)
			return (*it).record_num;
	}
	return recordnum;
}

string  Catalog_Manager::getPrimaryKey(string tableName)
{
	string s;
	vector<Table>::const_iterator it;
	string primary = "FUCK";
	//对表列表遍历，对文件名匹配查找
	for(it=tables.begin();it!=tables.end();++it)
	{
		if ((*it).table_name == tableName)
		{
			return (*it).pri_key;
		}
	}
	return primary;
}
vector<string> Catalog_Manager::getUniq(string tableName)
{
	vector<string> v;
	vector<Table>::const_iterator it;

	//对表列表遍历，对文件名匹配查找
	for (it = tables.begin(); it != tables.end(); ++it)
	{
		if ((*it).table_name == tableName)
		{
			for (int i = 0; i < (*it).col_num; i++)
			{
				if ((*it).col_unique[i] == 1)
				{
					v.push_back((*it).col_name[i]);
					break;
				}
			}
			break;
		}
	}
	return v;
}