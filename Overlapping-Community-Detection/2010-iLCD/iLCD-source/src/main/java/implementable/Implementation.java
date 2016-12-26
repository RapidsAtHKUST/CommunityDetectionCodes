package implementable;


import gnu.trove.set.hash.THashSet;

import java.util.ArrayList;

import communityDetectionPackage.ILCDMetaAlgorithm;


import TemporalNetworkManipulation.Community;
import TemporalNetworkManipulation.Node;


/**this class is the one you need to implement if you want to propose your own version of iLCD.
 * You can look at the provided implementation (implementationRNHM) to look at how to write it.
 * 
 *
 * @author remy cazabet
 *
 */
public abstract class Implementation {

	/**
	 * 
	 * @param candidate the node tested
	 * @param com the community tested
	 * @return true if the checked node must be added to the community, false otherwise
	 */
	public abstract boolean GROWTH(Node candidate, Community com);
	
	/**
	 * 
	 * @param n1 a node
	 * @param n2 another node, with a link to n2 which have just been created
	 * @return the list of new communities
	 */	
	//new communities are created inside the function, but these communities are not integrated in the "official"/"actual"
	//list of communities. They'll be automatically added by iLCD
	public abstract ArrayList<Community> BIRTH(Node n1, Node n2);
	
	/**
	 * 
	 * @param affectedCom the community affected
	 * @param testedNode the node for which we want to know if it must stay in the community or not
	 * @param ilcdMetaAlgorithm
	 * 
	 * return the list of new communities. Be careful, you're manipulating the real one !
	 * you have to create copies if you want to "try" modifications.
	 * @return list of communities. Only one if no splitting 
	 */
	//in this method, we pass the iLCD Meta-algorithm. This is for a technical reason : the method needs to remove nodes, and the cleanest
	//way I can think of is to call the fonction from iLCDMetaAlgorithm... Should be improved, but probably means changing a lot of things
	public abstract ArrayList<Community> CONTRACTION_DIVISION(Community affectedCom, Node testedNode, ILCDMetaAlgorithm ilcdMetaAlgorithm);
	
	/**
	 * 
	 * @param testedCom the community for which we want to know if it must die or not
	 * @return true if the community must die.
	 */
	public abstract boolean DEATH(Community testedCom);
	
	/**
	 * 
	 * @param c1 the first community
	 * @param c2 the communities with which we want to check if c1 must be merged with
	 * @param ilcdMetaAlgorithm same problem as for CONTRACTION_DIVISION
	 * @return a list of communities : c1 and c2 if no need to merge, c1 or c2, maybe modified, if merged.
	 */
	//
	public abstract ArrayList<Community> FUSION(Community c1, Community c2, ILCDMetaAlgorithm ilcdMetaAlgorithm);
	
	
	//this method is only used for optimisation. If a node is a hub, it is not representative of its communities.
	//if it is not representative, then there is no way it can provoque the fusion of communities or the integration of
	//a node to a community.
	//by default, return all the communities the node belong to
	public THashSet<Community> getCommunitiesForWhichIsRepresentative(Node n) {
		return n.getCommunities();
	}
}
