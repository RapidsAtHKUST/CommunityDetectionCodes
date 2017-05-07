// Includes
#include "tokeniser.h"
#include <sstream>
#include <algorithm>

namespace toolkit {

void Tokeniser::Reset() {
	this->stream.seekg(0);
	this->stream.clear();
	this->buffer = "";
}

void Tokeniser::AddSkipSymbol(const std::string & symbol) {
	if (find(this->skip_symbols.begin(),this->skip_symbols.end(),symbol) == this->skip_symbols.end())
		this->skip_symbols.push_back(symbol);
}

void Tokeniser::RemoveSkipSymbol(const std::string & symbol) {
	std::vector<std::string>::iterator itor;
	if ((itor = find(this->skip_symbols.begin(),this->skip_symbols.end(),symbol)) != this->skip_symbols.end())
		this->skip_symbols.erase(itor);
}
		
void Tokeniser::AddSpecialSymbol(const std::string & symbol) {
	if (find(this->special_symbols.begin(),this->special_symbols.end(),symbol) == this->special_symbols.end())
		this->special_symbols.push_back(symbol);
}

void Tokeniser::RemoveSpecialSymbol(const std::string & symbol) {
	std::vector<std::string>::iterator itor;
	if ((itor = find(this->special_symbols.begin(),this->special_symbols.end(),symbol)) != this->special_symbols.end())
		this->special_symbols.erase(itor);
}

void Tokeniser::AddCommentLineSymbol(const std::string & symbol) {
	if (find(this->comment_line_symbols.begin(),this->comment_line_symbols.end(),symbol) == this->comment_line_symbols.end())
		this->comment_line_symbols.push_back(symbol);
}

void Tokeniser::AddCommentBlockSymbol(const std::pair<std::string,std::string> & symbols) {
	if (find(this->comment_block_symbols.begin(),this->comment_block_symbols.end(),symbols) == this->comment_block_symbols.end())
		this->comment_block_symbols.push_back(symbols);
}

void Tokeniser::AddStringSymbol(const std::string & symbol) {
	if (find(this->string_symbols.begin(),this->string_symbols.end(),symbol) == this->string_symbols.end())
		this->string_symbols.push_back(symbol);
}

const std::string Tokeniser::GetNextToken(bool eof_exception, bool delete_token) {
	std::string token;
	bool read_comment = false;
	// Look for a token as long as EOF is not reached
	while (token.empty() && (!this->buffer.empty() || !this->stream.eof())) {
		// If no data in buffer, fill it in if possible
		if (this->buffer.empty()) {
			std::getline(this->stream,this->buffer);
			this->buffer += '\n';
			++this->line;
		}
		// If the buffer has data, process it
		if (!this->buffer.empty()) {
			// Length of a potential token
			unsigned int length = 0, sym_length;
			// Check and remove skip characters
			while (	(length < this->buffer.size()) &&
					(this->isSkipSymbol(this->buffer,length,sym_length)) )
				length += sym_length;
			this->buffer = this->buffer.substr(length,this->buffer.size()-length);
			length = 0;
			// Read
			bool read = true, read_string = false;
			while ((length < this->buffer.size()) && read) {
				// If a comment block is being read
				if (read_comment) {
					// If end of comment is reached
					if (this->isCommentBlockEndSymbol(this->buffer,length,sym_length)) {
						// Remove the comment symbol from the buffer
						this->buffer = this->buffer.substr(sym_length,this->buffer.size()-sym_length);
						// End of comment mode
						read_comment = false;
						// Exit the loop
						read = false;
					}
					// Otherwise remove the current symbol
					else this->buffer = this->buffer.substr(1,this->buffer.size()-1);
				}
				// If a string delimiter is reached
				else if (this->isStringSymbol(this->buffer,length,sym_length)) {
					if (read_string)
						read = false;
					else
						read_string = true;
					length += sym_length;
				}
				// If a string is being read
				else if (read_string)
					// Move to the next symbol
					++length;
				else {
					// If a comment line is reached
					if (this->isCommentLineSymbol(this->buffer,length,sym_length)) {
						// Remove the comment from the buffer (less the carriage return)
						this->buffer = this->buffer.substr(0,length) + '\n';
						// Exit the loop
						read = false;
					}
					// If a comment block is reached
					else if (this->isCommentBlockBeginSymbol(this->buffer,length,sym_length)) {
						// Set comment mode
						read_comment = true;
						// Exit the loop
						read = false;
					}
					// If a special character is reached
					else if (this->isSpecialSymbol(this->buffer,length,sym_length)) {
						// If no token was found before, take the symbol
						if (length == 0) length = sym_length;
						// Exit the loop
						read = false;
					}
					// If a skip character is reached
					else if (this->isSkipSymbol(this->buffer,length,sym_length)) {
						// If a token has been found before, exit the loop
						if (length > 0)	read = false;
						// Otherwise syntax error (should not happen)
						else {
							std::stringstream ss; ss << this->line;
							std::cerr << this->name << " : Syntax error @ line " << ss.str() << " : Skip character found where expecting a token (this case should never happen)" << std::endl;
							return "";
						}
					}
					// Else move to the next symbol
					else ++length;
				}
			}
			// Token found
			if (length > 0) {
				// Store token
				token = this->buffer.substr(0,length);
				// Remove processed data from buffer is required
				if (delete_token)
					this->buffer = this->buffer.substr(length,this->buffer.size()-length);
			}
		}
	}
	// If an exception is required in case of EOF with no token, throw it
	if (eof_exception && this->stream.eof() && token.empty()) {
		std::stringstream ss; ss << this->line;
		std::cerr << this->name << " : Syntax error @ line " << ss.str() << " : EOF reached when expecting a token" << std::endl;
		return "";
	}
	// Retrun read token
	return token;
}

void Tokeniser::AssertNextToken(const std::string & word) {
	std::string token = this->GetNextToken(true);
	if (token != word) {
		std::stringstream ss; ss << this->line;
		std::cerr << this->name << " : Syntax error @ line " << ss.str() << " : \"" << token << "\" found where expecting \"" << word << "\"" << std::endl;
	}
}

const bool Tokeniser::IsSkipSymbol(const std::string & token) const {
	return std::find(this->skip_symbols.begin(),this->skip_symbols.end(),token) != this->skip_symbols.end();
}

const bool Tokeniser::IsSpecialSymbol(const std::string & token) const {
	return std::find(this->special_symbols.begin(),this->special_symbols.end(),token) != this->special_symbols.end();
}

const bool Tokeniser::IsCommentLineSymbol(const std::string & token) const {
	return std::find(this->comment_line_symbols.begin(),this->comment_line_symbols.end(),token) != this->comment_line_symbols.end();
}

const bool Tokeniser::IsCommentBlockBeginSymbol(const std::string & token) const {
	for (unsigned int i=0; i<this->comment_block_symbols.size(); ++i)
		if (this->comment_block_symbols[i].first == token)
			return true;
	return false;
}

const bool Tokeniser::IsCommentBlockEndSymbol(const std::string & token) const {
	for (unsigned int i=0; i<this->comment_block_symbols.size(); ++i)
		if (this->comment_block_symbols[i].second == token)
			return true;
	return false;
}

const bool Tokeniser::IsStringSymbol(const std::string & token) const {
	return std::find(this->string_symbols.begin(),this->string_symbols.end(),token) != this->string_symbols.end();
}

const bool Tokeniser::IsWord(const std::string & token) const {
	return (!this->IsSkipSymbol(token)				&&
			!this->IsSpecialSymbol(token)			&&
			!this->IsCommentLineSymbol(token)		&&
			!this->IsCommentBlockBeginSymbol(token)	&&
			!this->IsCommentBlockEndSymbol(token)	&&
			!this->IsStringSymbol(token)			&&
			!this->IsString(token));
}

const bool Tokeniser::IsString(const std::string & token) const {
	for (unsigned int i=0; i<this->string_symbols.size(); ++i) {
		if (token.size() >= 2*this->string_symbols[i].size()) {
			if ((token.substr(0,this->string_symbols[i].size()) == this->string_symbols[i]) &&
				(token.substr(token.size()-this->string_symbols[i].size(),this->string_symbols[i].size()) == this->string_symbols[i]))
				return true;
		}
	}
	return false;
}

const bool Tokeniser::ContainsNoSymbol(const std::string & token) const {
	unsigned int tmp;
	for (unsigned int i=0; i<token.size(); ++i) {
		if ((this->isSpecialSymbol(token,i,tmp)) ||
			 this->isStringSymbol(token,i,tmp) ||
			 this->isSkipSymbol(token,i,tmp) ||
			 this->isCommentLineSymbol(token,i,tmp) ||
			 this->isCommentBlockBeginSymbol(token,i,tmp) ||
			 this->isCommentBlockEndSymbol(token,i,tmp))
			return false;
	}
	return true;
}

const std::string Tokeniser::GetTrimmedString(const std::string & token) const {
	for (unsigned int i=0; i<this->string_symbols.size(); ++i) {
		if (token.size() >= 2*this->string_symbols[i].size()) {
			if ((token.substr(0,this->string_symbols[i].size()) == this->string_symbols[i]) &&
				(token.substr(token.size()-this->string_symbols[i].size(),this->string_symbols[i].size()) == this->string_symbols[i]))
				return token.substr(this->string_symbols[i].size(),token.size()-2*this->string_symbols[i].size());
		}
	}
	return token;
}

const bool Tokeniser::isSkipSymbol(
	const std::string & str,
	const unsigned int pos,
	unsigned int & sym_length
) const {
	for (unsigned int i=0; i<this->skip_symbols.size(); ++i) {
		unsigned int size = this->skip_symbols[i].size();
		if (	(pos + size <= str.size()) &&
				(str.substr(pos,size) == this->skip_symbols[i])	) {
			sym_length = size;
			return true;
		}
	}
	return false;
}

const bool Tokeniser::isSpecialSymbol(
	const std::string & str,
	const unsigned int pos,
	unsigned int & sym_length
) const {
	for (unsigned int i=0; i<this->special_symbols.size(); ++i) {
		unsigned int size = this->special_symbols[i].size();
		if (	(pos + size <= str.size()) &&
				(str.substr(pos,size) == this->special_symbols[i])	) {
			sym_length = size;
			return true;
		}
	}
	return false;
}

const bool Tokeniser::isCommentLineSymbol(
	const std::string & str,
	const unsigned int pos,
	unsigned int & sym_length
) const {
	for (unsigned int i=0; i<this->comment_line_symbols.size(); ++i) {
		unsigned int size = this->comment_line_symbols[i].size();
		if (	(pos + size <= str.size()) &&
				(str.substr(pos,size) == this->comment_line_symbols[i])	) {
			sym_length = size;
			return true;
		}
	}
	return false;
}

const bool Tokeniser::isCommentBlockBeginSymbol(
	const std::string & str,
	const unsigned int pos,
	unsigned int & sym_length
) const {
	for (unsigned int i=0; i<this->comment_block_symbols.size(); ++i) {
		unsigned int size = this->comment_block_symbols[i].first.size();
		if (	(pos + size <= str.size()) &&
				(str.substr(pos,size) == this->comment_block_symbols[i].first)	) {
			sym_length = size;
			return true;
		}
	}
	return false;
}

const bool Tokeniser::isCommentBlockEndSymbol(
	const std::string & str,
	const unsigned int pos,
	unsigned int & sym_length
) const {
	for (unsigned int i=0; i<this->comment_block_symbols.size(); ++i) {
		unsigned int size = this->comment_block_symbols[i].second.size();
		if (	(pos + size <= str.size()) &&
				(str.substr(pos,size) == this->comment_block_symbols[i].second)	) {
			sym_length = size;
			return true;
		}
	}
	return false;
}

const bool Tokeniser::isStringSymbol(
	const std::string & str,
	const unsigned int pos,
	unsigned int & sym_length
) const {
	for (unsigned int i=0; i<this->string_symbols.size(); ++i) {
		unsigned int size = this->string_symbols[i].size();
		if (	(pos + size <= str.size()) &&
				(str.substr(pos,size) == this->string_symbols[i])	) {
			sym_length = size;
			return true;
		}
	}
	return false;
}

} // namespace toolkit
