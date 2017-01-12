#ifndef _RANGE_HPP_
#define _RANGE_HPP_

#include <utility>
#include <string>
#include <istream>
#include <functional>
#include <boost/function.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/utility/result_of.hpp>
#include <tr1/type_traits>
#include <memory>

/* This is a C++ clone of http://www.digitalmars.com/d/2.0/phobos/std_range.html
 * I had assumed such a thing would be in Boost, but I can't find it
 * There is a range library in Boost which seems relevant, it might be working syncing with that a bit more.
 *
 * This great presentation is what originally taught be the idea of ranges. http://lambda-the-ultimate.org/node/3520
 * In particular, I want zip() and chain()
 */

template <class Iterator>
class IteratorRange {
private:
	Iterator current;
	const Iterator end;
	static Iterator dummy[0]; typedef typeof(*dummy[0]) Iterated; // I don't like this dummy thing, but it lets me get the right typeof

public:
	IteratorRange(const Iterator & _begin, const Iterator & _end) : current(_begin), end(_end) {}
	IteratorRange(std::pair<Iterator,Iterator> be) : current(be.first), end(be.second) {}

	bool empty() const { return current == end; }
	Iterated & front() const { return *current; }
	Iterator   frontIterator() const { return  current; }
	void popFront() { ++current; }
};

template <class Container>
class ContainerProperties {
	static Container c;
public:
	typedef typeof(c.begin()) BeginType;
	typedef typeof(*(c.begin())) BeginTypeDerefed;
};
template <class Container>
class ContainerRange : public IteratorRange< typename ContainerProperties<Container>::BeginType > {
public:
	ContainerRange(Container &c) : IteratorRange< typename ContainerProperties<Container>::BeginType > (c.begin(), c.end()) {}
};



template<class Range>
class RangeTypes {
	static Range r;
public:
	typedef typeof(r.front()) ElementType;
};

template <class Range>
class ChainedRange { // Chain Two together
	Range first;
	Range second;
public:
	ChainedRange(const Range &_first, const Range &_second) : first(_first), second(_second) {}
	bool empty() const { return first.empty() && second.empty(); }
	bool firstEmpty() const { return first.empty(); }
	typename RangeTypes<Range>::ElementType front() const { return first.empty() ? second.front() : first.front(); }
	void popFront() { if (first.empty()) second.popFront(); else first.popFront(); }
};

template <class T>
struct Range {
	virtual bool empty() const = 0;
	virtual void popFront() = 0;
	virtual T front() const = 0;
	virtual ~Range() { }
	typedef T value_type; // I think I should remove this, it's confusing with the ::reference type common in many containers.
	typedef T front_type; // The type returned by front()
};

template<class Iterator>
struct RangeContainer
: public Range< typename Iterator::reference >
{
	Iterator _current;
	Iterator _last;
	explicit RangeContainer(Iterator first, Iterator last) : _current(first), _last(last) {
	}
	virtual bool empty() const { return _current == _last; }
	virtual void popFront() { ++_current; }
	virtual typename Iterator::reference front() const { return *_current; }
	virtual ~RangeContainer() {
		_current = _last;
	}
};

