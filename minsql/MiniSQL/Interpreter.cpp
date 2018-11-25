
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

//第一层，读取第个关键字
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

//第二层
	if (firstKey == CREATE)		//第一关键字为create，则需要读取第二关键字判断创建对象。
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
			if (!word.empty())			//create table tablename 正确
				fileName = word;
			else
			{
				cout<<"Error, Usage: "<<firstKey<<" table or index fileName"<<endl;
				return 0;
			}
			if (secondKey == TABLE)			//第二关键字为table，即创建表
			{
				word = getWord(s,&tmp);
				if (word.empty() || strcmp(word.c_str(),"(") != 0)
				{
					cout<<"Error in syntax!"<<endl;
					return 0;
				}
				else				//在读取左括号后逐一记录一下的列名和属性。
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
					if (strcmp(word.c_str(),"primary") == 0)	//当当前操作字符为primary时，判断为主键定义，退出列名属性读取的循环。
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
		else if (secondKey == INDEX)			//当第二关键字为index时，判定为创建索引。
		{
			word = getWord(s,&tmp);
			if (!word.empty())			//create index indexname 正确
				fileName = word;
			else
			{
				cout<<"Error in syntax!"<<endl;
				return 0;
			}
			if (secondKey == INDEX)			//对创建索引的语句进行深入语法判断
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


	else if (firstKey == SELECT)		//第一关键字为select
	{
		cout << "yes!I'm select!" << endl;
		word = getWord(s,&tmp);
		if (strcmp(word.c_str(), "*") != 0)	//又要minisql只支持select *的查找方式，所以这里对除了select *以外的情况作为错误处理。
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
			
			word = getWord(s,&tmp);		//表名
			cout << "table: " << word << endl;
			if (!word.empty())
				fileName = word;
			else
			{
				cout<<"Error in syntax!"<<endl;
				return 0;
			}
			
			word = getWord(s,&tmp);
			if (word.empty())	//无条件
				condNum = 0;
			else if (strcmp(word.c_str(),"where") == 0)	//存在where
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
				while (!word.empty())			//第2-n个条件
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

	else if (firstKey == DROP)		//当第一关键字为drop时，同样判断第二关键字以确定操作对象
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
	

	else if (firstKey == DELETE)		//第一关键字为select
	{
		cout << "yes!i'm delete" << endl;
		word = getWord(s,&tmp);
			if (strcmp(word.c_str(), "from") != 0)
			{
				cout<<"Error in syntax!"<<endl;
				return 0;
			}
			
			word = getWord(s,&tmp);		//表名
			if (!word.empty())
				fileName = word;
			else
			{
				cout<<"Error in syntax!"<<endl;
				return 0;
			}
			
			word = getWord(s,&tmp);
			if (word.empty())	//无条件
				condNum = 0;
			else if (strcmp(word.c_str(),"where") == 0)	//存在where
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
				while (!word.empty())			//第2-n个条件
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


	else if (firstKey == INSERT)		//第一关键字为insert时
	{
		word = getWord(s,&tmp);
		if (strcmp(word.c_str(),"into") == 0)
		{
			word = getWord(s,&tmp);
			if (!word.empty())
			{
				fileName = word;
				word = getWord(s,&tmp);
				if (strcmp(word.c_str(),"values") == 0)		//对values后面的值逐一读取。
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



string Interpreter::getWord(string s, int *tmp)		//子函数，作用：读取SQL命令中下一个关键字。
{
	string word;
	int idx1,idx2,i;

	while ((s[*tmp] == ' ' || s[*tmp] == 10) && s[*tmp] != 0)	//去除前面无用的空格及换行符
	{
		(*tmp)++;
	}
	idx1 = *tmp;

	if (s[*tmp] == '(' || s[*tmp] == ',' || s[*tmp] == ')')		//对左括号、逗号和右括号做特别处理。
	{
		(*tmp)++;
		idx2 = *tmp;
		word = s.substr(idx1,idx2-idx1);
		return word;
	}
	else if (s[*tmp] == 39)					//对单引号内字符做特别处理。
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
	else				//除上述几种特殊情况外的普通处理
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


