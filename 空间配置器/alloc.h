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
	if (pos == string::npos)        //npos为-1
		return  path;
	else
		return  path.substr(pos + 1);  //找到/后，返回后面的文件名
}
//  用于调试追溯的trace log
inline  static  void  __trace_debug(const  char*  function,
	const  char  *  filename, int  line, char*  format, ...)
{
#ifdef  __DEBUG__
	//  输出调用函数的信息
	fprintf(stdout, "【  %s:%d】%s", GetFileName(filename).c_str(), line, function);
	//  输出用户打的trace信息
	va_list  args;
	va_start(args, format);
	vfprintf(stdout, format, args);
	va_end(args);
#endif
}
#define  __TRACE_DEBUG(...)  \
__trace_debug(__FUNCTION__  ,  __FILE__,  __LINE__,  __VA_ARGS__);

//一级配置器
template<int inst>
class __MallocAllocTemplate
{
public:
	static void * Allocate(size_t n)
	{
		//分配成功直接返回
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

	//当内存不足时，允许用户传入函数指针f，释放f指向的内存，让一级配置器开辟空间
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
		//分配失败后，检查是否有用户设置的handler
		//有则调用以后再分配。不断重复这个过程，直到分配成功
		//没有设置处理的handler，则直接结束程序。
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

//二级配置器

template <bool threads,int inst>
class __DefaultAllocTemplate
{
public:
	enum { __ALIGN = 8 };
	enum { __MAX_BYTES = 128 };
	enum { __NFREELISTS = __MAX_BYTES / __ALIGN };
	static size_t ROUND_UP(size_t bytes)
	{
		//对齐
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
		union Obj* _freeListLink;   //指向下一个内存块的指针
	char _clientData[1];   /* The client sees this.*/
	Obj()
		:_freeListLink(NULL)
	{}
	};
	static Obj* volatile _freeList[__NFREELISTS];    //自由链表
	static char* _startFree;                        //内存池水位线开始
	static char* _endFree;                          //内存池水位线结束
	static size_t _heapSize;                        //从系统堆分配的总大小
												  //获取大块内存插入到自由链表中
	static void* Refill(size_t n);
	//从内存池中分配大块内存
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
	//大于__MAX_BYTES则调用一级配置器
	if (n > __MAX_BYTES)
	{
		return MallocAlloc::Allocate(n);
	}
	int index = FREELIST_INDEX(n);
	Obj* head = _freeList[index];
	//如果自由链表中没有内存则通过Refill填充
	//如果自由链表中有则直接返回一个节点块内存
	if (head == NULL)
	{
		return Refill(ROUND_UP(n));
	}
	else
	{
		__TRACE_DEBUG("自由链表中取内存_freeList[%d]\n", index);
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
	//分配njobs个n字节的内存
	//如果不够njobs能分配多少分多少
	//
	int njobs = 20;
	char* chunk= ChunkAlloc(n,njobs);
	if (njobs == 1)
	{
		return chunk;
	}
	Obj* result = (Obj*)chunk; //第一个内存块返回
	Obj* cur = (Obj*)(chunk + n);   //从第二个内存块开始链在_freeList[]后。
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
	// 1.内存池中的内存足够，bytesHad>=bytesNeed，则直接从内存池中取。
	// 2.内存池中的内存不足，但是够一个bytesHad >= size，则直接取能够取出来。
	// 3.内存池中的内存不足，则从系统堆分配大块内存到内存池中。
	//
	if (bytesHad >= bytesNeed)
	{
		__TRACE_DEBUG("内存池中内存足够分配%d个对象\n", njobs);
		result = _startFree;
		_startFree += bytesNeed;
	}
	else if (bytesHad > size)
	{
		__TRACE_DEBUG("内存池中内存不够分配%d个对象，只能分配%d个对象\n", njobs, bytesHad / size);
		result = _startFree;
		njobs = bytesHad / size;
		_startFree += njobs*size;
	}
	else
	{
		//若内存池中还有小块剩余内存，则将它头插到合适的自由链表
		if (bytesHad > 0)
		{
			size_t index = FREELIST_INDEX(bytesHad);
			((Obj*)_startFree)->_freeListLink = _freeList[index];
			_freeList[index] = (Obj*)_startFree;
			_startFree = NULL;
			__TRACE_DEBUG("将内存池中剩余的空间，分配给freeList[%d]\n", index);
		}

		//从系统堆分配两倍+已分配的heapSize/8的内存到内存池中
		size_t bytesGet = 2 * bytesNeed + ROUND_UP(_heapSize >> 4);
		_startFree = (char*)malloc(bytesGet);
		__TRACE_DEBUG("内存池空间不足，系统堆分配%u bytes内存\n", bytesGet);
		//如果在系统堆中内存分配失败，则尝试到自由链表中更大的节点中分配
		//
		if (_startFree == NULL)
		{
			__TRACE_DEBUG("系统堆已无足够内存，无奈之下，只能到自由链表中看看\n");
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
			//自由链表中也没有分配到内存，则再到一级配置器中分配内存，
			//一级配置器中可能有设置的处理内存，或许能分配到内存。
			//
			__TRACE_DEBUG("系统堆和自由链表都已无内存，一级配置器做最后一根稻草\n");
			_startFree = (char*)MallocAlloc::Allocate(bytesGet);
		}
		//从系统堆分配的总字节数。（可用于下次分配时进行调节）
		_heapSize += bytesGet;
		_endFree = _startFree + bytesGet;
		//递归调用获取内存
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
	// 测试调用一级配置器分配内存
	cout << " 测试调用一级配置器分配内存 " << endl;
	char*p1 = SimpleAlloc< char, Alloc>::Allocate(129);
	SimpleAlloc<char, Alloc>::Deallocate(p1, 129);
	// 测试调用二级配置器分配内存
	cout << " 测试调用二级配置器分配内存 " << endl;
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
		printf(" 测试第%d次分配 \n", i + 1);
		char*p = SimpleAlloc< char, Alloc>::Allocate(128);
	}
}
// 测试特殊场景
void Test2()
{
	cout << " 测试内存池空间不足分配个 " << endl;
	// 8*20->8*2->320
	char*p1 = SimpleAlloc< char, Alloc>::Allocate(8);
	char*p2 = SimpleAlloc< char, Alloc>::Allocate(8);
	cout << " 测试内存池空间不足，系统堆进行分配 " << endl;
	char*p3 = SimpleAlloc< char, Alloc>::Allocate(12);
}
// 测试系统堆内存耗尽的场景
void Test3()
{
	cout << " 测试系统堆内存耗尽 " << endl;
	SimpleAlloc<char, Alloc>::Allocate(1024 * 1024 * 1024);
	//SimpleAlloc<char, Alloc>::Allocate(1024*1024*1024);
	SimpleAlloc<char, Alloc>::Allocate(1024 * 1024);
	// 不好测试，说明系统管理小块内存的能力还是很强的。
	for (int i = 0; i < 100000; ++i)
	{
		char* p1 = SimpleAlloc< char, Alloc>::Allocate(128);
	}
}



#endif





