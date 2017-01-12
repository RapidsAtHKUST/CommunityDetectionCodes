/*
lcelib, A high performance library for complex networks research
Copyright (C) 2011  Aalto University

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

/** Functions to compute the location of bin edges positioned either linearly or logarithmically on the x-axis and bin the data.

1. Use double * spacing(char * bin_type, double bin_start_edge,double bin_end_edge, int number_of_bins) method to get array of edges.

2. Use double * giveBinCenters(char * bin_type,double * bin_edges, int number_of_bins) method to get array of bin centers.

3. Use double * meansc(...) to get array of bin means.

Options:
"lin" = spacing is done linearly
"log" = spacing is done logarithmically
"forward" = counts with same value than edge are put in a bin where edge is lower bound
"backward" = counts with same value than edge are put in a bin where edge is upper bound


For a usage example, see /Examples/bintest.cc.

@author Antti Niinikoski
@date 9.11.2005

*/
#ifndef BINDATA_H
#define BINDATA_H
#include <math.h>
#include <iostream>
#include <fstream>

using namespace std;
  
/** Generates a linearly spaced vector. 
    @param start First edge
    @param end Last edge
    @param number_of_bins Number of bins = number of edges
*/
  
double * linspace(double start,double end,int number_of_bins) {
  double * values = new double[number_of_bins+1];
  double interval = (end-start)/number_of_bins;
  
  for(int i=0; i<number_of_bins+1; i++){ 
    values[i]= start+i*interval;
  }

  return values;

}

  /** Generates a logarithmically spaced vector. 
      @param start First edge
      @param end Last edge
      @param number_of_bins Number of bins = number of edges -1
*/

double * logspace(double start,double end, int number_of_bins){ 
  double * values = new double[number_of_bins+1];
  double root = (double) 1/number_of_bins;
  values[0] =start;
  values[1] = pow( (pow(start,number_of_bins-1)*end) , root);
  
  for(int i=2; i<number_of_bins+1;i++) {
    values[i] = values[i-1]*values[i-1]/values[i-2];
  }
  return values;
}

  /** Generates spacing. 
      @param bin_type logarithmic or linear:  "log" or "lin"
      @param bin_start_edge 
  */

double * spacing(char * bin_type, double bin_start_edge,double bin_end_edge, int number_of_bins){
  double * bin_edges = new double[number_of_bins+1];
  
  if (strcmp(bin_type, "lin") == 0 || strcmp(bin_type,"linear") == 0){
    bin_edges = linspace(bin_start_edge, bin_end_edge, number_of_bins);
    }
  else if (strcmp(bin_type, "log") == 0 || strcmp(bin_type,"logarithmic") == 0){
    if (bin_start_edge <= 0){
      cerr <<"You have specified a non-positive bin_start_edge." << endl;
    }
    bin_edges = logspace(bin_start_edge, bin_end_edge, number_of_bins);
  }
  else {
    cerr << "You haven't specified a bin_type or it's wrong. It has to be log or lin."<<endl;
  }
  return bin_edges;
}


double * giveBinCenters(double * bin_edges, int number_of_bins){
  double * bin_centers = new double[number_of_bins];
  for(int i=0; i<(number_of_bins);i++) {
    bin_centers[i]=(bin_edges[i]+bin_edges[i+1])/2;
  }
  return bin_centers;
}

/** Calculates bin centers. Note "log" type! It is made differently than in JP´s matlab-code (bindata.m). To make it things similar use always "lin" or function giveBinCenters(double * bin_edges,int number_of_bins).
    @param bin_type Logaritmic or linear: "lin" or "log"
    @param bin_edges Edges
    @param number_of_bins Number of bins
*/

double * giveBinCenters(char * bin_type,double * bin_edges, int number_of_bins){
  double * bin_centers = new double[number_of_bins];
  if(strcmp(bin_type,"lin")==0){
    return giveBinCenters(bin_edges,number_of_bins);
  }
  if(strcmp(bin_type,"log")==0){
    for(int i = 0; i<number_of_bins;i++) {
      bin_centers[i]=bin_edges[i]+sqrt(bin_edges[i+1]/bin_edges[i]);
    } 
  }   
  return bin_centers;
}

/** Histogram count: counts the number of values in vector x that fall between the elements in the edges vector (which must contain monotonically non-decreasing values). 
    @param count_type "forward" or "backward"
    @param bin_edges Edges
    @param x Vector x
    @param number_of_bins Number of bins = number of edges - 1
    @param number_of_xvalues Size of vector x
*/

double * histc(char * count_type,double * bin_edges,double * x, int number_of_bins, int number_of_xvalues){
  double * count=new double[number_of_bins];
  int lost = 0;

  // Initializing

  for(int i=0;i < number_of_bins;i++) { 
    count[i]=0;
  }
  
  //Hist counting
  if(strcmp(count_type,"backward")==0) {
    for(int j=0; j < number_of_xvalues; j++){
      if(x[j]<=bin_edges[0] || x[j]>bin_edges[number_of_bins]){
	lost++;
      }
      else{
	for(int i=0; i < number_of_bins;i++){
	  if(bin_edges[i]<x[j] && bin_edges[i+1]>=x[j]){
	    count[i]=count[i]+1;
	    break;
	  }
	}
      }
    }
  }
  else if(strcmp(count_type,"forward")==0) {
    for(int j=0; j < number_of_xvalues; j++){
      if(x[j]>=bin_edges[number_of_bins] || x[j]<bin_edges[0]){
	lost++;
      }
      else{
	for(int i=0;i < number_of_bins;i++){
	  if(bin_edges[i]<=x[j] && bin_edges[i+1]>x[j]){
	    count[i]=count[i]+1;
	    break;
	  }
	}
      }
    }
  }
  else {
    cerr << "Check count_type." << endl;
  }

  cout << "Histc losted " << lost << " values." << endl;
  return count;
}

  /** Calculates differences between adjacent elements of parameter bin_edges. 
      @param bin_edges Edges
      @param number_of_bins Number of bins = number of edges - 1
  */
    
