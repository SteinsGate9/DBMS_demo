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
	int record_num; //��¼��(�����жϱ��Ƿ�Ϊ��?) 
	int index_num;
	string pri_key;
	vector<string> col_name; //�������� 
	vector<string> col_type; //����
	vector<bool> col_unique; //�Ƿ�Ψһ 
	vector<Index> indexes; //����(����ʱ���ܴ���) 
	vector<int> col_length;	//�ֶγ���(����char) 
};

class Catalog_Manager
{
	public:
		int table_num;
		vector<Table> tables;
		
		Catalog_Manager();
		~Catalog_Manager(){};
		
		//�������غ����������ڴ�����ʱ����,���ֵ��м����´��������Ϣ
		bool AddTable(string table_name,vector<string> col_name,vector<string> col_type,vector<bool> col_unique);
		bool AddTable(string table_name,vector<string> col_name,vector<string> col_type,vector<bool> col_unique,string pri_key);		
		
		//�������� 
		bool AddIndex(string index_name,string table_name,string col_name);
		
		//ɾ����
		bool DropTable(string table_name);
		
		//ɾ�����ϵ�����
		bool DropIndex(string table_name,string index_name);
		
		//���ֵ��ļ��еļ�¼����recordNum������insert values
		bool InsertRecord(string table_name);
		bool InsertRecord(string table_name,int record_num);
		
		//ɾ����¼ 
		bool DeleteRecord(string table_name);
		bool DeleteRecord(string table_name,int delete_num);
		
		//���㳤��
		int CalLength(string s); 
		int CalLength2(string table_name);
		
		//д���ļ� 
		bool WriteBack();
};
