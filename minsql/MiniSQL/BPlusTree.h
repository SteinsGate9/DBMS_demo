#include<vector>
#include<map>
#include<iostream>
#include <limits.h>

using namespace std;

template<class KeyType,class DataType>
class Tree
{
private:
	static const int M=4;
	static const int MinKeyNum=(M%2)?(M+1)/2:M/2;
	struct Node
	{
	    int KeyNum;
	    KeyType Key[M+1];
	    Node *Children[M+1];
	    Node *Next;
	};
private:
	Node *Root;
	map<KeyType,int> Index;
	vector<DataType> Data;
private:
	// Ϊ�½ڵ�����ռ� 
	Node* CreateNode()
	{
	    Node *p;
	    int i=0;
	    p = (Node*)malloc(sizeof(struct Node));
	    if ( p==NULL )
	        exit(EXIT_FAILURE);
	    // ��ʼ���½ڵ� 
	    while( i<M+1 )
		{
	        p->Key[i]=-1;			// KeyΪ-1ʱ�����ֵ 
	        p->Children[i]=NULL;
	        i++;
	    }
	    p->Next=NULL;
	    p->KeyNum=0;
	    return p;
	}
	
	// ���ĳ�ʼ�� 
	Node* Initialize()
	{
	    Node *T=CreateNode();
	    return T;
	}
	
	// �ҵ�����ߵĽڵ� 
	Node* FindFirst( Node *T )
	{
	    Node* p=T;
	    while( p!=NULL && p->Children[0]!=NULL )
	        p=p->Children[0];
	    return p;
	}
	
	// �ҵ����ұߵĽڵ� 
	Node* FindLast( Node *T )
	{
	    Node* p=T;
	    while( p!=NULL && p->Children[p->KeyNum-1]!=NULL )
	        p=p->Children[p->KeyNum-1];
	    return p;
	}
	 
	// Ѱ��һ���ؼ���δ�����ֵܽڵ㣬�Ҳ�������NULL
	Node* FindSibling1( Node* Father,int i)
	{
	    Node *Sibling=NULL;
	    if( i==0 )
		{
	        if( Father->Children[1]->KeyNum<M )
	            Sibling=Father->Children[1];
	    }
	    else if( Father->Children[i-1]->KeyNum<M )
	        Sibling=Father->Children[i-1];
	    else if( i<Father->KeyNum-1 && Father->Children[i+1]->KeyNum<M )
	        Sibling=Father->Children[i+1];
	    return Sibling;
	}
	 
	// Ѱ��һ���ؼ���������M/2���ֵܽڵ㣬�Ҳ�������NULL
	Node* FindSibling2( Node *Father, int i, int *j )
	{
	    Node *Sibling;
	    Sibling=NULL;
	    if ( i==0 )
		{
	        if( Father->Children[1]->KeyNum>MinKeyNum )
			{
	            Sibling=Father->Children[1];
	            *j = 1;
	        }
	    }
	    else{
	        if (Father->Children[i - 1]->KeyNum > Limit){
	            Sibling = Father->Children[i - 1];
	            *j = i - 1;
	        }
	        else if (i + 1 < Father->KeyNum && Father->Children[i + 1]->KeyNum > Limit){
	            Sibling = Father->Children[i + 1];
	            *j = i + 1;
	        }
	        
	    }
	    
	    return Sibling;
	}
	 
