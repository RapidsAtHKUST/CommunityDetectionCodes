#include "Format.h"

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
void ChangeDelimiter ( const string& filein, const string& fileout, const char& old_delim, const char& new_delim ){
  //Set up input/output
  ofstream fout( fileout.c_str() );
  ifstream fin; openFileHarsh(&fin, filein);
  vector < string > fields, tags;

  string new_dl = "", old_dl = "";
  new_dl += new_delim;
  old_dl += old_delim;

  //Split on the old delimiter
  while (fline_tr(&fin, &fields, old_dl)){
    for (int i = 0; i < fields.size(); i++){
      //Split on the new delimiter
      split_tr(fields[i].c_str(), &tags, new_dl);

      //Reprint using the old delimiter
      for (int j = 0; j < tags.size() - 1; j++){
	fout << tags[j] << old_delim;
      }
      fout << tags[tags.size() - 1];
      
      //Reprint with the new delimiter
      if (i < fields.size() - 1) fout << new_delim;
    }
    
    fout << endl;
  }
  
  //Clean up
  fin.close();
  fout.close();
}

void EncodeNetwork ( const string& filein, const string& fileout, const char& old_delim, const char& new_delim, vector < string >& lookup ){
  ofstream fout( fileout.c_str() );
  ifstream fin; openFileHarsh(&fin, filein);
  vector < string > fields, tags;

  string new_dl = "", old_dl = "";
  new_dl += new_delim;
  old_dl += old_delim;

  map < string, int > ID;
  map < string, int >::iterator it_id;
  int next = 0;

  //Split on the old delimiter
  while (fline_tr(&fin, &fields, old_dl)){
    if ( fields.size() != 3 ) continue;
    
    if ( ( it_id = ID.find(fields[0]) ) == ID.end() ){
      fout << next << new_dl;
      ID.insert(pair < string, int > (fields[0], next++));
    } else {
      fout << it_id->second << new_dl;
    }

    if ( ( it_id = ID.find(fields[1]) ) == ID.end() ){
      fout << next << new_dl;
      ID.insert(pair < string, int > (fields[1], next++));
    } else {
      fout << it_id->second << new_dl;
    }

    fout << fields[2] << endl;
  }

  lookup.clear();
  lookup.resize(next);
  
  for ( it_id = ID.begin(); it_id != ID.end(); it_id++ ){
    lookup[it_id->second] = it_id->first;
  }

  ID.clear();

  //Clean up
  fin.close();
  fout.close();
}


void DecodeCommunities ( const string& filein, const string& fileout, const char& old_delim, const char& new_delim, const vector < string >& lookup ) {
  ofstream fout( fileout.c_str() );
  ifstream fin; openFileHarsh(&fin, filein);
  vector < string > fields, tags;

  string new_dl = "", old_dl = "";
  new_dl += new_delim;
  old_dl += old_delim;

  //Split on the old delimiter
  while (fline_tr(&fin, &fields, old_dl)){    
    int first = str_to < int > ( fields[0] );
    fout << lookup[first];
  
    for ( int i = 1; i < fields.size(); i++ ){
      fout << new_dl << lookup[str_to<int>(fields[i])];
    }
    
    fout << endl;
  }
  //Clean up
  fin.close();
  fout.close();
}