namespace amd {
template <typename C>
std::auto_ptr< Range< typename C::iterator::reference > > mk_range(C &c) {
	return std::auto_ptr< Range<typename C::iterator::reference> > (new RangeContainer<typename C::iterator>(c.begin(), c.end()));
}
template <typename C>
std::auto_ptr< Range< typename C::const_iterator::reference > > mk_range(const C &c) {
	return std::auto_ptr< Range<typename C::const_iterator::reference> > (new RangeContainer<typename C::const_iterator>(c.begin(), c.end())); 
}
template <typename C>
std::auto_ptr< Range< typename C::const_iterator::reference > > mk_range_c(const C &c) {
	return std::auto_ptr< Range<typename C::const_iterator::reference> > (new RangeContainer<typename C::const_iterator>(c.begin(), c.end())); 
}
template <typename C>
std::auto_ptr< Range< typename C::reverse_iterator::reference > > mk_reverse_range(C &c) {
	return std::auto_ptr< Range<typename C::reverse_iterator::reference> > (new RangeContainer<typename C::reverse_iterator>(c.rbegin(), c.rend()));
}
template <typename C>
std::auto_ptr< Range< typename C::const_reverse_iterator::reference > > mk_reverse_range(const C &c) {
	return std::auto_ptr< Range<typename C::const_reverse_iterator::reference> > (new RangeContainer<typename C::const_reverse_iterator>(c.rbegin(), c.rend())); 
}


class RangeOverStream : public Range<std::string> {
	std::istream &istr;
	const char * delims;
	bool isEmpty;
	std::string currentLine;
	void nextLine();
public:
	RangeOverStream(std::istream &_istr, const char * _delims);
	bool empty() const;
	std::string front() const;
	void popFront();
};
std::auto_ptr< Range<std::string> > rangeOverStream(std::istream &_istr, const char * _delims = "\n");

template<class T, class F>
class RangeMapper : public Range<typename boost::result_of<F(typename T::value_type)>::type> { // T is a range. F is a function which is applied to members of T. The type of this RangeMapper is the result of F applied to elements of T
	T mapped;
	typedef T source_range_type;
	typedef typename source_range_type::value_type source_value_type;
	F f;
public:
	typedef typename boost::result_of<F(typename T::value_type)>::type value_type;
	RangeMapper(const T & mapped_, F f_) : mapped(mapped_), f(f_) {}
	bool empty() const { return mapped.empty(); }
	void popFront() { mapped.popFront(); }
	value_type front() const {
		return f(mapped.front());
	}
};
template<class T, class F>
RangeMapper<T, F> make_mapper_range(const T& m, F f) {
	RangeMapper<T, F> x(m, f);
	return x;
}
template<class T, class F>
RangeMapper<T, F> map_range(const F &f, const T& m) {
	return make_mapper_range(m,f);
}

template<class R, class P>
class RangeFilter : public Range<typename R::value_type> {
	R r;
	P p;
public:
	typedef typename R::value_type value_type;
	RangeFilter(const R& _r, const P& _p) : r(_r), p(_p) {
		if(!r.empty() && !p(r.front())) {
			this->popFront();
		}
	}
	virtual bool empty() const { return r.empty(); }
	virtual void popFront() {
		do {
			r.popFront();
		} while(!r.empty() && !p(r.front()));
	}
	virtual value_type front() const {
		return r.front();
	}
};
template<class R, class P>
RangeFilter<R,P> make_filter_range(const R& r, const P& p) {
	RangeFilter<R, P> x(r,p);
	return x;
}
template<class R, class P>
RangeFilter<R,P> filter(const P& p, const R& r) {
	return make_filter_range(r,p);
}
template<class R>
int range_length(R r) {
	int l = 0;
	while(!r.empty()) {
		++l;
		r.popFront();
	}
	return l;
}

template<class I>
class RangeEnum : public Range<I> {
	I a;
public:
	RangeEnum(I a_) : a(a_) {}
	virtual bool empty() const { return false; }
	virtual I front() const { return a; }
	virtual void popFront() { ++a; }
};

template<class I>
RangeEnum<I> make_counter_range(I first) {
	RangeEnum<I> x(first);
	return x;
}

template<class L, class R>
class RangeZipper : public Range< std::pair<
		 				typename std::tr1::remove_reference<typename L::value_type>::type
						,typename std::tr1::remove_reference<typename R::value_type>::type
			> > {
	L l;
	R r;
public:
	typedef std::pair<
		 typename std::tr1::remove_reference<typename L::value_type>::type
		,typename std::tr1::remove_reference<typename R::value_type>::type
	> value_type;
	RangeZipper(const L &l_, const R &r_) : l(l_), r(r_) {}
	virtual bool empty() const { return l.empty() || r.empty(); }
	virtual value_type front() const { return make_pair(l.front(), r.front()); }
	virtual void popFront() { l.popFront(); r.popFront(); }
};

template<class L, class R>
RangeZipper<L,R> zip(const L &l, const R &r) {
	RangeZipper<L,R> x(l,r);
	return x;
}

template<class L, class R>
class RangeChain : public Range< typename L::value_type > {
	L l;
	R r;
public:
	typedef typename L::value_type value_type;
	RangeChain(const L &l_, const R &r_) : l(l_), r(r_) {}
	virtual bool empty() const { return l.empty() && r.empty(); }
	virtual value_type front() const {
		if(!l.empty())
			return l.front();
		else
			return r.front();
	}
	virtual void popFront() {
		if(!l.empty())
			l.popFront();
		else
			r.popFront();
	}
};
template<class L, class R>
RangeChain<L,R> chain(const L &l, const R &r) {
	RangeChain<L,R> x(l,r);
	return x;
}

}


#define macro_cat(a, b) a ## b
#define macro_xcat(x,y) macro_cat(x,y)
#define crazy234identifier  macro_xcat(arbitrary_Foreach_lsdfjk,__LINE__)

#define Foreach(DECL, RANGE_TO_FOREACH)                                       \
		for(bool crazy234identifier = false; !crazy234identifier && !(RANGE_TO_FOREACH).empty() ; ({ if(!crazy234identifier) (RANGE_TO_FOREACH).popFront() ; }) )  /* break (and continue) work as expected */        \
		if((crazy234identifier = true))           \
		for(DECL = (RANGE_TO_FOREACH).front() ; crazy234identifier ; crazy234identifier=false)      
#define ForeachContainer(DECL, CONTAINER_TO_FOREACH)                                       /* break (and continue) work as expected */        \
		for(pair<bool, ContainerRange< typeof(CONTAINER_TO_FOREACH) > > crazy234identifier(false, CONTAINER_TO_FOREACH); !crazy234identifier.first && !(crazy234identifier.second).empty() ; ({ if(!crazy234identifier.first) (crazy234identifier.second).popFront() ; }) )   \
		if((crazy234identifier.first = true))           \
		for(DECL = (crazy234identifier.second).front() ; crazy234identifier.first ; crazy234identifier.first=false)      

template <class Type>
class auto_ptrWithPairedBool : public std::auto_ptr<Type> {
public:
	auto_ptrWithPairedBool(Type *p) : std::auto_ptr<Type> (p), interrupted(false) {}
	bool interrupted;
};

#define forEach(DECL, RANGE_TO_FOREACH)                                       \
		for(auto_ptrWithPairedBool< typeof(*(RANGE_TO_FOREACH)) > crazy234identifier ((RANGE_TO_FOREACH).release()); !crazy234identifier.interrupted && !(crazy234identifier)->empty() ; ({ if(!crazy234identifier.interrupted) crazy234identifier->popFront() ; }) )  /* break (and continue) work as expected */        \
		if((crazy234identifier.interrupted = true))           \
		for(DECL = (crazy234identifier)->front() ; crazy234identifier.interrupted ; crazy234identifier.interrupted=false)

#endif // _RANGE_HPP_
