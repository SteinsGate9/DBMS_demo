#include <iostream>
#include <string>
#include <vector>
using namespace std;

struct Index
{
	string index_name;
	string key_name;
};

struct Table
{
	string table_name;
	int col_num;
	int record_num; //记录数(用于判断表是否为空?) 
	int index_num;
	string pri_key;
	vector<string> col_name; //属性名称 
	vector<string> col_type; //类型
	vector<bool> col_unique; //是否唯一 
	vector<Index> indexes; //索引(建表时不能创建) 
	vector<int> col_length;	//字段长度(用于char) 
};

class Catalog_Manager
{
	public:
		int table_num;
		vector<Table> tables;
		
		Catalog_Manager();
		~Catalog_Manager(){};
		
		//两个重载函数，用来在创建表时调用,向字典中加入新创建表的信息
		bool AddTable(string table_name,vector<string> col_name,vector<string> col_type,vector<bool> col_unique);
		bool AddTable(string table_name,vector<string> col_name,vector<string> col_type,vector<bool> col_unique,string pri_key);		
		
		//创建索引 
		bool AddIndex(string index_name,string table_name,string col_name);
		
		//删除表
		bool DropTable(string table_name);
		
		//删除表上的索引
		bool DropIndex(string table_name,string index_name);
		
		//在字典文件中的记录数加recordNum，用于insert values
		bool InsertRecord(string table_name);
		bool InsertRecord(string table_name,int record_num);
		
		//删除记录 
		bool DeleteRecord(string table_name);
		bool DeleteRecord(string table_name,int delete_num);
		
		//计算长度
		int CalLength(string s); 
		int CalLength2(string table_name);
		
		//写回文件 
		bool WriteBack();
};
