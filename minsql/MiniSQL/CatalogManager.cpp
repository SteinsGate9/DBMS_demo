#include "stdafx.h"
#include "CatalogManager.h"
#include <iostream>
#include <fstream>
#include <io.h>
#define UNKNOWN_FILE 8                                 //三个宏定义，用于定义文件类型
#define TABLE_FILE 9   
#define INDEX_FILE 10
using namespace std;
CatalogManager::CatalogManager()
{
	char tmp[126];
	string s;
	int n=0;
	ifstream file("db.info");                          //创建字典文件对象
	if(!file)
	{
		cout<<"Fail to open db.info when read dictionary information"<<endl;
		exit(0);
	}
	else
	{
		file>>tableNum;
		file>>indexNum;
		file>>tmp;
		while(strcmp(tmp,"$end")!=0)                      //判断是否读到文件尾
		{	
			if(tmp[0]=='@')                               //读到表,接下去输入表的各种信息
			{
				CatalogManager::Table tt;                //创建表结构实体
                                                         //将实体的内容读入
				tt.tableName.assign(tmp,1,strlen(tmp)-1);  
				file>>tt.collNum;
				for(int i=0;i<tt.collNum;i++)            //读入字段名
				{
					file>>tmp;
					s.assign(tmp,0,strlen(tmp));
					tt.collName.push_back(s);
				}
				for(int i=0;i<tt.collNum;i++)            //读入字段类型
				{
					file>>tmp;
					s.assign(tmp,0,strlen(tmp));
					tt.collType.push_back(s);
				}
				for(int i=0;i<tt.collNum;i++)            //读入字段是否惟一
				{
					file>>n;
					tt.collUniq.push_back(n);
				}
				for(int i=0;i<tt.collNum;i++)            //读入字段的索引信息
				{
					file>>tmp;
					s.assign(tmp,0,strlen(tmp));
					tt.collIndex.push_back(s);
				}
				file>>tmp;
				tt.primKey.assign(tmp,0,strlen(tmp));     //读入关键字信息
				file>>tt.recordNum;                       //读入表中的记录数目
				tt.recordLen=calcuteLenth(tt.collType);

				tv.push_back(tt);                           //将表结构放入表列表中

			}
			else if(tmp[0]=='#')                            //读到索引文件，接下去输入索引文件的信息
			{
				CatalogManager::Index ti;                   //创建一个索引的结构实体
				ti.indexName.assign(tmp,1,strlen(tmp)-1);
				file>>ti.searchTable;
				file>>ti.searchKey;

				iv.push_back(ti);                           //将索引结构放入索引列表中

			}
			else                                           //错误提示
			{
				cout<<"Fail to read db.info,storing error found"<<endl;
			}
			file>>tmp;
		}	
	}
}

////将内存中的字典信息按照固定的格式写回文件
int CatalogManager::writeBack()
{
	const char * fileName="db.info";
	ofstream file(fileName);
	if(!file)
	{
		cout<<"Fail to open db.info when write";
		return 0;
	}
	else
	{
		file<<tableNum<<endl;
		file<<indexNum<<endl;
		for(int i=0;i<tableNum;i++)
		{
			file<<"@"<<tv[i].tableName<<endl;
			file<<tv[i].collNum<<endl;
			for(int j=0;j<tv[i].collNum;j++)
				file<<tv[i].collName[j]<<" ";
			file<<endl;
			for(int j=0;j<tv[i].collNum;j++)
				file<<tv[i].collType[j]<<" ";
			file<<endl;
			for(int j=0;j<tv[i].collNum;j++)
				file<<tv[i].collUniq[j]<<" ";
			file<<endl;
			for(int j=0;j<tv[i].collNum;j++)
				file<<tv[i].collIndex[j]<<" ";
			file<<endl;
			file<<tv[i].primKey<<endl;
			file<<tv[i].recordNum<<endl;
		}
		for(int i=0;i<indexNum;i++)
		{
			file<<"#"<<iv[i].indexName<<endl;
			file<<iv[i].searchTable<<" "<<iv[i].searchKey<<" "<<endl;
		}
		file<<"$end";
	}
	return 1;
}

//计算一个字段类型的长度，并返回
int CatalogManager::calcuteLenth2(string type)
{
		const char * s;
		int tmp;
		s=type.c_str();
		if(strcspn(s,"int")==0)           //如果是整型的字段类型
		{
			tmp=sizeof(int);
		}
		else if(strcspn(s,"char")==0)     //如果是char型的字段类型,则要返回()中的char型长度
		{
			const char * p1, *p2;
			char p[5];
			int n;
			p1=strstr(s,"(");
			p1++;
			p2=strstr(s,")");
			strcpy_s(p,6,p1);
			p[p2-p1]=0;
			n=atoi(p);
			tmp=n*sizeof(char);
		}
		else if(strcspn(s,"float")==0)      //如果是float型字段类型
		{
			tmp=sizeof(float);
		}
		else
		{
			cout<<"Fail to calcute record length,undefined type found"<<endl;
			return -1;
		}
		return tmp;
}

