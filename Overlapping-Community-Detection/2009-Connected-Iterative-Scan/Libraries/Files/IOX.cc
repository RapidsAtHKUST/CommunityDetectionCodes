#include "IOX.h"

/**
 * @fn bool fline_tr(ifstream* fin, vector <string> fields, string delim)

 * Retrieve a line from fin using delimiters in delim, and trim all strings retrieved.
 *
 *	Removed Empty (after trimmed) spaces.
 * @param fin File stream to read from
 * @param fields Container to put results
 * @param delim Delimiters to use
 * @return if line retrieval was successful
 */

bool fline_tr(ifstream* fin, vector<string>* fields, string delim)
{
  fields->clear();
  vector<string>::iterator p, q;
 
    string full;
    getline(*fin, full);
    
    if (fin->fail() && fin->eof())
      return false;
    
    char *pt = strtok((char*)(full.c_str()), delim.c_str());
    while (pt != NULL){
      fields->push_back(pt);
      pt = strtok(NULL, delim.c_str());
    }
    
  for (p = fields->begin(); p != fields->end(); p++)
    {
      while (trim(*p).compare("") == 0 && p != fields->end())
      {
	q = p;
	fields->erase(q);
      }
      
      if (p == fields->end())
	break;
      
      *p = trim(*p);
    }
  
  return true;
}

/** @fn void split_tr(vector <string>* fields, string delim)
 *
 */
void split_tr(string input, vector < string >* fields, string delim){
  vector < string >::iterator p, q;

  fields->clear();

  char *pt = strtok((char*)(input.c_str()), delim.c_str());
    while (pt != NULL){
      fields->push_back(pt);
      pt = strtok(NULL, delim.c_str());
    }
    
  for (p = fields->begin(); p != fields->end(); p++)
    {
      while (trim(*p).compare("") == 0 && p != fields->end())
      {
	q = p;
	fields->erase(q);
      }
      
      if (p == fields->end())
	break;
      
      *p = trim(*p);
    }
}

/**
 * @fn bool openFile(ifstream* fin, string filename)
 *
 * Open an ifstream with some basic checks
 *
 * @param fin ifstream to use to open file
 * @param filename file to open
 * @return if open successful
 */
bool openFile(ifstream* fin, string filename)
{
  fin->open(filename.c_str());
  if (!(fin->is_open()))
  {
    return false;
  }
  
  return true;
}

/**
 * @fn bool openFileHarsh(ifstream* fin, string filename)
 *
 * Open an ifstream with some basic checks. If the file is not
 *    opened, the program is exited. If error handling is 
 *    desired, use 'openFile'.
 *
 * @param fin ifstream to use to open file
 * @param filename file to open
 */
void openFileHarsh(ifstream* fin, string filename)
{
  fin->open(filename.c_str());
  if (!(fin->is_open()))
  {
    FileError(filename);
    exit(3);
  }
}


void FileError(const string& filename){
  cerr << "Could not open file " << filename << endl;
}
