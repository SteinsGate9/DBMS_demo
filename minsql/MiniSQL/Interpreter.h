
#ifndef INTERPRETER_H
#define INTERPRETER_H
#include <string>
#include <vector>
#include "stdafx.h"

using namespace std;
class Interpreter{
public:
	int firstKey;		//��һ�ؼ��֡�
	int secondKey;		//�ڶ��ؼ��֡�
	string fileName;	//��������


	int condNum;		//where�������ĸ�����
	
	//logic is always and
	vector<int> logic;		//����֮����߼����ӷ���
	
	//0 or more operaters
	/*
	string operater1;	//����һ�еĹ�ϵ�������
	string operater2;	//�������еĹ�ϵ�������
	*/
	
	//0 or more conditions
	/*
	string col1;		//where�ؼ��ֺ��һ���������漰��������
	string col2;		//where�ؼ��ֺ�ڶ����������漰�������� 
	string  condition1;	//where�ؼ��ֺ��һ���������߼��������ĳ�����
	string  condition2;	//where�ؼ��ֺ�ڶ����������߼��������ĳ�����
	*/
/*
	vector<string> col1;		//where�ؼ��ֺ������ 
	vector<string> operater;	//where�ؼ��ֺ�Ĺ�ϵ����� 
	vector<string> condition;	//where�ؼ��ֺ�and�߼��������ĳ��� 
*/	
	vector<COND> cond;
	
	vector<string> insertValue;  	//insert�����values�������еĸ���ֵ��������һ��vector�����У�

	
	vector<string> type;		//create��佨��ʱ�������ԣ���˳�����һ��vector������
	vector<string> col;		//create��佨��ʱ��������
	vector<int> uniq;		//create��佨��ʱ��������
	string primKey;			//create��佨��ʱ����ĸñ��primary key

	string tableName;
	string colName;

	int interpreter(string s);	

	string getWord(string s, int *st);
	
	Interpreter(){}
	~Interpreter(){}
};

#endif
