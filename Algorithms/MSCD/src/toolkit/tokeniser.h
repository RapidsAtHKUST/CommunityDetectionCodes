/*
 *  tokeniser.h
 *  MSCD
 *
 *  Created by Erwan Le Martelot on 28/11/2011.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef MSCD_TOKENISER_H_
#define MSCD_TOKENISER_H_

// Includes
#include <string>
#include <iostream>
#include <iomanip>
#include <vector>
#include <utility>

namespace toolkit {

/**
 *  This class defines a tokeniser
 */
class Tokeniser {

	public:
		
		/// Constructor
		Tokeniser(std::istream & stream, const std::string & name = "")
		: stream(stream), name(name), buffer(""), line(0) {}
		
		/// Return the name associated with the tokeniser
		const std::string & GetName() const { return this->name; }
		
		/// Return the current line
		const unsigned int GetLine() const { return this->line; }
		
		/// Reset the tokeniser
		void Reset();
		
		/// Add a skip symbol
		void AddSkipSymbol(const std::string &);
		
		/// Remove a skip symbol
		void RemoveSkipSymbol(const std::string &);
		
		/// Add a special symbol
		void AddSpecialSymbol(const std::string &);
		
		/// Remove a special symbol
		void RemoveSpecialSymbol(const std::string &);
		
		/// Add a comment line symbol
		void AddCommentLineSymbol(const std::string &);
		
		/// Add a comment block symbol
		void AddCommentBlockSymbol(const std::pair<std::string,std::string> &);
		
		/// Add a string delimiter symbol
		void AddStringSymbol(const std::string &);
		
		/// Returns the next token (remove it from buffer is required, true by default)
		const std::string GetNextToken(bool = false, bool = true);
		
		/// Returns the next token but don't remove it from buffer
		const std::string PreviewNextToken(bool eof_exc) { return this->GetNextToken(eof_exc,false); }
		
		/// Throw an exception if the next word is not the one expected
		void AssertNextToken(const std::string &);

		/// True is the given token is a skip symbol
		const bool IsSkipSymbol(const std::string &) const;
		
		/// True is the given token is a special symbol
		const bool IsSpecialSymbol(const std::string &) const;
		
		/// True is the given token is a comment line symbol
		const bool IsCommentLineSymbol(const std::string &) const;
		
		/// True is the given token is a comment block begin symbol
		const bool IsCommentBlockBeginSymbol(const std::string &) const;
		
		/// True is the given token is a comment block end symbol
		const bool IsCommentBlockEndSymbol(const std::string &) const;
		
		/// True is the given token is a string symbol
		const bool IsStringSymbol(const std::string &) const;
		
		/// True if the given token is a word
		const bool IsWord(const std::string &) const;
		
		/// True if the given token is a string
		const bool IsString(const std::string &) const;

		// True if the given token does not contain any used symbol
		const bool ContainsNoSymbol(const std::string &) const;

		/// Returns the token trimmed of its string delimiters
		const std::string GetTrimmedString(const std::string &) const;

	protected:
		
		/// Input stream
		std::istream & stream;

		/// Name
		std::string name;
		
		/// Buffer
		std::string buffer;
		
		/// Line
		unsigned int line;
		
		/// Skip symbols
		std::vector<std::string> skip_symbols;
		
		/// Special symbols
		std::vector<std::string> special_symbols;
		
		/// Comment line string
		std::vector<std::string> comment_line_symbols;
		
		/// Comment block string
		std::vector< std::pair<std::string,std::string> > comment_block_symbols;
		
		/// String delimiter symbols
		std::vector<std::string> string_symbols;
		
		/// True is the given string at the given location is a skip symbol
		const bool isSkipSymbol(
			const std::string &,
			const unsigned int,
			unsigned int &
		) const;
		
		/// True is the given string at the given location is a special symbol
		const bool isSpecialSymbol(
			const std::string &,
			const unsigned int,
			unsigned int &
		) const;
		
		/// True is the given string at the given location is a comment line symbol
		const bool isCommentLineSymbol(
			const std::string &,
			const unsigned int,
			unsigned int &
		) const;
		
		/// True is the given string at the given location is a comment block begin symbol
		const bool isCommentBlockBeginSymbol(
			const std::string &,
			const unsigned int,
			unsigned int &
		) const;
		
		/// True is the given string at the given location is a comment block end symbol
		const bool isCommentBlockEndSymbol(
			const std::string &,
			const unsigned int,
			unsigned int &
		) const;
		
		/// True is the given string at the given location is a string symbol
		const bool isStringSymbol(
			const std::string &,
			const unsigned int,
			unsigned int &
		) const;

};

} // namespace toolkit

#endif