	/* ��Ҫ��X����Key��ʱ��i��X��Father��λ�ã�j��KeyҪ�����λ��
	   ��Ҫ��Father����X�ڵ��ʱ��i��Ҫ�����λ�ã�Key��j��ֵû����
	 */
	Node* InsertElement(int isKey, Node* Father,Node* X,KeyType Key,int i,int j){
	    
	    int k;
	    if (isKey){
	        /* ����key */
	        k = X->KeyNum - 1;
	        while (k >= j){
	            X->Key[k + 1] = X->Key[k];k--;
	        }
	        
	        X->Key[j] = Key;
	        
	        if (Father != NULL)
	            Father->Key[i] = X->Key[0];
	        
	        X->KeyNum++;
	        
	    }else{
	        /* ����ڵ� */
	        
	        /* ����Ҷ�ڵ�������� */
	        if (X->Children[0] == NULL){
	            if (i > 0)
	                Father->Children[i - 1]->Next = X;
	            X->Next = Father->Children[i];
	        }
	        
	        k = Father->KeyNum - 1;
	        while (k >= i){
	            Father->Children[k + 1] = Father->Children[k];
	            Father->Key[k + 1] = Father->Key[k];
	            k--;
	        }
	        Father->Key[i] = X->Key[0];
	        Father->Children[i] = X;
	        
	        Father->KeyNum++;
	        
	    }
	    return X;
	}
	 
	 
	Node* RemoveElement(int isKey, Node* Father,Node* X,int i,int j){
	    
	    int k,Limit;
	    
	    if (isKey){
	        Limit = X->KeyNum;
	        /* ɾ��key */
	        k = j + 1;
	        while (k < Limit){
	            X->Key[k - 1] = X->Key[k];k++;
	        }
	        
	        X->Key[X->KeyNum - 1] = -1;
	        
	        Father->Key[i] = X->Key[0];
	        
	        X->KeyNum--;
	    }else{
	        /* ɾ���ڵ� */
	        
	        /* �޸���Ҷ�ڵ������ */
	        if (X->Children[0] == NULL && i > 0){
	            Father->Children[i - 1]->Next = Father->Children[i + 1];
	        }
	        Limit = Father->KeyNum;
	        k = i + 1;
	        while (k < Limit){
	            Father->Children[k - 1] = Father->Children[k];
	            Father->Key[k - 1] = Father->Key[k];
	            k++;
	        }
	        
	        Father->Children[Father->KeyNum - 1] = NULL;
	        Father->Key[Father->KeyNum - 1] = -1;
	        
	        Father->KeyNum--;
	        
	    }
	    return X;
	}
	 
	/* Src��Dst���������ڵĽڵ㣬i��Src��Father�е�λ�ã�
	 ��Src��Ԫ���ƶ���Dst�� ,n���ƶ�Ԫ�صĸ���*/
	Node* MoveElement(Node* Src,Node* Dst,Node* Father,int i,int n){
	    KeyType TmpKey;
	    Node* Child;
	    int j,SrcInFront;
	    
	    SrcInFront = 0;
	    
	    if (Src->Key[0] < Dst->Key[0])
	        SrcInFront = 1;
	    
	    j = 0;
	    /* �ڵ�Src��Dstǰ�� */
	    if (SrcInFront){
	        if (Src->Children[0] != NULL){
	            while (j < n) {
	                Child = Src->Children[Src->KeyNum - 1];
	                RemoveElement(0, Src, Child, Src->KeyNum - 1, -1);
	                InsertElement(0, Dst, Child, -1, 0, -1);
	                j++;
	            }
	        }else{
	            while (j < n) {
	                TmpKey = Src->Key[Src->KeyNum -1];
	                RemoveElement(1, Father, Src, i, Src->KeyNum - 1);
	                InsertElement(1, Father, Dst, TmpKey, i + 1, 0);
	                j++;
	            }
	            
	        }
	        
	        Father->Key[i + 1] = Dst->Key[0];
	        /* ����Ҷ�ڵ��������� */
	        if (Src->KeyNum > 0)
	            FindLast(Src)->Next = FindFirst(Dst);
	        
	    }else{
	        if (Src->Children[0] != NULL){
	            while (j < n) {
	                Child = Src->Children[0];
	                RemoveElement(0, Src, Child, 0, -1);
	                InsertElement(0, Dst, Child, -1, Dst->KeyNum, -1);
	                j++;
	            }
	            
	        }else{
	            while (j < n) {
	                TmpKey = Src->Key[0];
	                RemoveElement(1, Father, Src, i, 0);
	                InsertElement(1, Father, Dst, TmpKey, i - 1, Dst->KeyNum);
	                j++;
	            }
	            
	        }
	        
	        Father->Key[i] = Src->Key[0];
	        if (Src->KeyNum > 0)
	            FindLast(Dst)->Next = FindFirst(Src);
	        
	    }
	    
	    return Father;
	}
	 
