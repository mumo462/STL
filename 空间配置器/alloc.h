#pragma once
#include <malloc.h>
#include <string>
#include<stdarg.h>
#define  __DEBUG__
//#define __USE_MALLOC
static string GetFileName(const string&  path)
{
	char  ch = '/';
#ifdef  _WIN32
	ch = '\\';
#endif
	size_t  pos = path.rfind(ch);
	if (pos == string::npos)        //nposΪ-1
		return  path;
	else
		return  path.substr(pos + 1);  //�ҵ�/�󣬷��غ�����ļ���
}
//  ���ڵ���׷�ݵ�trace log
inline  static  void  __trace_debug(const  char*  function,
	const  char  *  filename, int  line, char*  format, ...)
{
#ifdef  __DEBUG__
	//  ������ú�������Ϣ
	fprintf(stdout, "��  %s:%d��%s", GetFileName(filename).c_str(), line, function);
	//  ����û����trace��Ϣ
	va_list  args;
	va_start(args, format);
	vfprintf(stdout, format, args);
	va_end(args);
#endif
}
#define  __TRACE_DEBUG(...)  \
__trace_debug(__FUNCTION__  ,  __FILE__,  __LINE__,  __VA_ARGS__);

//һ��������
template<int inst>
class __MallocAllocTemplate
{
public:
	static void * Allocate(size_t n)
	{
		//����ɹ�ֱ�ӷ���
		string s = "adasd";
		__TRACE_DEBUG("(n:%u)\n", n);
		void *result = malloc(n);
		if (0 == result)
			result = OomMalloc(n);
		return result;
	}

	static void Deallocate(void *p, size_t /* n */)
	{
		__TRACE_DEBUG("(p:%p)\n", p);
		free(p);
	}

	static void * Reallocate(void *p, size_t /* old_sz */, size_t new_sz)
	{
		void * result = realloc(p, new_sz);
		if (0 == result)
			result = OomRealloc(p, new_sz);
		return result;
	}

	//���ڴ治��ʱ�������û����뺯��ָ��f���ͷ�fָ����ڴ棬��һ�����������ٿռ�
	typedef void (*ALLOC_FUNC)();
	static ALLOC_FUNC SetMallocHandler(ALLOC_FUNC f)
	{
		FUNC old = __MallocAllocOomHandler;
		__MallocAllocOomHandler = f;
		return(old);
	}
private:
	
	static void *OomMalloc(size_t n)
	{
		//����ʧ�ܺ󣬼���Ƿ����û����õ�handler
		//��������Ժ��ٷ��䡣�����ظ�������̣�ֱ������ɹ�
		//û�����ô����handler����ֱ�ӽ�������
		ALLOC_FUNC handler;
		void *result;
		while (1)
		{
			handler = __MallocAllocOomHandler;
			if (handler == 0)
			{
				cout << "out of memory" << endl;
				exit(1);
			}
			handler();
			result = malloc(n);
			if (result)
				return(result);
		}
	}
	static void *OomRealloc(void *p, size_t n)
	{
		ALLOC_FUNC handler;
		void* result;
		while(1) 
		{
			handler = ___MallocAllocOomHandler;
				if (0 == handler)
				{
					cout << "out of memory" << endl;
						exit(-1);
				}
			handler;
			result = realloc(p, n);
			if (result)
				return(result);
		}
	}

	static void(*__MallocAllocOomHandler)();

};

template <>
void(*__MallocAllocTemplate<0>::__MallocAllocOomHandler)() = 0;

typedef __MallocAllocTemplate<0> MallocAlloc;
#ifdef __USE_MALLOC
typedef MallocAlloc Alloc;
#else

//����������

template <bool threads,int inst>
class __DefaultAllocTemplate
{
public:
	enum { __ALIGN = 8 };
	enum { __MAX_BYTES = 128 };
	enum { __NFREELISTS = __MAX_BYTES / __ALIGN };
	static size_t ROUND_UP(size_t bytes)
	{
		//����
		return((bytes + __ALIGN - 1) & ~(__ALIGN - 1));
	}
	static size_t FREELIST_INDEX(size_t bytes)
	{
		// bytes == 9
		// bytes == 8
		// bytes == 7
		return((bytes + __ALIGN - 1) / __ALIGN - 1);
	}
	union Obj
	{
		union Obj* _freeListLink;   //ָ����һ���ڴ���ָ��
	char _clientData[1];   /* The client sees this.*/
	Obj()
		:_freeListLink(NULL)
	{}
	};
	static Obj* volatile _freeList[__NFREELISTS];    //��������
	static char* _startFree;                        //�ڴ��ˮλ�߿�ʼ
	static char* _endFree;                          //�ڴ��ˮλ�߽���
	static size_t _heapSize;                        //��ϵͳ�ѷ�����ܴ�С
												  //��ȡ����ڴ���뵽����������
	static void* Refill(size_t n);
	//���ڴ���з������ڴ�
	static char* ChunkAlloc(size_t size, int& njobs);
	static void* Allocate(size_t n);
	static void Deallocate(void* p, size_t n);
	static void* Reallocate(void*p, size_t old_sz, size_t new_sz);
};
template<bool threads, int inst>
typename __DefaultAllocTemplate<threads, inst>::Obj* volatile 
__DefaultAllocTemplate<threads, inst>::_freeList[__NFREELISTS] = {0};
template<bool threads,int inst>
char* __DefaultAllocTemplate<threads,inst>::_endFree = 0;
template<bool threads, int inst>
size_t __DefaultAllocTemplate<threads, inst>::_heapSize = 0;
template<bool threads, int inst>
char* __DefaultAllocTemplate<threads, inst>::_startFree = 0;

