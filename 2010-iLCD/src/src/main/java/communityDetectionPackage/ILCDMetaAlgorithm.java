package communityDetectionPackage;

import gnu.trove.set.hash.THashSet;
import implementable.Implementation;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;

import tools.DebugMode;

import Outputs.OutputCTNF;
import Outputs.OutputHandlerInterface;
import TemporalNetworkManipulation.Community;
import TemporalNetworkManipulation.Network;
import TemporalNetworkManipulation.Node;
import TemporalNetworkManipulation.Operations.BirthCommunityOperation;
import TemporalNetworkManipulation.Operations.DeathCommunityOperation;
import TemporalNetworkManipulation.Operations.FusionCommunityOperation;
import TemporalNetworkManipulation.Operations.GrowthCommunityOperation;
import TemporalNetworkManipulation.Operations.Operation;
import TemporalNetworkManipulation.Operations.ContractCommunityOperation;
import TemporalNetworkManipulation.Operations.Operation.OperationType;
import TemporalNetworkManipulation.OperationsHandler;

/**
 * the iLCD meta-algorithm.
 * Not completely clean yet.
 * This algorithm is described in my PhD thesis, and an English version is provided on my personal page.
 * Compared to this version, there might be very minor change in this version, in particular for performance
 * or code simplicity reasons.
 * <p>
 * If possible, do not change this class. Please report any question or problem.
 * <p>
 * NB : a parameter, called parameterABUSIVE, has just been added.
 * It's effect is to skip all calls to the implementation caused by nodes which are "hubs", nodes belonging to
 * many communities. This STRONGLY improve performance in networks with large hubs.
 * However, it might change the results, and might even lead to very poor results in the case of networks with
 * VERY LARGE overlap (most nodes belonging to many communities.)
 * it MUST be change in the future.
 *
 * @author remy cazabet
 */
public class ILCDMetaAlgorithm implements OperationsHandler {


    //ArrayList<Community> deadCommunities = new ArrayList<Community>();

    //THashSet<Node> rejectedNodesToCheck = new THashSet<Node>();

    private Implementation usedImplementation;
    //private OutputHandlerInterface usedOutput;
    private Network network;
    private int parameterABUSIVE = 10;

    public ILCDMetaAlgorithm(Implementation implementationToUse, Network network) {
        this.usedImplementation = implementationToUse;
        this.network = network;
    }

