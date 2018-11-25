
#include "stdafx.h"

//����ͷ�ļ�
#include "Interpreter.h"
#include "API.h"
#include "my_catalog_manager.h"
#include "BufferManager.h"

#include <iostream>
#include <string>
#include <fstream>


using namespace std;

#define CREATE 0  //SQL���ĵ�һ���ؼ���
#define SELECT 1
#define DROP 2
#define DELETE 3
#define INSERT 4
#define QUIT 5 
#define EXECFILE 14

#define TABLE 6   //SQL���ĵڶ����ؼ���
#define INDEX 7
#define UNKNOWN 8

//ȫ�ֱ����Ķ��壬����������������ʹ��
Catalog_Manager cm;
BufferManager bm;
RecordManager q;
API ap;


int main()
{
	//�����ʾ��Ϣ
	cout<<"*******************MiniSQL**********************"<<endl;
	
		std::string colName;
		std::string cond;
		std::string operater;
	vector<COND> x;
	x.push_back(COND(string("a"), string("1"), string(">=")));
	x.push_back(COND(string("a"), string("4"), string("<=")));
	x.push_back(COND(string("b"), string("6.1"), string("=")));

	vector<string> m = { "5","6.2","fuck2" };
	char a[4];
	int fuck = 1000;
	int shit;
	memcpy(a, &fuck, 4);
	memcpy(&shit, a, 4);
    //q.selectRecord(string("c"), x);
	//BufferManager::File * fp = bm.getFile("c");
	//BufferManager::Block * bp = bm.getBlock(fp,0);
	//if(bp)
	//bm.reconstruct(bp);
	
	//ap.printRecord("c");
	/*int i = 10000;
	while (i--)
	{
		ap.insertRecord("f2", m);
		cout << i << endl;
	}*/
	//ap.insertRecord("f2", m);
	//ap.deleteValue("f2", x);
	//ap.printRecord("c", x);
	//ap.deleteValue("c", x);
	//ap.printRecord("f2");
	//ap.deleteValue("c");
	//ap.printRecord("c");



	int flag=1;           //��Ƕ�ȡSQL����״̬�����flag=1,��Ϊ�ļ��ж������flag=0����Ϊ��׼IO�ж�ȡ
	ifstream file;        //��������ű�
	while(1)
	{
		Interpreter in;   //�﷨��������
		string s;
		//���flag==0�������ļ��ж��룬�ӱ�׼IO���룬�������ʾ��
		if(flag)
		{
			//������ʾ��Ϣ
			cout<<">>";   
			//��';'��ΪSQL�������ı�־,����һ��SQL���
			getline(cin,s,';');             
		}
		//���flag==1, ���ļ��ж���SQL���
		else
		{
			cout<<endl;
			getline(file,s,';');

			//��������ű�ĩβ��ǣ����˳��ļ���ȡ״̬������flag=0;
			int sss=s.find("$end");
			if(sss>=0)
			{
				flag=1;
				file.close();
				in.~Interpreter();
				continue;
			}
		}

		//��SQL�����н������������ʧ�ܣ����˳����ܽ�����ļ���ȡ״̬�����¶���SQL���
		if(!in.interpreter(s))
		{
			flag=1;
			//�ж��ļ��Ƿ�򿪣�����򿪣�����ر�
			if(file.is_open())
				file.close();
			//����in����
			in.~Interpreter();
			continue;
		}
		
		//��firstKey���б��������ദ��
		switch(in.firstKey)
		{
			//firstKeyΪcreate
			case CREATE:
				//������
				if(in.secondKey==TABLE)
					ap.createTable(in.fileName,in.col,in.type,in.uniq,in.primKey);
				//��������
				else if(in.secondKey==INDEX)
					ap.createIndex(in.fileName,in.tableName,in.colName);
				else
					cout<<"Error. Usage: create name"<<endl;
				break;
			//firstKeyΪselect
			case SELECT:
				//��where������Ѱ
				if(in.condNum==0)
					ap.printRecord(in.fileName);
				else
					ap.printRecord(in.fileName,in.cond);
				break;
			//firstKdyΪdrop
			case DROP:
				//ɾ����
				if(in.secondKey==TABLE)
					ap.dropTable(in.fileName);
				//ɾ������
				else if(in.secondKey==INDEX)
					ap.dropIndex(in.fileName);
				else
					cout<<"Error. Usage: drop table name or index name"<<endl;
				break;
			//firstKeyΪdelete
			case DELETE:
				//������ɾ�����м�¼
				if(in.condNum==0)
					ap.deleteValue(in.fileName);
				//����n��where����ɾ�����������ļ�¼
				else 
					ap.deleteValue(in.fileName,in.cond);
				break;
			//firstKeyΪinsert
			case INSERT:
				ap.insertRecord(in.fileName,in.insertValue);
				break;
			//firstKeyΪquit
			case QUIT:
				//���ֵ���Ϣд��
				if(!cm.WriteBack())
				{
					cout<<"Error:Fail to write back db.info"<<endl;
					return 0;
				}
				//��������Ϣд���ļ�
				if(!bm.flushAll())
				{
					cout<<"Error:Fail to flush all the block into database"<<endl;
					return 0;
				}
				return 1;

			//ִ�нű�
			case EXECFILE:
				
				//���ļ�,���ʧ�ܣ������������Ϣ
				file.open(in.fileName.c_str());
				if(!file.is_open())
				{
					cout<<"ERROR:Fail to open "<<in.fileName<<endl;
					break;
				}

				//��״̬��Ϊ���ļ�����
				flag=0;
				break;
		}
		//����inʵ��
		in.~Interpreter();
	}
	return 1;
}