	Node* SplitNode(Node* Father,Node* X,int i){
	    int j,k,Limit;
	    Node* NewNode;
	    
	    NewNode = CreateNode();
	    
	    k = 0;
	    j = X->KeyNum / 2;
	    Limit = X->KeyNum;
	    while (j < Limit){
	        if (X->Children[0] != NULL){
	            NewNode->Children[k] = X->Children[j];
	            X->Children[j] = NULL;
	        }
	        NewNode->Key[k] = X->Key[j];
	        X->Key[j] = -1;
	        NewNode->KeyNum++;X->KeyNum--;
	        j++;k++;
	    }
	    
	    if (Father != NULL)
	        InsertElement(0, Father, NewNode, -1, i + 1, -1);
	    else{
	        /* �����X�Ǹ�����ô�����µĸ������� */
	        Father = CreateNode();
	        InsertElement(0, Father, X, -1, 0, -1);
	        InsertElement(0, Father, NewNode, -1, 1, -1);
	        
	        return Father;
	    }
	    
	    return X;
	}
	 
	/* �ϲ��ڵ�,X����M/2�ؼ��֣�S�д��ڻ����M/2���ؼ���*/
	Node* MergeNode(Node* Father, Node* X,Node* S,int i){
	    int Limit;
	    
	    /* S�Ĺؼ�����Ŀ����M/2 */
	    if (S->KeyNum > MinKeyNum){
	        /* ��S���ƶ�һ��Ԫ�ص�X�� */
	        MoveElement(S, X, Father, i,1);
	    }else{
	        /* ��Xȫ��Ԫ���ƶ���S�У�����Xɾ�� */
	        Limit = X->KeyNum;
	        MoveElement(X,S, Father, i,Limit);
	        RemoveElement(0, Father, X, i, -1);
	        
	        free(X);
	        X = NULL;
	    }
	    
	    return Father;
	}
	 
	bool recursive_search( Node *T, KeyType key )
	{
		if( T->KeyNum==0 )
			return false;
		int j=0;
		while( j<T->KeyNum && key>=T->Key[j] )
		{
			if( key==T->Key[j] )
				return true;
			j++;
		}
		if( j!=0 && T->Children[0]!=NULL )
			j--;
		if( T->Children[0]==NULL )
			return false;
		else
			return recursive_search(T->Children[j],key);
	}
	
	Node* findNode( Node *T, KeyType key )
	{
		if( T->KeyNum==0 )
			return NULL;
		int j=0;
		while( j<T->KeyNum && key>=T->Key[j] )
			j++;
		if( j!=0 && T->Children[0]!=NULL )
			j--;
		if( T->Children[0]==NULL )
			return T;
		else
			return findNode(T->Children[j],key);
	}
	
	Node* RecursiveInsert(Node* T,KeyType Key,int i,Node* Father){
	    int j,Limit;
	    Node* Sibling;
	    
	    /* ���ҷ�֧ */
	    j = 0;
	    while (j < T->KeyNum && Key >= T->Key[j]){
	        /* �ظ�ֵ������ */
	        if (Key == T->Key[j])
	            return T;
	        j++;
	    }
	    if (j != 0 && T->Children[0] != NULL) j--;
	    
	    /* ��Ҷ */
	    if (T->Children[0] == NULL)
	        T = InsertElement(1, Father, T, Key, i, j);
	    /* �ڲ��ڵ� */
	    else
	        T->Children[j] = RecursiveInsert(T->Children[j], Key, j, T);
	    
	    /* �����ڵ� */
	    
	    Limit = M;
	    
	    if (T->KeyNum > Limit){
	        /* �� */
	        if (Father == NULL){
	            /* ���ѽڵ� */
	            T = SplitNode(Father, T, i);
	        }
	        else{
	            Sibling = FindSibling1(Father, i);
	            if (Sibling != NULL){
	                /* ��T��һ��Ԫ�أ�Key����Child���ƶ���Sibing�� */
	                MoveElement(T, Sibling, Father, i, 1);
	            }else{
	                /* ���ѽڵ� */
	                T = SplitNode(Father, T, i);
	            }
	        }
	        
	    }
	    
	    if (Father != NULL)
	        Father->Key[i] = T->Key[0];
	    
	    
	    return T;
	}
	 
