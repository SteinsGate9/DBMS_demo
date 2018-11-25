/****************************此文件包括MiniSQL中BufferManager类的定义 **********************/
/****************************作者：黄士诚   时间：2018年6月25日********************************/
#pragma once
#include "stdafx.h"
#define BLOCK_SIZE 4096        //定义一个文件块的存储大小
#define FILENAME_SIZE 127      //定义一个文件头块的大小
#define MAX_BLOCK_NUM 262144   //定义预先分配的文件块的数目
#define MAX_FILE_NUM 5         //定义预先分配的文件节点的数目
#define ADDRESS_SIZE 32        //定义一条地址的大小
#define ADDRESS_BYTE 4         //定义一个地址的BYTE数
#define MAX_DATA_LENGTH  40    //定义一个DATA的最大长度
 
class BufferManager{
public:
	//定义文件块信息和存储空间
	struct Block{
		int offsetNum;         //块对应的文件偏移位置，用于文件的读写
		int dirty;             //dirty为1的时候为脏
		int lru;               //lru为1的时候为多一条命
		int xlock;             //xlock
		int slock;             //slock
		int usage;             //块数据中内存使用点，如果为-1，表示这个块还没有被插入到二维链表
		int recordLength;      //record+record+...+7
		char  address[BLOCK_SIZE];        //1 char = 1 BYTE，二进制流存贮数
		char * fileName;       //文件名
		Block * nextBlock;     //指向下一个文件块节点
		Block * preBlock;      //指向前一个文件块节点
	};
	//定义文件节点存储信息和存储空间
	struct File{
		int type;              //文件类型
		int recordLength;      //表文件中的记录长度
		int usage;             //标志有没有被插入到二维链表，没有为-1，有为0
		char  fileName[FILENAME_SIZE];       //文件名
		Block * blockHead;    //指向文件块链表
	};
	//无参构造函数
	BufferManager();
	//析构函数
	~BufferManager();
	//写回所有块
	int  flushAll();    
	//写会当前块
	int  flush(Block * b,int Force);
	//块重构
	int  reconstruct(Block *b);
	//文件中所有块重构
	int  reconstructAll(File *fp);
	//得到文件名对应的文件块头
	File  * getFile(const char * fileName);
	//按照需求得到相应的数据块
	Block * getBlock(BufferManager::File * file, int requirement);

private:
	int totalBlock;
	int totalFile;
	//找到数据块的使用多少字节数
	int findUsage(const char * fileName,const char * address);
	File   f[MAX_FILE_NUM];
	Block  b[MAX_BLOCK_NUM];
	//找到要替换的或者找到的块
	Block * findReplaceBlock(BufferManager::File * file);
	
};
