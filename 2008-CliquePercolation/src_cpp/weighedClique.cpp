/*
scp, The sequential clique percolation algorithm.
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

#include "weighedClique.h"


clique::clique()
{
    nodes.resize(0);
}


std::ostream& operator <<(std::ostream &os,const clique &obj)
{
  //os<<obj.strVal;
      return os;
}

const size_t clique::size() const
{
    return nodes.size();
}

const size_t clique::at(const size_t i) const
{
    return nodes.at(i);
}

/*void clique::addNodes(const std::vector<size_t> & unsorted_nodes)
{
    for (std::vector<size_t>::const_iterator iter = unsorted_nodes.begin(); iter != unsorted_nodes.end(); iter++)
        nodes.push_back(*iter);
    std::sort( nodes.begin(), nodes.end() ); // sort nodes
}*/



void clique::copyClique(const clique & old_clique)
{
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

void clique::replaceNodes( const std::vector<size_t> & unsorted_nodes )
{
    nodes = unsorted_nodes;
    std::sort( nodes.begin(), nodes.end() ); // sort nodes
}


void clique::printClique() const
{
    for (std::vector<size_t>::const_iterator i = nodes.begin(); i != nodes.end(); i++)
        std::cout << *i << " ";
    std::cout << std::endl;
}


clique::clique(const std::vector<size_t> & unsorted_nodes)
{
    replaceNodes(unsorted_nodes);
}





bool clique::operator== ( const clique & clique2 ) const
{
    const size_t cliqueSize = size();
    if ( cliqueSize == clique2.size() )
    {
        bool allEqual = true;
        for ( size_t i = 0; allEqual && i < cliqueSize; i++)
        {
            if ( nodes.at(i) != clique2.at(i) ) allEqual = false;
        }
        return allEqual;
    }
    else return false;
}

bool clique::operator!= ( const clique & clique2 ) const   // tän vois korvata operator==:n avulla
{
    const size_t cliqueSize = size();
    if ( cliqueSize == clique2.size() )
    {
        bool anyDifferent = false;
        for ( size_t i = 0; !anyDifferent && i < cliqueSize; i++)
        {
            if ( nodes.at(i) != clique2.at(i) ) anyDifferent = true;
        }
        return anyDifferent;
    }
    else return true;
}


bool clique::operator< ( const clique & clique2) const
{
    const size_t cliqueSize = size();
    if ( cliqueSize == clique2.size() )
    {
        bool definetelySmaller = false, know_answer = false;
        for (size_t i = 0; i < cliqueSize && !know_answer; ++i)
        {
            if ( (*this).at(i) < clique2.at(i) )
            {
                definetelySmaller = true;
                know_answer = true;
            }
            else if ( (*this).at(i) > clique2.at(i) )
            {
                definetelySmaller = false;
                know_answer = true;
            }
        }
        return definetelySmaller;
    }
    else
    {
        std::cerr << "comparing cliques of different sizes!\n";
        return false;
    }
}



//gets link weight for every link in the clique
/*void weighedClique::getLinks(const  NetType & net )
{
    linkWs.clear();
    for (size_t i = 0; i < size()-1; ++i)
    {
        for (size_t j = i+1; j < size(); ++j)
        {
            if (  net(nodes.at(i))[nodes.at(j)]  > 0 )
            {
                linkWs.push_back( net(nodes.at(i))[nodes.at(j)] );
                // std::cerr <<  nodes.at(i) << "\t" << nodes.at(j) << "\t" <<  net(nodes.at(i))[nodes.at(j)]  << "\n";
            }
            else
            {
                std::cerr << "link weight was zero or less!\n";
                std::cerr <<  nodes.at(i) << "\t" << nodes.at(j) << "\t" <<  net(nodes.at(i))[nodes.at(j)]  << "\n";
            }
        }
    }
    if ( linkWs.size() != size()*(size()-1)/2 )
    {
        std::cerr << "obtained link number does not match clique link number!\n";
        std::cerr << linkWs.size() << "\n";
    }
}*/


void weighedClique::addNodes(const std::vector<size_t> & unsorted_nodes, const  NetType & net)
{
    for (std::vector<size_t>::const_iterator iter = unsorted_nodes.begin(); iter != unsorted_nodes.end(); iter++)
        nodes.push_back(*iter);
    std::sort( nodes.begin(), nodes.end() ); // sort nodes
    weight = (this->*weightFunc)(net); // update weight
}



float weighedClique::minWeight(const NetType & net) const
{
    float min = FLT_MAX;
    for (std::vector<size_t>::const_iterator i = nodes.begin(); i != nodes.end(); i++)
    {
        std::vector<size_t>::const_iterator j = i;
        for (j++; j != nodes.end(); j++)
            if (net[*i][*j] < min)
                min = net[*i][*j];
    }
    return min;
}

/*float weighedClique::maxWeight() const
{
    const size_t size = weights.size();
    float minWeight = weights.at(0);
    for (size_t i = 1; i < size; ++i)
        if ( weights.at(i) > minWeight ) minWeight = weights.at(i);

    return minWeight;
}*/


float weighedClique::intensity(const NetType & net) const
{
    float intensity = 1;
    for (std::vector<size_t>::const_iterator i = nodes.begin(); i != nodes.end(); i++)
    {
        std::vector<size_t>::const_iterator j = i;
        for (j++; j != nodes.end(); j++)
            intensity *= net[*i][*j];
    }
    intensity = pow(intensity, 2.0/(nodes.size() * (nodes.size() - 1)));
    return intensity;
}







weighedClique::weighedClique()   // uses the default constructor
{
    weight = 0;
    weightFunc = &weighedClique::minWeight;
}


weighedClique::weighedClique( const std::vector<size_t> & unsorted_nodes, const NetType & net, size_t funcType )  : clique( unsorted_nodes )   // copy nodes using parent constructor
{
    // nodes should now be ok, loop over them and build list of link weights as well as the weight
    //getLinks( net );
    //    std::cerr << "funcType: " << funcType << "\n";
    switch ( funcType )
    {
    case 0:
        weightFunc = &weighedClique::minWeight;
        break;
    case 2:
//        weightFunc = &weighedClique::maxWeight;
        break;
    case 1:
        weightFunc = &weighedClique::intensity;
        break;
    default:
        weightFunc = &weighedClique::minWeight;
    }
    weight = (this->*weightFunc)(net); // update weight
}


void weighedClique::copyClique(const weighedClique & old_clique)
{
    std::vector<size_t> tmp_nodes;
    for (size_t i = 0; i < old_clique.size(); ++i) tmp_nodes.push_back( old_clique.at(i) ); //
    clique::replaceNodes( tmp_nodes );
    weight = old_clique.getWeight();
}

void weighedClique::replaceNodes(  const std::vector<size_t> & unsorted_nodes, const NetType & net )
{

    clique::replaceNodes(unsorted_nodes);
    //getLinks( net );
    weight = (this->*weightFunc)(net); // update weight
}

void weighedClique::printWeightedClique(std::ostream & file) const
{
    for (std::vector<size_t>::const_iterator i = nodes.begin(); i != nodes.end(); i++)
        file << *i << " ";
    file << "weight: " << getWeight() << std::endl;
}
