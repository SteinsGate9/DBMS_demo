

#include "stdafx.h"
#include "RecordManager.h"
#include "BufferManager.h"
#include "API.h"
#include <cstring>
#include <iostream>

//�궨�壬���С���߼���
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


//�ⲿ��������
extern BufferManager bm;   
extern API ap;

using namespace std;
//���ݸ����ı����������еļ�¼ȫ��ɾ��
int RecordManager::deleteValue(string tableName)
{
	//�����ļ��ڵ�
	BufferManager::File * fp=bm.getFile(tableName.c_str());	
	if(fp!=NULL)
	{
		BufferManager::Block * bp=fp->blockHead;

		//����ļ��ڵ�����û�п�ڵ���أ�����ļ��ж�ȡһ���飬�������ҵ��ļ��ڵ�����
		if(bp==NULL)
		{
			bp=bm.getBlock(fp,0);
			fp->blockHead = bp;
		}
		if (bp == NULL || bp->usage == 0)
			return 0;
		while(bp)
		{
			//lru���� ��ʾ��ʱ����
			bp->lru = 0;
			//dirty��1��ʾ�Ѿ��޸�
			bp->dirty = 1;
			//���ڴ�����ȫ����0,ʹ������Ϊ0
			memset(bp->address,0,BLOCK_SIZE);
			bp->usage=0;

			//�����������û����һ���飬��ȡ�ļ��е���һ������й���
			if(bp->nextBlock==NULL)
				bp=bm.getBlock(fp,1);
			else
				bp=bp->nextBlock;
		}
		
	}
	//����ɾ��������Ŀ
	return ap.getRecordNum(tableName);
}