    public void handle(Operation currentOperation) {
        HashSet<Community> modifiedComs = new HashSet<Community>();

        switch (currentOperation.op) {
            case ADD:
                //usedOutput.handleNewEdge(currentOperation.getNode1(), currentOperation.getNode2());
                //growing of communities
                if (currentOperation.getNode1().getCommunities().size() < this.parameterABUSIVE) {
                    for (Community c : currentOperation.getNode1().getCommunities()) {
                        //if the other node does not belong to the community, try to add it
                        if (!c.getComponents().contains(currentOperation.getNode2())) {

                            //compute only if the node has more than 2 neighbors in the community. remove noise, and more efficient.
                            if (currentOperation.getNode2().getNeighborsInCommunity(c).size() >= 2) {
                                if (usedImplementation.GROWTH(currentOperation.getNode2(), c)) {
                                    this.addNodeToCommunity(currentOperation.getNode2(), c);
                                    modifiedComs.add(c);//memorize that communities have been modified.
                                }
                            }
                        }
                    }
                }
                if (currentOperation.getNode2().getCommunities().size() < this.parameterABUSIVE) {

                    for (Community c : currentOperation.getNode2().getCommunities()) {
                        //if the other node does not belong to the community, try to add it
                        if (!c.getComponents().contains(currentOperation.getNode1())) {

                            //compute only if the node had more than 2 neighbors in the community. remove noise, and more efficient.
                            if (currentOperation.getNode1().getNeighborsInCommunity(c).size() >= 2) {

                                if (usedImplementation.GROWTH(currentOperation.getNode1(), c)) {
                                    this.addNodeToCommunity(currentOperation.getNode1(), c);
                                    modifiedComs.add(c);//memorize that communities have been modified.
                                }
                            }
                        }
                    }
                }

                //birth of communities
                ArrayList<Community> createdCommunities = usedImplementation.BIRTH(currentOperation.getNode1(), currentOperation.getNode2());
                for (Community c : createdCommunities) {
                    this.addCommunity(c);
                    modifiedComs.add(c);//memorize that communities have been modified.
                    //I'm not checking if the community is not included in any existing one. I think this can improve performance.
                    //This property must be checked in the Birth fonction.
                }

                break;
            case REMOVE:
                //contraction and division of communities
                ArrayList<Community> commonComs = new ArrayList<Community>(currentOperation.getNode1().getCommunities());
                commonComs.retainAll(currentOperation.getNode2().getCommunities());

                //if the removed link is inside a community, check successively for the two nodes if their belonging to the
                //community is affected, which can lead to a split.
                for (Community c : commonComs) {
                    Community tempSavedCom = new Community(c);

                    ArrayList<Community> resultingCommunities = new ArrayList<Community>();

                    ArrayList<Community> CommunitiesAfterFirstPass = usedImplementation.CONTRACTION_DIVISION(c, currentOperation.getNode1(), this);

                    for (Community c2 : CommunitiesAfterFirstPass) {
                        resultingCommunities.addAll(usedImplementation.CONTRACTION_DIVISION(c2, currentOperation.getNode2(), this));
                    }

                    if (resultingCommunities.size() > 1) {
                        this.diviseCommunity(tempSavedCom, resultingCommunities);
                        modifiedComs.addAll(resultingCommunities);//memorize that communities have been modified.

                    } else {
                        if (resultingCommunities.get(0).isDifferent(tempSavedCom)) {
                            //this.contract(tempSavedCom,resultingCommunities.get(0));

                            //for all nodes rejected from the communities, check if they do not belong to a new community
                            //this could be seen as a splitting, but is not handled as one currently
                            ArrayList<Node> rejected = new ArrayList<Node>(tempSavedCom.getComponents());
                            rejected.removeAll(resultingCommunities.get(0).getComponents());

                            //-----------------------------------------------------------
                            //-----------------------------------------------------------
                            for (Node n : rejected) {
                                for (Node n2 : n.getNeighborsInCommunity(resultingCommunities.get(0))) {
                                    ArrayList<Community> createdCommunities2 = usedImplementation.BIRTH(n, n2);
                                    for (Community cc : createdCommunities2) {
                                        this.addCommunity(cc);
                                        modifiedComs.add(cc);//memorize that communities have been modified.
                                        //I'm not checking if the community is not included in any existing one. I think this can improve performance.
                                        //This property must be checked in the Birth fonction.
                                    }
                                }
                            }
                            //-----------------------------------------------------------

                            modifiedComs.addAll(resultingCommunities);//memorize that communities have been modified.

                        }
                    }

                    for (Community c2 : resultingCommunities) {
                        if (usedImplementation.DEATH(c2)) {
                            this.deathOf(c2);
                            modifiedComs.remove(c2);
                        }
                    }
                }
        }

        //----------------------------------------------------
        //End of switch : handling fusions :
        //----------------------------------------------------

        //be careful ! Optimisation not fully checked....


        ArrayList<Community> modifiedComsSorted = new ArrayList<Community>(modifiedComs);

        //sort from youngest to oldest
        Collections.sort(modifiedComsSorted);
        Collections.reverse(modifiedComsSorted);

        for (Community c : modifiedComsSorted) {
            THashSet<Community> communitiesCandidateToFusionWith = new THashSet<Community>();
            THashSet<Community> communitiesCandidateToFusionWithOPTIMISATION = new THashSet<Community>();


            //find the communities with which this community might fusion
            for (Node n : c.getComponents()) {
                if (n.getCommunities().size() < this.parameterABUSIVE) { //OPTIMISATION abusive, this number is too restrictive, should find another solution
                    //optimisation : check only for communities with at least 2 common nodes. Remove noise and faster
                    for (Community ccc : usedImplementation.getCommunitiesForWhichIsRepresentative(n)) {
                        if (communitiesCandidateToFusionWithOPTIMISATION.contains(ccc))
                            communitiesCandidateToFusionWith.add(ccc);
                    }
                    communitiesCandidateToFusionWithOPTIMISATION.addAll(usedImplementation.getCommunitiesForWhichIsRepresentative(n));
                }
            }
            communitiesCandidateToFusionWith.remove(c);

            for (Community cTested : communitiesCandidateToFusionWith) {
                //System.out.println("test fusion "+cTested.getID()+" "+c.getID()+" "+c.getCommonNodes(cTested).size());
                Community tempSavedC = new Community(c); //to be absorbed  //save the previous state
                Community tempSavedCTested = new Community(cTested); //to absorb

                ArrayList<Community> fusionResult = usedImplementation.FUSION(c, cTested, this);

                if (fusionResult.size() == 1) {

					/*ArrayList<Node> rejected = new ArrayList<Node>(tempSavedC.getComponents());
                    rejected.removeAll(fusionResult.get(0).getComponents());*/

                    this.fusion(fusionResult.get(0), c, cTested, tempSavedC, tempSavedCTested);

                    //-------------------------------------------------------------------
                    //-------------------------------------------------------------------

				/*	for(Node n : rejected){
						if(n.getName().equals("dup"))
							System.out.println("=======================");
						for(Node n2 : n.getNeighborsInCommunity(fusionResult.get(0))){
							if(n.getName().equals("dup"))
								System.out.println("======================="+n2.getName());
							ArrayList<Community> createdCommunities2 = usedImplementation.BIRTH(n, n2);
							for(Community cc : createdCommunities2){
								if(n.getName().equals("dup"))
									System.out.println("======================="+cc.getComponents().size());
								this.addCommunity(cc);
								modifiedComs.add(cc);//memorize that communities have been modified.
								//I'm not checking if the community is not included in any existing one. I think this can improve performance.
								//This property must be checked in the Birth fonction.
							}
						}
					}*/
                    //-------------------------------------------------------------------
                    //-------------------------------------------------------------------


                    break;//if the tested community is fusionned with another, no need to try to fusion it to other ones.
                }
            }

        }

    }

