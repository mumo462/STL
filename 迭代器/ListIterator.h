#pragma once
#include <list>
template <class T>
struct __ListNode
{
	T _data;
	__ListNode<T>* _prev;
	__ListNode<T>* _next;

	__ListNode(const T& data=T())
		:_data(data)
		,_prev(NULL)
		,_next(NULL)
	{}
};
template <class T,class Ref,class Ptr>
struct __ListIterator
{
	typedef __ListIterator<T,T&,T*> Iterator;
	typedef __ListIterator<T, const T&,const T*> ConstIterator;
	typedef BidirectionalIteratorTag IteratorCategory;
	typedef int DifferenceType;
	typedef Ptr Pointer;
	typedef Ref Reference;
	typedef T ValueType;
	typedef __ListNode<T>* LinkType;
	typedef __ListIterator<T, Ref, Ptr>           self;

	LinkType _node;

	__ListIterator(LinkType node = NULL)
		:_node(node)
	{}
	__ListIterator(const Iterator& x)
		:_node(x._node)
	{}
	
	self& operator++()
	{

		_node = _node->_next;
		return *this;
	}

	self operator++(int)
	{
		self tmp(_node);
		_node = _node->_next;
		return tmp;
	}

	self operator--(int)
	{
		self tmp(_node);
		_node = _node->_prev;
		return tmp;
	}

	self& operator--()
	{
		_node = _node->_prev;
		return *this;
	}

	bool operator==(const self& it)const
	{
		return _node == it._node;
	}
	bool operator!=(const self& it)const
	{
		return _node != it._node;
	}
	Reference operator*()const
	{
		return _node->_data;
	}
	Pointer operator->()const
	{
		return &_node->_data;
	}
};

template <class T>
class List
{
public:
	typedef  __ListIterator<T,T&,T*>  Iterator;
	typedef  __ListIterator<T,const T&,const T*>  ConstIterator;
	typedef  T  ValueType;
	typedef  __ListNode<T>*  LinkType;

	List()
	{
		head = new __ListNode<T>;
		head->_next = head;
		head->_prev = head;
	}
	
	Iterator Insert(Iterator pos,const ValueType& x)
	{
		LinkType tmp = new __ListNode<T>(x);
		LinkType prev = pos._node->_prev;
		tmp->_next = pos._node;
		pos._node->_prev = tmp;

		prev->_next = tmp;
		tmp->_prev = prev;
		return tmp;
	}
	ConstIterator Begin()const 
	{
		return head->_next;
	}
	ConstIterator End() const
	{
		return head;
	}
	Iterator Begin()
	{
		return head->_next;
	}
	Iterator End()
	{
		return head;
	}
	void PushBack(const T& x)
	{
		Insert(End(),x);
	}
	void PushFront(const T& x)
	{
		Insert(Begin(),x);
	}
	Iterator Erase(Iterator pos)
	{
		assert(pos._node!=head);
		LinkType prev = pos._node->_prev;
		LinkType cur = pos._node->_next;
		prev->_next = cur;
		cur->_prev = prev;

		delete pos._node;

		return Iterator(cur);
	}
	void PopBack()
	{
		Erase(--End());
	}
	void PopFront()
	{
		Erase(Begin());
	}
protected:
	LinkType head;
};

void ListTest()
{
	//List<int> l;

	//l.PushBack(1);
	//l.PushBack(2);
	//l.PushBack(5);
	//l.PushBack(4);
	//l.PushBack(3);
	//l.PopBack();
	//l.PopBack();
	//l.PopBack();
	//l.PopBack();
	//l.PopBack();


	//List<int>::Iterator it = l.Begin();
	//while (it != l.End())
	//{
	//	cout << *it << " ";
	//	it++;
	//}
	//cout << endl;

	//迭代器失效
	//it = l.Begin();
	//while (it != l.End())
	//{
	//	if (*it % 2 == 0)
	//	{
	//		l.Erase(it);   //删除后it仍指向删除后的结点，++访问会越界
	//	}
	//	it++;
	//}
	//修改
	//it = l.Begin();
	//while (it != l.End())
	//{
	//	if (*it % 2 == 0)
	//	{
	//		it = l.Erase(it);    //Erase删除会返回下一个迭代器
	//	}
	//	else
	//	{
	//		cout << *it << " ";
	//		it++;
	//	}
	//}

}

void ListTest1()
{
	List<int> l;
	list<int> l1;
	l1.push_back(1);
	l1.push_back(2);

	l.PushBack(1);
	l.PushBack(2);
	l.PushBack(5);
	l.PushBack(4);
	l.PushBack(3);
//	

	List<int>::ConstIterator it = l.Begin();
	while (it != l.End())
	{
		//*it = 1;
		cout << *it << " ";
		it++;
	}
	//list<int>::const_iterator it ;
	//it = l1.begin();
	//while (it != l1.end())
	//{
	//	//*it = 1;
	//	cout << *it << " ";
	//	it++;
	//}
	//cout << endl;
}