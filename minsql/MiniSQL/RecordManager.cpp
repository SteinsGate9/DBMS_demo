

#include "stdafx.h"
#include "RecordManager.h"
#include "BufferManager.h"
#include "API.h"
#include <cstring>
#include <iostream>

//宏定义，块大小，逻辑符
#define BLOCK_SIZE 4096   
#define EXTRA_LEN  7
#define EXTRA_LENshort 6
#define JINGHAO 35
#define INFI 4294967295
#define MAX_ATTRIBUTES 32
#define OUT_SIGNAL -1
#define AND 11
#define OR 12
class BufferManager;
class API;


//外部变量声明
extern BufferManager bm;   
extern API ap;

using namespace std;
//根据给定的表名，将表中的记录全部删除
int RecordManager::deleteValue(string tableName)
{
	//查找文件节点
	BufferManager::File * fp=bm.getFile(tableName.c_str());	
	if(fp!=NULL)
	{
		BufferManager::Block * bp=fp->blockHead;

		//如果文件节点下面没有块节点挂载，则从文件中读取一个块，并将它挂到文件节点下面
		if(bp==NULL)
		{
			bp=bm.getBlock(fp,0);
			fp->blockHead = bp;
		}
		if (bp == NULL || bp->usage == 0)
			return 0;
		while(bp)
		{
			//lru清零 表示随时可用
			bp->lru = 0;
			//dirty置1表示已经修改
			bp->dirty = 1;
			//将内存数据全部清0,使用量设为0
			memset(bp->address,0,BLOCK_SIZE);
			bp->usage=0;

			//如果块链表中没有下一个块，读取文件中的下一个块进行挂载
			if(bp->nextBlock==NULL)
				bp=bm.getBlock(fp,1);
			else
				bp=bp->nextBlock;
		}
		
	}
	//返回删除的行数目
	return ap.getRecordNum(tableName);
}


