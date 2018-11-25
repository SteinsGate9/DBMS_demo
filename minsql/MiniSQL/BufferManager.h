/****************************���ļ�����MiniSQL��BufferManager��Ķ��� **********************/
/****************************���ߣ���ʿ��   ʱ�䣺2018��6��25��********************************/
#pragma once
#include "stdafx.h"
#define BLOCK_SIZE 4096        //����һ���ļ���Ĵ洢��С
#define FILENAME_SIZE 127      //����һ���ļ�ͷ��Ĵ�С
#define MAX_BLOCK_NUM 262144   //����Ԥ�ȷ�����ļ������Ŀ
#define MAX_FILE_NUM 5         //����Ԥ�ȷ�����ļ��ڵ����Ŀ
#define ADDRESS_SIZE 32        //����һ����ַ�Ĵ�С
#define ADDRESS_BYTE 4         //����һ����ַ��BYTE��
#define MAX_DATA_LENGTH  40    //����һ��DATA����󳤶�
 
class BufferManager{
public:
	//�����ļ�����Ϣ�ʹ洢�ռ�
	struct Block{
		int offsetNum;         //���Ӧ���ļ�ƫ��λ�ã������ļ��Ķ�д
		int dirty;             //dirtyΪ1��ʱ��Ϊ��
		int lru;               //lruΪ1��ʱ��Ϊ��һ����
		int xlock;             //xlock
		int slock;             //slock
		int usage;             //���������ڴ�ʹ�õ㣬���Ϊ-1����ʾ����黹û�б����뵽��ά����
		int recordLength;      //record+record+...+7
		char  address[BLOCK_SIZE];        //1 char = 1 BYTE����������������
		char * fileName;       //�ļ���
		Block * nextBlock;     //ָ����һ���ļ���ڵ�
		Block * preBlock;      //ָ��ǰһ���ļ���ڵ�
	};
	//�����ļ��ڵ�洢��Ϣ�ʹ洢�ռ�
	struct File{
		int type;              //�ļ�����
		int recordLength;      //���ļ��еļ�¼����
		int usage;             //��־��û�б����뵽��ά����û��Ϊ-1����Ϊ0
		char  fileName[FILENAME_SIZE];       //�ļ���
		Block * blockHead;    //ָ���ļ�������
	};
	//�޲ι��캯��
	BufferManager();
	//��������
	~BufferManager();
	//д�����п�
	int  flushAll();    
	//д�ᵱǰ��
	int  flush(Block * b,int Force);
	//���ع�
	int  reconstruct(Block *b);
	//�ļ������п��ع�
	int  reconstructAll(File *fp);
	//�õ��ļ�����Ӧ���ļ���ͷ
	File  * getFile(const char * fileName);
	//��������õ���Ӧ�����ݿ�
	Block * getBlock(BufferManager::File * file, int requirement);

private:
	int totalBlock;
	int totalFile;
	//�ҵ����ݿ��ʹ�ö����ֽ���
	int findUsage(const char * fileName,const char * address);
	File   f[MAX_FILE_NUM];
	Block  b[MAX_BLOCK_NUM];
	//�ҵ�Ҫ�滻�Ļ����ҵ��Ŀ�
	Block * findReplaceBlock(BufferManager::File * file);
	
};
