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

// Models.H
/* A file that includes all header files related to network models. */


#ifndef MODELS_H
#define MODELS_H

#include "NetExtras.H"   
#include "Distributions.H" 
#include "models/SimpleNets.H" 
#include "models/SeedNet.H" 
#include "models/HolmeKim.H" 
#include "models/ErdosRenyi.H" 
#include "models/CommunityNet.H"  // old version currently in lcelib/nets/OldModels.H
#include "models/BBV.H" 
// #include "models/BA.H" // perhaps Jussi will write this one
#include "models/Davidsen.H" 

#endif //~ MODELS_H



/* 
 The network model functions take in a reference to a network 
 of fixed size, whose edges they modify. The type of seed network 
 used for growing network models is specified with the enumerated 
 variables RANDOM (referring to an Erdös-Renyi random network) 
 CLIQUE (fully connected network), CHAIN or RING. 

 Each function also takes in a reference to a random number generator 
 of type RandNumGen (from lcelib/Randgens.H). 

 (This information may or may not change in the future as more
 network models  are included and more features added. It held 
 true at Aug 30 2005.) 
*/
