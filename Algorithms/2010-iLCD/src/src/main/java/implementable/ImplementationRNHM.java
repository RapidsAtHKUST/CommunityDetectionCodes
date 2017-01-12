package implementable;


import gnu.trove.map.hash.THashMap;
import gnu.trove.set.hash.THashSet;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;

import tools.DebugMode;

import communityDetectionPackage.ILCDMetaAlgorithm;

import TemporalNetworkManipulation.Community;
import TemporalNetworkManipulation.Node;
import TemporalNetworkManipulation.Operations.BirthCommunityOperation;
import TemporalNetworkManipulation.Operations.ContractCommunityOperation;

//RNHM stands for representative nodes and Hub minimization.
public class ImplementationRNHM extends Implementation {

    private int initialComSize;
    private float integrateParameter;
    private float fusionParameter;


    //for speed optimisation
    THashMap<Community, Float> memorizedcohesion = new THashMap<Community, Float>();


    public ImplementationRNHM(int intialComSize, float integrateParameter, float fusionParameter) {
        this.initialComSize = intialComSize;
        this.integrateParameter = integrateParameter;
        this.fusionParameter = fusionParameter;
    }

    @Override
    public boolean GROWTH(Node candidate, Community com) {
        //DebugMode.printDebug("------ ? integrate "+candidate.getName()+" in "+com.getID()+" "+this.getBelongingStrength(candidate, com)+" > "+this.getIntrinsicCohesion(com));

        if (this.getBelongingStrength(candidate, com) >= integrateParameter * this.getIntrinsicCohesion(com)) {
            //OPTIMISATION
            this.memorizedcohesion.remove(com);

            return true;
        }


        return false;
    }

    @Override
    public ArrayList<Community> BIRTH(Node n1, Node n2) {
        ArrayList<Community> newCommunitiesToReturn = new ArrayList<Community>();

        if (this.initialComSize < 3 || this.initialComSize > 4) {
            System.err.println("sorry but, currently, intial communities must have a size of 3 or 4. Contact me if questions.");
            System.exit(-1);
        }

        //OPTIONAL (OPTIMIZATION): we do not check for the creation of a new community if the link is created inside
        //an existing community. The reason is that this link, as it is created inside a community, is very probable
        //and therefore is not an argument to create a new community.

        //getting communities in common
        THashSet<Community> commonComs = new THashSet<Community>(n1.getCommunities());
        commonComs.retainAll(n2.getCommunities());

        if (commonComs.size() == 0) {
            if (this.initialComSize == 3)
                this.birthCase3(newCommunitiesToReturn, commonComs, n1, n2);
            if (this.initialComSize == 4)
                this.birthCase4(newCommunitiesToReturn, commonComs, n1, n2);

        }
        return newCommunitiesToReturn;
    }

    @Override
    public ArrayList<Community> CONTRACTION_DIVISION(Community affectedCom, Node testedNode, ILCDMetaAlgorithm ilcd) {
        //------------------------------
        //no divisions with this version
        //------------------------------


        ArrayList<Community> result = new ArrayList<Community>();
        result.add(affectedCom);

        //if the node has already been removed, no contraction
        if (!testedNode.getCommunities().contains(affectedCom)) {
            return result;
        }


        //compute the intrinsic cohesion of the community without the node
        float adaptedIntrinsicCohesion = this.getIntrinsicCohesion(affectedCom) - this.getRepresentativity(testedNode, affectedCom);

        if (this.getBelongingStrength(testedNode, affectedCom) >= integrateParameter * adaptedIntrinsicCohesion) {
            return result;
        } else {
            //OPTIMISATION
            this.memorizedcohesion.remove(affectedCom);

            //affectedCom.removeNodeFromCommunity(testedNode);
            ilcd.contract(testedNode, affectedCom);
            //Operation op = new ContractCommunityOperation(affectedCom, testedNode);

            //for all neighbors in the same com, check if they must be removed
            for (Node n : testedNode.getNeighborsInCommunity(affectedCom)) {
                this.CONTRACTION_DIVISION(affectedCom, n, ilcd);
            }

            return (result);
        }
    }

    @Override
    public boolean DEATH(Community testedCom) {
        //OPTIMISATION
        this.memorizedcohesion.remove(testedCom);

        return testedCom.getComponents().size() < this.initialComSize;

    }