    public void addNodeToCommunity(Node node, Community c) {
        Operation op = new GrowthCommunityOperation(c, node);
        network.doOperation(op);
        //c.addNodeToCommunity(node);
        //usedOutput.handleGrowth(node, c);

    }

    private void fusion(Community resultingCommunity, Community c1, Community c2, Community tempC1, Community tempC2) {
        if (resultingCommunity == c1) {
            Operation op = new FusionCommunityOperation(resultingCommunity, c2);
            network.doOperation(op);

            //usedOutput.handleFusion(resultingCommunity,c2,tempC1);
            //this.deathOf(c2);

        } else {
            Operation op = new FusionCommunityOperation(resultingCommunity, c1);
            network.doOperation(op);

            //usedOutput.handleFusion(resultingCommunity,c1,tempC2);
            //this.deathOf(c1);
        }


    }

    private void deathOf(Community c) {
        Operation op = new DeathCommunityOperation(c);
        network.doOperation(op);

        //network.removeCommunity(c);
		/*usedOutput.handleDeath(c);
		for(Node n : c.getComponents()){
			n.removeCommunity(c);
		}*/

    }

    /*private void contract(Community tempSavedCom, Community community) {
        ArrayList<Node> removed = new ArrayList<Node>(tempSavedCom.getComponents());
        removed.removeAll(community.getComponents());
        for(Node n : removed){
            Operation op = new ContractCommunityOperation(tempSavedCom, n);
            network.doOperation(op);
        }
        //usedOutput.handleContraction(tempSavedCom,community);

    }*/
    public void contract(Node n, Community community) {
        //ArrayList<Node> removed = new ArrayList<Node>(tempSavedCom.getComponents());
        //removed.removeAll(community.getComponents());
        //for(Node n : removed){
        Operation op = new ContractCommunityOperation(community, n);
        network.doOperation(op);
        //}
        //usedOutput.handleContraction(tempSavedCom,community);

    }

    private void diviseCommunity(Community tempSavedCom, ArrayList<Community> resultingCommunities) {
        System.err.println("error : currently, the division is not fully supported. Please send me a mail for further information (it can be done easily)");
        System.exit(-1);

    }


    public void addCommunity(Community c) {
        Operation op = new BirthCommunityOperation(c);
        network.doOperation(op);
        //c.setBirthDate(network.getCurrentTime());
        //network.addCommunity(c);
        //usedOutput.handleBirth(c);

    }


}
