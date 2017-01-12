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

#ifndef CLIQUE2_H
#define CLIQUE2_H

//#include <list>
// #include <algorithm> 

class clique {

protected:

  std::vector<size_t> nodes;

public: 



  clique() {
    nodes.resize(0);
  }


  const size_t size() const {
    return nodes.size();
  }

  const size_t at(const size_t i) const {
    return nodes.at(i);
  }



  virtual void copyClique(const clique & old_clique) {
    const size_t newSize = old_clique.size();
    nodes.resize(newSize);
    for (size_t i = 0; i < newSize; i++) 
      nodes.at(i) = old_clique.at(i);
  }

  /*  
  clique( clique & old_clique) {
    copyClique(old_clique);
  }

  clique( const clique & old_clique) {
    copyClique(old_clique);
  }
  */

  void replaceNodes( const std::vector<size_t> & unsorted_nodes ) {
    nodes = unsorted_nodes;
    std::sort( nodes.begin(), nodes.end() ); // sort nodes
  }


  clique(const std::vector<size_t> & unsorted_nodes) {
    replaceNodes(unsorted_nodes);
  }




  
  bool operator== ( const clique & clique2 ) const {
    const size_t cliqueSize = size();
    if ( cliqueSize == clique2.size() ) {
      bool allEqual = true;
      for ( size_t i = 0; allEqual && i < cliqueSize; i++) {
	if ( nodes.at(i) != clique2.at(i) ) allEqual = false;
      }
      return allEqual;
    }
    else return false;
  }

  bool operator!= ( const clique & clique2 ) const {
    const size_t cliqueSize = size();
    if ( cliqueSize == clique2.size() ) {
      bool anyDifferent = false;
      for ( size_t i = 0; !anyDifferent && i < cliqueSize; i++) {
	if ( nodes.at(i) != clique2.at(i) ) anyDifferent = true;
      }
      return anyDifferent;
    }
    else return true;
  }


  bool operator< ( const clique & clique2) const
  {
    const size_t cliqueSize = size();
    if ( cliqueSize == clique2.size() ) {
      bool definetelySmaller = false, know_answer = false;
      for (size_t i = 0; i < cliqueSize && !know_answer; ++i) {
	if ( (*this).at(i) < clique2.at(i) ) {
	  definetelySmaller = true;
	  know_answer = true;
	}
	else if ( (*this).at(i) > clique2.at(i) ) {
	  definetelySmaller = false;
	  know_answer = true;
	}
      }
      return definetelySmaller;
    }
    else {
      std::cerr << "comparing cliques of different sizes!\n";
      return false;
    }
  }
  


  ~clique() {
    //     nodes.clear();
  }



};
#endif




#ifndef W_CLIQUE_H
#define W_CLIQUE_H

// #include <math.h>
#include "../Nets.H"   
//#include "clique2.H"

//#ifndef NET_TYPE
//#define NET_TYPE
typedef  SymmNet<float> NetType; 
//#endif


class weighedClique: public clique {

private: 
  float weight;
  std::vector<float> linkWs;
  float (weighedClique::*weightFunc)(const std::vector<float> & ) const; // define function pointer

  void getLinks(const  NetType & net ) {
    linkWs.clear();
    for (size_t i = 0; i < size()-1; ++i) {
      for (size_t j = i+1; j < size(); ++j) {
	if (  net(nodes.at(i))[nodes.at(j)]  > 0 ) {
	  linkWs.push_back( net(nodes.at(i))[nodes.at(j)] );
	  // std::cerr <<  nodes.at(i) << "\t" << nodes.at(j) << "\t" <<  net(nodes.at(i))[nodes.at(j)]  << "\n";
	}
	else {
	  std::cerr << "link weight was zero or less!\n";
	  std::cerr <<  nodes.at(i) << "\t" << nodes.at(j) << "\t" <<  net(nodes.at(i))[nodes.at(j)]  << "\n";
	}
      }
    }
    if ( linkWs.size() != size()*(size()-1)/2 ) { 
      std::cerr << "obtained link number does not match clique link number!\n";
      std::cerr << linkWs.size() << "\n";
    }
  }



  float minWeight(const std::vector<float> & weights) const {
    const size_t size = weights.size();
    float minWeight = weights.at(0);
    for (size_t i = 1; i < size; ++i) 
      if ( weights.at(i) < minWeight ) minWeight = weights.at(i);

    return minWeight;
  }

  float maxWeight(const std::vector<float> & weights) const {
    const size_t size = weights.size();
    float minWeight = weights.at(0);
    for (size_t i = 1; i < size; ++i) 
      if ( weights.at(i) > minWeight ) minWeight = weights.at(i);

    return minWeight;
  }


  float intensity(const std::vector<float> & weights) const {

    const size_t size = weights.size();
    float minWeight = weights.at(0);
    for (size_t i = 1; i < size; ++i) 
      minWeight = minWeight*weights.at(i);
    minWeight = pow(minWeight, 1.0/size);

    return minWeight;
  }



public:





  weighedClique() { // uses the default constructor
    weight = 0;
    linkWs.resize(0);
    weightFunc = &weighedClique::minWeight;
  }


  weighedClique( const std::vector<size_t> & unsorted_nodes, const NetType & net, size_t funcType = 0 )  : clique( unsorted_nodes ) { // copy nodes using parent constructor
    // nodes should now be ok, loop over them and build list of link weights as well as the weight
    getLinks( net );
    //    std::cerr << "funcType: " << funcType << "\n";
    switch( funcType ) {
    case 0:
      weightFunc = &weighedClique::minWeight; 
      break;
    case 1: 
      weightFunc = &weighedClique::maxWeight; 
      break;
    case 2:
      weightFunc = &weighedClique::intensity;
      break;
    default:
      weightFunc = &weighedClique::minWeight; 
    }
    weight = (*this.*weightFunc)(linkWs); // update weight
  }


  void copyClique(const weighedClique & old_clique) {
    std::vector<size_t> tmp_nodes;
    for (size_t i = 0; i < old_clique.size(); ++i) tmp_nodes.push_back( old_clique.at(i) ); // 
    clique::replaceNodes( tmp_nodes );
    weight = old_clique.getWeight();
  }

  void replaceNodes(  const std::vector<size_t> & unsorted_nodes, const NetType & net ) {

    clique::replaceNodes(unsorted_nodes);
    getLinks( net );
    weight = (*this.*weightFunc)(linkWs); // update weight
  }

  float getWeight() const {
    return weight;
  }

};

#endif




#ifndef LINK_H
#define LINK_H

class Link: public clique {

private:
 float weight;

public:

  Link() {
    std::vector<size_t>  u_nodes;
    u_nodes.push_back(0);
    u_nodes.push_back(1);
    replaceNodes( u_nodes ); 
    weight = 1;
  }

  Link(const size_t startNode, const size_t endNode, const float linkWeight = 1) {
    std::vector<size_t> u_nodes;
    u_nodes.push_back(startNode);
    u_nodes.push_back(endNode);
    replaceNodes( u_nodes ); 
    weight = linkWeight;
  }
  
  float getWeight() {
    return weight;
  }

};
#endif