//通过给定的字段列表，计算记录长度
int CatalogManager::calcuteLenth(vector<string> sv)
{
	int tmp=0;
	vector<string>::const_iterator pos;
	for(pos=sv.begin();pos!=sv.end();++pos)            //对字段列表中的每一个type，计算其长度，计算总和
		tmp=tmp+calcuteLenth2(*pos);
	tmp += 7;
	return tmp;
}

//通过给定的表名，计算记录的长度
int CatalogManager::calcuteLenth(string tableName)
{

	int tmp=0;
	vector<string> sv;
	vector<CatalogManager::Table>::const_iterator it;
	for(it=tv.begin();it!=tv.end();++it)                  //查找字典信息中的列type的列表
	{
		if((*it).tableName==tableName)
			sv=(*it).collType;
	}

	tmp=calcuteLenth(sv);                                //计算type列表的总长度

	return tmp;
}

//查找文件信息，是否为索引或者表，如果后者，则包括表长度和记录数
void CatalogManager::getFileInfo(string fileName,int * n)
{
	if(findFile(fileName)==UNKNOWN_FILE)                    //如果没有这个文件存在，则在数组中返回UNKNOWN_FILE
		n[0]=UNKNOWN_FILE;
	else if (findFile(fileName)==INDEX_FILE)                //如果是个索引文件，则在数组中返回INDEX_FILE
		n[0]=INDEX_FILE;
	else                                                    //如果是个表文件，则在数组中返回TABLE_FILE，
	{                                                       //和表的记录长度，记录数目
		for(unsigned int i=0;i<tv.size();i++)
		{
			if(tv[i].tableName==fileName)
			{
				n[0]=TABLE_FILE;
				n[1]=tv[i].recordNum;
				n[2]=tv[i].recordLen;
			}
		}
	}
}

//删除一个表中所有的记录信息
int CatalogManager::deleteValue(string tableName)
{
	return deleteValue(tableName,getRecordNum(tableName));	
}

//删除一个表中固定数目的记录信息
int CatalogManager::deleteValue(string tableName,int deleteNum)
{
	unsigned int i=0;
	//在字典信息中找到对应的文件，并将recordNum修改
	for(i=0;i<tv.size();i++)
	{
		if(tv[i].tableName==tableName)
		{
			tv[i].recordNum=tv[i].recordNum-deleteNum;
			return 1;
		}
	}
	return 0;
}

//查找文件，如果是表，则返回TABLE_FILE
//如果是索引，则返回INDEX_FILE
//如果存在这个文件，则返回UNKNOWN_FILE
int CatalogManager::findFile(string fileName)
{
	for(unsigned int i=0;i<tv.size();i++)
	{
		if(tv[i].tableName==fileName)
			return TABLE_FILE;
	}
	for(unsigned int i=0;i<iv.size();i++)
	{
		if(iv[i].indexName==fileName)
			return INDEX_FILE;
	}
	return UNKNOWN_FILE;
}

//在字典信息中删除一个表
int CatalogManager::dropTable(string tableName)
{
	int flag=0;
	tableNum--;                                   //将表数目减一
	vector<CatalogManager::Table>::iterator pos;
	for(pos=tv.begin();pos!=tv.end();++pos)
	{
		if((*pos).tableName==tableName)
		{
			//在索引信息中删除列中可能存在的索引
			for(unsigned i=0;i<(*pos).collIndex.size();i++)
				if((*pos).collIndex.at(i)!="*")
					dropIndex((*pos).collIndex.at(i));
			//删除tv中对就的项
			pos=tv.erase(pos);
			flag=1;
			break;
		}
	}
	if(flag==0)                //如果文件不存在，则提示出错信息
	{
		cout<<"Table "<<tableName<<" does not exists"<<endl;
		return 0;
	}

	remove(tableName.c_str());


	return 1;
}

//在字典文件中删除一个索引文件
int CatalogManager::dropIndex(string indexName)
{
	int flag=0;
	indexNum--;
	vector<CatalogManager::Index>::iterator pos;
	for(pos=iv.begin();pos!=iv.end();++pos)
	{
		//在iv索引表中找到对应的索引
		if((*pos).indexName==indexName)
		{			
			for(unsigned int i=0;i<tv.size();i++)
			{
				//找到tv中索引表所创建的表项
				if(tv[i].tableName==(*pos).searchTable)
				{
					for(unsigned int j=0;j<tv[i].collIndex.size();j++)
					{
						if(tv[i].collName[j]==(*pos).searchKey)
						{
							tv[i].collIndex[j]="*";
							break;
						}
					}
					break;
				}
				
			}
			//删除iv中对应的Index项
			pos=iv.erase(pos);
			flag=1;
			break;
		}
	}
	if(flag==0)
	{
		cout<<"Index "<<indexName<<" does not exists"<<endl;
		return 0;
	}

	remove(indexName.c_str());

	return 1;
}