double * diff(double * bin_edges, int number_of_bins){
  double * differences = new double[number_of_bins]; 
  
  for(int i = 0; i < number_of_bins;i++) {
    differences[i]=bin_edges[i+1]-bin_edges[i];
  }

  return differences;
}

  /** Counts bin means. (If y was not given as input,) we are doing density estimation for x

 Here n_i is the number of observations in bin i and dx_i is the width
 of bin i. In general, dx_1 != dx_2 != ... != dx_n, i.e. bin width is NOT constant.
 Observations: [n_1, n_2, ..., n_n]
 Unnormalized density: [n_1/dx_1, n_2/dx_2, ..., n_n/dx_n]
 Normalization constant: n_1/dx_1 * dx_1 + ... + n_n/dx_n * dx_n = sum(n_i)
 Normalized density = "Unnormalized density" / "Normalization constant"
   = [n_1/dx_1, n_2/dx_2, ..., n_n/dx_n] / sum(n_i)

*/

double * meansc(char * count_type, double * bin_edges, double * x, int number_of_bins, int number_of_xvalues){
    
  double * n = new double[number_of_bins]; // Number of observations in bin n[i]
  double * diff_bin_edges = new double[number_of_bins];
  double * unnorm_dens = new double[number_of_bins];
  double norm_const = 0.0;
  double * norm_dens = new double[number_of_bins];
  
  n = histc(count_type,bin_edges,x,number_of_bins,number_of_xvalues);
  diff_bin_edges = diff(bin_edges,number_of_bins);

  for(int i=0;i<number_of_bins;i++){
    unnorm_dens[i] = n[i]/diff_bin_edges[i];
    norm_const = norm_const + n[i];
  }

  // Normalization
  for(int i=0; i < number_of_bins;i++){
    norm_dens[i] = unnorm_dens[i] / norm_const;
  }
  
  return norm_dens;
  
}

/** Function calculates an average of the y-values for which the corresponding x-values are limited by the bin edges. The average is returned.
*/

double * meansc(char * count_type, double *bin_edges, double * x, double * y, int number_of_bins, int number_of_values){
  double * bin_means = new double[number_of_bins];
  double * counterY = new double[number_of_bins];
  double * sum = new double[number_of_bins];
  int lost = 0;

  //Initializing of tables
  
  for(int i=0;i < number_of_bins;i++){
    counterY[i] = 0;
    sum[i] = 0;
  }
  
  //Counting
  if(strcmp(count_type,"backward")==0) {
    for(int j=0; j < number_of_values; j++){
      for(int i=0; i < number_of_bins;i++){
	if(x[j]<=bin_edges[0]){
	  lost++;
	  break;
	}
	else if(bin_edges[i]<x[j] && bin_edges[i+1]>=x[j]){
	  counterY[i] = counterY[i] + 1;
	  sum[i] = sum[i]+y[j];
	  break;
	}
      }
    }
  }
  else if(strcmp(count_type,"forward")==0) {
    for(int j=0; j < number_of_values; j++){
      for(int i=0;i < number_of_bins;i++){
	if(x[j]>=bin_edges[number_of_bins]){
	  lost++;
	  break;
	}
	else if(bin_edges[i]<=x[j] && bin_edges[i+1]>x[j]){
	  counterY[i] = counterY[i] + 1;
	  sum[i] = sum[i]+y[j];
	  break;
	}
      }
    }
  }
  else
    cerr << "Count type is specified wrongly. " << endl;
  
  cerr << lost << " values were losted." << endl;
  
  // Averaging
  for(int i=0;i < number_of_bins;i++){
    if(counterY[i] != 0)
      bin_means[i] = sum[i]/counterY[i];
    else
      bin_means[i] = 0;
  }
  return bin_means;

}

/** Returns number of elements in a file.
    @param filename Name of the file
*/
int countElements(char * filename){
  ifstream in(filename);
  int count = 0;
  double readedValue;
  
  while(in >> readedValue){
    count++;
  }
  return count;
}

/** Reads values from a file.
    @param filename Name of the file
    @param number_of_elements Number of elements that are read from file. 
*/

double * readFile(char * filename){
  ifstream in(filename);
  double readedValue;
  double * values = new double[countElements(filename)];
  int j = 0;
  
  while(in >> readedValue){
    values[j] = readedValue;
    j++;
  }
  return values;
}

/** Writes data in a file. Separates data with delimiter. 
    @param filename Name of the file.
    @param data Data that is mentioned to be writed.
    @number_of_elements Number of elements.
    @delimiter Delimiter
*/

void writeData(char * filename, double * data, int number_of_elements){
  ofstream out(filename);
  for(int i = 0; i<number_of_elements; i++){
    out << data[i] << " ";
  }

}

#endif //~ BINDATA_H