//根据给定的表名和一个where条件，删除表中的满足条件的记录
int RecordManager::deleteValue(string tableName, string colName1,string cond1,string operater1)
{
	//得到文件节点
	BufferManager::File * fp=bm.getFile(tableName.c_str());	
	int deleteNum=0;

	if(fp!=NULL)
	{
		vector<string> collName = ap.getCollName(tableName);
		vector<string> collType = ap.getCollType(tableName);
		string primaryKey = ap.getPrimaryKey(tableName);
        int recordLen = ap.calcuteLenth(tableName);
		char * p = NULL;
		char * pStart = NULL;
		char * lastNodeChanged = NULL;
		char * tobeChanged = NULL;
		char * pPrev = NULL;
		char value[255];
		int findstart = 0;
		int typeLen = 0;
		int flag = 0;
		string type;
		
		
		

		BufferManager::Block * bp=fp->blockHead;
 		if(bp==NULL)
		{
			bp=bm.getBlock(fp,1);
			fp->blockHead=bp;
		}
		if (bp == NULL || bp->usage == 0)
			return 0;
		//准备工作
		else if (bp != NULL)
		{
			for (unsigned int j = 0; j < collName.size(); j++)
			{
				type = collType.at(j);
				typeLen = ap.calcuteLenth2(type);
				//找到对应的比较字段，提取其值
				if (collName.at(j) == colName1)
				{
					break;
				}
				findstart += typeLen;
			}
		}

		//分情况讨论
		if (colName1 == primaryKey && (operater1 == ">" || operater1 == ">="))
		{
			while (bp)
			{
				//p先给一个块头的地址
				p = bp->address;
				//判断真.块大小
				int realBlockSize = BLOCK_SIZE - BLOCK_SIZE % bp->recordLength;
				//此处每次*P的判断，都是一个字段的开始字节，所以当*P为0时，这个块已经被读完
				while (p - bp->address < realBlockSize)
				{
					//pStart指向每条数据的头
					pStart = p;    	
					//p作为游标移动
					p += findstart;
					//value赋值
					memcpy(value, p, typeLen);
					//找到新的地址
					char * tmp = pStart + recordLen - EXTRA_LENshort;
					int x;
					memcpy(&x, tmp, sizeof(int));
					//也就是删除前原来记录的位置
					if (condCheck(type, value, cond1, operater1))
					{
						//直接清空这条语句，然后把记录
						memset(pStart, 0, recordLen);
						//记录第一次突破>的地址
						if (!flag)
						{
							//有可能是preV或者第一条地址
							tobeChanged = (pPrev ? pPrev : pStart);
							flag = 1;
						}
						//更新usage
						bp->usage = bp->usage - recordLen;
						//更新dirty
						if (bp->dirty != 1)
							bp->dirty = 1;
						//更新deleteNum
						deleteNum++;
					}
					//如果是out
					if (x != OUT_SIGNAL)  //0xFFFF
					{
						p = bp->address + ((x) - bp->offsetNum *(realBlockSize) / bp->recordLength) *recordLen;
						pPrev = pStart;
					}
					//如果不是
					else
					{
						//更新未被删除的最后一个节点
						for (int i = 0; i < 4; i++)
						{
							tobeChanged[i + recordLen - EXTRA_LENshort] = -1;
						}
						return deleteNum;
					}
				}
				//找到下一个块
				if (bp->nextBlock == NULL)
					bp = bm.getBlock(fp, 1);
				else
					bp = bp->nextBlock;
			}
		}
		
		else if ((colName1 == primaryKey && (operater1 == "<" || operater1 == "<=")))
		{
			while (bp)
			{
				//p先给一个块头的地址
				p = bp->address;
				//判断真.块大小
				int realBlockSize = BLOCK_SIZE - BLOCK_SIZE % bp->recordLength;
				while (p - bp->address < realBlockSize)
				{
					//pStart指向每条数据的头
					pStart = p;
					//p作为游标移动
					p += findstart;
					//value赋值
					memcpy(value, p, typeLen);
					//找到新的地址
					char * tmp = pStart + recordLen - EXTRA_LENshort;
					int x;
					memcpy(&x, tmp, sizeof(int));
					if (condCheck(type, value, cond1, operater1))
					{	
						//直接清空这条语句，然后把记录
						memset(pStart, 0, recordLen);
						//更新usage
						bp->usage = bp->usage - recordLen;
						//更新dirty
						if (bp->dirty != 1)
							bp->dirty = 1;
						//更新deleteNum
						deleteNum++;
					}
					else
					{
						flag = 1;
					}
					if (x != OUT_SIGNAL)  //0xFFFF
					{
						p = bp->address + ((x) - bp->offsetNum *(realBlockSize) / bp->recordLength) *recordLen;
					}
					if (flag == 1)
					{
						//把未被删除的第一条传给地址的第一个位置
						char *tmp = pStart;
						for (int i = 0; i < recordLen; i++)
						{
							bp->address[i] = tmp[i];
							tmp[i] = 0;
						}
						return deleteNum;
					}
					else if (x == OUT_SIGNAL)
					{
						return deleteNum;
					}
				}

				if (bp->nextBlock == NULL)
					bp = bm.getBlock(fp, 1);
				else
					bp = bp->nextBlock;
			}
		}
		else if (operater1 == "=")
		{
			while (bp)
			{
				//p先给一个块头的地址
				p = bp->address;
				//判断真.块大小
				int realBlockSize = BLOCK_SIZE - BLOCK_SIZE % bp->recordLength;
				while (p - bp->address < realBlockSize)
				{
					//pStart指向每条数据的头
					pStart = p;
					//p作为游标移动
					p += findstart;
					//value赋值
					memcpy(value, p, typeLen);
					//找到新的地址
					char * tmp = pStart + recordLen - EXTRA_LENshort;
					int x;
					memcpy(&x, tmp, sizeof(int));
					if (condCheck(type, value, cond1, operater1))
					{
						if (pStart == bp->address)
						{
							flag = 1;
						}
						else
						{
							flag = 2;
						}	
						bp->usage = bp->usage - recordLen;
						//更新deleteNum
						deleteNum++;
						if (bp->dirty != 1)
							bp->dirty = 1;
					}
					if (x != OUT_SIGNAL)  //0xFFFF
					{
						p = bp->address + ((x) - bp->offsetNum *(BLOCK_SIZE - BLOCK_SIZE % bp->recordLength) / bp->recordLength) *recordLen;
					}
					if (!flag)
					{
						pPrev = pStart;
					}
					if (flag == 1)//第一
					{
						//把下一条语句挪到第一个
						char *temp = bp->address + ((x) - bp->offsetNum *(BLOCK_SIZE - BLOCK_SIZE % bp->recordLength) / bp->recordLength) *recordLen;
						for (int i = 0; i < recordLen; i++)
						{
							bp->address[i] = temp[i];
							temp[i] = 0;
						}
						
						return deleteNum;
					}
					else if (flag == 2)//非第一
					{ 
						//直接清空这条语句，然后把记录
						memset(pStart, 0, recordLen);
						//更新上一条语句的值
						pPrev = pPrev + recordLen - EXTRA_LENshort;
						memcpy(pPrev, tmp, 4);
						return deleteNum;
						
					}
					else if (x == OUT_SIGNAL)
					{
						return deleteNum;
					}
				}

				if (bp->nextBlock == NULL)
					bp = bm.getBlock(fp, 1);
				else
					bp = bp->nextBlock;
			}
		}
		//如果是非主键删除
		else
		{
			while (bp)
			{
				//p先给一个块头的地址
				p = bp->address;
				//判断真.块大小
				int realBlockSize = BLOCK_SIZE - BLOCK_SIZE % bp->recordLength;
				//
				int maxLines = realBlockSize / recordLen;
				while (p - bp->address < realBlockSize)
				{
					//pStart指向每条数据的头
					pStart = p;
					//p作为游标移动
					p += findstart;
					//value赋值
					memcpy(value, p, typeLen);
					//找到新的地址
					char * tmp = pStart + recordLen - EXTRA_LENshort;
					int x;
					memcpy(&x, tmp, sizeof(int));
					if (condCheck(type, value, cond1, operater1))
					{
						memset(pStart, 0, recordLen);
						bp->usage = bp->usage - recordLen;
						//更新deleteNum
						deleteNum++;
						if (bp->dirty != 1)
							bp->dirty = 1;
					}
					if (x != OUT_SIGNAL)  //0xFFFF
					{
						bp = fp->blockHead;
						while (x >= maxLines)
						{
							if (bp->nextBlock)
								bp = bp->nextBlock;
							else
								bp = bm.getBlock(fp, 0);
							x = x - maxLines;
						}
						p = bp->address + x*recordLen;
					}
					else if (x == OUT_SIGNAL)
					{
						bm.reconstruct(bp);
						return deleteNum;
					}
				}
				if (bp->nextBlock == NULL)
					bp = bm.getBlock(fp, 1);
				else
					bp = bp->nextBlock;
			}
		}
	}	
	return deleteNum;
}
//