template<bool threads, int inst>
void* __DefaultAllocTemplate<threads, inst>::Allocate(size_t n)
{
	__TRACE_DEBUG("(n:%u)\n",n);
	//����__MAX_BYTES�����һ��������
	if (n > __MAX_BYTES)
	{
		return MallocAlloc::Allocate(n);
	}
	int index = FREELIST_INDEX(n);
	Obj* head = _freeList[index];
	//�������������û���ڴ���ͨ��Refill���
	//�����������������ֱ�ӷ���һ���ڵ���ڴ�
	if (head == NULL)
	{
		return Refill(ROUND_UP(n));
	}
	else
	{
		__TRACE_DEBUG("����������ȡ�ڴ�_freeList[%d]\n", index);
		_freeList[index] = head->_freeListLink;
		return head;
	}
}

template<bool threads, int inst>
void __DefaultAllocTemplate<threads, inst>::Deallocate(void* p, size_t n)
{
	__TRACE_DEBUG("(p:%p, n: %u)\n", p, n);
	if (n > __NFREELISTS)
	{
		MallocAlloc::Deallocate(p,n);
		return;
	}
	size_t index = FREELIST_INDEX(n);
	Obj* tmp = (Obj*)p;
	tmp->_freeListLink = _freeList[index];
	_freeList[index] = tmp;
}

template<bool threads, int inst>
void* __DefaultAllocTemplate<threads, inst>::Refill(size_t n)
{
	__TRACE_DEBUG("(n:%u)\n",n);
	//����njobs��n�ֽڵ��ڴ�
	//�������njobs�ܷ�����ٷֶ���
	//
	int njobs = 20;
	char* chunk= ChunkAlloc(n,njobs);
	if (njobs == 1)
	{
		return chunk;
	}
	Obj* result = (Obj*)chunk; //��һ���ڴ�鷵��
	Obj* cur = (Obj*)(chunk + n);   //�ӵڶ����ڴ�鿪ʼ����_freeList[]��
	size_t index = FREELIST_INDEX(n);
	_freeList[index] = cur;
	for (int i = 2; i < njobs; i++)
	{
		cur->_freeListLink = (Obj*)(chunk+n*i);
		cur = cur->_freeListLink;
	}
	cur->_freeListLink = 0;
	return result;
}

