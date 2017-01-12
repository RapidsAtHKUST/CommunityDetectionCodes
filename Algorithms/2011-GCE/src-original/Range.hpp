#ifndef _RANGE_HPP_
#define _RANGE_HPP_

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
	typedef typeof(c.rbegin()) RBeginType; // I wonder what'll happen for types without an rbegin()? Hopefully won't be a compile-fail, unless we actually use this type. Might need some template magic
};
template <class Container>
class ContainerRange : public IteratorRange< typename ContainerProperties<Container>::BeginType > {
public:
	ContainerRange(Container &c) : IteratorRange< typename ContainerProperties<Container>::BeginType > (c.begin(), c.end()) {}
};
template <class Container>
class ContainerRangeReverse : public IteratorRange< typename ContainerProperties<Container>::RBeginType > {
public:
	ContainerRangeReverse(Container &c) : IteratorRange< typename ContainerProperties<Container>::RBeginType > (c.rbegin(), c.rend()) {}
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
#define ForeachContainerReverse(DECL, CONTAINER_TO_FOREACH)                                       /* break (and continue) work as expected */        \
		for(pair<bool, ContainerRangeReverse< typeof(CONTAINER_TO_FOREACH) > > crazy234identifier(false, CONTAINER_TO_FOREACH); !crazy234identifier.first && !(crazy234identifier.second).empty() ; ({ if(!crazy234identifier.first) (crazy234identifier.second).popFront() ; }) )   \
		if((crazy234identifier.first = true))           \
		for(DECL = (crazy234identifier.second).front() ; crazy234identifier.first ; crazy234identifier.first=false)      




#endif
