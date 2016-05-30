#pragma once
#include <vector>
template <class T>
class Vector
{
public:
	typedef T* Interator;
	typedef T ValueType;
	typedef RandomAccessIteratorTag IteratorCateGory;
	typedef int DifferenceType;
	Vector()
		:_Start(NULL)
		,_Finish(NULL)
		,_EndOfStorge(NULL)
	{}
	Interator Begin()
	{
		return _Start;
	}
	Interator End()
	{
		return _Finish;
	}
	void Insert(Interator pos,const ValueType& x)
	{
		assert(pos>=_Start&&pos<=_Finish);
		bool a = true;
		if (pos == _Finish)
		{
			a = false;
		}
		CheckStorage();
		if (a == false)
		{
			pos = _Finish;
		}
		for (int i = (_Finish - _Start); i >= (pos - _Start); i--)
		{
			_Start[i] = _Start[i-1];
		}
		_Start[pos - _Start] = x;
		_Finish++;
	}
	void PushBack(const ValueType& x)
	{
		if (_Finish != _EndOfStorge)
		{
			*_Finish = x;
			++_Finish;
		}
		else
		{
			Insert(End(), x);
		}
	}
	Interator Erase(Interator pos)
	{
		assert(pos >= _Start&&pos <= _Finish);
		for (int i = pos - _Start; i < _Finish - _Start; i++)
		{
			_Start[i] = _Start[i + 1];
		}
		_Finish--;
		return pos;
	}
	void PopBack()
	{
		Erase(End());
	}
	ValueType& operator[](int size)
	{
		assert(size<(_Finish-_Start));
		return _Start[size];
	}
protected:
	void CheckStorage()
	{
		if (_Finish == _EndOfStorge)
		{
			int Storge = _EndOfStorge - _Start;
			Interator tmp = new ValueType[Storge*2+3];
			for (int i = 0; i < Storge; i++)
			{
				tmp[i] = _Start[i];
			}
			delete _Start;
			_Start = tmp;
			_Finish = _Start + Storge;
			_EndOfStorge = _Start + Storge * 2 + 3;
		}
	}
	Interator _Start;
	Interator _Finish;
	Interator _EndOfStorge;
};

void VectorTest()
{
	Vector<int> v;
	vector<int> v2;
	v.PushBack(1);
	v.PushBack(2);
	v.PushBack(6);
	v.PushBack(3);
	v.PushBack(5);
	v.PushBack(7);
	v.PushBack(8);
	for (int i = 0; i < 8; i++)
	{
		v2.push_back(i+1);
	}

	/*for (int i = 0; i < 7; i++)
	{
		cout << v[i] << " ";
	}*/
	Vector<int>::Interator it = v.Begin();
	vector<int>::iterator it2 = v2.begin();

	while (it2 != v2.end())
	{
		if (*it2 % 2 == 0)
		{
			v2.erase(it2);
		}
		else
		{
			it2++;
		}
	}
}