/*
 * fileOpts.h
 *
 *  Created on: Oct 17, 2011
 *      Author: Jerry
 */

#ifndef FILEOPTS_H_
#define FILEOPTS_H_


#include <string>
#include <vector>
using namespace std;

bool isFileExist(const char* fileName);
void writeToTxt(string fileName, bool isappend, vector<string>& data);
void extractFileName_FullPath (const string& str,string& shortfile, string& file, string& path);


#endif /* FILEOPTS_H_ */
