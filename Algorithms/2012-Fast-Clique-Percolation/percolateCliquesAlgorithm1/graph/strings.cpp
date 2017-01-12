#include "strings.hpp"
#include <tr1/unordered_set>

namespace strings {
StrH :: StrH(int _i) : i(_i) {}
size_t StrH :: hasher :: operator() (StrH s)         const { return std :: tr1 :: hash<int>()(s.i); }
bool StrH :: operator == (StrH r) const { return this->i == r.i; }
int StrH :: get_underlying_id() const { return this->i; }

StringArray :: ~StringArray() {
}

} // namespace strings
