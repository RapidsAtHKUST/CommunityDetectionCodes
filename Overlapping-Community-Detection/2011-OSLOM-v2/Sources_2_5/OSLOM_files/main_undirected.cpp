

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *


 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                               *
 *	This program is free software; you can redistribute it and/or modify         *
 *  it under the terms of the GNU General Public License as published by         *
 *  the Free Software Foundation; either version 2 of the License, or            *
 *  (at your option) any later version.                                          *
 *                                                                               *
 *  This program is distributed in the hope that it will be useful,              *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of               *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                *
 *  GNU General Public License for more details.                                 *
 *                                                                               *
 *  You should have received a copy of the GNU General Public License            *
 *  along with this program; if not, write to the Free Software                  *
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA    *
 *                                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                               *
 *  Created by Andrea Lancichinetti on 15/5/09 (email: arg.lanci@gmail.com)      *
 *  Location: ISI foundation, Turin, Italy                                       *
 *                                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
#include "standard_package/standard_include.cpp"


#include "log_table.h"





double log_fact_table::right_cumulative_function(int k1, int k2, int tm, int x) {

	if(x>k1 || x>k2)
		return 0;
	
	
	if(k1*k1<tm)
		return cum_hyper_right(x, k2, tm, k1);
	
	
	// k1 is the degree of the node
	// k2 is the degree of the other node (the bigger)
	// k3 is 2m - k1 - k2
	
	
	int k3=tm -k1;
	
	int H=(k3 - k1 -k2) /2;
	int l1=max(0, -H);
	
	if(x==l1)
		return 1;
	
	
	int mode= max(cast_int( k2 / double(k1+k3) * k1), l1);		// this mode in underestimated anyway
	if(mode>k2)
		mode=k2;
	
	//cout<<"mode: "<<mode<<endl;
	if(x<mode)
		return cum_hyper_right(x, k2, tm, k1);
	
	
	return fast_right_cum_symmetric_eq(k1, k2, H, x, mode, tm);

}






#include "set_parameters.h"

log_fact_table LOG_TABLE;
Parameters paras;



#include "module_collection.h"
#include "undir_weighted_tabdeg.h"



#include "undirected_network.h"
#include "louvain_oslomnet.h"
#include "undirected_oslomnet_evaluate.h"
#include "oslom_net_global.h"
#include "try_homeless_undir.cpp"

#include "hierarchies.h"






void program_statement(char * b) {
	
	
	cout<<"\n\n\n***************************************************************************************************************************************************"<<endl;

	cout<<"This program implements the OSLO-method for undirected networks"<<endl;
	
		
	general_program_statement(b);
	
	
	
}








#include "main_body.cpp"