//通过给定文件名查找所有的表中的记录。
int RecordManager::selectRecord(string tableName)
{
	BufferManager::File * fp=bm.getFile(tableName.c_str());	
	
	if(fp!=NULL)
	{
		int recordLen=ap.calcuteLenth(tableName);
		vector<string> collName=ap.getCollName(tableName);
		vector<string> collType=ap.getCollType(tableName);
		char * p;
		char value[255];
		memset(value,0,255);  //将数组置0，否则在输出字符串时没有结尾
		int valueLen;
		int maxLines = 0;
		int realBlockSize = 0;
		int selectNum = 0;
		string type;
		

		//如果文件节点下面没有块节点挂载，则从文件中读取一个块，并将它挂到文件节点下面
		BufferManager::Block * bp=fp->blockHead;
		if (bp == NULL)
		{
			bp = bm.getBlock(fp, 1);
			fp->blockHead = bp;
		}
		//读了之后再检验
        if (bp == NULL||bp->usage == 0)
			return 0;

		//画横线为了美观
		{
			cout << " ";
			for (unsigned int i = 0; i<collName.size(); i++)cout << "-----";cout << "\n";cout << "|";
			for (unsigned int i = 0; i < collName.size(); i++)
			{
				cout << collName.at(i);
				if (i != collName.size() - 1)
					cout << "\t";
			}cout << "|";cout << "\n";
			cout << " ";
			for (unsigned int i = 0; i<collName.size(); i++)cout << "-----";cout << "\n";
		}
		//开始正式输出
		while(bp)
		{			
			p = bp->address;
			//判断真.块大小
			realBlockSize = BLOCK_SIZE - BLOCK_SIZE % recordLen;
			//
			maxLines = realBlockSize / recordLen;
			while(p - bp->address < realBlockSize)
			{
				
				for(unsigned int j=0;j<collName.size();j++)
				{
					type=collType.at(j);
					valueLen = ap.calcuteLenth2(type);	
					memcpy(value, p, valueLen);
					p = p + valueLen;
					if(j==0)
					cout << "|";
					//按照对应的类型，将字段的值输出
					if(type=="int")
					{
						int * x;
						x=(int *)value;
						if (j != 0)
							cout << "\t" << (*x);
						else
							cout << (*x);
					}
					else if(type=="float")
					{
						float * x;
						x=(float *)value;
						if (j != 0)
						cout << "\t" << (*x);
						else
							cout << (*x);
					}
					else
						if (j != 0)
						cout << "\t" << value;
						else
							cout << (value);
	
				}
				selectNum++;
				char * tmp = p + 1;
				int x;
				memcpy(&x, tmp, sizeof(int));
				cout << "|" << endl;
				if (x != OUT_SIGNAL)  //0xFFFF
				{	
					bp = fp->blockHead;
					while (x >= maxLines)
					{
						if (bp->nextBlock)
							bp = bp->nextBlock;
						else
							bp = bm.getBlock(fp, 0);
						x = x - maxLines;
					}
					p = bp->address + x * recordLen;
				}
				else
				{
					cout << " ";
					for (unsigned int i = 0; i<collName.size(); i++)
						cout << "-----";
					cout << "\n";
					return selectNum;
				}
			}
	
			if(bp->nextBlock==NULL)
				bp=bm.getBlock(fp,1);
			else
				bp=bp->nextBlock;
		}
		return selectNum;
	}
	else
		return 0;
}