template<bool threads, int inst>
char* __DefaultAllocTemplate<threads, inst>::ChunkAlloc(size_t size, int& njobs)
{
	__TRACE_DEBUG("(size: %u, njobs: %d)\n", size, njobs);

	char* result;
	size_t bytesNeed = size*njobs;
	size_t bytesHad = _endFree - _startFree;
	//
	// 1.�ڴ���е��ڴ��㹻��bytesHad>=bytesNeed����ֱ�Ӵ��ڴ����ȡ��
	// 2.�ڴ���е��ڴ治�㣬���ǹ�һ��bytesHad >= size����ֱ��ȡ�ܹ�ȡ������
	// 3.�ڴ���е��ڴ治�㣬���ϵͳ�ѷ������ڴ浽�ڴ���С�
	//
	if (bytesHad >= bytesNeed)
	{
		__TRACE_DEBUG("�ڴ�����ڴ��㹻����%d������\n", njobs);
		result = _startFree;
		_startFree += bytesNeed;
	}
	else if (bytesHad > size)
	{
		__TRACE_DEBUG("�ڴ�����ڴ治������%d������ֻ�ܷ���%d������\n", njobs, bytesHad / size);
		result = _startFree;
		njobs = bytesHad / size;
		_startFree += njobs*size;
	}
	else
	{
		//���ڴ���л���С��ʣ���ڴ棬����ͷ�嵽���ʵ���������
		if (bytesHad > 0)
		{
			size_t index = FREELIST_INDEX(bytesHad);
			((Obj*)_startFree)->_freeListLink = _freeList[index];
			_freeList[index] = (Obj*)_startFree;
			_startFree = NULL;
			__TRACE_DEBUG("���ڴ����ʣ��Ŀռ䣬�����freeList[%d]\n", index);
		}

		//��ϵͳ�ѷ�������+�ѷ����heapSize/8���ڴ浽�ڴ����
		size_t bytesGet = 2 * bytesNeed + ROUND_UP(_heapSize >> 4);
		_startFree = (char*)malloc(bytesGet);
		__TRACE_DEBUG("�ڴ�ؿռ䲻�㣬ϵͳ�ѷ���%u bytes�ڴ�\n", bytesGet);
		//�����ϵͳ�����ڴ����ʧ�ܣ����Ե����������и���Ľڵ��з���
		//
		if (_startFree == NULL)
		{
			__TRACE_DEBUG("ϵͳ�������㹻�ڴ棬����֮�£�ֻ�ܵ����������п���\n");
			for (int i = size; i <= __MAX_BYTES; i += __ALIGN)
			{
				Obj* head = _freeList[FREELIST_INDEX(size)];
				if (head)
				{
					_startFree = (char*)head;
					head = head->_freeListLink;
					_endFree = _startFree + i;
					return ChunkAlloc(size, njobs);
				}
			}
			//����������Ҳû�з��䵽�ڴ棬���ٵ�һ���������з����ڴ棬
			//һ���������п��������õĴ����ڴ棬�����ܷ��䵽�ڴ档
			//
			__TRACE_DEBUG("ϵͳ�Ѻ��������������ڴ棬һ�������������һ������\n");
			_startFree = (char*)MallocAlloc::Allocate(bytesGet);
		}
		//��ϵͳ�ѷ�������ֽ��������������´η���ʱ���е��ڣ�
		_heapSize += bytesGet;
		_endFree = _startFree + bytesGet;
		//�ݹ���û�ȡ�ڴ�
		return ChunkAlloc(size, njobs);
	}
	return result;

}

typedef __DefaultAllocTemplate<0, 0> Alloc;

template <class T, class Alloc>
class SimpleAlloc {
public:
	static T *Allocate(size_t n)
	{
		return 0 == n ? 0 : (T*)Alloc::Allocate(n * sizeof(T));
	}
	static T *Allocate(void)
	{
		return (T*)Alloc::Allocate(sizeof(T));
	}
	static void Deallocate(T *p, size_t n)
	{
		if (0 != n) Alloc::Deallocate(p, n * sizeof(T));
	}
	static void Deallocate(T *p)
	{
		Alloc::Deallocate(p, sizeof(T));
	}
};
void Test1()
{
	// ���Ե���һ�������������ڴ�
	cout << " ���Ե���һ�������������ڴ� " << endl;
	char*p1 = SimpleAlloc< char, Alloc>::Allocate(129);
	SimpleAlloc<char, Alloc>::Deallocate(p1, 129);
	// ���Ե��ö��������������ڴ�
	cout << " ���Ե��ö��������������ڴ� " << endl;
	char*p2 = SimpleAlloc< char, Alloc>::Allocate(128);
	char*p3 = SimpleAlloc< char, Alloc>::Allocate(128);
	char*p4 = SimpleAlloc< char, Alloc>::Allocate(128);
	char*p5 = SimpleAlloc< char, Alloc>::Allocate(128);
	SimpleAlloc<char, Alloc>::Deallocate(p2, 128);
	SimpleAlloc<char, Alloc>::Deallocate(p3, 128);
	SimpleAlloc<char, Alloc>::Deallocate(p4, 128);
	SimpleAlloc<char, Alloc>::Deallocate(p5, 128);
	for (int i = 0; i < 21; ++i)
	{
		printf(" ���Ե�%d�η��� \n", i + 1);
		char*p = SimpleAlloc< char, Alloc>::Allocate(128);
	}
}
// �������ⳡ��
void Test2()
{
	cout << " �����ڴ�ؿռ䲻������ " << endl;
	// 8*20->8*2->320
	char*p1 = SimpleAlloc< char, Alloc>::Allocate(8);
	char*p2 = SimpleAlloc< char, Alloc>::Allocate(8);
	cout << " �����ڴ�ؿռ䲻�㣬ϵͳ�ѽ��з��� " << endl;
	char*p3 = SimpleAlloc< char, Alloc>::Allocate(12);
}
// ����ϵͳ���ڴ�ľ��ĳ���
void Test3()
{
	cout << " ����ϵͳ���ڴ�ľ� " << endl;
	SimpleAlloc<char, Alloc>::Allocate(1024 * 1024 * 1024);
	//SimpleAlloc<char, Alloc>::Allocate(1024*1024*1024);
	SimpleAlloc<char, Alloc>::Allocate(1024 * 1024);
	// ���ò��ԣ�˵��ϵͳ����С���ڴ���������Ǻ�ǿ�ġ�
	for (int i = 0; i < 100000; ++i)
	{
		char* p1 = SimpleAlloc< char, Alloc>::Allocate(128);
	}
}



#endif