//���ݸ����ı�����һ��where������ɾ�����е����������ļ�¼
int RecordManager::deleteValue(string tableName, string colName1,string cond1,string operater1)
{
	//�õ��ļ��ڵ�
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
		//׼������
		else if (bp != NULL)
		{
			for (unsigned int j = 0; j < collName.size(); j++)
			{
				type = collType.at(j);
				typeLen = ap.calcuteLenth2(type);
				//�ҵ���Ӧ�ıȽ��ֶΣ���ȡ��ֵ
				if (collName.at(j) == colName1)
				{
					break;
				}
				findstart += typeLen;
			}
		}

		//���������
		if (colName1 == primaryKey && (operater1 == ">" || operater1 == ">="))
		{
			while (bp)
			{
				//p�ȸ�һ����ͷ�ĵ�ַ
				p = bp->address;
				//�ж���.���С
				int realBlockSize = BLOCK_SIZE - BLOCK_SIZE % bp->recordLength;
				//�˴�ÿ��*P���жϣ�����һ���ֶεĿ�ʼ�ֽڣ����Ե�*PΪ0ʱ��������Ѿ�������
				while (p - bp->address < realBlockSize)
				{
					//pStartָ��ÿ�����ݵ�ͷ
					pStart = p;    	
					//p��Ϊ�α��ƶ�
					p += findstart;
					//value��ֵ
					memcpy(value, p, typeLen);
					//�ҵ��µĵ�ַ
					char * tmp = pStart + recordLen - EXTRA_LENshort;
					int x;
					memcpy(&x, tmp, sizeof(int));
					//Ҳ����ɾ��ǰԭ����¼��λ��
					if (condCheck(type, value, cond1, operater1))
					{
						//ֱ�����������䣬Ȼ��Ѽ�¼
						memset(pStart, 0, recordLen);
						//��¼��һ��ͻ��>�ĵ�ַ
						if (!flag)
						{
							//�п�����preV���ߵ�һ����ַ
							tobeChanged = (pPrev ? pPrev : pStart);
							flag = 1;
						}
						//����usage
						bp->usage = bp->usage - recordLen;
						//����dirty
						if (bp->dirty != 1)
							bp->dirty = 1;
						//����deleteNum
						deleteNum++;
					}
					//�����out
					if (x != OUT_SIGNAL)  //0xFFFF
					{
						p = bp->address + ((x) - bp->offsetNum *(realBlockSize) / bp->recordLength) *recordLen;
						pPrev = pStart;
					}
					//�������
					else
					{
						//����δ��ɾ�������һ���ڵ�
						for (int i = 0; i < 4; i++)
						{
							tobeChanged[i + recordLen - EXTRA_LENshort] = -1;
						}
						return deleteNum;
					}
				}
				//�ҵ���һ����
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
				//p�ȸ�һ����ͷ�ĵ�ַ
				p = bp->address;
				//�ж���.���С
				int realBlockSize = BLOCK_SIZE - BLOCK_SIZE % bp->recordLength;
				while (p - bp->address < realBlockSize)
				{
					//pStartָ��ÿ�����ݵ�ͷ
					pStart = p;
					//p��Ϊ�α��ƶ�
					p += findstart;
					//value��ֵ
					memcpy(value, p, typeLen);
					//�ҵ��µĵ�ַ
					char * tmp = pStart + recordLen - EXTRA_LENshort;
					int x;
					memcpy(&x, tmp, sizeof(int));
					if (condCheck(type, value, cond1, operater1))
					{	
						//ֱ�����������䣬Ȼ��Ѽ�¼
						memset(pStart, 0, recordLen);
						//����usage
						bp->usage = bp->usage - recordLen;
						//����dirty
						if (bp->dirty != 1)
							bp->dirty = 1;
						//����deleteNum
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
						//��δ��ɾ���ĵ�һ��������ַ�ĵ�һ��λ��
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
				//p�ȸ�һ����ͷ�ĵ�ַ
				p = bp->address;
				//�ж���.���С
				int realBlockSize = BLOCK_SIZE - BLOCK_SIZE % bp->recordLength;
				while (p - bp->address < realBlockSize)
				{
					//pStartָ��ÿ�����ݵ�ͷ
					pStart = p;
					//p��Ϊ�α��ƶ�
					p += findstart;
					//value��ֵ
					memcpy(value, p, typeLen);
					//�ҵ��µĵ�ַ
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
						//����deleteNum
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
					if (flag == 1)//��һ
					{
						//����һ�����Ų����һ��
						char *temp = bp->address + ((x) - bp->offsetNum *(BLOCK_SIZE - BLOCK_SIZE % bp->recordLength) / bp->recordLength) *recordLen;
						for (int i = 0; i < recordLen; i++)
						{
							bp->address[i] = temp[i];
							temp[i] = 0;
						}
						
						return deleteNum;
					}
					else if (flag == 2)//�ǵ�һ
					{ 
						//ֱ�����������䣬Ȼ��Ѽ�¼
						memset(pStart, 0, recordLen);
						//������һ������ֵ
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
		//����Ƿ�����ɾ��
		else
		{
			while (bp)
			{
				//p�ȸ�һ����ͷ�ĵ�ַ
				p = bp->address;
				//�ж���.���С
				int realBlockSize = BLOCK_SIZE - BLOCK_SIZE % bp->recordLength;
				//
				int maxLines = realBlockSize / recordLen;
				while (p - bp->address < realBlockSize)
				{
					//pStartָ��ÿ�����ݵ�ͷ
					pStart = p;
					//p��Ϊ�α��ƶ�
					p += findstart;
					//value��ֵ
					memcpy(value, p, typeLen);
					//�ҵ��µĵ�ַ
					char * tmp = pStart + recordLen - EXTRA_LENshort;
					int x;
					memcpy(&x, tmp, sizeof(int));
					if (condCheck(type, value, cond1, operater1))
					{
						memset(pStart, 0, recordLen);
						bp->usage = bp->usage - recordLen;
						//����deleteNum
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


//ͨ�������ļ����������еı��еļ�¼��
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
		memset(value,0,255);  //��������0������������ַ���ʱû�н�β
		int valueLen;
		int maxLines = 0;
		int realBlockSize = 0;
		int selectNum = 0;
		string type;
		

		//����ļ��ڵ�����û�п�ڵ���أ�����ļ��ж�ȡһ���飬�������ҵ��ļ��ڵ�����
		BufferManager::Block * bp=fp->blockHead;
		if (bp == NULL)
		{
			bp = bm.getBlock(fp, 1);
			fp->blockHead = bp;
		}
		//����֮���ټ���
        if (bp == NULL||bp->usage == 0)
			return 0;

		//������Ϊ������
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
		//��ʼ��ʽ���
		while(bp)
		{			
			p = bp->address;
			//�ж���.���С
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
					//���ն�Ӧ�����ͣ����ֶε�ֵ���
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


//ͨ��һ���������ļ�����һ��where���������ұ��еļ�¼��
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
		memset(value, 0, 255);  //��������0������������ַ���ʱû�н�β
		int valueLen;
		string type;
		int typeLen;
		int findstart = 0;
		int selectNum = 0;
		int selected = 0;
		int realBlockSize = 0;
		int maxLines = 0;
		//����ļ��ڵ�����û�п�ڵ���أ�����ļ��ж�ȡһ���飬�������ҵ��ļ��ڵ�����
		BufferManager::Block * bp = fp->blockHead;
		if (bp == NULL)
		{
			bp = bm.getBlock(fp, 1);
			fp->blockHead = bp;
		}
		//����֮���ټ���
		if (bp == NULL || bp->usage == 0)
			return 0;
		else if (bp != NULL)
		{
			p = bp->address;
			for (unsigned int j = 0; j < collName.size(); j++)
			{
				type = collType.at(j);
				typeLen = ap.calcuteLenth2(type);
				//�ҵ���Ӧ�ıȽ��ֶΣ���ȡ��ֵ
				if (collName.at(j) == colName1)
				{
					break;
				}
				findstart += typeLen;
			//	p += typeLen;
			}
		}
		//������Ϊ������
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
			//����p��ֵ
			p = bp->address;
			//�ж���.���С
			realBlockSize = BLOCK_SIZE - BLOCK_SIZE % recordLen;
			//dasd
			maxLines = realBlockSize / recordLen;
			while (p - bp->address < realBlockSize)
			{	
				pStart = p;    //��tmpp��¼ÿ����¼�Ŀ�ʼλ��	
				p += findstart;
				memcpy(value, p, typeLen);
				p = pStart;
				selected = 0;
				//ȡ��ַ
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
						//���ն�Ӧ�����ͣ����ֶε�ֵ���
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
		memset(value, 0, 255);  //��������0������������ַ���ʱû�н�β
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
		//����ļ��ڵ�����û�п�ڵ���أ�����ļ��ж�ȡһ���飬�������ҵ��ļ��ڵ�����
		BufferManager::Block * bp = fp->blockHead;
		if (bp == NULL)
		{
			bp = bm.getBlock(fp, 1);
			fp->blockHead = bp;
		}
		//����֮���ټ���
		if (bp == NULL || bp->usage == 0)
			return 0;
		else if (bp != NULL)
		{
			for (unsigned int i = 0; i < x.size(); i++)
			{
				//��i��findstart
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
		//������Ϊ������
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
		//��ʼ��ʽ����
		while (bp)
		{
			p = bp->address;
			//�ж���.���С
			realBlockSize = BLOCK_SIZE - BLOCK_SIZE % recordLen;
			//maxLines 
			maxLines = realBlockSize / recordLen;
			while (p - bp->address < realBlockSize)
			{			
				//��pStart��¼ÿ����¼�Ŀ�ʼλ��	
			    pStart = p;
				//flag = 1
				flag = 1;
				//��ʼ���ø��ֲ���
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

				//�����Ƿ�ѡ����
				selected = 0;
				//��ַ
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
						//���ն�Ӧ�����ͣ����ֶε�ֵ���
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
		char value[10][255];	for (int i = 0; i < 10; i++)memset(value[i], 0, 255);  //��������0������������ַ���ʱû�н�β
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
		//����ļ��ڵ�����û�п�ڵ���أ�����ļ��ж�ȡһ���飬�������ҵ��ļ��ڵ�����
		BufferManager::Block * bp = fp->blockHead;
		if (bp == NULL)
		{
			bp = bm.getBlock(fp, 1);
			fp->blockHead = bp;
		}
		//����֮���ټ���
		if (bp == NULL || bp->usage == 0)
			return 0;
		else if (bp != NULL)
		{
			for (unsigned int i = 0; i < x.size(); i++)
			{
				//��i��findstart
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
			//p�ȸ�һ����ͷ�ĵ�ַ
			p = bp->address;
			//�ж���.���С
			realBlockSize = BLOCK_SIZE - BLOCK_SIZE % recordLen;
			//maxLines
			maxLines = realBlockSize / recordLen;
			while (p - bp->address < realBlockSize)
			{
				//pStartָ��ÿ�����ݵ�ͷ
				pStart = p;
				//����flag 
				flag = 1;
				for (unsigned int i = 0; i < x.size(); i++)
				{
					p += findstart[i];
					memcpy(value[i], p, typeLen[i]);
					p = pStart;
					if (!condCheck(type[i], value[i], x[i].cond, x[i].operater))
						flag = 0;
				}
				//�ҵ��µĵ�ַ
				char * tmp = pStart + recordLen - EXTRA_LENshort;
				int x;
				memcpy(&x, tmp, sizeof(int));
				//���ͬʱ��������where������Ϊand�߼�ʱ��
				//�����������е�һ��������ΪOR�߼�ʱ�������ɾ������
				if (flag)
				{
					//ֱ�����������䣬Ȼ��Ѽ�¼
					memset(pStart, 0, recordLen);
					//����usage
					bp->usage = bp->usage - recordLen;
					//����deleteNum
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
//�����ṩ�ı����Ͳ���ļ�¼�ֽڣ������ݿ��в����¼��
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
	//�õ���Ӧ���ļ���
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
		//�õ��ļ���Ӧ�ĵ�һ����
		BufferManager::Block * bp=fp->blockHead;
		//����Ƿǿ��ļ�
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
				//��ʹ�õĿ��lru����1
				bp->lru = 1;
				//�ж���.���С
				realBlockSize = BLOCK_SIZE - BLOCK_SIZE % recordLen;
				//�ж���������
				maxLines = realBlockSize / recordLen;
				//����ļ����Ӧ�Ŀ��л��㹻���룬�򽫼�¼д�������β�������뵽�ļ��м䣬������ĩβ
				if(bp->usage < BLOCK_SIZE - BLOCK_SIZE % recordLen)
				{
					p = bp->address;
					//�ҵ��յ�ַ���
					p=p+bp->usage;
					//������һ��record��ָ���ַΪ��ǰ��ַ
					vector<string> collType = ap.getCollType(tableName);
					if (bp == fp->blockHead && bp->usage == 0)
					{
						goto usageZero;
					}
					while (bpForFindMin)
					{
						
						//ÿ��findmin����lru
						bpForFindMin->lru = 1;
						//ÿ��findmin���ó�ʼ
						t = bpForFindMin->address;
						//ÿ��findmin����first��һ��
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
					//�������ڵĵ�ַΪ-1
					recordNum = INFI;
					memcpy(a, &recordNum, sizeof(int));
					//����#b1b2b3b4 OD OA
					char g3[EXTRA_LEN] = { JINGHAO,a[0],a[1],a[2],a[3],13,10 };
					memcpy(p + recordLen - EXTRA_LEN, g3, EXTRA_LEN);
					//��������
					memcpy(p, s, recordLen - EXTRA_LEN);
					//���¿���ļ������
					bp->usage = bp->usage + recordLen;
					bp->dirty = 1;

					//cout << "Insert Succeeded" << endl;
					return 1;
  
				nFirstMinOut:
					recordNum = ap.getRecordNum(tableName);
					int previous;
					//�������ڵĵ�ַΪ��һ���ڵ�yuanben de xiayige
					memcpy(&previous, pPrev + recordLen - EXTRA_LENshort, sizeof(int));
					memcpy(a, &previous, sizeof(int));
					char g[7] = { JINGHAO,a[0],a[1],a[2],a[3],13,10 };
					memcpy(p + recordLen - EXTRA_LEN, g, EXTRA_LEN);
					//������һ���ڵ�
					int preNext = recordNum;
					memcpy(pPrev + recordLen - EXTRA_LENshort, &preNext, sizeof(int));
					//��������
					memcpy(p, s, recordLen - EXTRA_LEN);
					//���¿���ļ������
					bp->usage = bp->usage + recordLen;
					bp->dirty = 1;

					//cout << "Insert Succeeded" << endl;
					return 1;

				FirstMinOut:
					//����recordNum
					recordNum = ap.getRecordNum(tableName);
					//����Ŀǰ�ڵ�
					memcpy(a, &recordNum, sizeof(int));
					char g2[7] = { JINGHAO,a[0],a[1],a[2],a[3],13,10 };
					memcpy(p + recordLen - EXTRA_LEN, g2, EXTRA_LEN);
					//��������
					memcpy(p, s, recordLen - EXTRA_LEN);
					//������0���͵�ǰ�ڵ�
					char temp;
					for (int i = 0; i < recordLen; i++)
					{
						temp = t[i];
						t[i] = p[i];
						p[i] = temp;
					}
					//���²���
					bp->usage = bp->usage + recordLen;
					bp->dirty = 1;

					//cout << "Insert Succeeded" << endl;
					return 1;
				
				
				
				EndOut:
					recordNum = ap.getRecordNum(tableName);
					//������һ���ڵ�
					memcpy(pPrev + recordLen - EXTRA_LENshort, &recordNum, sizeof(int));
					//�������ڵĵ�ַΪ-1
					recordNum = INFI;
					memcpy(a, &recordNum, sizeof(int));
					//����#b1b2b3b4 OD OA
					char g4[EXTRA_LEN] = {JINGHAO,a[0],a[1],a[2],a[3],13,10};
					memcpy(p+recordLen-EXTRA_LEN, g4, EXTRA_LEN);
					//��������
					memcpy(p,s,recordLen-EXTRA_LEN);
					//���¿���ļ������
					bp->usage = bp->usage+recordLen;
					bp->dirty = 1;

					//cout<<"Insert Succeeded"<<endl;
					return 1;
				}

				//��������еĿ�����꣬��Ҫ�õ��滻�µĿ��������,�����ν�µĿ������ļ����жԾͶε�
				if (bp->nextBlock == NULL)
				{
					bp = bm.getBlock(fp, 0);
					if (bp == NULL)
					{
						cout << "ERROR:Memory Out" << endl;
						return 0;
					}
				}
				//��������еĿ黹û�����ֱ꣬��ָ�������е���һ����
				else
					bp=bp->nextBlock;
			}
			return 0;
	  }
	  return 0;
}


//�ж�ĳ��ֵ�ڱȽϷ�operater���Ƿ��������cond,���Ϸ���1�������Ϸ���0
int RecordManager::condCheck(string type,char * value,string scond,string operater)
{
	int tmp;
	//�Ƚ�value �� cond����value �� cond�Ĳ��Ϊtmp
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
	
	//ͨ����ͬ��operater�����ߵĲ�����ȷ���Ƿ���������
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

	//��ʾ�Ƚϳ���
	else
	{
		cout<<"ERROR:Fail to compare values"<<endl;
		return -1;
	}
}
//���ڷ���1
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