
#include"stdafx.h"
#include "BufferManager.h"
//#include "my_catalog_manager.h"
#include "API.h"
#include <iostream>
#include <string>

#define UNKNOWN_FILE 8    //����δ֪�ļ�����
#define TABLE_FILE 9      //������ļ�����
#define INDEX_FILE 10     //���������ļ�����

using namespace std;


//����par����
typedef pair<string, string>par;
//ȫ�ֱ����ⲿ����
extern API            ap;
bool compare(string type, const char * value, const char * value2)
{
	int tmp = 0;
	if (type == "int")
	{
		int v1;
		int v2;
		memcpy(&v1, value, sizeof(int));
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
	if (tmp >= 0)
		return true;
	else
		return false;
}
bool cmp(const par &a, const par &b) {
	return compare("int", b.first.c_str(), a.first.c_str());
}
bool cmp1(const par &a, const par &b) {
	return compare("float", b.first.c_str(), a.first.c_str());
}
bool cmp2(const par &a, const par &b) {
	return a.first < b.first;
}


//���캯����Ԥ��Ϊ�ļ��ڵ���ļ���ڵ����ÿռ䣬��ʼ������
BufferManager::BufferManager()
	:totalBlock(0),
	 totalFile(0)
{
	for(int i=0;i<MAX_BLOCK_NUM;i++)
	{
		//�����ļ������ݴ洢�ռ䣬����0
		memset(b[i].address,0,BLOCK_SIZE);
		//Ϊ�ļ��ڵ��ļ����洢����ռ䣬����ʼ��Ϊ0
		b[i].fileName = new char[FILENAME_SIZE-1];
		memset(b[i].fileName, 0, FILENAME_SIZE-1);
		//Ϊ�ļ����ʼ������
		b[i].offsetNum=-1;
		b[i].dirty=0;
		b[i].xlock=0;
		b[i].lru=0;
		b[i].usage=-1;
		b[i].nextBlock=NULL;
		b[i].preBlock=NULL;
	}
	for(int i=0;i<MAX_FILE_NUM;i++)
	{
		f[i].type=-1;
		//Ϊ�ļ��ڵ��ļ����洢����ռ䣬����ʼ��Ϊ0
		memset(f[i].fileName,0,FILENAME_SIZE);
		//��ʼ���ļ��ڵ����
		//f[i].recordNum=-1;
		f[i].recordLength=-1;
		f[i].usage=-1;
		f[i].blockHead=NULL;
	}
}

//������������Ԥ�ȷ�����ļ��ڵ���ļ���ռ��ͷ�
BufferManager::~BufferManager()
{
	;
}

//�����ļ��ڵ㣬���ҵ��Ļ����·���ʹ�õĽڵ�����ļ��ڵ�����
BufferManager::File * BufferManager::getFile(const char * fileName)
{
	string s(fileName);
	BufferManager::Block * bp = NULL;
	BufferManager::File  * fp = NULL;



//�ҵ��Ļ�
	for(int i=0; i<MAX_FILE_NUM; i++)
		if(!strcmp(f[i].fileName,fileName))         
			return &f[i];	              

//�Ҳ����Ļ�
	if(fp==NULL)
	{
		//���totalFile<MAX_FILE_NUM,ʹ�ÿ�����ļ���
		if (totalFile < MAX_FILE_NUM)
		{
			fp = &f[totalFile];
			totalFile++;
			fp->usage = 0;
			strcpy_s(fp->fileName, FILENAME_SIZE, fileName);
		}
		//���totalFile>=MAX_FILE_NUM,�滻�ļ���
		else
		{
			fp = &f[0];
			fp->usage = 0;
			memset(fp->fileName,0,FILENAME_SIZE); //���ԭ�����ļ���
			strcpy_s(fp->fileName,FILENAME_SIZE,fileName);
			for(bp=fp->blockHead;bp!=NULL;bp++)
			{			
				bp->usage=0;
				//���ļ�����Ŀ�ȫ��д���ļ�
				if(!flush(bp,1))
				{
					cout<<"Failed to flush block"<<endl;
					return NULL;
				}
			}
			fp->blockHead=NULL;
			//�����ļ����ڿ������е�λ�ò���
		}
	}

//���ֵ��ļ���Ϣ�ж�����Ϣ�������ļ����ж�Ӧ��������
	int n[3];
	ap.GetFileInfo(s,n);
	//�������ļ����Ӧ����һ��table
	if(n[0]==TABLE_FILE)
	{
		fp->type=TABLE_FILE;
		//fp->recordNum=n[1];
		fp->recordLength=n[2];
	}
	//�������ļ����Ӧ����һ��index
	else if(n[0]==INDEX_FILE)
	{
		fp->type=INDEX_FILE;
		fp->recordLength=-1;          //����������ļ��������Ǽ�¼���Ⱥͼ�¼��Ŀ
		//fp->recordNum=-1;
	}
	//����ֵ���Ϣ�в���������ļ�
	else
	{
		cout<<"File not found"<<endl;
		return NULL;
	}


	return fp;
}


//��һ��block�ļ��ĺ���,requirement=1ʱҪ�󷵻ؿ�û��ȡ������ʱΪnull��offsetNum����,usage����
BufferManager::Block * BufferManager::getBlock(BufferManager::File * file,int requirement)
{
	BufferManager::File  * fp = NULL;
	BufferManager::Block * bp = NULL;
	BufferManager::Block * temp = NULL;
	FILE *tempfile = NULL;
	if (file == NULL)//�ļ����ǿյĵ�Ȼ��Ҳû����
	{
		return NULL;
	}
	else
	{	
		//���ҿ�,��������еĻ�
		bp = findReplaceBlock(file);
		bp->fileName = file->fileName;
		//���ò���
		//������ҵĿ��Ƿ��ڱ�������У���ֻҪ��������ԭ���������з���
		if (bp->preBlock||bp->nextBlock)
		{
			bp->preBlock->nextBlock = bp->nextBlock;
			bp->nextBlock->preBlock = bp->preBlock;
		}
		//������ҵĿ�û�б�ʹ��
		else
		{
			totalBlock++;
		}
		//����offsetNum,����������
		if (file->blockHead)
		{
			for (temp = file->blockHead; temp->nextBlock; temp = temp->nextBlock)
			{
				;
			}
			bp->offsetNum = temp->offsetNum + 1;
			temp->nextBlock = bp; 
			bp->preBlock = temp;
			bp->nextBlock = NULL;
		}
		else
		{
			bp->offsetNum = 0;
			file->blockHead = bp;
			bp->nextBlock = NULL;
		}
		if (!fopen_s(&tempfile, file->fileName, "rb+"))
		{
			bp->recordLength = file->recordLength;
			int realBlockSize = BLOCK_SIZE - BLOCK_SIZE % bp->recordLength;
			fseek(tempfile, bp->offsetNum * realBlockSize, 0);
			fread(bp->address, 1, realBlockSize, tempfile);
		}
		else
		{
			//fclose(tempfile);
			cout << "ERROR:Fail to open the file " << file->fileName << endl;
			return NULL;
		}
		fclose(tempfile);
		bp->usage = findUsage(bp->fileName,bp->address);
	}
	if (requirement && bp->usage == 0)
		return NULL;
	return bp;
}



//�����滻��,lru����
BufferManager::Block * BufferManager::findReplaceBlock(BufferManager::File * file)
{
	BufferManager::Block * LRUBlock;

	//�������û�б�ʹ�õ�block����ֱ�ӷ���totalBlockλ�õ�block

	if(totalBlock<MAX_BLOCK_NUM)
	{
		LRUBlock = &b[totalBlock];

	}
	//��������ڿ����block��ʹ��lru�㷨���п��滻,new
	else
	{
		//����
		while (1)
		{
			int i = 0;
			LRUBlock = &b[i];
			//�ҵ�lru = 0��ֱ�Ӹɵ�
			if (LRUBlock->lru == 0)
			{
				break;
			}
			//�ҵ�lru = 1�ĸ���һ�λ���
			else 
			{
				LRUBlock->lru = 0;
			}
			i = (++i) % MAX_BLOCK_NUM;
		}
		//��϶�����ģ���Ҫ�Ƚ���д���ļ�
			if (!flush(LRUBlock, 1))
			{
				cout << "ERROR:Fail to flush the block" << endl;
				return NULL;
			}
	}
	LRUBlock->lru = 1;
	return LRUBlock;	
}

//���ݸ������ļ���������д���ļ�
int BufferManager::flush(BufferManager::Block * b,int Force)
{
	
		//���ԭ���Ŀ�û���������Ҳ�ǿ��ִ�У�����д�ļ�����ӷ���0
		if (!b->dirty && Force==0)
			return 1;
		else
		{
			FILE * fp;
			if (!fopen_s(&fp,b->fileName, "rb+"))   //���ļ�
			{
				int realBlockSize = BLOCK_SIZE - BLOCK_SIZE % b->recordLength;
				fseek(fp, b->offsetNum * realBlockSize, 0);
				if (fwrite(b->address, realBlockSize, 1, fp) != 1)            //���д������򷵻�0
				{
					fclose(fp);
					cout << "ERROR:Fail to flush file " << b->fileName << endl;
					return 0;
				}
				else
				{
					fclose(fp);
					totalBlock--;
					b->dirty = 0;             //��������λ������Ϊ0
					b->lru = 0;               //lru��������ʹ��
					b->usage = 0;             //�����������
					memset(b->address, 0, BLOCK_SIZE);
					return 1;
				}
			}
			else
			{
				cout << "ERROR:Fail to open file " << b->fileName << endl;
				return 0;
			}
			
		}
}

//���ڴ��е���������д���ļ���Ҳ���Ƕ�ÿ�����ݿ�д��
int BufferManager::flushAll()
{
	BufferManager::Block * bp;
	for(int i=0; i<MAX_FILE_NUM; i++)
	{
		string s(f[i].fileName);
		if(ap.FindFile(s)!=UNKNOWN_FILE)
		{
			for(bp = f[i].blockHead; bp; bp=bp->nextBlock)
				if (flush(bp, 1) == 0)
				{
					return 0;
				}
		}

	}
	return 1;
}

//��������һ�������Ѿ�����¼ʹ�õ��ֽ���Ŀ
int BufferManager::findUsage(const char * fileName,const char * address)
{
	string s(fileName);
	int recordLen = ap.calcuteLenth(s);
	const char * p = address;
	//��һ����¼�ĳ���Ϊ�������Կ����ݽ��б���
	int realBlockSize = BLOCK_SIZE - BLOCK_SIZE%recordLen;
	int num = 0;
	while(p-address<realBlockSize)
	{
		const char * tmp = p;
		//��ÿһ����¼��ÿ���ֽڽ��б�������������з�0Ԫ�أ��˳���
		//���һ����¼û�з�0Ԫ�أ������Ϊ������¼���Ƿ�ʹ����
		int i;
		for(i=0;i<recordLen;i++)
		{
			if (*tmp != 0)
			{
				num += recordLen;
				break;
			}
			tmp++;
		}
		p=p+recordLen;
	}
	//����ֹͣ�����ʼַ֮�Ϊ��ʹ���ֽ���
	return num;
}
int  BufferManager::reconstruct(Block *bp)
{
	char *p = NULL;
	char *pStart = NULL;
	int valueLen = 0;
	int typeLen = 0;
	int findStart = 0;
	int realBlockSize = BLOCK_SIZE - BLOCK_SIZE % bp->recordLength;
	int maxRecordNum = realBlockSize/bp->recordLength;
	int reconstructNum = 0;
	
	vector<string> collName = ap.getCollName(bp->fileName);
	vector<string> collType = ap.getCollType(bp->fileName);
	vector<par> valueAndAddress;
	string primaryKey = ap.getPrimaryKey(bp->fileName);
	string type;


	if (bp == NULL || bp->usage == 0)
		return 0;
	else
	{
		//�������
		int recordLen = bp->recordLength;
		//��ͷ��һ��ֵ
		p = bp->address;
		//������ֵ
		int i = 0;
		//�ҵ�������λ��
		for (unsigned int j = 0; j < collName.size(); j++)
		{
			type = collType.at(j);
			typeLen = ap.calcuteLenth2(type);
			if (collName.at(j) == primaryKey)
			{
				break;
			}
			findStart = findStart + typeLen;
		}
		while (p-bp->address < realBlockSize)
		{
			//���ÿ�ͷ��λ��
			pStart = p;
			//ȡ������ֵ
			p += findStart;
	        //���û��
			if (pStart[recordLen-7] != 35)
			{
				//�ҵ���һ�еĵ�ַ
				p = pStart + recordLen;
				continue;
			}
			//value��ֵ
			string *x = new string(typeLen,'\0');
			for (int i = 0; i < typeLen; i++) (*x)[i] = p[i];
			//��addressES��ֵ��ǰ��ַ
			string *q = new string(recordLen, '\0');
			for (int i = 0; i < recordLen; i++) (*q)[i] = pStart[i];
			valueAndAddress.push_back(par(*x, *q));
			//����
			memset(pStart, 0, recordLen);
			//�ҵ���һ�еĵ�ַ
			p = pStart + recordLen;
			//i++
			i++;
		}

		//����
		if(type == "int")
			sort(valueAndAddress.begin(), valueAndAddress.end(),cmp);
		else if (type == "float")
			sort(valueAndAddress.begin(), valueAndAddress.end(), cmp1);
		else
			sort(valueAndAddress.begin(), valueAndAddress.end(), cmp2);
			


		//����� 
		char t[ADDRESS_BYTE];
		char max[ADDRESS_BYTE] = { -1,-1,-1,-1 };
		for (int k = 0; k < i; k++)//ִ������ѡ�񷨡�
		{
			for (int i = 0; i < ADDRESS_BYTE; i++)
				t[i] = ((valueAndAddress[k]).second)[i];
			if (strcmp(t, max))
			{
				for (int i = 0; i < ADDRESS_BYTE; i++)
					max[i] = t[i];
			}
		}
		int g = 0;
		g += max[0] + max[1] * 16 + max[2] * 16 * 16 + max[3] * 16 * 16 * 16;
		if (g < maxRecordNum)
		{
			max[0] = max[1] = max[2] = max[3] = -1;
		}
		//���ñ���
		p = bp->address;
		int h = 0;
		//��ֵ
		while (h < i)
		{
			for (int j = 0; j < recordLen; j++)
			{
				p[j] = (valueAndAddress[h].second)[j];
		    }
			if(h != i-1)
				for (int j = 0; j < ADDRESS_BYTE; j++)
				{
					p[j + recordLen - 6] = (j==0)? h + 1 : 0;
				}
			else
			{
				for (int j = 0; j < ADDRESS_BYTE; j++)
				{
					p[j + recordLen - 6] = max[j];
				}
			}
			h++;
			p += recordLen;
		}

		return 1;
	}

}

int  BufferManager::reconstructAll(File *fp)
{
	char *p = NULL;
	char *pStart = NULL;
	int valueLen = 0;
	int typeLen = 0;
	int findStart = 0;
	int recordLen = fp->blockHead->recordLength;
	int realBlockSize = BLOCK_SIZE - BLOCK_SIZE % recordLen;
	int maxRecordNum = realBlockSize / recordLen;
	int reconstructNum = 0;

	vector<string> collName = ap.getCollName(fp->fileName);
	vector<string> collType = ap.getCollType(fp->fileName);
	vector<par> valueAndAddress;
	string primaryKey = ap.getPrimaryKey(fp->fileName);
	string type;
	Block *bp = fp->blockHead;
	Block *f = fp->blockHead;
	if (bp == NULL)
	{
		bp = getBlock(fp, 1);
		fp->blockHead = bp;
		f = bp;
	}
	if (bp == NULL)
		return 0;
	if (!bp->usage)
		return 0;
	else
	{

		//������ֵ
		int i = 0;
		//�ҵ�������λ��
		for (unsigned int j = 0; j < collName.size(); j++)
		{
			type = collType.at(j);
			typeLen = ap.calcuteLenth2(type);
			if (collName.at(j) == primaryKey || primaryKey == "no")
			{
				break;
			}
			findStart = findStart + typeLen;
		}
		while (bp)
		{
			//��ͷ��һ��ֵ
			p = bp->address;
			while (p - bp->address < realBlockSize)
			{
				//���ÿ�ͷ��λ��
				pStart = p;
				//ȡ������ֵ
				p += findStart;
				//���û��
				if (pStart[recordLen - 7] != 35)
				{
					//�ҵ���һ�еĵ�ַ
					p = pStart + recordLen;
					continue;
				}
				//value��ֵ
				string *x = new string(typeLen, '\0');
				for (int i = 0; i < typeLen; i++) (*x)[i] = p[i];
				//��addressES��ֵ��ǰ��ַ
				string *q = new string(recordLen, '\0');
				for (int i = 0; i < recordLen; i++) (*q)[i] = pStart[i];
				valueAndAddress.push_back(par(*x, *q));
				//����
				memset(pStart, 0, recordLen);
				//�ҵ���һ�еĵ�ַ
				p = pStart + recordLen;
				//i++
				i++;
			}
			if (bp->nextBlock == NULL)
				bp = getBlock(fp, 1);
			else
				bp = bp->nextBlock;
		}
		//����
		if(type == "int")
			sort(valueAndAddress.begin(), valueAndAddress.end(), cmp);
		else if (type == "float")
			sort(valueAndAddress.begin(), valueAndAddress.end(), cmp1);
		else
			sort(valueAndAddress.begin(), valueAndAddress.end(), cmp2);

		//����� 
		char t[ADDRESS_BYTE];
		char max[ADDRESS_BYTE] = { -2,-2,-2,-2 };
		char end[ADDRESS_BYTE] = { -1,-1,-1,-1 };
		int flag = 0;
		for (int k = 0; k < i; k++)//ִ������ѡ�񷨡�
		{
			for (int i = 0; i < ADDRESS_BYTE; i++)
				t[i] = ((valueAndAddress[k]).second)[i];
			if (strcmp(t, end))
			{
				flag = 1;
			}
			if (strcmp(t, max))
			{
				for (int i = 0; i < ADDRESS_BYTE; i++)
					max[i] = t[i];
			}
		}
		if (flag = 1)
		{
			max[0] = max[1] = max[2] = max[3] = -1;
		}
		//���ñ���
		p = f->address;
		int h = 0;
		//��ֵ
		while (h < i)
		{
			for (int j = 0; j < recordLen; j++)
			{
				p[j] = (valueAndAddress[h].second)[j];
			}
			if (h != i - 1)
			{
				int x = h + 1;
				memcpy(p + recordLen - 6, &(x), ADDRESS_BYTE);
			}
				
			else
			{
				for (int j = 0; j < ADDRESS_BYTE; j++)
				{
					p[j + recordLen - 6] = max[j];
				}
			}
			h++;
			p += recordLen;
			if (0 == (h%maxRecordNum))
			{
				if (f->nextBlock)
					f = f->nextBlock;
				else
					f = getBlock(fp, 0);
				p = f->address;
			}
		}

		return 1;
	}

}