	/* ���� */
	Node* Insert(Node* T,KeyType Key){
	    return RecursiveInsert(T, Key, 0, NULL);
	}
	
	
	 
	Node* RecursiveRemove(Node* T,KeyType Key,int i,Node* Father){
	    
	    int j,NeedAdjust;
	    Node* Sibling, *Tmp;
	    
	    Sibling = NULL;
	    
	    /* ���ҷ�֧ */
	    j = 0;
	    while (j < T->KeyNum && Key >= T->Key[j]){
	        if (Key == T->Key[j])
	            break;
	        j++;
	    }
	    
	    if (T->Children[0] == NULL){
	        /* û�ҵ� */
	        if (Key != T->Key[j] || j == T->KeyNum)
	            return T;
	    }else
	        if (j == T->KeyNum || Key < T->Key[j]) j--;
	    
	    
	    
	    /* ��Ҷ */
	    if (T->Children[0] == NULL){
	        T = RemoveElement(1, Father, T, i, j);
	    }else{
	        T->Children[j] = RecursiveRemove(T->Children[j], Key, j, T);
	    }
	    
	    NeedAdjust = 0;
	    /* ���ĸ�������һƬ��Ҷ���������������2��M֮�� */
	    if (Father == NULL && T->Children[0] != NULL && T->KeyNum < 2)
	        NeedAdjust = 1;
	    /* �����⣬���з���Ҷ�ڵ�Ķ�������[M/2]��M֮�䡣(����[]��ʾ����ȡ��) */
	    else if (Father != NULL && T->Children[0] != NULL && T->KeyNum < MinKeyNum)
	        NeedAdjust = 1;
	    /* ���Ǹ�����Ҷ�йؼ��ֵĸ���Ҳ��[M/2]��M֮�� */
	    else if (Father != NULL && T->Children[0] == NULL && T->KeyNum < MinKeyNum)
	        NeedAdjust = 1;
	    
	    /* �����ڵ� */
	    if (NeedAdjust){
	        /* �� */
	        if (Father == NULL){
	            if(T->Children[0] != NULL && T->KeyNum < 2){
	                Tmp = T;
	                T = T->Children[0];
	                free(Tmp);
	                return T;
	            }
	            
	        }else{
	            /* �����ֵܽڵ㣬��ؼ�����Ŀ����M/2 */
	            Sibling = FindSibling2(Father, i,&j);
	            if (Sibling != NULL){
	                MoveElement(Sibling, T, Father, j, 1);
	            }else{
	                if (i == 0)
	                    Sibling = Father->Children[1];
	                else
	                    Sibling = Father->Children[i - 1];
	                
	                Father = MergeNode(Father, T, Sibling, i);
	                T = Father->Children[i];
	            }
	        }
	        
	    }
	 
	    
	    return T;
	}
	 
	/* ɾ�� */
	Node* Remove(Node* T,KeyType Key){
	    return RecursiveRemove(T, Key, 0, NULL);
	}
	 