//通过一个给定的文件名和一个where条件来查找表中的记录。
int RecordManager::selectRecord(string tableName, string colName1, string cond1, string operater1)
{
	BufferManager::File * fp = bm.getFile(tableName.c_str());

	if (fp != NULL)
	{
		int recordLen = ap.calcuteLenth(tableName);
		string primaryKey = ap.getPrimaryKey(tableName);
		vector<string> collName = ap.getCollName(tableName);
		vector<string> collType = ap.getCollType(tableName);
		char * p = NULL;
		char * pStart = NULL;
		char value[255];
		memset(value, 0, 255);  //将数组置0，否则在输出字符串时没有结尾
		int valueLen;
		string type;
		int typeLen;
		int findstart = 0;
		int selectNum = 0;
		int selected = 0;
		int realBlockSize = 0;
		int maxLines = 0;
		//如果文件节点下面没有块节点挂载，则从文件中读取一个块，并将它挂到文件节点下面
		BufferManager::Block * bp = fp->blockHead;
		if (bp == NULL)
		{
			bp = bm.getBlock(fp, 1);
			fp->blockHead = bp;
		}
		//读了之后再检验
		if (bp == NULL || bp->usage == 0)
			return 0;
		else if (bp != NULL)
		{
			p = bp->address;
			for (unsigned int j = 0; j < collName.size(); j++)
			{
				type = collType.at(j);
				typeLen = ap.calcuteLenth2(type);
				//找到对应的比较字段，提取其值
				if (collName.at(j) == colName1)
				{
					break;
				}
				findstart += typeLen;
			//	p += typeLen;
			}
		}
		//画横线为了美观
		{
			for (unsigned int i = 0; i<collName.size(); i++)
				cout << "-----";
			cout << "\n";

			cout << "|";
			for (unsigned int i = 0; i < collName.size(); i++)
			{
				cout << collName.at(i);
				if (i != collName.size() - 1)
					cout << "\t";
			}
			cout << "|";

			cout << "\n";
			for (unsigned int i = 0; i<collName.size(); i++)
				cout << "-----";
			cout << "\n";

		}
		while (bp)
		{
			//设置p初值
			p = bp->address;
			//判断真.块大小
			realBlockSize = BLOCK_SIZE - BLOCK_SIZE % recordLen;
			//dasd
			maxLines = realBlockSize / recordLen;
			while (p - bp->address < realBlockSize)
			{	
				pStart = p;    //用tmpp记录每条记录的开始位置	
				p += findstart;
				memcpy(value, p, typeLen);
				p = pStart;
				selected = 0;
				//取地址
				char * tmp = pStart + recordLen - EXTRA_LENshort;
				int x;
				memcpy(&x, tmp, sizeof(int));
				if (condCheck(type, value, cond1, operater1))
				{
					selected = 1;
					selectNum++;
					for (unsigned int j = 0; j < collName.size(); j++)
					{
						type = collType.at(j);
						valueLen = ap.calcuteLenth2(type);
						memcpy(value, p, valueLen);
						p = p + valueLen;
						if (j == 0)
							cout << "|";
						//按照对应的类型，将字段的值输出
						if (type == "int")
						{
							int * x;
							x = (int *)value;
							if (j != 0)
								cout << "\t" << (*x);
							else
								cout << (*x);
						}
						else if (type == "float")
						{
							float * x;
							x = (float *)value;
							if (j != 0)
								cout << "\t" << (*x);
							else
								cout << (*x);
						}
						else
							if (j != 0)
								cout << "\t" << value;
							else
								cout << (value);
					}
					if (colName1 == primaryKey && operater1 == "=")
					{
						if (selected)
							cout << "|" << endl;
						for (unsigned int i = 0; i < collName.size(); i++)
							cout << "-----";
						cout << "\n";
						return 1;
					}
				  }
				else
				{
					if (colName1 == primaryKey && operater1 == "<" || operater1 == "<=")
					{
						if (selected)
							cout << "|" << endl;
						for (unsigned int i = 0; i < collName.size(); i++)
							cout << "-----";
						cout << "\n";
						return selectNum;
					}
						
				}
				if (x != OUT_SIGNAL)  //0xFFFF
				{
					if (selected)
						cout << "|" << endl;
					bp = fp->blockHead;
					while (x >= maxLines)
					{
						if (bp->nextBlock)
							bp = bp->nextBlock;
						else
							bp = bm.getBlock(fp, 0);
						x = x - maxLines;
					}
					p = bp->address + x*recordLen;
				}
				else
				{
					if (selected)
						cout << "|" << endl;
					for (unsigned int i = 0; i < collName.size(); i++)
						cout << "-----";
					cout << "\n";
					return selectNum;
				}
			}
			if (bp->nextBlock == NULL)
				bp = bm.getBlock(fp, 1);
			else
				bp = bp->nextBlock;
		}
		return selectNum;
	}
	else
		return 0;
}



