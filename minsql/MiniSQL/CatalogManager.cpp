#include "stdafx.h"
#include "CatalogManager.h"
#include <iostream>
#include <fstream>
#include <io.h>
#define UNKNOWN_FILE 8                                 //�����궨�壬���ڶ����ļ�����
#define TABLE_FILE 9   
#define INDEX_FILE 10
using namespace std;
CatalogManager::CatalogManager()
{
	char tmp[126];
	string s;
	int n=0;
	ifstream file("db.info");                          //�����ֵ��ļ�����
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
		while(strcmp(tmp,"$end")!=0)                      //�ж��Ƿ�����ļ�β
		{	
			if(tmp[0]=='@')                               //������,����ȥ�����ĸ�����Ϣ
			{
				CatalogManager::Table tt;                //������ṹʵ��
                                                         //��ʵ������ݶ���
				tt.tableName.assign(tmp,1,strlen(tmp)-1);  
				file>>tt.collNum;
				for(int i=0;i<tt.collNum;i++)            //�����ֶ���
				{
					file>>tmp;
					s.assign(tmp,0,strlen(tmp));
					tt.collName.push_back(s);
				}
				for(int i=0;i<tt.collNum;i++)            //�����ֶ�����
				{
					file>>tmp;
					s.assign(tmp,0,strlen(tmp));
					tt.collType.push_back(s);
				}
				for(int i=0;i<tt.collNum;i++)            //�����ֶ��Ƿ�Ωһ
				{
					file>>n;
					tt.collUniq.push_back(n);
				}
				for(int i=0;i<tt.collNum;i++)            //�����ֶε�������Ϣ
				{
					file>>tmp;
					s.assign(tmp,0,strlen(tmp));
					tt.collIndex.push_back(s);
				}
				file>>tmp;
				tt.primKey.assign(tmp,0,strlen(tmp));     //����ؼ�����Ϣ
				file>>tt.recordNum;                       //������еļ�¼��Ŀ
				tt.recordLen=calcuteLenth(tt.collType);

				tv.push_back(tt);                           //����ṹ������б���

			}
			else if(tmp[0]=='#')                            //���������ļ�������ȥ���������ļ�����Ϣ
			{
				CatalogManager::Index ti;                   //����һ�������Ľṹʵ��
				ti.indexName.assign(tmp,1,strlen(tmp)-1);
				file>>ti.searchTable;
				file>>ti.searchKey;

				iv.push_back(ti);                           //�������ṹ���������б���

			}
			else                                           //������ʾ
			{
				cout<<"Fail to read db.info,storing error found"<<endl;
			}
			file>>tmp;
		}	
	}
}

////���ڴ��е��ֵ���Ϣ���չ̶��ĸ�ʽд���ļ�
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