	/* ���� */
	Node* Destroy(Node* T){
	    int i,j;
	    if (T != NULL){
	        i = 0;
	        while (i < T->KeyNum + 1){
	            Destroy(T->Children[i]);i++;
	        }
	        
	        printf("Destroy:(");
	        j = 0;
	        while (j < T->KeyNum)/*  T->Key[i] != -1*/
	            printf("%d:",T->Key[j++]);
	        printf(") ");
	        free(T);
	        T = NULL;
	    }
	    
	    return T;
	}
	 
	void RecursiveTravel(Node* T,int Level){
	    int i;
	    if (T != NULL){
	        printf("  ");
	        printf("[Level:%d]-->",Level);
	        printf("(");
	        i = 0;
	        while (i < T->KeyNum)/*  T->Key[i] != -1*/
	            printf("%d:",T->Key[i++]);
	        printf(")\n");
	        
	        Level++;
	        
	        i = 0;
	        while (i <= T->KeyNum) {
	            RecursiveTravel(T->Children[i], Level);
	            i++;
	        }
	        
	        
	    }
	}
	 
	/* ���� */
	void Travel(Node* T){
	    RecursiveTravel(T, 0);
	    printf("\n");
	}
	 
	/* ������Ҷ�ڵ������ */
	void TravelData(Node* T){
	    Node* Tmp;
	    int i;
	    if (T == NULL)
	        return ;
	    printf("All Data:");
	    Tmp = T;
	    while (Tmp->Children[0] != NULL)
	        Tmp = Tmp->Children[0];
	    /* ��һƬ��Ҷ */
	    while (Tmp != NULL){
	        i = 0;
	        while (i < Tmp->KeyNum)
	            printf(" %d",Tmp->Key[i++]);
	        Tmp = Tmp->Next;
	    }
	}
public:
//	Node* CreateNode()
//	{
//		Node *p=new Node();
//		for( int i=0; i<M+1; i++ )
//		{
//			p->Key[i]=-1;
//			p->Children[i]=NULL;
//		}
//		p->Next=NULL;
//		p->KeyNum=0;
//		return p;
//	}
	Tree()
	{
//		Root=CreateNode();
		Root=Initialize();
		Data.push_back(0);
	}
	
	void insert( KeyType key, DataType data )
	{
		if( search(key) )
			return;
		if( Index[key] )
		{
			Data[Index[key]]=data;
		}
		else
		{
			int index=Data.size();
			Index[key]=index;
			Data.push_back(data);
		}
		Root = Insert(Root,key);
	}
	
	void remove( KeyType key )
	{
		Root = Remove(Root,key);
	}
	
	void destroy()
	{
		Root = Destroy(Root);
	}
	
	void travel()
	{
		Travel(Root);
	}
	
	void travelData()
	{
		TravelData(Root);
	}
	
	bool search( KeyType key )
	{
		return recursive_search(Root,key);
	}
	
	bool query( KeyType key, DataType &result )
	{
		if( search(key) )
		{
			result=Data[Index[key]];
			return true;
		}
		else
		{
			result=NULL;
			return false;
		}
	}
	
	bool range_query( KeyType first, KeyType second, DataType result[] )
	{
		Node *T1=findNode(Root,first);
		Node *T2=findNode(Root,second);
		int index=0;
		while( T1 )
		{
			int i;
			for( i=0; i<T1->KeyNum; i++ )
				if( T1->Key[i]>=first && T1->Key[i]<=second )
					break;
			while( i<T1->KeyNum )
			{
				if( T1->Key[i]>=first && T1->Key[i]<=second )
					result[index++]=Data[Index[T1->Key[i]]];
				else
					break;
				i++;
			}
			if( T1==T2 )
				break;
			else
				T1=T1->Next;
		}
		result[index]=NULL;
		if( index==0 )
			return false;
		else
			return true;
	}
	
	void test()
	{
		Node *T = findNode(Root,5);
		for( int i=0; i<T->KeyNum; i++ )
			cout<<T->Key[i]<<endl;
		T = findNode(Root,40);
		for( int i=0; i<T->KeyNum; i++ )
			cout<<T->Key[i]<<endl;
	}
};