int RecordManager::selectRecord(string tableName, vector<COND> x)
{
	BufferManager::File * fp = bm.getFile(tableName.c_str());

	if (fp != NULL)
	{
		int recordLen = ap.calcuteLenth(tableName);
		vector<string> collName = ap.getCollName(tableName);
		vector<string> collType = ap.getCollType(tableName);
		char * p;
		char * pStart;
		char value[10][255];
		memset(value, 0, 255);  //将数组置0，否则在输出字符串时没有结尾
		int valueLen = 0;
		int flag = 1;
		int typeLen[10] = {0};
		int findstart[10] = {0};
		int selectNum = 0;
		int selected = 0;
		int realBlockSize = 0;
		int maxLines = 0;
		string type[10];
		string tmp;
		char tmpvalue[255]; memset(tmpvalue, 0, 255);
		//如果文件节点下面没有块节点挂载，则从文件中读取一个块，并将它挂到文件节点下面
		BufferManager::Block * bp = fp->blockHead;
		if (bp == NULL)
		{
			bp = bm.getBlock(fp, 1);
			fp->blockHead = bp;
		}
		//读了之后再检验
		if (bp == NULL || bp->usage == 0)
			return 0;
		else if (bp != NULL)
		{
			for (unsigned int i = 0; i < x.size(); i++)
			{
				//第i个findstart
				for (unsigned int j = 0; j < collName.size(); j++)
				{
					type[i] = collType.at(j);
					typeLen[i] = ap.calcuteLenth2(type[i]);
					if (collName.at(j) == x[i].colName)
					{
						break;
					}
					findstart[i] += typeLen[i];
				}
			}
		}
		//画横线为了美观
		{
			for (unsigned int i = 0; i<collName.size(); i++)
				cout << "-----";
			cout << "\n";

			cout << "|";
			for (unsigned int i = 0; i < collName.size(); i++)
			{
				cout << collName.at(i);
				if (i != collName.size() - 1)
					cout << "\t";
			}
			cout << "|";

			cout << "\n";
			for (unsigned int i = 0; i<collName.size(); i++)
				cout << "-----";
			cout << "\n";

		}
		//开始正式查找
		while (bp)
		{
			p = bp->address;
			//判断真.块大小
			realBlockSize = BLOCK_SIZE - BLOCK_SIZE % recordLen;
			//maxLines 
			maxLines = realBlockSize / recordLen;
			while (p - bp->address < realBlockSize)
			{			
				//用pStart记录每条记录的开始位置	
			    pStart = p;
				//flag = 1
				flag = 1;
				//开始设置各种参数
				for (unsigned int i = 0; i < x.size(); i++)
				{
					
					p += findstart[i];
					memcpy(value[i], p, typeLen[i]);
					p = pStart;

						if (!condCheck(type[i], value[i], x[i].cond, x[i].operater))
						{
							flag = 0;
							break;
						}
				}

				//设置是否选择中
				selected = 0;
				//地址
				char * temp = pStart + recordLen - EXTRA_LENshort;
				int x;
				memcpy(&x, temp, sizeof(int));
				if (flag)
				{
					selected = 1;
					selectNum++;
					for (unsigned int j = 0; j < collName.size(); j++)
					{
						tmp = collType.at(j);
						valueLen = ap.calcuteLenth2(tmp);
						memcpy(tmpvalue, p, valueLen);
						p = p + valueLen;
						if (j == 0)
							cout << "|";
						//按照对应的类型，将字段的值输出
						if (tmp == "int")
						{
							int * x;
							x = (int *)tmpvalue;
							if (j != 0)
								cout << "\t" << (*x);
							else
								cout << (*x);
						}
						else if (tmp == "float")
						{
							float * x;
							x = (float *)tmpvalue;
							if (j != 0)
								cout << "\t" << (*x);
							else
								cout << (*x);
						}
						else
							if (j != 0)
								cout << "\t" << tmpvalue;
							else
								cout << (tmpvalue);

					}
				}
					if (x != OUT_SIGNAL)  //0xFFFF
					{
						if (selected)
							cout << "|" << endl;

						bp = fp->blockHead;
						while (x >= maxLines)
						{
							if (bp->nextBlock)
								bp = bp->nextBlock;
							else
								bp = bm.getBlock(fp, 0);
							x = x - maxLines;
						}
						p = bp->address + x*recordLen;
					}
					else
					{
						if (selected)
							cout << "|" << endl;
						for (unsigned int i = 0; i < collName.size(); i++)
							cout << "-----";
						cout << "\n";
						return selectNum;
					}
				
			}
			if (bp->nextBlock == NULL)
				bp = bm.getBlock(fp, 1);
			else
				bp = bp->nextBlock;
		}
		return selectNum;
	}
	else
		return 0;
}
int RecordManager::deleteValue(string tableName, vector<COND> x)
{
	BufferManager::File * fp = bm.getFile(tableName.c_str());

	if (fp != NULL)
	{
		int recordLen = ap.calcuteLenth(tableName);
		int deleteNum = 0;
		vector<string> collName = ap.getCollName(tableName);
		vector<string> collType = ap.getCollType(tableName);
		char * p;
		char * pStart;
		char value[10][255];	for (int i = 0; i < 10; i++)memset(value[i], 0, 255);  //将数组置0，否则在输出字符串时没有结尾
		int valueLen = 0;
		int flag = 1;
		int typeLen[10] = { 0 };
		int findstart[10] = { 0 };
		int selectNum = 0;
		int selected = 0;
		int maxLines = 0;
		int realBlockSize = 0;
		string type[10];
		string tmp;
		char tmpvalue[255]; memset(tmpvalue, 0, 255);
		//如果文件节点下面没有块节点挂载，则从文件中读取一个块，并将它挂到文件节点下面
		BufferManager::Block * bp = fp->blockHead;
		if (bp == NULL)
		{
			bp = bm.getBlock(fp, 1);
			fp->blockHead = bp;
		}
		//读了之后再检验
		if (bp == NULL || bp->usage == 0)
			return 0;
		else if (bp != NULL)
		{
			for (unsigned int i = 0; i < x.size(); i++)
			{
				//第i个findstart
				for (unsigned int j = 0; j < collName.size(); j++)
				{
					type[i] = collType.at(j);
					typeLen[i] = ap.calcuteLenth2(type[i]);
					if (collName.at(j) == x[i].colName)
					{
						break;
					}
					findstart[i] += typeLen[i];
				}
			}
		}

		while (bp)
		{
			//p先给一个块头的地址
			p = bp->address;
			//判断真.块大小
			realBlockSize = BLOCK_SIZE - BLOCK_SIZE % recordLen;
			//maxLines
			maxLines = realBlockSize / recordLen;
			while (p - bp->address < realBlockSize)
			{
				//pStart指向每条数据的头
				pStart = p;
				//重置flag 
				flag = 1;
				for (unsigned int i = 0; i < x.size(); i++)
				{
					p += findstart[i];
					memcpy(value[i], p, typeLen[i]);
					p = pStart;
					if (!condCheck(type[i], value[i], x[i].cond, x[i].operater))
						flag = 0;
				}
				//找到新的地址
				char * tmp = pStart + recordLen - EXTRA_LENshort;
				int x;
				memcpy(&x, tmp, sizeof(int));
				//如果同时满足两个where条件且为and逻辑时，
				//或者满足其中的一个条件且为OR逻辑时，则进行删除操作
				if (flag)
				{
					//直接清空这条语句，然后把记录
					memset(pStart, 0, recordLen);
					//更新usage
					bp->usage = bp->usage - recordLen;
					//更新deleteNum
					deleteNum++;
					if (bp->dirty != 1)
						bp->dirty = 1;
				}
				else
				{
				
				}
				if (x != OUT_SIGNAL)  //0xFFFF
				{
					bp = fp->blockHead;
					while (x >= maxLines)
					{
						if (bp->nextBlock)
							bp = bp->nextBlock;
						else
							bp = bm.getBlock(fp, 0);
						x = x - maxLines;
					}
					p = bp->address + x*recordLen;
				}
				else if (x == OUT_SIGNAL)
				{
					if(deleteNum)
						bm.reconstructAll(fp);
					return deleteNum;
				}

			}

			if (bp->nextBlock == NULL)
				bp = bm.getBlock(fp, 1);
			else
				bp = bp->nextBlock;
		}
		if(deleteNum)
						bm.reconstructAll(fp);
		return deleteNum;
	}
	return 0;


}
//根据提供的表名和插入的记录字节，向数据块中插入记录。
int RecordManager::insertRecord(string tableName, char * s)
{
	int recordLen = ap.calcuteLenth(tableName);
	int typeLen = 0;
	int typeLenUniq[MAX_ATTRIBUTES]; 
	int findStart = 0;
	int findStartUniq[MAX_ATTRIBUTES] = { 0 }; memset(findStartUniq, 0, MAX_ATTRIBUTES * sizeof(int));
	int maxLines = 0;
	int realBlockSize = 0;

	string type;
	string typeUniq[MAX_ATTRIBUTES]; 
	vector<string> collName = ap.getCollName(tableName);
	vector<string> collType = ap.getCollType(tableName);
	string primaryKey = ap.getPrimaryKey(tableName);
	vector<string> collUniq = ap.getUniq(tableName);
	//得到对应的文件块
 	BufferManager::File * fp=bm.getFile(tableName.c_str());	
	BufferManager::Block * bpForFindMin = fp->blockHead;

	char *pPrev = NULL;
	char *pStart = NULL;
	char * p = NULL;
	char * first = NULL;
	char * t = NULL;
	char a[4];
	char value[255];
	char valueForMin[255];
	char valueForUniq[255];
	if(fp!=NULL)
	{
		//得到文件对应的第一个块
		BufferManager::Block * bp=fp->blockHead;
		//如果是非空文件
		if (bp == NULL)
		{
			bp = bm.getBlock(fp, 0);
			bpForFindMin = bp;
		}
		if (bp != NULL)
		{
			for (unsigned int j = 0; j < collName.size(); j++)
			{
				type = collType.at(j);
				typeLen = ap.calcuteLenth2(type);
				if (collName.at(j) == primaryKey || primaryKey == "no")
				{
					break;
				}
				findStart += typeLen;
			}
			for(unsigned int i = 0; i < collUniq.size(); i++)
				for (unsigned int j = 0; j < collName.size(); j++)
				{
					typeUniq[i] = collType.at(j);
					typeLenUniq[i] = ap.calcuteLenth2(typeUniq[i]);
					if (collName.at(j) == collUniq[i])
					{
						break;
					}
					findStartUniq[i] += typeLen;
				}
		}
			while(1)
			{
				//对使用的块的lru增加1
				bp->lru = 1;
				//判断真.块大小
				realBlockSize = BLOCK_SIZE - BLOCK_SIZE % recordLen;
				//判断最大的行数
				maxLines = realBlockSize / recordLen;
				//如果文件块对应的块中还足够插入，则将记录写入这个块尾，即插入到文件中间，而不是末尾
				if(bp->usage < BLOCK_SIZE - BLOCK_SIZE % recordLen)
				{
					p = bp->address;
					//找到空地址起点
					p=p+bp->usage;
					//更新上一个record的指向地址为当前地址
					vector<string> collType = ap.getCollType(tableName);
					if (bp == fp->blockHead && bp->usage == 0)
					{
						goto usageZero;
					}
					while (bpForFindMin)
					{
						
						//每个findmin设置lru
						bpForFindMin->lru = 1;
						//每个findmin设置初始
						t = bpForFindMin->address;
						//每个findmin设置first第一个
						first = t;
						while (t-bpForFindMin->address < realBlockSize)
						{
							pStart = t;
							for (unsigned int i = 0; i < collUniq.size(); i++)
							{
								memcpy(valueForUniq, t + findStartUniq[i], typeLenUniq[i]);
								memcpy(value, s + findStartUniq[i], typeLenUniq[i]);
								if (strcmp(value, valueForUniq) == 0)
								{
									cout << "ERROR:Uniq Key error" << endl;
									return 0;
								}
							}
							memcpy(value, s + findStart, typeLen);
							memcpy(valueForMin, t + findStart, typeLen);
							if (compare(type, valueForMin, value) == 0 && primaryKey != "no")
							{
								cout << "ERROR:Primary Key error" << endl;
								return 0;
							}
							if (compare(type, valueForMin, value) >= 0)
							{
								if (t == first)
									goto FirstMinOut;
								else
									goto nFirstMinOut;
							}
							char * tmp = t + recordLen - EXTRA_LENshort;
							int x;
							memcpy(&x, tmp, sizeof(int));

							if (x != OUT_SIGNAL)  //0xFFFF
							{
								bpForFindMin = fp->blockHead;
								while (x >= maxLines)
								{
									if (bpForFindMin->nextBlock)
										bpForFindMin = bpForFindMin->nextBlock;
									else
										bpForFindMin = bm.getBlock(fp, 0);
									x = x - maxLines;
								}
								t = bpForFindMin->address + x*recordLen;
								pPrev = pStart;
							}
							else
							{
								pPrev = t;
								goto EndOut;
							}				
						}
						if (bp->nextBlock)
							bpForFindMin = bpForFindMin->nextBlock;
						else
							bpForFindMin = bm.getBlock(fp, 1);
					}
					int recordNum;
				usageZero:
					recordNum = ap.getRecordNum(tableName);
					//更新现在的地址为-1
					recordNum = INFI;
					memcpy(a, &recordNum, sizeof(int));
					//插入#b1b2b3b4 OD OA
					char g3[EXTRA_LEN] = { JINGHAO,a[0],a[1],a[2],a[3],13,10 };
					memcpy(p + recordLen - EXTRA_LEN, g3, EXTRA_LEN);
					//插入数据
					memcpy(p, s, recordLen - EXTRA_LEN);
					//更新块和文件块参数
					bp->usage = bp->usage + recordLen;
					bp->dirty = 1;

					//cout << "Insert Succeeded" << endl;
					return 1;
  
				nFirstMinOut:
					recordNum = ap.getRecordNum(tableName);
					int previous;
					//更新现在的地址为上一个节点yuanben de xiayige
					memcpy(&previous, pPrev + recordLen - EXTRA_LENshort, sizeof(int));
					memcpy(a, &previous, sizeof(int));
					char g[7] = { JINGHAO,a[0],a[1],a[2],a[3],13,10 };
					memcpy(p + recordLen - EXTRA_LEN, g, EXTRA_LEN);
					//更新上一个节点
					int preNext = recordNum;
					memcpy(pPrev + recordLen - EXTRA_LENshort, &preNext, sizeof(int));
					//插入数据
					memcpy(p, s, recordLen - EXTRA_LEN);
					//更新块和文件块参数
					bp->usage = bp->usage + recordLen;
					bp->dirty = 1;

					//cout << "Insert Succeeded" << endl;
					return 1;

				FirstMinOut:
					//更新recordNum
					recordNum = ap.getRecordNum(tableName);
					//更新目前节点
					memcpy(a, &recordNum, sizeof(int));
					char g2[7] = { JINGHAO,a[0],a[1],a[2],a[3],13,10 };
					memcpy(p + recordLen - EXTRA_LEN, g2, EXTRA_LEN);
					//插入数据
					memcpy(p, s, recordLen - EXTRA_LEN);
					//交换第0个和当前节点
					char temp;
					for (int i = 0; i < recordLen; i++)
					{
						temp = t[i];
						t[i] = p[i];
						p[i] = temp;
					}
					//更新参数
					bp->usage = bp->usage + recordLen;
					bp->dirty = 1;

					//cout << "Insert Succeeded" << endl;
					return 1;
				
				
				
				EndOut:
					recordNum = ap.getRecordNum(tableName);
					//更新上一个节点
					memcpy(pPrev + recordLen - EXTRA_LENshort, &recordNum, sizeof(int));
					//更新现在的地址为-1
					recordNum = INFI;
					memcpy(a, &recordNum, sizeof(int));
					//插入#b1b2b3b4 OD OA
					char g4[EXTRA_LEN] = {JINGHAO,a[0],a[1],a[2],a[3],13,10};
					memcpy(p+recordLen-EXTRA_LEN, g4, EXTRA_LEN);
					//插入数据
					memcpy(p,s,recordLen-EXTRA_LEN);
					//更新块和文件块参数
					bp->usage = bp->usage+recordLen;
					bp->dirty = 1;

					//cout<<"Insert Succeeded"<<endl;
					return 1;
				}

				//如果链表中的块遍历完，则要得到替换新的块放入链表,这个所谓新的块是在文件中有对就段的
				if (bp->nextBlock == NULL)
				{
					bp = bm.getBlock(fp, 0);
					if (bp == NULL)
					{
						cout << "ERROR:Memory Out" << endl;
						return 0;
					}
				}
				//如果链表中的块还没遍历完，直接指向链表中的下一个块
				else
					bp=bp->nextBlock;
			}
			return 0;
	  }
	  return 0;
}


