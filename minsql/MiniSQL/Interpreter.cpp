
#include "Interpreter.h"
#include <string>
#include <iostream>
using namespace std;
#define CREATE 0  //first key
#define SELECT 1
#define DROP 2
#define DELETE 3
#define INSERT 4
#define QUIT 5 
#define COMMIT 13
#define EXECFILE 14

#define TABLE 6   //second key
#define INDEX 7
#define UNKNOWN 8

#define AND 11	//logic
//#define OR 12


int Interpreter::interpreter(string s)
{

	int tmp=0;
	condNum = 0;
	string word;
	string col1;
	string op;
	string con;
	COND a;

	word = getWord(s, &tmp);

//��һ�㣬��ȡ�ڸ��ؼ���
	if (strcmp(word.c_str(), "create") == 0)
		firstKey = CREATE;
	else if (strcmp(word.c_str(), "select") == 0)
		firstKey = SELECT;
	else if (strcmp(word.c_str(), "drop") == 0)
		firstKey = DROP;
	else if (strcmp(word.c_str(), "delete") == 0)
		firstKey = DELETE;
	else if (strcmp(word.c_str(), "insert") == 0)
		firstKey = INSERT;
	else if (strcmp(word.c_str(), "quit") == 0)
		firstKey = QUIT;
	else if (strcmp(word.c_str(), "commit") == 0)
		firstKey = COMMIT;
	else if (strcmp(word.c_str(), "execfile") ==0)
		firstKey = EXECFILE;
	else
	{
		cout<<"Error, command "<<word<<" not found"<<endl;
		return 0;
	}

//�ڶ���
	if (firstKey == CREATE)		//��һ�ؼ���Ϊcreate������Ҫ��ȡ�ڶ��ؼ����жϴ�������
	{
		word = getWord(s,&tmp);
		if (strcmp(word.c_str(), "table") == 0)
			secondKey = TABLE;
		else if(strcmp(word.c_str(), "index") == 0)
			secondKey = INDEX;
		else
		{
			cout<<"Error, Usage: "<<firstKey<<" table or index fileName"<<endl;
			return 0;
		}

		if (secondKey == TABLE)
		{
			word = getWord(s,&tmp);
			if (!word.empty())			//create table tablename ��ȷ
				fileName = word;
			else
			{
				cout<<"Error, Usage: "<<firstKey<<" table or index fileName"<<endl;
				return 0;
			}
			if (secondKey == TABLE)			//�ڶ��ؼ���Ϊtable����������
			{
				word = getWord(s,&tmp);
				if (word.empty() || strcmp(word.c_str(),"(") != 0)
				{
					cout<<"Error in syntax!"<<endl;
					return 0;
				}
				else				//�ڶ�ȡ�����ź���һ��¼һ�µ����������ԡ�
				{
					word = getWord(s,&tmp);
					while (!word.empty() && strcmp(word.c_str(),"primary") != 0 && strcmp(word.c_str(),")") != 0)
					{
						col.push_back(word);
						word = getWord(s,&tmp);
						if (!word.empty())
							type.push_back(word);
						else
						{
							cout<<"Error in syntax!"<<endl;
							return 0;
						}

						if (secondKey == TABLE)
						{
							word = getWord(s,&tmp);
							if (strcmp(word.c_str(),"unique") == 0)
							{
								uniq.push_back(1);
								word = getWord(s,&tmp);
								if (strcmp(word.c_str(),",") != 0)
								{
									cout<<"Error in syntax!"<<endl;
									return 0;
								}
							}
							else if (strcmp(word.c_str(),",") == 0 || strcmp(word.c_str(),")") == 0)
								uniq.push_back(0);
							else if (word.empty() || strcmp(word.c_str(),")") != 0)
							{
								cout<<"Error in syntax!"<<endl;
								return 0;
							}
						}
						if (strcmp(word.c_str(),")") != 0)
							word = getWord(s,&tmp);
					}
					if (strcmp(word.c_str(),"primary") == 0)	//����ǰ�����ַ�Ϊprimaryʱ���ж�Ϊ�������壬�˳��������Զ�ȡ��ѭ����
					{
						word = getWord(s,&tmp);
						if (strcmp(word.c_str(),"key") != 0)
						{
							cout<<"Error in syntax!"<<endl;
							return 0;
						}
						else
						{
							word = getWord(s,&tmp);
							if (strcmp(word.c_str(),"(") == 0)
							{
								word = getWord(s,&tmp);
								primKey = word;
								word = getWord(s,&tmp);
								if (strcmp(word.c_str(),")") != 0)
								{
									cout<<"Error in syntax!"<<endl;
									return 0;
								}
							}
							else
							{
								cout<<"Error in syntax!"<<endl;
								return 0;
							}
						}
					}
					else if (word.empty())
					{
						cout<<"Error in syntax!"<<endl;
						return 0;
					}
				}
			}
		}
		else if (secondKey == INDEX)			//���ڶ��ؼ���Ϊindexʱ���ж�Ϊ����������
		{
			word = getWord(s,&tmp);
			if (!word.empty())			//create index indexname ��ȷ
				fileName = word;
			else
			{
				cout<<"Error in syntax!"<<endl;
				return 0;
			}
			if (secondKey == INDEX)			//�Դ��������������������﷨�ж�
			{
				word = getWord(s,&tmp);
				if (strcmp(word.c_str(),"on") == 0)
				{
					word = getWord(s,&tmp);
					if (!word.empty())
					{
						tableName = word;
						word = getWord(s,&tmp);
						if (strcmp(word.c_str(),"(") == 0)
						{
							word = getWord(s,&tmp);
							if (!word.empty())
							{
								colName = word;
								word = getWord(s,&tmp);
								if (strcmp(word.c_str(),")") != 0)
								{
									cout<<"Error in syntax!"<<endl;
									return 0;
								}
							}
							else
							{
								cout<<"Error in syntax!"<<endl;
								return 0;
							}
						}
						else
						{
							cout<<"Error in syntax!"<<endl;
							return 0;
						}
					}
					else
					{
						cout<<"Error in syntax!"<<endl;
						return 0;
					}
				}
				else
				{
					cout<<"Error in syntax!"<<endl;
					return 0;
				}
			}
		}
	}


	else if (firstKey == SELECT)		//��һ�ؼ���Ϊselect
	{
		cout << "yes!I'm select!" << endl;
		word = getWord(s,&tmp);
		if (strcmp(word.c_str(), "*") != 0)	//��Ҫminisqlֻ֧��select *�Ĳ��ҷ�ʽ����������Գ���select *����������Ϊ������
		{
			cout<<"Error in syntax!"<<endl;
			return 0;
		}

			word = getWord(s,&tmp);
			if (strcmp(word.c_str(), "from") != 0)
			{
				cout<<"Error in syntax!"<<endl;
				return 0;
			}
			
			word = getWord(s,&tmp);		//����
			cout << "table: " << word << endl;
			if (!word.empty())
				fileName = word;
			else
			{
				cout<<"Error in syntax!"<<endl;
				return 0;
			}
			
			word = getWord(s,&tmp);
			if (word.empty())	//������
				condNum = 0;
			else if (strcmp(word.c_str(),"where") == 0)	//����where
			{
				cout << "condition:" << endl;
				word = getWord(s,&tmp);		//col1
				if (!word.empty())
				{
					//col1.push_back(word);
					col1 = word;
					condNum = 1;
					word = getWord(s,&tmp);
					if (word.empty())
					{
						condNum = -1;
						cout<<"Error in syntax!"<<endl;
						return 0;
					}
					else if (strcmp(word.c_str(),"<=") == 0)
						op = "<=";
					else if (strcmp(word.c_str(),">=") == 0)
						op = ">=";
					else if (strcmp(word.c_str(),"<") == 0)
						op = "<";
					else if (strcmp(word.c_str(),">") == 0)
						op = ">";
					else if (strcmp(word.c_str(),"=") == 0)
						op = "=";
					else if (strcmp(word.c_str(),"<>") == 0)
						op = "<>";
					else
					{
						condNum = -1;
						cout<<"Error in syntax!"<<endl;
						return 0;
					}
					word = getWord(s,&tmp);
					if (!word.empty())
					{
						if (word[0] == 39 && word[word.size()-1] == 39)
							word = word.substr(1,word.size()-2);
						con = word;
					}
					else
					{
						condNum = -1;
						cout<<"Error in syntax!"<<endl;
						return 0;
					}
				}
				else
				{
					condNum = -1;
					cout<<"Error in syntax!"<<endl;
					return 0;
				} 
				a.colName = col1;
				a.operater = op;
				a.cond = con;
				cout << "col1: " << col1 << " op: " << op << " con: " << con << endl;
				cond.push_back(a);


				word = getWord(s,&tmp);
				while (!word.empty())			//��2-n������
				{
					if (strcmp(word.c_str(),"and") == 0)
						logic.push_back(AND);
					else
					{
						condNum = -1;
						cout<<"Error in syntax! The editor only support 'and'."<<endl;
						return 0;
					}

					word = getWord(s,&tmp);		//col
					if (!word.empty())
					{
						col1 = word;
						condNum++;
						word = getWord(s,&tmp);
						if (word.empty())
						{
							condNum = -1;
							cout<<"Error in syntax!"<<endl;
							return 0;
						}
						else if (strcmp(word.c_str(),"<=") == 0)
							op = "<=";
						else if (strcmp(word.c_str(),">=") == 0)
							op = ">=";
						else if (strcmp(word.c_str(),"<") == 0)
							op = "<";
						else if (strcmp(word.c_str(),">") == 0)
							op = ">";
						else if (strcmp(word.c_str(),"=") == 0)
							op = "=";
						else if (strcmp(word.c_str(),"<>") == 0)
							op = "<>";
						else
						{
							condNum = -1;
							cout<<"Error in syntax!"<<endl;
							return 0;
						}
						word = getWord(s,&tmp);
						if (!word.empty())
						{
							if (word[0] == 39 && word[word.size()-1] == 39)
								word = word.substr(1,word.size()-2);
							con = word;
						}
						else
						{
							condNum = -1;
							cout<<"Error in syntax!"<<endl;
							return 0;
						}	
					}
					else
					{
						condNum = -1;
						cout<<"Error in syntax!"<<endl;
						return 0;
					}
					a.colName = col1;
					a.operater = op;
					a.cond = con;
					cond.push_back(a);
					word = getWord(s,&tmp);
				}
			}
			if (condNum < 0)
			{
				cout<<"Error in syntax!"<<endl;
				return 0;
			}
		
	}

	else if (firstKey == DROP)		//����һ�ؼ���Ϊdropʱ��ͬ���жϵڶ��ؼ�����ȷ����������
	{
		word = getWord(s,&tmp);
		if (strcmp(word.c_str(), "table") == 0)
			secondKey = TABLE;
		else if(strcmp(word.c_str(), "index") == 0)
			secondKey = INDEX;
		else
		{
			cout<<"Error in syntax!"<<endl;
			return 0;
		}
		if (secondKey == TABLE)
		{
			word = getWord(s,&tmp);
			if (!word.empty())
				fileName = word;
			else
			{
				cout<<"Error in syntax!"<<endl;
				return 0;
			}
		}
		else if (secondKey == INDEX)
		{
			word = getWord(s,&tmp);
			if (!word.empty())
				fileName = word;
			else
			{
				cout<<"Error in syntax!"<<endl;
				return 0;
			}
		}
	}
	

	else if (firstKey == DELETE)		//��һ�ؼ���Ϊselect
	{
		cout << "yes!i'm delete" << endl;
		word = getWord(s,&tmp);
			if (strcmp(word.c_str(), "from") != 0)
			{
				cout<<"Error in syntax!"<<endl;
				return 0;
			}
			
			word = getWord(s,&tmp);		//����
			if (!word.empty())
				fileName = word;
			else
			{
				cout<<"Error in syntax!"<<endl;
				return 0;
			}
			
			word = getWord(s,&tmp);
			if (word.empty())	//������
				condNum = 0;
			else if (strcmp(word.c_str(),"where") == 0)	//����where
			{
				word = getWord(s,&tmp);		//col1
				if (!word.empty())
				{
					//col1.push_back(word);
					col1 = word;
					condNum = 1;
					word = getWord(s,&tmp);
					if (word.empty())
					{
						condNum = -1;
						cout<<"Error in syntax!"<<endl;
						return 0;
					}
					else if (strcmp(word.c_str(),"<=") == 0)
						op = "<=";
					else if (strcmp(word.c_str(),">=") == 0)
						op = ">=";
					else if (strcmp(word.c_str(),"<") == 0)
						op = "<";
					else if (strcmp(word.c_str(),">") == 0)
						op = ">";
					else if (strcmp(word.c_str(),"=") == 0)
						op = "=";
					else if (strcmp(word.c_str(),"<>") == 0)
						op = "<>";
					else
					{
						condNum = -1;
						cout<<"Error in syntax!"<<endl;
						return 0;
					}
					word = getWord(s,&tmp);
					if (!word.empty())
					{
						if (word[0] == 39 && word[word.size()-1] == 39)
							word = word.substr(1,word.size()-2);
						con = word;
					}
					else
					{
						condNum = -1;
						cout<<"Error in syntax!"<<endl;
						return 0;
					}
				}
				else
				{
					condNum = -1;
					cout<<"Error in syntax!"<<endl;
					return 0;
				}
				a.colName = col1;
				a.operater = op;
				a.cond = con;
				cout << "col1: " << col1 << " op: " << op << " con: " << con << endl;
				cond.push_back(a);


				word = getWord(s,&tmp);
				while (!word.empty())			//��2-n������
				{
					if (strcmp(word.c_str(),"and") == 0)
						logic.push_back(AND);
					else
					{
						condNum = -1;
						cout<<"Error in syntax! The editor only support 'and'."<<endl;
						return 0;
					}

					word = getWord(s,&tmp);		//col
					if (!word.empty())
					{
						col1 = word;
						condNum++;
						word = getWord(s,&tmp);
						if (word.empty())
						{
							condNum = -1;
							cout<<"Error in syntax!"<<endl;
							return 0;
						}
						else if (strcmp(word.c_str(),"<=") == 0)
							op = "<=";
						else if (strcmp(word.c_str(),">=") == 0)
							op = ">=";
						else if (strcmp(word.c_str(),"<") == 0)
							op = "<";
						else if (strcmp(word.c_str(),">") == 0)
							op = ">";
						else if (strcmp(word.c_str(),"=") == 0)
							op = "=";
						else if (strcmp(word.c_str(),"<>") == 0)
							op = "<>";
						else
						{
							condNum = -1;
							cout<<"Error in syntax!"<<endl;
							return 0;
						}
						word = getWord(s,&tmp);
						if (!word.empty())
						{
							if (word[0] == 39 && word[word.size()-1] == 39)
								word = word.substr(1,word.size()-2);
							con = word;
						}
						else
						{
							condNum = -1;
							cout<<"Error in syntax!"<<endl;
							return 0;
						}	
					}
					else
					{
						condNum = -1;
						cout<<"Error in syntax!"<<endl;
						return 0;
					}
					a.colName = col1;
					a.operater = op;
					a.cond = con;
					//cout << "col1: " << col1 << " op: " << op << " con: " << con << endl;
					cond.push_back(a);
					word = getWord(s,&tmp);
				}
			}
			if (condNum < 0)
			{
				cout<<"Error in syntax!"<<endl;
				return 0;
			}
		
	}


	else if (firstKey == INSERT)		//��һ�ؼ���Ϊinsertʱ
	{
		word = getWord(s,&tmp);
		if (strcmp(word.c_str(),"into") == 0)
		{
			word = getWord(s,&tmp);
			if (!word.empty())
			{
				fileName = word;
				word = getWord(s,&tmp);
				if (strcmp(word.c_str(),"values") == 0)		//��values�����ֵ��һ��ȡ��
				{
					word = getWord(s,&tmp);
					if (strcmp(word.c_str(),"(") == 0)
					{
						word = getWord(s,&tmp);
						while (!word.empty() && strcmp(word.c_str(),")") != 0)
						{
							insertValue.push_back(word);
							word = getWord(s,&tmp);
							if (strcmp(word.c_str(),",") == 0)
								word = getWord(s,&tmp);
						}
						if (strcmp(word.c_str(),")") != 0)
						{
							cout<<"Error in syntax!"<<endl;
							return 0;
						}
					}
					else
					{
						cout<<"Error in syntax!"<<endl;
						return 0;
					}
				}
				else
				{
					cout<<"Error in syntax!"<<endl;
					return 0;
				}
			}
			else
			{
				cout<<"Error in syntax!"<<endl;
				return 0;
			}
		}
		else
		{
			cout<<"Error in syntax!"<<endl;
			return 0;
		}
	}


	else if (firstKey == QUIT)
	{}

	else if (firstKey == COMMIT)
	{}
	else if (firstKey == EXECFILE)
	{
		fileName = getWord(s,&tmp);

	}
	return 1;
	
}



