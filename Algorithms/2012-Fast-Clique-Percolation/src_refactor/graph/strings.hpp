#ifndef _STRINGS_HPP_
#define _STRINGS_HPP_
#include <cstddef>
namespace strings {
class StrH { // string handle. It just wraps an int that refers to the memory mapped file
	int i;
public:
	explicit StrH(int _i);
	bool operator == (StrH r) const;
	struct hasher;
	int get_underlying_id() const; // Shouldn't really call this too much.
};
struct StrH :: hasher {
	std :: size_t operator() (StrH s)         const;
};
class StringArray {
public:
	virtual const char * operator[] (StrH s) const = 0;
	virtual StrH StringToStringId(const char *s) const = 0;
	virtual ~StringArray();
};
} // namespace strings
#endif
