



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
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
	
	return cum_hyper_right(x, k2, tm, k1);
	
}







#include "set_parameters.h"

log_fact_table LOG_TABLE;
Parameters paras;
ofstream fileout;



#include "module_collection.h"
#include "dir_weighted_tabdeg.h"

#include "directed_network.h"
#include "louvain_oslomnet_dir.h"
#include "directed_oslomnet_evaluate.h"
#include "oslom_net_global.h"
#include "try_homeless_dir.cpp"

#include "hierarchies.h"




void program_statement(char * b) {
	
	
	cout<<"\n\n\n***************************************************************************************************************************************************"<<endl;

	cout<<"This program implements the OSLO-method for directed networks"<<endl;
	
		
	general_program_statement(b);
	
	
	
}


#include "main_body.cpp"











