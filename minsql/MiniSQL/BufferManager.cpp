
#include"stdafx.h"
#include "BufferManager.h"
//#include "my_catalog_manager.h"
#include "API.h"
#include <iostream>
#include <string>

#define UNKNOWN_FILE 8    //定义未知文件类型
#define TABLE_FILE 9      //定义表文件类型
#define INDEX_FILE 10     //定义索引文件类型

using namespace std;


//定义par变量
typedef pair<string, string>par;
//全局变量外部声明
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


//构造函数，预先为文件节点和文件块节点分配好空间，初始化变量
BufferManager::BufferManager()
	:totalBlock(0),
	 totalFile(0)
{
	for(int i=0;i<MAX_BLOCK_NUM;i++)
	{
		//分配文件块数据存储空间，并置0
		memset(b[i].address,0,BLOCK_SIZE);
		//为文件节点文件名存储分配空间，并初始化为0
		b[i].fileName = new char[FILENAME_SIZE-1];
		memset(b[i].fileName, 0, FILENAME_SIZE-1);
		//为文件块初始化变量
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
		//为文件节点文件名存储分配空间，并初始化为0
		memset(f[i].fileName,0,FILENAME_SIZE);
		//初始化文件节点变量
		//f[i].recordNum=-1;
		f[i].recordLength=-1;
		f[i].usage=-1;
		f[i].blockHead=NULL;
	}
}

//析构函数，把预先分配的文件节点和文件块空间释放
BufferManager::~BufferManager()
{
	;
}

//查找文件节点，将找到的或者新分配使用的节点插入文件节点链表
BufferManager::File * BufferManager::getFile(const char * fileName)
{
	string s(fileName);
	BufferManager::Block * bp = NULL;
	BufferManager::File  * fp = NULL;



//找到的话
	for(int i=0; i<MAX_FILE_NUM; i++)
		if(!strcmp(f[i].fileName,fileName))         
			return &f[i];	              

//找不到的话
	if(fp==NULL)
	{
		//如果totalFile<MAX_FILE_NUM,使用空余的文件块
		if (totalFile < MAX_FILE_NUM)
		{
			fp = &f[totalFile];
			totalFile++;
			fp->usage = 0;
			strcpy_s(fp->fileName, FILENAME_SIZE, fileName);
		}
		//如果totalFile>=MAX_FILE_NUM,替换文件块
		else
		{
			fp = &f[0];
			fp->usage = 0;
			memset(fp->fileName,0,FILENAME_SIZE); //清空原来的文件名
			strcpy_s(fp->fileName,FILENAME_SIZE,fileName);
			for(bp=fp->blockHead;bp!=NULL;bp++)
			{			
				bp->usage=0;
				//将文件下面的块全部写回文件
				if(!flush(bp,1))
				{
					cout<<"Failed to flush block"<<endl;
					return NULL;
				}
			}
			fp->blockHead=NULL;
			//保留文件块在块链表中的位置不变
		}
	}

//从字典文件信息中读入信息，放入文件块中对应的属性中
	int n[3];
	ap.GetFileInfo(s,n);
	//如果这个文件块对应的是一个table
	if(n[0]==TABLE_FILE)
	{
		fp->type=TABLE_FILE;
		//fp->recordNum=n[1];
		fp->recordLength=n[2];
	}
	//如果这个文件块对应的是一个index
	else if(n[0]==INDEX_FILE)
	{
		fp->type=INDEX_FILE;
		fp->recordLength=-1;          //如果是索引文件，不考虑记录长度和记录数目
		//fp->recordNum=-1;
	}
	//如果字典信息中不存在这个文件
	else
	{
		cout<<"File not found"<<endl;
		return NULL;
	}


	return fp;
}


