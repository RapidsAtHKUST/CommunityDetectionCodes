/*
 *  tools.h
 *
 *  Created by Erwan Le Martelot on 12/01/2012.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef MSCD_TOOLS_H_
#define MSCD_TOOLS_H_

// Includes
#include <vector>
#include <string>
#include <sstream>
#include <cmath>
#include "tokeniser.h"
#include <cstdlib>

namespace toolkit {
	
/// Parse a range of values given as a string using a Matlab-like format
bool ParseRange(const std::string & str, std::vector<double> & ps) {
	std::stringstream ss; ss << str;
	Tokeniser tokeniser(ss);
	tokeniser.AddSkipSymbol("[");
	tokeniser.AddSkipSymbol("]");
	tokeniser.AddSkipSymbol("\n");
	tokeniser.AddSpecialSymbol(":");
	tokeniser.AddSpecialSymbol(",");
	std::string token;
	int state = 0;
	double from, step, to;
	do {
		token = tokeniser.GetNextToken();
		if ((!tokeniser.IsSpecialSymbol(token)) && (atof(std::string(token).append("1").c_str())==0.))
			return false;
		if (token.empty()) continue;
		switch (state) {
			case 0:
				from = atof(token.c_str());
				ps.push_back(from);
				state = 1;
				break;
			case 1:
				if (token == ":")
					state = 2;
				else if (token == ",")
					state = 0;
				else return false;
				break;
			case 2:
				step = atof(token.c_str());
				state = 3;
				break;
			case 3:
				if (token == ":")
					state = 4;
				else return false;
				break;
			case 4:
				to = atof(token.c_str());
				if (from < to) {
					if (step <= 0) return false;
					for (double f=from+step; f<to; f+=step) ps.push_back(f);
					ps.push_back(to);
				}
				else if (from > to) {
					if (step >= 0) return false;
					for (double f=from+step; f>to; f+=step) ps.push_back(f);
					ps.push_back(to);
				}
				else return false;
				state = 5;
				break;
			case 5:
				if (token == ",")
					state = 0;
				else return false;
				break;
			default:
				return false;
		}
		
	}
	while (!token.empty());
	return true;
}

/// Convert vector of T to string
template <class T>
std::string NumList2Str(const std::vector<T> & ps) {
	std::stringstream ss;
	ss << "[";
	for (int i=0; i<ps.size(); ++i) {
		ss << ps[i];
		if (i<ps.size()-1) ss << ",";
	}
	ss << "]";
	return ss.str();
}
	
} // namespace toolkit

#endif