//����һ���ֶ����͵ĳ��ȣ�������
int CatalogManager::calcuteLenth2(string type)
{
		const char * s;
		int tmp;
		s=type.c_str();
		if(strcspn(s,"int")==0)           //��������͵��ֶ�����
		{
			tmp=sizeof(int);
		}
		else if(strcspn(s,"char")==0)     //�����char�͵��ֶ�����,��Ҫ����()�е�char�ͳ���
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
		else if(strcspn(s,"float")==0)      //�����float���ֶ�����
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

//ͨ���������ֶ��б������¼����
int CatalogManager::calcuteLenth(vector<string> sv)
{
	int tmp=0;
	vector<string>::const_iterator pos;
	for(pos=sv.begin();pos!=sv.end();++pos)            //���ֶ��б��е�ÿһ��type�������䳤�ȣ������ܺ�
		tmp=tmp+calcuteLenth2(*pos);
	tmp += 7;
	return tmp;
}

//ͨ�������ı����������¼�ĳ���
int CatalogManager::calcuteLenth(string tableName)
{

	int tmp=0;
	vector<string> sv;
	vector<CatalogManager::Table>::const_iterator it;
	for(it=tv.begin();it!=tv.end();++it)                  //�����ֵ���Ϣ�е���type���б�
	{
		if((*it).tableName==tableName)
			sv=(*it).collType;
	}

	tmp=calcuteLenth(sv);                                //����type�б���ܳ���

	return tmp;
}

//�����ļ���Ϣ���Ƿ�Ϊ�������߱�������ߣ���������Ⱥͼ�¼��
void CatalogManager::getFileInfo(string fileName,int * n)
{
	if(findFile(fileName)==UNKNOWN_FILE)                    //���û������ļ����ڣ����������з���UNKNOWN_FILE
		n[0]=UNKNOWN_FILE;
	else if (findFile(fileName)==INDEX_FILE)                //����Ǹ������ļ������������з���INDEX_FILE
		n[0]=INDEX_FILE;
	else                                                    //����Ǹ����ļ������������з���TABLE_FILE��
	{                                                       //�ͱ�ļ�¼���ȣ���¼��Ŀ
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

//ɾ��һ���������еļ�¼��Ϣ
int CatalogManager::deleteValue(string tableName)
{
	return deleteValue(tableName,getRecordNum(tableName));	
}

//ɾ��һ�����й̶���Ŀ�ļ�¼��Ϣ
int CatalogManager::deleteValue(string tableName,int deleteNum)
{
	unsigned int i=0;
	//���ֵ���Ϣ���ҵ���Ӧ���ļ�������recordNum�޸�
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

//�����ļ�������Ǳ��򷵻�TABLE_FILE
//������������򷵻�INDEX_FILE
//�����������ļ����򷵻�UNKNOWN_FILE
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

//���ֵ���Ϣ��ɾ��һ����
int CatalogManager::dropTable(string tableName)
{
	int flag=0;
	tableNum--;                                   //������Ŀ��һ
	vector<CatalogManager::Table>::iterator pos;
	for(pos=tv.begin();pos!=tv.end();++pos)
	{
		if((*pos).tableName==tableName)
		{
			//��������Ϣ��ɾ�����п��ܴ��ڵ�����
			for(unsigned i=0;i<(*pos).collIndex.size();i++)
				if((*pos).collIndex.at(i)!="*")
					dropIndex((*pos).collIndex.at(i));
			//ɾ��tv�жԾ͵���
			pos=tv.erase(pos);
			flag=1;
			break;
		}
	}
	if(flag==0)                //����ļ������ڣ�����ʾ������Ϣ
	{
		cout<<"Table "<<tableName<<" does not exists"<<endl;
		return 0;
	}

	remove(tableName.c_str());


	return 1;
}

//���ֵ��ļ���ɾ��һ�������ļ�
int CatalogManager::dropIndex(string indexName)
{
	int flag=0;
	indexNum--;
	vector<CatalogManager::Index>::iterator pos;
	for(pos=iv.begin();pos!=iv.end();++pos)
	{
		//��iv���������ҵ���Ӧ������
		if((*pos).indexName==indexName)
		{			
			for(unsigned int i=0;i<tv.size();i++)
			{
				//�ҵ�tv���������������ı���
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
			//ɾ��iv�ж�Ӧ��Index��
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


//���ֵ���Ϣ�в����¼
int CatalogManager::insertRecord(string tableName,int recordNum)
{
	int flag=0;
	vector<CatalogManager::Table>::iterator pos;
	for(pos=tv.begin();pos!=tv.end();++pos)
	{
		//�ҵ���Ӧ�ı�������¼������һ��
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

//���ֵ���Ϣ��Ϊ���е��ֶ�����������Ϣ
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
					    //�ж������Ƿ��Ѿ����ڣ���������������ʾ������0
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

	//����Ӧ�������ṹ����뵽����vector��
	CatalogManager::Index it;
	it.indexName=indexName;
	it.searchTable=tableName;
	it.searchKey=collName;
	iv.push_back(it);
	indexNum++;

	//���������ļ�
	ofstream outfile(indexName.c_str());
	if(!outfile)
	{
		cout<<"Fail to create file"<<endl;
		return 0;
	}
	outfile.close();
	

	return 1;
}

//���ݸ����ı��������ֶ����б�
vector<string> CatalogManager::getCollName(string tableName)
{
	vector<string> v;
	vector<CatalogManager::Table>::const_iterator it;

	//�Ա��б���������ļ���ƥ�����
	for(it=tv.begin();it!=tv.end();++it)
	{
		if((*it).tableName==tableName)
			return (*it).collName;
	}
	return v;
}

//���ݸ����ı��������ֶ������б�
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

//���ݸ����ı��������ֶ��Ƿ�Ωһ�б�
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

//���ݸ����ı��������ֶ�������Ϣ�б�
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

//�ڴ�����ʱ�����ֵ���Ϣ������һ�������Ϣ
int CatalogManager::addTable(string tableName,vector<string> col,vector<string> type,vector<int> uniq)
{
	string p("no");
	if(addTable(tableName,col,type,uniq,p))
		return 1;
	else 
		return 0;
	
}

//�ڴ�����ʱ�����ֵ���Ϣ������һ�������Ϣ
int CatalogManager::addTable(string tableName,vector<string> col,vector<string> type,vector<int> uniq,string primKey)
{
	tableNum++;
	CatalogManager::Table t;       //����һ�����struct
	t.tableName=tableName;         //���ṩ�ı���Ϣ�������struct
	t.recordNum=0;
	t.recordLen=calcuteLenth(type);
	t.collNum=col.size();
	t.primKey=primKey;
	t.collName=col;
	t.collType=type;
	t.collUniq=uniq;

	//��������CatalogManagerģ�������ӵģ�������Interpreterģ�������ӵ�
	for(unsigned i=0;i<col.size();i++)     
		t.collIndex.push_back("*");
	tv.push_back(t);


	//Ϊ�����ı�����Ӧ���ļ�
	ofstream outfile(tableName.c_str());
	if(!outfile)
	{
		cout<<"Fail to create file"<<endl;
		return 0;
	}
	outfile.close();
	return 1;
}

//����һ�����еļ�¼��Ŀ
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