//加一个block文件的后面,requirement=1时要求返回块没读取到数据时为null，offsetNum设置,usage设置
BufferManager::Block * BufferManager::getBlock(BufferManager::File * file,int requirement)
{
	BufferManager::File  * fp = NULL;
	BufferManager::Block * bp = NULL;
	BufferManager::Block * temp = NULL;
	FILE *tempfile = NULL;
	if (file == NULL)//文件都是空的当然块也没法用
	{
		return NULL;
	}
	else
	{	
		//先找块,如果本身有的话
		bp = findReplaceBlock(file);
		bp->fileName = file->fileName;
		//设置参数
		//如果查找的块是否处在别的链表中，则只要将这个块从原来的链表中分离
		if (bp->preBlock||bp->nextBlock)
		{
			bp->preBlock->nextBlock = bp->nextBlock;
			bp->nextBlock->preBlock = bp->preBlock;
		}
		//如果查找的块没有被使用
		else
		{
			totalBlock++;
		}
		//设置offsetNum,并插入链表
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



//查找替换块,lru设置
BufferManager::Block * BufferManager::findReplaceBlock(BufferManager::File * file)
{
	BufferManager::Block * LRUBlock;

	//如果还有没有被使用的block，则直接返回totalBlock位置的block

	if(totalBlock<MAX_BLOCK_NUM)
	{
		LRUBlock = &b[totalBlock];

	}
	//如果不存在空余的block，使用lru算法进行块替换,new
	else
	{
		//遍历
		while (1)
		{
			int i = 0;
			LRUBlock = &b[i];
			//找到lru = 0的直接干掉
			if (LRUBlock->lru == 0)
			{
				break;
			}
			//找到lru = 1的给他一次机会
			else 
			{
				LRUBlock->lru = 0;
			}
			i = (++i) % MAX_BLOCK_NUM;
		}
		//块肯定是脏的，则要先将它写回文件
			if (!flush(LRUBlock, 1))
			{
				cout << "ERROR:Fail to flush the block" << endl;
				return NULL;
			}
	}
	LRUBlock->lru = 1;
	return LRUBlock;	
}

//根据给定的文件名，将块写回文件
int BufferManager::flush(BufferManager::Block * b,int Force)
{
	
		//如果原来的块没有脏数据且不强制执行，则不用写文件，真接返回0
		if (!b->dirty && Force==0)
			return 1;
		else
		{
			FILE * fp;
			if (!fopen_s(&fp,b->fileName, "rb+"))   //打开文件
			{
				int realBlockSize = BLOCK_SIZE - BLOCK_SIZE % b->recordLength;
				fseek(fp, b->offsetNum * realBlockSize, 0);
				if (fwrite(b->address, realBlockSize, 1, fp) != 1)            //如果写入出错，则返回0
				{
					fclose(fp);
					cout << "ERROR:Fail to flush file " << b->fileName << endl;
					return 0;
				}
				else
				{
					fclose(fp);
					totalBlock--;
					b->dirty = 0;             //将脏数据位重新设为0
					b->lru = 0;               //lru可以重新使用
					b->usage = 0;             //所有数据清空
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

//将内存中的所有数据写回文件，也就是对每个数据块写回
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

//用来查找一个块中已经被记录使用的字节数目
int BufferManager::findUsage(const char * fileName,const char * address)
{
	string s(fileName);
	int recordLen = ap.calcuteLenth(s);
	const char * p = address;
	//以一条记录的长度为步长，对块内容进行遍历
	int realBlockSize = BLOCK_SIZE - BLOCK_SIZE%recordLen;
	int num = 0;
	while(p-address<realBlockSize)
	{
		const char * tmp = p;
		//对每一条记录的每个字节进行遍历，如果发现有非0元素，退出，
		//如果一条记录没有非0元素，则可以为这条记录区是非使用区
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
	//遍历停止点和起始址之差即为块使用字节数
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
		//方便查找
		int recordLen = bp->recordLength;
		//开头第一个值
		p = bp->address;
		//遍历赋值
		int i = 0;
		//找到主键的位置
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
			//设置开头的位置
			pStart = p;
			//取主键的值
			p += findStart;
	        //如果没有
			if (pStart[recordLen-7] != 35)
			{
				//找到下一行的地址
				p = pStart + recordLen;
				continue;
			}
			//value赋值
			string *x = new string(typeLen,'\0');
			for (int i = 0; i < typeLen; i++) (*x)[i] = p[i];
			//给addressES赋值当前地址
			string *q = new string(recordLen, '\0');
			for (int i = 0; i < recordLen; i++) (*q)[i] = pStart[i];
			valueAndAddress.push_back(par(*x, *q));
			//清零
			memset(pStart, 0, recordLen);
			//找到下一行的地址
			p = pStart + recordLen;
			//i++
			i++;
		}

		//排序
		if(type == "int")
			sort(valueAndAddress.begin(), valueAndAddress.end(),cmp);
		else if (type == "float")
			sort(valueAndAddress.begin(), valueAndAddress.end(), cmp1);
		else
			sort(valueAndAddress.begin(), valueAndAddress.end(), cmp2);
			


		//找最大 
		char t[ADDRESS_BYTE];
		char max[ADDRESS_BYTE] = { -1,-1,-1,-1 };
		for (int k = 0; k < i; k++)//执行排序。选择法。
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
		//重置变量
		p = bp->address;
		int h = 0;
		//赋值
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

		//遍历赋值
		int i = 0;
		//找到主键的位置
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
			//开头第一个值
			p = bp->address;
			while (p - bp->address < realBlockSize)
			{
				//设置开头的位置
				pStart = p;
				//取主键的值
				p += findStart;
				//如果没有
				if (pStart[recordLen - 7] != 35)
				{
					//找到下一行的地址
					p = pStart + recordLen;
					continue;
				}
				//value赋值
				string *x = new string(typeLen, '\0');
				for (int i = 0; i < typeLen; i++) (*x)[i] = p[i];
				//给addressES赋值当前地址
				string *q = new string(recordLen, '\0');
				for (int i = 0; i < recordLen; i++) (*q)[i] = pStart[i];
				valueAndAddress.push_back(par(*x, *q));
				//清零
				memset(pStart, 0, recordLen);
				//找到下一行的地址
				p = pStart + recordLen;
				//i++
				i++;
			}
			if (bp->nextBlock == NULL)
				bp = getBlock(fp, 1);
			else
				bp = bp->nextBlock;
		}
		//排序
		if(type == "int")
			sort(valueAndAddress.begin(), valueAndAddress.end(), cmp);
		else if (type == "float")
			sort(valueAndAddress.begin(), valueAndAddress.end(), cmp1);
		else
			sort(valueAndAddress.begin(), valueAndAddress.end(), cmp2);

		//找最大 
		char t[ADDRESS_BYTE];
		char max[ADDRESS_BYTE] = { -2,-2,-2,-2 };
		char end[ADDRESS_BYTE] = { -1,-1,-1,-1 };
		int flag = 0;
		for (int k = 0; k < i; k++)//执行排序。选择法。
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
		//重置变量
		p = f->address;
		int h = 0;
		//赋值
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