string Interpreter::getWord(string s, int *tmp)		//�Ӻ��������ã���ȡSQL��������һ���ؼ��֡�
{
	string word;
	int idx1,idx2,i;

	while ((s[*tmp] == ' ' || s[*tmp] == 10) && s[*tmp] != 0)	//ȥ��ǰ�����õĿո񼰻��з�
	{
		(*tmp)++;
	}
	idx1 = *tmp;

	if (s[*tmp] == '(' || s[*tmp] == ',' || s[*tmp] == ')')		//�������š����ź����������ر���
	{
		(*tmp)++;
		idx2 = *tmp;
		word = s.substr(idx1,idx2-idx1);
		return word;
	}
	else if (s[*tmp] == 39)					//�Ե��������ַ����ر���
	{
		(*tmp)++;
		while (s[*tmp] != 39 && s[*tmp] !=0)
			(*tmp)++;
		if (s[*tmp] == 39)
		{
			idx1++;
			idx2 = *tmp;
			(*tmp)++;
			word = s.substr(idx1,idx2-idx1);
			return word;
		}
		else
		{
			word = "";
			return word;
		}
	}
	else				//����������������������ͨ����
	{
		while (s[*tmp] != ' ' && s[*tmp] != 10 && s[*tmp] != 0 && s[*tmp] != ')' && s[*tmp] != ',')
			(*tmp)++;
		idx2 = *tmp;
		if (idx1 != idx2)
			word = s.substr(idx1,idx2-idx1);
		else
			word = "";
		i = word.find("char");
		if (i >= 0 && s[*tmp] == ')')
		{
			(*tmp)++;
			idx2 = *tmp;
			word = s.substr(idx1,idx2-idx1);
		}
		return word;
	}
}