//判断某个值在比较符operater下是否符合条件cond,符合返回1，不符合返回0
int RecordManager::condCheck(string type,char * value,string scond,string operater)
{
	int tmp;
	//比较value 和 cond，将value 和 cond的差记为tmp
	const char * cond;
	cond=scond.c_str();
	if(type=="int")
	{
		int v;
		memcpy(&v, value, sizeof(int));
		int cv;
		cv=atoi(cond);
		tmp=(v)-cv;
	}

	else if(type=="float")
	{
		float v;
		memcpy(&v, value, sizeof(float));
		float cv;
		cv=(float)atof(cond);


		if((v)<cv)
			tmp=-1;
		else if((v)==cv)
			tmp=0;
		else
			tmp=1;
	}
	else
	{
		tmp=strcmp(value,cond);
	}
	
	//通过不同的operater和两者的差，来最后确定是否满足条件
	if(operater=="<")
	{
		if(tmp<0)
			return 1;
		else
			return 0;
	}
	else if (operater == "<>")
	{
		if (tmp!= 0)
			return 1;
		else
			return 0;
	}
	else if(operater=="<=")
	{
		if(tmp<=0)
			return 1;
		else
			return 0;
	}
	else if(operater==">")
	{
		if(tmp>0)
			return 1;
		else
			return 0;
	}
	else if(operater==">=")
	{
		if(tmp>=0)
			return 1;
		else
			return 0;
	}
	else if(operater=="=")
	{
		if(tmp==0)
			return 1;
		else
			return 0;
	}

	//提示比较出错
	else
	{
		cout<<"ERROR:Fail to compare values"<<endl;
		return -1;
	}
}
//大于返回1
int RecordManager::compare(string type, const char * value, const char * value2)
{
	int tmp = 0;
	if (type == "int")
	{
		int v1;
		int v2;
		memcpy(&v1,value, sizeof(int));
		memcpy(&v2, value2, sizeof(int));
		tmp = v1 - v2;
	}
	else if (type == "float")
	{
		int v1;
		int v2;
		memcpy(&v1, value, sizeof(float));
		memcpy(&v2, value2, sizeof(float));
		if (v1 > v2)
			tmp = 1;
		else if (v1 == v2)
			tmp = 0;
		else
			tmp = -1;
	}
	else
	{
		tmp = strcmp(value, value2);
	}
	return tmp;
}