
#include "stdafx.h"

//引用头文件
#include "Interpreter.h"
#include "API.h"
#include "my_catalog_manager.h"
#include "BufferManager.h"

#include <iostream>
#include <string>
#include <fstream>


using namespace std;

#define CREATE 0  //SQL语句的第一个关键字
#define SELECT 1
#define DROP 2
#define DELETE 3
#define INSERT 4
#define QUIT 5 
#define EXECFILE 14

#define TABLE 6   //SQL语句的第二个关键字
#define INDEX 7
#define UNKNOWN 8

//全局变量的定义，可以在整个工程内使用
Catalog_Manager cm;
BufferManager bm;
RecordManager q;
API ap;


int main()
{
	//输出揭示信息
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



	int flag=1;           //标记读取SQL语句的状态，如果flag=1,则为文件中读，如果flag=0，则为标准IO中读取
	ifstream file;        //用于输入脚本
	while(1)
	{
		Interpreter in;   //语法解析对象
		string s;
		//如果flag==0，不从文件中读入，从标准IO读入，则输出提示符
		if(flag)
		{
			//输入提示信息
			cout<<">>";   
			//以';'作为SQL语句结束的标志,输入一条SQL语句
			getline(cin,s,';');             
		}
		//如果flag==1, 从文件中读入SQL语句
		else
		{
			cout<<endl;
			getline(file,s,';');

			//如果读到脚本末尾标记，则退出文件读取状态，设置flag=0;
			int sss=s.find("$end");
			if(sss>=0)
			{
				flag=1;
				file.close();
				in.~Interpreter();
				continue;
			}
		}

		//对SQL语句进行解析，如果解析失败，则退出可能进入的文件读取状态，重新读入SQL语句
		if(!in.interpreter(s))
		{
			flag=1;
			//判断文件是否打开，如果打开，则将其关闭
			if(file.is_open())
				file.close();
			//析构in对象
			in.~Interpreter();
			continue;
		}
		
		//对firstKey进行遍历，分类处理
		switch(in.firstKey)
		{
			//firstKey为create
			case CREATE:
				//创建表
				if(in.secondKey==TABLE)
					ap.createTable(in.fileName,in.col,in.type,in.uniq,in.primKey);
				//创建索引
				else if(in.secondKey==INDEX)
					ap.createIndex(in.fileName,in.tableName,in.colName);
				else
					cout<<"Error. Usage: create name"<<endl;
				break;
			//firstKey为select
			case SELECT:
				//无where条件查寻
				if(in.condNum==0)
					ap.printRecord(in.fileName);
				else
					ap.printRecord(in.fileName,in.cond);
				break;
			//firstKdy为drop
			case DROP:
				//删除表
				if(in.secondKey==TABLE)
					ap.dropTable(in.fileName);
				//删除索引
				else if(in.secondKey==INDEX)
					ap.dropIndex(in.fileName);
				else
					cout<<"Error. Usage: drop table name or index name"<<endl;
				break;
			//firstKey为delete
			case DELETE:
				//无条件删除所有记录
				if(in.condNum==0)
					ap.deleteValue(in.fileName);
				//根据n个where条件删除满足条件的记录
				else 
					ap.deleteValue(in.fileName,in.cond);
				break;
			//firstKey为insert
			case INSERT:
				ap.insertRecord(in.fileName,in.insertValue);
				break;
			//firstKey为quit
			case QUIT:
				//将字典信息写回
				if(!cm.WriteBack())
				{
					cout<<"Error:Fail to write back db.info"<<endl;
					return 0;
				}
				//将数据信息写回文件
				if(!bm.flushAll())
				{
					cout<<"Error:Fail to flush all the block into database"<<endl;
					return 0;
				}
				return 1;

			//执行脚本
			case EXECFILE:
				
				//打开文件,如果失败，则输出错误信息
				file.open(in.fileName.c_str());
				if(!file.is_open())
				{
					cout<<"ERROR:Fail to open "<<in.fileName<<endl;
					break;
				}

				//将状态设为从文件输入
				flag=0;
				break;
		}
		//析构in实例
		in.~Interpreter();
	}
	return 1;
}
