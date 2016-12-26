#include "../Files/IOX.h"
#include <vector>
#include <map>

using namespace std;

/**
 *@fn void ChangeDelimiter ( const string& filein, const string& fileout, const char& new_delim )
 *
 * Changes the network file from being delimited by a pipe (|) to delimited by the given character.
 * All instances of the given character in the original network are also replaced by a pipe.
 * The result is more of a switch between the pipe character and the new delimiter character.
 *
 * @param filein File that holds the network information
 * @param fileout File to write new format to
 * @param new_delim Delimiter to change to
 */
void ChangeDelimiter ( const string& filein, const string& fileout, const char& old_delim, const char& new_delim ); 

void EncodeNetwork ( const string& filein, const string& fileout, const char& old_delim, const char& new_delim, vector < string >& lookup );

void DecodeCommunities ( const string& filein, const string& fileout, const char& old_delim, const char& new_delim, const vector < string >& lookup );
