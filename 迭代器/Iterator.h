#pragma once

struct  InputIteratorTag {};
struct  OutputIteratorTag {};
struct  ForwardIteratorTag : public  InputIteratorTag {};
struct  BidirectionalIteratorTag : public  ForwardIteratorTag {};
struct  RandomAccessIteratorTag : public  BidirectionalIteratorTag {};


template <class Category,class T,class Distance=int,
class Pointer=T*,class Reference=T&>
struct Interator
{
	typedef Category IteratorCategory;
	typedef T ValueType;
	typedef Distance DifferentType;
	typedef Pointer Pointer;
	typedef Reference Reference;
};

template <class Iterator>
struct IteratorTraits
{
	typedef typename Iterator::IteratorCategory IteratorCategory;
	typedef  typename  Iterator::ValueType         ValueType;
	typedef  typename  Iterator::DifferenceType    DifferenceType;
	typedef  typename  Iterator::Pointer            Pointer;
	typedef  typename  Iterator::Reference          Reference;
};

template <class T>
struct IteratorTraits<T*>
{
	typedef  RandomAccessIteratorTag  IteratorCategory;
	typedef  T                             ValueType;
	typedef  ptrdiff_t                     DifferenceType;
	typedef  T  *                          Pointer;
	typedef  T  &                          Reference;
};

template <class T>
struct IteratorTraits<const T*>
{
	typedef  RandomAccessIteratorTag  IteratorCategory;
	typedef  T                             ValueType;
	typedef  ptrdiff_t                     DifferenceType;
	typedef  const  T*                    Pointer;
	typedef  const  T&                    Reference;
};

template <class InputIterator>
inline typename int
_Distance(InputIterator first, InputIterator second, InputIteratorTag)
{
	int n = 0;
	while (first != second)
	{
		n++;
		first++;
	}
	return n;
}

template <class RandomAccessIterator>
inline int
_Distance(RandomAccessIterator first, RandomAccessIterator second, RandomAccessIteratorTag)
{
	return second - first;
}

template <class InputIterator>
inline int
Distance(InputIterator first, InputIterator second)
{
	return _Distance(first,second, IteratorTraits<InputIterator>::IteratorCategory());
}