    @Override
    public ArrayList<Community> FUSION(Community toBeAbsorbed, Community toAbsorb, ILCDMetaAlgorithm ilcd) {


        ArrayList<Community> result = new ArrayList<Community>();
        result.add(toBeAbsorbed);
        result.add(toAbsorb);


        ArrayList<Node> commonNodes = new ArrayList<Node>(toBeAbsorbed.getComponents());
        commonNodes.retainAll(toAbsorb.getComponents());

        float representativityOfCommonNodes = 0;
        for (Node n : commonNodes) {
            representativityOfCommonNodes += this.getRepresentativity(n, toBeAbsorbed);
        }

        //if the fusion must be done
        if (representativityOfCommonNodes > this.getIntrinsicCohesion(toBeAbsorbed) * this.fusionParameter) {

            //OPTIMISATION
            this.memorizedcohesion.remove(toBeAbsorbed);
            this.memorizedcohesion.remove(toAbsorb);

            //remove the younger community
            result.remove(toBeAbsorbed);
            //do the fusion
            ArrayList<Node> mightIntegrate = new ArrayList<Node>(toBeAbsorbed.getComponents());
            mightIntegrate.removeAll(commonNodes);
            for (Node n : mightIntegrate) {
                if (this.GROWTH(n, toAbsorb))
                    ilcd.addNodeToCommunity(n, toAbsorb);
            }
        }

        return result;

    }

    private float getRepresentativity(Node n, Community c) {

        String idReltion = this.getIdRelation(n, c);

        THashSet<Node> neighborsInC = new THashSet<Node>(n.getNeighbors());
        float nbNeighbors = neighborsInC.size();
        neighborsInC.retainAll(c.getComponents());
        float nbNeighborsInC = neighborsInC.size();

        return nbNeighborsInC / nbNeighbors;

    }

    private String getIdRelation(Node n, Community c) {
        return c.getID() + n.getName();
    }

    private float getBelongingStrength(Node n, Community c) {

        THashSet<Node> neighborsInC = new THashSet<Node>(c.getComponents());
        neighborsInC.retainAll(n.getNeighbors());


        //will probably need an optimization for not computing again values already computed
        float belongingStrength = 0;
        if (neighborsInC.size() < 2) {
            return 0;
        } else {
            for (Node neighb : neighborsInC) {
                belongingStrength += this.getRepresentativity(neighb, c);
            }
        }

        return belongingStrength;

    }

    private float getIntrinsicCohesion(Community c) {
        if (this.memorizedcohesion.containsKey(c))
            return this.memorizedcohesion.get(c);

        //will probably need an optimization for not computing again values already computed
        float intrinsicCohesion = 0;
        for (Node component : c.getComponents()) {
            intrinsicCohesion += this.getRepresentativity(component, c);
        }

        this.memorizedcohesion.put(c, intrinsicCohesion);
        return intrinsicCohesion;
    }

    //more efficient
    /*public THashSet<Community> getCommunitiesForWhichIsRepresentative(Node n){
        THashSet<Community> result = new THashSet<Community>();
		for(Community c : n.getCommunities()){
			if(this.getRepresentativity(n, c)>this.getIntrinsicCohesion(c)/(float)c.getComponents().size()){
				result.add(c);
			}
		}
		return result;
	}*/

    private ArrayList<Community> birthCase3(ArrayList<Community> newCommunitiesToReturn, THashSet<Community> commonComs, Node n1, Node n2) {

        //getting neighbors in common
        THashSet<Node> commonNeighbors = new THashSet<Node>(n1.getNeighbors());
        commonNeighbors.retainAll(n2.getNeighbors());

        for (Node candidate : commonNeighbors) {
            //THashSet<Community> commonComsAllNodes= new THashSet<Community>(candidate.getCommunities());
            //commonComsAllNodes.retainAll(commonComs);
            //if and only if these nodes are not already in the same community
            //if(commonComsAllNodes.size()==0){
            //HashSet<Node> initialNodes = new HashSet<Node>();
            //initialNodes.add(n1);
            //initialNodes.add(n2);
            //initialNodes.add(candidate);
            //Operation op = new BirthCommunityOperation();


            Community newCom = new Community();
            newCom.addNodeToCommunity(n1);
            newCom.addNodeToCommunity(n2);
            newCom.addNodeToCommunity(candidate);
            newCommunitiesToReturn.add(newCom);
            //}
        }

        return newCommunitiesToReturn;
    }

    private ArrayList<Community> birthCase4(ArrayList<Community> newCommunitiesToReturn, THashSet<Community> commonComs, Node n1, Node n2) {

        //getting neighbors in common
        ArrayList<Node> commonNeighbors = new ArrayList<Node>(n1.getNeighbors());
        commonNeighbors.retainAll(n2.getNeighbors());

        for (int i = 0; i < commonNeighbors.size(); i++) {
            for (int j = i; j < commonNeighbors.size(); j++) {
                if (commonNeighbors.get(i).getNeighbors().contains(commonNeighbors.get(j))) {
                    Community newCom = new Community();
                    newCom.addNodeToCommunity(n1);
                    newCom.addNodeToCommunity(n2);
                    newCom.addNodeToCommunity(commonNeighbors.get(i));
                    newCom.addNodeToCommunity(commonNeighbors.get(j));

                    newCommunitiesToReturn.add(newCom);
                }
            }
        }


        return newCommunitiesToReturn;

    }
}
