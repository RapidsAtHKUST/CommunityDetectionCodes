#include "network.hpp"

#include <string>
#include <exception>
#include <memory>

namespace graph {
namespace loading {

struct BadlyFormattedLine : public std :: exception {
	const int line_number;
	const std :: string bad_line;
	BadlyFormattedLine(int _line_number, std :: string _bad_line);
	virtual const char* what() const throw();
	virtual ~ BadlyFormattedLine() throw();
};
	
std :: auto_ptr< graph :: NetworkInterface<graph :: NodeNameIsInt64>  > make_Network_from_edge_list_int64 (const std :: string file_name, const bool directed, const bool weighted) throw(BadlyFormattedLine);
std :: auto_ptr< graph :: NetworkInterface<graph :: NodeNameIsString> > make_Network_from_edge_list_string(const std :: string file_name, const bool directed, const bool weighted) throw(BadlyFormattedLine);

} // namespace loading
} // namespace graph