//在字典信息中插入记录
int CatalogManager::insertRecord(string tableName,int recordNum)
{
	int flag=0;
	vector<CatalogManager::Table>::iterator pos;
	for(pos=tv.begin();pos!=tv.end();++pos)
	{
		//找到对应的表，并将记录数增加一个
		if((*pos).tableName==tableName)
		{
			flag=1;
			(*pos).recordNum=(*pos).recordNum+recordNum;
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
int CatalogManager::addIndex(string indexName,string tableName,string collName)
{
	vector<CatalogManager::Table>::iterator pos;
	for(pos=tv.begin();pos!=tv.end();++pos)
	{
		if((*pos).tableName==tableName)
		{
			unsigned int i=0;
			for(i=0;i<(*pos).collName.size();i++)
			{
				if((*pos).collName[i]==collName)
				{       
					    //判断索引是否已经存在，如果存在则输出提示，返回0
						if(strcmp((*pos).collIndex[i].c_str(),"*")!=0)
						{
							cout<<"The index already exists"<<endl;
							return 0;
						}
						(*pos).collIndex[i]=indexName;
						break;
				}
			}
			if(i==(*pos).collName.size())
			{
				cout<<"Fail to create index, coll "<<collName<<" does not exist"<<endl;
				return 0;
			}
			break;
		}
	}
	if(pos==tv.end())
	{
		cout<<"Fail to create index, table "<<tableName<<" does not exist"<<endl;
		return 0;
	}

	//将对应的索引结构项放入到索引vector中
	CatalogManager::Index it;
	it.indexName=indexName;
	it.searchTable=tableName;
	it.searchKey=collName;
	iv.push_back(it);
	indexNum++;

	//创建索引文件
	ofstream outfile(indexName.c_str());
	if(!outfile)
	{
		cout<<"Fail to create file"<<endl;
		return 0;
	}
	outfile.close();
	

	return 1;
}

//根据给定的表名返回字段名列表
vector<string> CatalogManager::getCollName(string tableName)
{
	vector<string> v;
	vector<CatalogManager::Table>::const_iterator it;

	//对表列表遍历，对文件名匹配查找
	for(it=tv.begin();it!=tv.end();++it)
	{
		if((*it).tableName==tableName)
			return (*it).collName;
	}
	return v;
}

//根据给定的表名返回字段类型列表
vector<string> CatalogManager::getCollType(string tableName)
{
	vector<string> v;
	vector<CatalogManager::Table>::const_iterator it;
	for(it=tv.begin();it!=tv.end();++it)
	{
		if((*it).tableName==tableName)
			return (*it).collType;
	}
	return v;
}

//根据给定的表名返回字段是否惟一列表
vector<int> CatalogManager::getCollUniq(string tableName)
{
	vector<int> v;
	vector<CatalogManager::Table>::const_iterator it;
	for(it=tv.begin();it!=tv.end();++it)
	{
		if((*it).tableName==tableName)
			return (*it).collUniq;
	}
	return v;
}

//根据给定的表名返回字段索引信息列表
vector<string> CatalogManager::getCollIndex(string tableName)
{
	vector<string> v;
	vector<CatalogManager::Table>::const_iterator it;
	for(it=tv.begin();it!=tv.end();++it)
	{
		if((*it).tableName==tableName)
			return (*it).collIndex;
	}
	return v;
}

//在创建表时，向字典信息中增加一个表的信息
int CatalogManager::addTable(string tableName,vector<string> col,vector<string> type,vector<int> uniq)
{
	string p("no");
	if(addTable(tableName,col,type,uniq,p))
		return 1;
	else 
		return 0;
	
}

//在创建表时，向字典信息中增加一个表的信息
int CatalogManager::addTable(string tableName,vector<string> col,vector<string> type,vector<int> uniq,string primKey)
{
	tableNum++;
	CatalogManager::Table t;       //创建一个表的struct
	t.tableName=tableName;         //把提供的表信息放入这个struct
	t.recordNum=0;
	t.recordLen=calcuteLenth(type);
	t.collNum=col.size();
	t.primKey=primKey;
	t.collName=col;
	t.collType=type;
	t.collUniq=uniq;

	//索引是在CatalogManager模块中增加的，不是在Interpreter模块中增加的
	for(unsigned i=0;i<col.size();i++)     
		t.collIndex.push_back("*");
	tv.push_back(t);


	//为建立的表创建相应的文件
	ofstream outfile(tableName.c_str());
	if(!outfile)
	{
		cout<<"Fail to create file"<<endl;
		return 0;
	}
	outfile.close();
	return 1;
}

//返回一个表中的记录数目
int CatalogManager::getRecordNum(string tableName)
{
	int recordNum=0;
	for(unsigned int i=0;i<tv.size();i++)
		if(tv.at(i).tableName==tableName)
		{
			recordNum=tv.at(i).recordNum;
			break;
		}
		return recordNum;
}

string  CatalogManager::getPrimaryKey(string tableName)
{
	string prime;
	for (unsigned int i = 0; i<tv.size(); i++)
		if (tv.at(i).tableName == tableName)
		{
			prime = tv.at(i).primKey;
			return prime;
		}
}
