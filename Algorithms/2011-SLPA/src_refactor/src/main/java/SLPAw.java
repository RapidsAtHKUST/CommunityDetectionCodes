//
// Source code recreated from a .class file by IntelliJ IDEA
// (powered by Fernflower decompiler)
//

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.List;
import java.util.Random;
import java.util.Map.Entry;

public class SLPAw {
    boolean isRELESE = false;
    Hashtable<String, Long> ht_name_ID;
    Random rndGlobal;
    public Net net;
    public String netName;
    public String fileName_net;
    public Boolean isUseLargestComp = Boolean.valueOf(false);
    boolean isSymmetrize = false;
    boolean isWeighted = true;
    ArrayList<Double> THRS;
    ArrayList<Integer> THRCS;
    Boolean isSyn = Boolean.valueOf(false);
    int maxT;
    int maxRun;
    int WeigthVersion = 3;
    int thr_commsize_min = 2;
    int thr_commsize_max = -1;
    public ArrayList<ArrayList<Long>> cpm_good = new ArrayList();
    public ArrayList<ArrayList<Long>> cpm_tooLarge = new ArrayList();
    public boolean isEmbeddedSLPAw = false;
    public int embslpaw_WeigthVersion = 1;
    public double embslpaw_loopcn = 1.0D;
    Hashtable<Long, Long> singleDegNode_Hub_Table = new Hashtable();
    Hashtable<Long, ArrayList<Long>> Hub_singleDegNodeList_Table = new Hashtable();
    Hashtable<Long, ArrayList<Integer>> nodes_comIDs_Table = new Hashtable();
    String outputDir;
    boolean isWriteOvfile = false;
    int writeNODECOMFlag = 0;
    boolean isOhis1 = false;
    boolean isOhis2 = false;
    boolean isOMem1 = false;
    double EC = -1.0D;
    static final Comparator<Entry> ComHistogramdec = new Comparator<Entry>() {
        public int compare(Entry e1, Entry e2) {
            Integer i1 = (Integer) e1.getValue();
            Integer i2 = (Integer) e2.getValue();
            return i2.compareTo(i1);
        }
    };
    static final Comparator<Entry> ComHistogramdec_IntDlb = new Comparator<Entry>() {
        public int compare(Entry e1, Entry e2) {
            Double i1 = (Double) e1.getValue();
            Double i2 = (Double) e2.getValue();
            return i2.compareTo(i1);
        }
    };

    public SLPAw(String fileName_net, ArrayList<Double> THRS, int maxRun, int maxT, String outputDir, boolean isUseLargestComp) {
        this.netName = fileOpts.extractFileName_FullPath(fileName_net);
        this.fileName_net = fileName_net;
        String networkPath = "";
        this.net = new Net(networkPath, this.netName);
        this.THRS = THRS;
        this.maxRun = maxRun;
        this.maxT = maxT;
        this.outputDir = outputDir;
        this.isUseLargestComp = Boolean.valueOf(isUseLargestComp);
    }

    public void printNodesInfo_Directed() {
        for (int i = 0; i < this.net.NODES.size(); ++i) {
            NODE v = (NODE) this.net.NODES.get(i);
            System.out.println("ID=" + v.ID);
            System.out.print("\tin[" + v.numNbs_In + "]::");

            int k;
            for (k = 0; k < v.numNbs_In; ++k) {
                System.out.print(v.nbList_In.get(k) + ",");
            }

            System.out.println("");
            System.out.print("\tout[" + v.numNbs_Out + "]::");

            for (k = 0; k < v.numNbs_Out; ++k) {
                System.out.print(v.nbList_Out.get(k) + ",");
            }

            System.out.println("");
        }

    }

    public void printSingleDeg_NodesInfo() {
        ArrayList keys = new ArrayList(this.singleDegNode_Hub_Table.keySet());
        Collections.sort(keys);

        int i;
        for (i = 0; i < keys.size(); ++i) {
            System.out.println("ID=" + keys.get(i) + "->" + this.singleDegNode_Hub_Table.get(keys.get(i)));
        }

        keys.clear();
        keys = new ArrayList(this.Hub_singleDegNodeList_Table.keySet());

        for (i = 0; i < keys.size(); ++i) {
            Long hubID = (Long) keys.get(i);
            System.out.println("hubID=" + hubID + " " + this.Hub_singleDegNodeList_Table.get(hubID));
        }

    }

    public int isSingleDegNode_directed(NODE v) {
        return v.nbList_In.size() == 1 && v.nbList_Out.size() == 0 ? 1 : (v.nbList_In.size() == 0 && v.nbList_Out.size() == 1 ? 2 : (v.nbList_In.size() == 1 && v.nbList_Out.size() == 1 && v.nbList_In.get(0) == v.nbList_Out.get(0) ? 3 : 0));
    }

    public void identify_singleDegNode_directed(Net net) {
        this.singleDegNode_Hub_Table.clear();
        this.Hub_singleDegNodeList_Table.clear();

        for (int i = 0; i < net.NODES.size(); ++i) {
            NODE v = (NODE) net.NODES.get(i);
            int flag = this.isSingleDegNode_directed(v);
            if (flag > 0) {
                long hubID = -1L;
                if (flag == 1) {
                    hubID = ((Long) v.nbList_In.get(0)).longValue();
                }

                if (flag == 2) {
                    hubID = ((Long) v.nbList_Out.get(0)).longValue();
                }

                if (flag == 3) {
                    hubID = ((Long) v.nbList_In.get(0)).longValue();
                }

                if (this.isSingleDegNode_directed((NODE) net.NODESTABLE.get(Long.valueOf(hubID))) == 0) {
                    this.singleDegNode_Hub_Table.put(Long.valueOf(v.ID), Long.valueOf(hubID));
                    if (!this.Hub_singleDegNodeList_Table.containsKey(Long.valueOf(hubID))) {
                        ArrayList oneList = new ArrayList();
                        this.Hub_singleDegNodeList_Table.put(Long.valueOf(hubID), oneList);
                    }

                    ((ArrayList) this.Hub_singleDegNodeList_Table.get(Long.valueOf(hubID))).add(Long.valueOf(v.ID));
                }
            }
        }

    }

    public void start() {
        this.net.isRELESE = this.isRELESE;
        this.net.readNetwork_EdgesList(this.fileName_net, this.isUseLargestComp.booleanValue(), this.isSymmetrize, this.ht_name_ID, this.isWeighted);
        if (!this.isEmbeddedSLPAw) {
            System.out.println("Info: Number of nodes=" + this.net.N + ", Number of edges=" + this.net.EdgesTable.size());
        }

        if (this.net.N < 12 && !this.isRELESE) {
            this.printNodesInfo_Directed();
        }

        this.net.EdgesTable.clear();
        this.identify_singleDegNode_directed(this.net);
        this.pre_initial_THRCS();

        for (int run = 1; run <= this.maxRun; ++run) {
            if (!this.isEmbeddedSLPAw) {
                System.out.println("Info: " + this.netName + " run=" + run + "......");
            }

            this.initWQueue();
            if (!this.isSyn.booleanValue()) {
                if (!this.isEmbeddedSLPAw) {
                    System.out.println("Info: Start label spreading.......");
                }

                this.SLPAw_asyn();
            }

            this.net.applyEvolutionCutoff(this.maxT, this.EC);
            String i;
            if (this.isOMem1) {
                i = this.outputDir + "//" + "SLPAw_" + this.netName + "_run" + run + ".L1.mem.txt";
                this.net.write2Txt_WQ_nameS(i, 0);
            }

            this.post_createWQHistogram_MapEntryList();
            if (!this.isEmbeddedSLPAw && this.isOhis1) {
                if (this.thr_commsize_max > this.thr_commsize_min) {
                    i = this.outputDir + "//" + "SLPAw_" + this.netName + "_run" + run + "_v" + this.WeigthVersion + "_minC" + this.thr_commsize_min + "_maxC" + this.thr_commsize_max + "_ev" + this.embslpaw_WeigthVersion + "_T" + this.maxT + ".icpm";
                } else if (this.thr_commsize_min > 2) {
                    i = this.outputDir + "//" + "SLPAw_" + this.netName + "_run" + run + "_v" + this.WeigthVersion + "_minC" + this.thr_commsize_min + "_T" + this.maxT + ".icpm";
                } else {
                    i = this.outputDir + "//" + "SLPAw_" + this.netName + "_run" + run + "_v" + this.WeigthVersion + "_T" + this.maxT + ".icpm";
                }

                this.write2txt_histInfo_nameS(i, 1);
            }

            for (int var7 = 0; var7 < this.THRCS.size(); ++var7) {
                int thrc = ((Integer) this.THRCS.get(var7)).intValue();
                double thrp = ((Double) this.THRS.get(var7)).doubleValue();
                if (!this.isRELESE) {
                    System.out.println("-----------thrp=" + thrp + "-------------------");
                }

                String fileName;
                if (this.thr_commsize_max > this.thr_commsize_min) {
                    fileName = this.outputDir + "//" + "SLPAw_" + this.netName + "_run" + run + "_r" + thrp + "_v" + this.WeigthVersion + "_minC" + this.thr_commsize_min + "_maxC" + this.thr_commsize_max + "_ev" + this.embslpaw_WeigthVersion + "_T" + this.maxT + ".icpm";
                } else if (this.thr_commsize_min > 2) {
                    fileName = this.outputDir + "//" + "SLPAw_" + this.netName + "_run" + run + "_r" + thrp + "_v" + this.WeigthVersion + "_minC" + this.thr_commsize_min + "_T" + this.maxT + ".icpm";
                } else {
                    fileName = this.outputDir + "//" + "SLPAw_" + this.netName + "_run" + run + "_r" + thrp + "_v" + this.WeigthVersion + "_T" + this.maxT + ".icpm";
                }

                if (!this.isEmbeddedSLPAw && this.isOhis2) {
                    this.copyHist2Histr();
                }

                if (!this.isEmbeddedSLPAw) {
                    System.out.println("Info: Start post-processing.......");
                }

                this.post_threshold_createCPM(thrc, fileName, thrp, !this.isEmbeddedSLPAw);
                if (!this.isEmbeddedSLPAw && this.isOhis2) {
                    this.write2txt_histInfo_nameS(fileName, 2);
                }
            }
        }

    }

    public void copyHist2Histr() {
        for (int i = 0; i < this.net.N; ++i) {
            NODE v = (NODE) this.net.NODES.get(i);
            v.WQHistgram_r.clear();
            Iterator it = v.WQHistgram.keySet().iterator();

            while (it.hasNext()) {
                Long label = (Long) it.next();
                v.WQHistgram_r.put(label, (Integer) v.WQHistgram.get(label));
            }
        }

    }

    public void identify_NodeCommTable_fromCPM(ArrayList<NODE> NODES, ArrayList<ArrayList<Long>> cpm, boolean isCheckEveryNode, boolean isPrintOv, boolean isWriteOv, int writeNodeCOMFlag, String fileName) {
        this.nodes_comIDs_Table.clear();

        int cn;
        int keys;
        for (int data = 0; data < NODES.size(); ++data) {
            Long oneLine = Long.valueOf(((NODE) NODES.get(data)).ID);
            cn = 0;

            for (keys = 0; keys < cpm.size(); ++keys) {
                if (((ArrayList) cpm.get(keys)).contains(oneLine)) {
                    ++cn;
                    if (!this.nodes_comIDs_Table.containsKey(oneLine)) {
                        ArrayList i = new ArrayList();
                        this.nodes_comIDs_Table.put(oneLine, i);
                    }

                    ((ArrayList) this.nodes_comIDs_Table.get(oneLine)).add(Integer.valueOf(keys));
                }
            }

            if (isCheckEveryNode && cn < 1) {
                System.err.println("ERROR:: Node " + oneLine + " does not belong to ANY community.");
                System.exit(1);
            }
        }

        ArrayList var16;
        StringBuffer var17;
        if (writeNodeCOMFlag > 0) {
            var16 = new ArrayList();
            var17 = new StringBuffer();
            ArrayList var18 = new ArrayList(this.nodes_comIDs_Table.keySet());

            for (keys = 0; keys < var18.size(); ++keys) {
                Long var20 = (Long) var18.get(keys);
                ArrayList ID = (ArrayList) this.nodes_comIDs_Table.get(var20);

                for (int comInds = 0; comInds < ID.size(); ++comInds) {
                    var17.setLength(0);
                    if (writeNodeCOMFlag == 1) {
                        var17.append(((NODE) this.net.NODESTABLE.get(var20)).name);
                        var17.append(" ");
                        var17.append(ID.get(comInds));
                    } else if (writeNodeCOMFlag == 2) {
                        var17.append(ID.get(comInds));
                        var17.append(" ");
                        var17.append(((NODE) this.net.NODESTABLE.get(var20)).name);
                    }

                    var16.add(var17.toString());
                }
            }

            if (writeNodeCOMFlag == 1) {
                fileOpts.writeToTxt(fileName + ".node-com.txt", false, var16);
            } else if (writeNodeCOMFlag == 2) {
                fileOpts.writeToTxt(fileName + ".com-node.txt", false, var16);
            }
        }

        if (isPrintOv || isWriteOv) {
            var16 = new ArrayList();
            var17 = new StringBuffer();
            cn = 0;
            ArrayList var19 = new ArrayList(this.nodes_comIDs_Table.keySet());

            for (int var21 = 0; var21 < var19.size(); ++var21) {
                Long var22 = (Long) var19.get(var21);
                ArrayList var23 = (ArrayList) this.nodes_comIDs_Table.get(var22);
                if (var23.size() > 1 && !this.singleDegNode_Hub_Table.containsKey(var22)) {
                    ++cn;
                    if (isPrintOv) {
                        System.out.println("ov node ID=" + var22 + " belongs to " + var23);
                    }

                    if (isWriteOv) {
                        var17.setLength(0);
                        var17.append(((NODE) this.net.NODESTABLE.get(var22)).name);
                        var17.append(" ->");

                        for (int j = 0; j < var23.size(); ++j) {
                            var17.append(var23.get(j));
                            var17.append(" ");
                        }

                        var16.add(var17.toString());
                    }
                }
            }

            if (isWriteOv) {
                fileOpts.writeToTxt(fileName + ".ovnodes.txt", false, var16);
            }

            if (!this.isRELESE) {
                System.out.println("Number of Over=" + cn);
            }
        }

    }

    public static void getUDUW_nbSet_from_nbList_InOut(Net net) {
        for (int i = 0; i < net.NODES.size(); ++i) {
            NODE v = (NODE) net.NODES.get(i);
            v.nbSet.clear();
            v.nbSet.addAll(v.nbList_In);
            v.nbSet.addAll(v.nbList_Out);
        }

    }

    public ArrayList<ArrayList<Long>> post_separate_disconnect_subcomponent(ArrayList<ArrayList<Long>> cpm) {
        if (!this.isRELESE) {
            System.out.println("post_separate disconnect subcomponent (UDUW nbSet-connection).............");
        }

        getUDUW_nbSet_from_nbList_InOut(this.net);
        return this.net.pre_findAllConnectedComponents_InOneCluster_CPM(cpm);
    }

    public ArrayList<ArrayList<Long>> post_processing(ArrayList<ArrayList<Long>> cpm) {
        cpm = this.post_removeSubset(cpm);
        cpm = this.post_reassign_singledeg_shallow(cpm);
        cpm = this.post_separate_disconnect_subcomponent(cpm);

        for (int h = 1; h <= 4; ++h) {
            cpm = this.post_mergeSmallComms(cpm, this.thr_commsize_min);
        }

        cpm = this.post_removeSubset(cpm);
        return cpm;
    }

    public ArrayList<ArrayList<Long>> post_smallComs_mergeAndFindconnectedComponent(ArrayList<ArrayList<Long>> cpm) {
        if (!this.isRELESE) {
            System.out.println("post_smallComs_mergeAndFindconnectedComponent (UDUW nbSet-connection).............");
        }

        ArrayList newcpm = new ArrayList();
        int scn = 0;
        ArrayList SmallCom = new ArrayList();

        for (int tmpcpm = 0; tmpcpm < cpm.size(); ++tmpcpm) {
            if (((ArrayList) cpm.get(tmpcpm)).size() < this.thr_commsize_min) {
                SmallCom.addAll((Collection) cpm.get(tmpcpm));
                ++scn;
            } else {
                newcpm.add((ArrayList) cpm.get(tmpcpm));
            }
        }

        if (scn == 0) {
            return cpm;
        } else {
            ArrayList var7 = new ArrayList();
            var7.add(SmallCom);
            var7 = this.post_separate_disconnect_subcomponent(var7);
            if (!this.isRELESE) {
                System.out.println(" >>new num of small com=" + var7.size());
            }

            for (int i = 0; i < var7.size(); ++i) {
                newcpm.add((ArrayList) var7.get(i));
            }

            return newcpm;
        }
    }

    public void post_threshold_createCPM(int thrc, String fileName, double thrp, boolean isMasterSLPAw) {
        ArrayList cpm = new ArrayList();
        Hashtable w_comidTable = new Hashtable();
        int comid = -1;
        int ov_cn = 0;

        int i;
        int isWriteOv;
        ArrayList tmpcpm;
        for (i = 0; i < this.net.N; ++i) {
            NODE v = (NODE) this.net.NODES.get(i);
            ArrayList isPrintOv = this.post_thresholding(v.WQHistMapEntryList, thrc);
            if (isPrintOv.size() < 1) {
                System.err.println("empty WS");
            }

            if (isPrintOv.size() > 1) {
                ++ov_cn;
            }

            for (isWriteOv = 0; isWriteOv < isPrintOv.size(); ++isWriteOv) {
                Long totalN = (Long) isPrintOv.get(isWriteOv);
                if (!w_comidTable.containsKey(totalN)) {
                    ++comid;
                    w_comidTable.put(totalN, Integer.valueOf(comid));
                    tmpcpm = new ArrayList();
                    cpm.add(tmpcpm);
                }

                int var22 = ((Integer) w_comidTable.get(totalN)).intValue();
                ((ArrayList) cpm.get(var22)).add(Long.valueOf(v.ID));
            }
        }

        cpm = this.post_processing(cpm);
        if (isMasterSLPAw) {
            cpm = this.post_applymaxC(thrp, cpm);
            cpm = this.post_smallComs_mergeAndFindconnectedComponent(cpm);
            i = 0;
            int var17 = 0;
            isWriteOv = 0;
            int var20 = 0;
            tmpcpm = new ArrayList();

            for (int i1 = 0; i1 < cpm.size(); ++i1) {
                if (((ArrayList) cpm.get(i1)).size() <= 0) {
                    ++isWriteOv;
                } else if (((ArrayList) cpm.get(i1)).size() < this.thr_commsize_min) {
                    ++i;
                    var20 += ((ArrayList) cpm.get(i1)).size();
                    tmpcpm.add((ArrayList) cpm.get(i1));
                } else if (((ArrayList) cpm.get(i1)).size() > this.thr_commsize_max) {
                    ++var17;
                }
            }

            if (i > 0) {
                this.write2txt_smallCom_nameS(fileName + ".smallComIDS.txt", tmpcpm);
            }

            if (!this.isRELESE) {
                System.out.println("Final cpm:");
                System.out.println("       K=" + (cpm.size() - isWriteOv));
                System.out.println("  largeK=" + var17);
                System.out.println("  smallK=" + i);
                System.out.println("  smallN=" + var20);
            }
        }

        Collections.sort(cpm, Net.ComCPMsizedec);
        if (isMasterSLPAw) {
            boolean var18 = true;
            boolean var19 = false;
            boolean var21 = this.isWriteOvfile;
            this.identify_NodeCommTable_fromCPM(this.net.NODES, cpm, var18, var19, var21, this.writeNODECOMFlag, fileName);
            this.write2txt_CPM_nameS(fileName, cpm);
        } else if (this.thr_commsize_max > this.thr_commsize_min) {
            if (!this.isRELESE) {
                System.out.println("returning....");
            }

            this.cpm_good.clear();
            this.cpm_tooLarge.clear();

            for (i = 0; i < cpm.size(); ++i) {
                if (((ArrayList) cpm.get(i)).size() < this.thr_commsize_max) {
                    this.cpm_good.add((ArrayList) cpm.get(i));
                } else {
                    this.cpm_tooLarge.add((ArrayList) cpm.get(i));
                }
            }
        }

    }

    public void write2txt_histInfo_nameS(String fileName, int Level) {
        byte topx = 5;
        if (!this.isRELESE) {
            System.out.println("Save hist info: at most top " + topx);
        }

        ArrayList dataL = new ArrayList();
        ArrayList dataP = new ArrayList();
        StringBuffer oneLineL = new StringBuffer();
        StringBuffer oneLineP = new StringBuffer();
        Collections.sort(this.net.NODES, NODE.ComNodeIDInc);

        for (int i = 0; i < this.net.NODES.size(); ++i) {
            NODE v = (NODE) this.net.NODES.get(i);
            Long ID = Long.valueOf(v.ID);
            String Name = v.name;
            ArrayList list = new ArrayList();
            if (Level == 1) {
                list = v.WQHistMapEntryList;
            } else if (Level == 2) {
                list = new ArrayList(v.WQHistgram_r.entrySet());
                Collections.sort(list, ComHistogramdec);
            }

            double totalCN = 0.0D;

            int j;
            for (j = 0; j < list.size(); ++j) {
                totalCN += (double) ((Integer) ((Entry) list.get(j)).getValue()).intValue();
            }

            oneLineL.setLength(0);
            oneLineP.setLength(0);
            oneLineL.append(Name);
            oneLineL.append(" ");
            oneLineP.append(Name);
            oneLineP.append(" ");

            for (j = 0; j < list.size() && j < topx; ++j) {
                long h = ((Long) ((Entry) list.get(j)).getKey()).longValue();
                double count = (double) ((Integer) ((Entry) list.get(j)).getValue()).intValue();
                oneLineL.append(h);
                oneLineL.append(" ");
                oneLineP.append(count / totalCN);
                oneLineP.append(" ");
            }

            for (int var20 = j; var20 < topx; ++var20) {
                oneLineL.append(-1);
                oneLineL.append(" ");
                oneLineP.append(0);
                oneLineP.append(" ");
            }

            dataL.add(oneLineL.toString());
            dataP.add(oneLineP.toString());
        }

        if (!this.isRELESE) {
            System.out.println(fileName + "_L" + Level + ".histlabel.txt");
        }

        fileOpts.writeToTxt(fileName + "_L" + Level + ".histlabel.txt", false, dataL);
        fileOpts.writeToTxt(fileName + "_L" + Level + ".histprob.txt", false, dataP);
    }

    public void write2txt_smallCom_nameS(String fileName, ArrayList<ArrayList<Long>> cpm) {
        ArrayList data = new ArrayList();
        StringBuffer oneLine = new StringBuffer();

        for (int i = 0; i < cpm.size(); ++i) {
            ArrayList oneComm = (ArrayList) cpm.get(i);

            for (int j = 0; j < oneComm.size(); ++j) {
                oneLine.setLength(0);
                oneLine.append(((NODE) this.net.NODESTABLE.get(oneComm.get(j))).name);
                data.add(oneLine.toString());
            }
        }

        fileOpts.writeToTxt(fileName, false, data);
    }

    public void update_WQHistgram_r_fromEmbeddedSLPAw_WQHistgram(ArrayList<Long> oneComIDS, SLPAw embslpaw) {
        for (int i = 0; i < oneComIDS.size(); ++i) {
            Long ID = (Long) oneComIDS.get(i);
            NODE v_master = (NODE) this.net.NODESTABLE.get(ID);
            NODE v_emb = (NODE) embslpaw.net.NODESTABLE.get(ID);
            v_master.WQHistgram_r.clear();
            Iterator it = v_emb.WQHistgram.keySet().iterator();

            while (it.hasNext()) {
                Long label = (Long) it.next();
                Integer count = (Integer) v_emb.WQHistgram.get(label);
                v_master.WQHistgram_r.put(label, count);
            }
        }

    }

    public ArrayList<ArrayList<Long>> post_applymaxC(double thrp, ArrayList<ArrayList<Long>> cpm) {
        if (this.thr_commsize_max > this.thr_commsize_min) {
            this.cpm_good.clear();
            this.cpm_tooLarge.clear();

            for (int maxloop = 0; maxloop < cpm.size(); ++maxloop) {
                if (((ArrayList) cpm.get(maxloop)).size() < this.thr_commsize_max) {
                    this.cpm_good.add((ArrayList) cpm.get(maxloop));
                } else {
                    this.cpm_tooLarge.add((ArrayList) cpm.get(maxloop));
                }
            }

            if (!this.isRELESE) {
                System.out.println("******Before embedded slpas: num of Com=" + cpm.size());
                System.out.println("******Before embedded slpas: num of toolargeCom=" + this.cpm_tooLarge.size());
            }

            double var11 = (double) this.cpm_tooLarge.size() * this.embslpaw_loopcn;
            double lcn = 0.0D;
            System.out.println("Info: Decomposing large community (max # of loops=" + var11 + " for the -maxC parameter)......");

            while (this.cpm_tooLarge.size() > 0 && lcn < var11) {
                ArrayList i = (ArrayList) this.cpm_tooLarge.get(0);
                SLPAw embslpaw = this.embededSLPAw_on_OneCOM_start(thrp, i, this.ht_name_ID);
                if (this.isOhis2) {
                    this.update_WQHistgram_r_fromEmbeddedSLPAw_WQHistgram(i, embslpaw);
                }

                this.cpm_tooLarge.remove(0);

                int i1;
                for (i1 = 0; i1 < embslpaw.cpm_tooLarge.size(); ++i1) {
                    this.cpm_tooLarge.add((ArrayList) embslpaw.cpm_tooLarge.get(i1));
                }

                if (!this.isRELESE) {
                    System.out.println("embslpaw.cpm_tooLarge.size()=" + embslpaw.cpm_tooLarge.size());
                }

                for (i1 = 0; i1 < embslpaw.cpm_good.size(); ++i1) {
                    this.cpm_good.add((ArrayList) embslpaw.cpm_good.get(i1));
                }

                if (!this.isRELESE) {
                    System.out.println("embslpaw.cpm_good.size()=" + embslpaw.cpm_good.size());
                }

                ++lcn;
                if (!this.isRELESE) {
                    System.out.println("loop-coundown" + (var11 - lcn) + ":cpm_tooLarge.size()=" + this.cpm_tooLarge.size());
                }
            }

            cpm.clear();
            cpm = this.cpm_good;

            for (int var12 = 0; var12 < this.cpm_tooLarge.size(); ++var12) {
                cpm.add((ArrayList) this.cpm_tooLarge.get(var12));
            }

            cpm = this.post_processing(cpm);
            if (!this.isRELESE) {
                System.out.println("embslpaw.WeigthVersion=" + this.embslpaw_WeigthVersion + "(" + this.embslpaw_loopcn + ")");
            }

            if (!this.isRELESE) {
                System.out.println("After embedded slpas: num of Com=" + cpm.size() + " toolarge=" + this.cpm_tooLarge.size());
            }
        }

        return cpm;
    }

    public ArrayList<ArrayList<Long>> post_sameLabelDisconnectedComponents(ArrayList<ArrayList<Long>> cpm) {
        ArrayList newcpm = this.net.pre_findAllConnectedComponents_InOneCluster_CPM(cpm);
        return newcpm;
    }

    public void show_cpm(ArrayList<ArrayList<Integer>> cpm) {
        for (int i = 0; i < cpm.size(); ++i) {
            Collections.sort((List) cpm.get(i));
            System.out.println("cpm" + i + " size=" + ((ArrayList) cpm.get(i)).size() + ((ArrayList) cpm.get(i)).toString());
        }

    }

    public ArrayList<Entry<Integer, Integer>> create_sortedMapEntryList_fromHistogram_IntInt_dec(Hashtable<Integer, Integer> hist) {
        ArrayList entrylist = new ArrayList(hist.entrySet());
        Collections.sort(entrylist, ComHistogramdec);
        return entrylist;
    }

    public ArrayList<ArrayList<Long>> post_mergeSmallComms(ArrayList<ArrayList<Long>> cpm, int thr_commsize) {
        if (!this.isRELESE) {
            System.out.println("post_mergeSmallComms (cpm + UDUW connection).............");
        }

        ArrayList newcpm = new ArrayList();
        ArrayList badCommInds = new ArrayList();
        ArrayList goodCommInds = new ArrayList();

        for (int targetCommInds = 0; targetCommInds < cpm.size(); ++targetCommInds) {
            if (((ArrayList) cpm.get(targetCommInds)).size() <= thr_commsize) {
                badCommInds.add(Integer.valueOf(targetCommInds));
            } else {
                goodCommInds.add(Integer.valueOf(targetCommInds));
            }
        }

        this.identify_NodeCommTable_fromCPM(this.net.NODES, cpm, true, false, false, 0, "");
        ArrayList var21 = new ArrayList();

        int mcn;
        int i;
        for (mcn = 0; mcn < badCommInds.size(); ++mcn) {
            i = ((Integer) badCommInds.get(mcn)).intValue();
            ArrayList goodComInd = (ArrayList) cpm.get(i);
            Hashtable toInd = new Hashtable();

            for (int isFoundAnyTargert = 0; isFoundAnyTargert < goodCommInds.size(); ++isFoundAnyTargert) {
                toInd.put((Integer) goodCommInds.get(isFoundAnyTargert), Integer.valueOf(0));
            }

            boolean var25 = false;

            for (int list = 0; list < goodComInd.size(); ++list) {
                Long ID = (Long) goodComInd.get(list);
                NODE v = (NODE) this.net.NODESTABLE.get(ID);
                HashSet nbSet = new HashSet();
                nbSet.addAll(v.nbList_In);
                nbSet.addAll(v.nbList_Out);
                Iterator it = nbSet.iterator();

                while (it.hasNext()) {
                    Long nbID = (Long) it.next();
                    ArrayList comInds = (ArrayList) this.nodes_comIDs_Table.get(nbID);

                    for (int x = 0; x < comInds.size(); ++x) {
                        Integer cind = (Integer) comInds.get(x);
                        if (goodCommInds.contains(cind)) {
                            toInd.put(cind, Integer.valueOf(((Integer) toInd.get(cind)).intValue() + 1));
                            var25 = true;
                        }
                    }
                }
            }

            ArrayList var26 = this.create_sortedMapEntryList_fromHistogram_IntInt_dec(toInd);
            if (!var25) {
                var21.add(Integer.valueOf(-1));
            } else {
                var21.add((Integer) ((Entry) var26.get(0)).getKey());
            }
        }

        mcn = 0;

        for (i = 0; i < badCommInds.size(); ++i) {
            Integer var22 = (Integer) badCommInds.get(i);
            Integer var24 = (Integer) var21.get(i);
            if (var24.intValue() > -1) {
                ((ArrayList) cpm.get(var24.intValue())).addAll((Collection) cpm.get(var22.intValue()));
                ++mcn;
            } else {
                newcpm.add((ArrayList) cpm.get(var22.intValue()));
            }
        }

        if (!this.isRELESE) {
            System.out.println("info: merge " + mcn + " small comms.");
        }

        for (i = 0; i < goodCommInds.size(); ++i) {
            int var23 = ((Integer) goodCommInds.get(i)).intValue();
            newcpm.add((ArrayList) cpm.get(var23));
        }

        return newcpm;
    }

    public ArrayList<ArrayList<Long>> post_reassign_singledeg_shallow(ArrayList<ArrayList<Long>> cpm) {
        if (!this.isRELESE) {
            System.out.println("post_reassign_singledeg_shallow.............");
        }

        ArrayList newcpm = new ArrayList();
        ArrayList keys = new ArrayList(this.singleDegNode_Hub_Table.keySet());

        int i;
        Long hubID;
        int maxsz;
        for (i = 0; i < keys.size(); ++i) {
            hubID = (Long) keys.get(i);

            for (maxsz = 0; maxsz < cpm.size(); ++maxsz) {
                if (((ArrayList) cpm.get(maxsz)).contains(hubID)) {
                    ((ArrayList) cpm.get(maxsz)).remove(hubID);
                }
            }
        }

        for (i = 0; i < cpm.size(); ++i) {
            if (((ArrayList) cpm.get(i)).size() > 0) {
                newcpm.add((ArrayList) cpm.get(i));
            }
        }

        cpm = newcpm;
        this.identify_NodeCommTable_fromCPM(this.net.NODES, newcpm, false, false, false, 0, "");
        keys.clear();
        keys = new ArrayList(this.Hub_singleDegNodeList_Table.keySet());

        for (i = 0; i < keys.size(); ++i) {
            hubID = (Long) keys.get(i);
            maxsz = -1;
            int comind = -1;
            ArrayList hub_comInds = (ArrayList) this.nodes_comIDs_Table.get(hubID);

            for (int k = 0; k < hub_comInds.size(); ++k) {
                int comsize = ((ArrayList) cpm.get(((Integer) hub_comInds.get(k)).intValue())).size();
                if (comsize > maxsz) {
                    maxsz = comsize;
                    comind = ((Integer) hub_comInds.get(k)).intValue();
                }
            }

            ((ArrayList) cpm.get(comind)).addAll((Collection) this.Hub_singleDegNodeList_Table.get(hubID));
        }

        return cpm;
    }

    public ArrayList<ArrayList<Long>> post_removeSubset(ArrayList<ArrayList<Long>> cpm) {
        if (!this.isRELESE) {
            System.out.println("removeSubset (CPM).............");
        }

        ArrayList newcpm = new ArrayList();
        ArrayList indicators = new ArrayList();

        int i;
        for (i = 0; i < cpm.size(); ++i) {
            indicators.add(Integer.valueOf(1));
        }

        Collections.sort(cpm, Net.ComCPMsizedec);

        for (i = 0; i < cpm.size() - 1; ++i) {
            if (((Integer) indicators.get(i)).intValue() != 0) {
                ArrayList commi = (ArrayList) cpm.get(i);

                for (int j = i + 1; j < cpm.size(); ++j) {
                    if (((Integer) indicators.get(j)).intValue() != 0) {
                        ArrayList commj = (ArrayList) cpm.get(j);
                        if (commi.containsAll(commj)) {
                            indicators.set(j, Integer.valueOf(0));
                        }
                    }
                }
            }
        }

        for (i = 0; i < cpm.size(); ++i) {
            if (((Integer) indicators.get(i)).intValue() != 0) {
                newcpm.add((ArrayList) cpm.get(i));
            }
        }

        return newcpm;
    }

    public void write2txt_CPM_nameS(String fileName, ArrayList<ArrayList<Long>> cpm) {
        ArrayList data = new ArrayList();
        StringBuffer oneLine = new StringBuffer();

        for (int i = 0; i < cpm.size(); ++i) {
            oneLine.setLength(0);
            ArrayList oneComm = (ArrayList) cpm.get(i);
            Collections.sort(oneComm);

            for (int j = 0; j < oneComm.size(); ++j) {
                oneLine.append(((NODE) this.net.NODESTABLE.get(oneComm.get(j))).name);
                oneLine.append(" ");
            }

            data.add(oneLine.toString());
        }

        fileOpts.writeToTxt(fileName, false, data);
    }

    public void pre_initial_THRCS() {
        this.THRCS = new ArrayList();

        for (int i = 0; i < this.THRS.size(); ++i) {
            this.THRCS.add(Integer.valueOf((int) Math.round(((Double) this.THRS.get(i)).doubleValue() * (double) this.maxT)));
        }

    }

    public void post_createWQHistogram_MapEntryList() {
        for (int i = 0; i < this.net.N; ++i) {
            NODE v = (NODE) this.net.NODES.get(i);
            v.WQHistgram = this.createHistogram(v.WQueue);
            v.WQHistMapEntryList = new ArrayList(v.WQHistgram.entrySet());
            Collections.sort(v.WQHistMapEntryList, ComHistogramdec);
            v.WQueue.clear();
        }

    }

    public ArrayList<Long> post_thresholding(ArrayList<Entry<Long, Integer>> list, int thrc) {
        ArrayList WS = new ArrayList();
        Integer maxv = (Integer) ((Entry) list.get(0)).getValue();
        Long label;
        int i;
        if (maxv.intValue() <= thrc) {
            i = 1;

            for (int i1 = 1; i1 < list.size() && (Integer) ((Entry) list.get(i1)).getValue() == maxv; ++i1) {
                ++i;
            }

            if (i == 1) {
                label = (Long) ((Entry) list.get(0)).getKey();
            } else {
                label = (Long) ((Entry) list.get(this.rndGlobal.nextInt(i))).getKey();
            }

            WS.add(label);
        } else {
            for (i = 0; i < list.size() && ((Integer) ((Entry) list.get(i)).getValue()).intValue() > thrc; ++i) {
                label = (Long) ((Entry) list.get(i)).getKey();
                WS.add(label);
            }
        }

        return WS;
    }

    public void SLPAw_asyn() {
        Long label = Long.valueOf(-1L);

        for (int t = 0; t < this.maxT - 1; ++t) {
            Collections.shuffle(this.net.NODES, this.rndGlobal);

            for (int i = 0; i < this.net.N; ++i) {
                NODE v = (NODE) this.net.NODES.get(i);
                if (v.numNbs_In > 0) {
                    if (this.WeigthVersion == 1) {
                        label = Long.valueOf(this.collect_labels_from_nbs_weightedDirected_randomThr(v));
                    } else if (this.WeigthVersion == 2) {
                        label = Long.valueOf(this.collect_labels_from_nbs_weightedDirected_sumprob(v));
                    } else if (this.WeigthVersion == 3) {
                        label = this.collect_labels_from_nbs_weightedDirected_maxprob(v);
                    } else {
                        System.out.println("Wrong weight version.");
                        System.exit(1);
                    }

                    v.WQueue.add(label);
                } else {
                    label = Long.valueOf(v.ID);
                    v.WQueue.add(label);
                }
            }
        }

    }

    public long collect_labels_from_nbs_weightedDirected_randomThr(NODE v) {
        ArrayList nbWs = new ArrayList();
        ArrayList selectedInNbs = new ArrayList();
        boolean ok = false;

        while (true) {
            Double j;
            do {
                if (ok) {
                    for (int var11 = 0; var11 < selectedInNbs.size(); ++var11) {
                        Long var12 = (Long) selectedInNbs.get(var11);
                        NODE nbv = (NODE) this.net.NODESTABLE.get(var12);
                        int wind = this.rndGlobal.nextInt(nbv.WQueue.size());
                        nbWs.add((Long) nbv.WQueue.get(wind));
                    }

                    long label = this.ceateHistogram_selRandMax(nbWs).longValue();
                    return label;
                }

                j = Double.valueOf(this.rndGlobal.nextDouble());
            } while (((Double) Collections.min(v.nbweightList_In_norm)).doubleValue() < j.doubleValue());

            ok = true;

            for (int nbID = 0; nbID < v.nbweightList_In_norm.size(); ++nbID) {
                if (((Double) v.nbweightList_In_norm.get(nbID)).doubleValue() >= j.doubleValue()) {
                    selectedInNbs.add((Long) v.nbList_In.get(nbID));
                }
            }
        }
    }

    public long collect_labels_from_nbs_weightedDirected_sumprob(NODE v) {
        Hashtable hist = new Hashtable();
        ArrayList LabelProbHist_MapEntryList = this.getNbs_label_weight_Hist_Incoming(v, hist);
        long label = this.selectOneLabel_withAccumuatedProb(LabelProbHist_MapEntryList).longValue();
        return label;
    }

    public Long collect_labels_from_nbs_weightedDirected_maxprob(NODE v) {
        Hashtable hist = new Hashtable();
        ArrayList LabelProbHist_MapEntryList = this.getNbs_label_weight_Hist_Incoming(v, hist);
        Long label = (Long) ((Entry) LabelProbHist_MapEntryList.get(0)).getKey();
        return label;
    }

    public ArrayList<Entry<Long, Double>> getNbs_label_weight_Hist_Incoming(NODE v, Hashtable<Long, Double> hist) {
        hist.clear();

        for (int list = 0; list < v.nbList_In.size(); ++list) {
            Long nbID = (Long) v.nbList_In.get(list);
            NODE nbv = (NODE) this.net.NODESTABLE.get(nbID);
            int wind = this.rndGlobal.nextInt(nbv.WQueue.size());
            Long label = (Long) nbv.WQueue.get(wind);
            double weight = ((Double) v.nbweightList_In_norm.get(list)).doubleValue();
            if (hist.containsKey(label)) {
                hist.put(label, Double.valueOf(((Double) hist.get(label)).doubleValue() + weight));
            } else {
                hist.put(label, Double.valueOf(weight));
            }
        }

        ArrayList var10 = new ArrayList(hist.entrySet());
        Collections.sort(var10, ComHistogramdec_IntDlb);
        return var10;
    }

    public Long selectOneLabel_withAccumuatedProb(ArrayList<Entry<Long, Double>> list) {
        Long label = Long.valueOf(-1L);
        Double p = Double.valueOf(this.rndGlobal.nextDouble());
        double sum = 0.0D;

        for (int i = 0; i < list.size(); ++i) {
            sum += ((Double) ((Entry) list.get(i)).getValue()).doubleValue();
            if (p.doubleValue() <= sum) {
                label = (Long) ((Entry) list.get(i)).getKey();
                break;
            }
        }

        if (label.longValue() == -1L) {
            System.err.println("label=-1");
            System.exit(1);
        }

        return label;
    }

    public void initWQueue() {
        for (int i = 0; i < this.net.N; ++i) {
            NODE v = (NODE) this.net.NODES.get(i);
            v.WQueue.clear();
            v.WQueue.add(Long.valueOf(v.ID));
        }

    }

    public Long ceateHistogram_selRandMax(ArrayList<Long> wordsList) {
        Hashtable hist = new Hashtable();

        for (int list = 0; list < wordsList.size(); ++list) {
            Long maxv = (Long) wordsList.get(list);
            if (hist.containsKey(maxv)) {
                hist.put(maxv, Integer.valueOf(((Integer) hist.get(maxv)).intValue() + 1));
            } else {
                hist.put(maxv, Integer.valueOf(1));
            }
        }

        ArrayList var8 = new ArrayList(hist.entrySet());
        Collections.sort(var8, ComHistogramdec);
        Integer var9 = (Integer) ((Entry) var8.get(0)).getValue();
        int cn = 1;

        for (int i = 1; i < var8.size() && (Integer) ((Entry) var8.get(i)).getValue() == var9; ++i) {
            ++cn;
        }

        Long label;
        if (cn == 1) {
            label = (Long) ((Entry) var8.get(0)).getKey();
        } else {
            label = (Long) ((Entry) var8.get(this.rndGlobal.nextInt(cn))).getKey();
        }

        return label;
    }

    public Hashtable<Long, Integer> createHistogram(ArrayList<Long> wordsList) {
        Hashtable hist = new Hashtable();

        for (int i = 0; i < wordsList.size(); ++i) {
            Long v0 = (Long) wordsList.get(i);
            if (hist.containsKey(v0)) {
                hist.put(v0, Integer.valueOf(((Integer) hist.get(v0)).intValue() + 1));
            } else {
                hist.put(v0, Integer.valueOf(1));
            }
        }

        return hist;
    }

    public static ArrayList<Double> pre_load_THRS() {
        ArrayList THRS = new ArrayList();
        THRS.add(Double.valueOf(0.01D));
        THRS.add(Double.valueOf(0.05D));
        THRS.add(Double.valueOf(0.1D));
        THRS.add(Double.valueOf(0.15D));
        THRS.add(Double.valueOf(0.2D));
        THRS.add(Double.valueOf(0.25D));
        THRS.add(Double.valueOf(0.3D));
        THRS.add(Double.valueOf(0.35D));
        THRS.add(Double.valueOf(0.4D));
        THRS.add(Double.valueOf(0.45D));
        THRS.add(Double.valueOf(0.5D));
        return THRS;
    }

    static final void printCitation() {
        String info1 = "\n-------------------------------------------------------------------------------\n                            GANXiSw (used to be SLPAw)               \n-------------------------------------------------------------------------------";
        String info2 = "More information about this algorithm or citation:";
        String paper1 = "   1. Jierui Xie, Boleslaw K. Szymanski and Xiaoming Liu, SLPA: Uncovering Overlapping Communities in Social Networks via A Speaker-listener Interaction Dynamic Process, IEEE ICDM workshop on DMCCI 2011.";
        String paper2 = "   2. Jierui Xie and Boleslaw K. Szymanski, Towards Linear Time Overlapping Community Detection in Social Networks, 16th Pacific-Asia Conference on Knowledge Discovery and Data Mining (PAKDD), 2012.";
        String info3 = "Performance comparision with other algorithms:";
        String paper3 = "   3. Jierui Xie, Stephen Kelley and Boleslaw K. Szymanski, Overlapping Community Detection in Networks: the State of the Art and Comparative Study, ACM Computing Surveys 45(12) December 2013.";
        String info4 = "@Copyright: see readme.pdf";
        String info5 = "Questions: jierui.xie@gmail.com and szymansk@cs.rpi.edu";
        String info6 = "Download: https://sites.google.com/site/communitydetectionslpa/\n\n";
        System.out.println(info1);
        System.out.println(info2);
        System.out.println(paper1);
        System.out.println(paper2);
        System.out.println(info3);
        System.out.println(paper3);
        System.out.println(info4);
        System.out.println(info5);
        System.out.println(info6);
    }

    static final void printUsage(String usage) {
        String info1 = "\n-------------------------------------------------------------------------------\n                            Usage               \n-------------------------------------------------------------------------------";
        System.out.println(info1);
        System.out.println(usage);
    }

    public static void main(String[] args) {
        Hashtable ht_name_ID = new Hashtable();
        long seed = (new Random()).nextLong();
        boolean isRELESE = true;
        String version = "3.0.2";
        int WeigthVersion = 3;
        int embslpaw_WeigthVersion = 1;
        double embslpaw_loopcn = 1.0D;
        int thr_commsize_min = 2;
        int thr_commsize_max = -1;
        boolean isWriteOvfile = false;
        byte writeNODECOMFlag = 0;
        boolean isWrieComNodefile = false;
        boolean isOhis1 = false;
        boolean isOhis2 = false;
        boolean isOMem1 = false;
        double EC = -1.0D;
        boolean isUsage = false;
        boolean isWeighted = true;
        boolean ishelp = false;
        String usage = "GANXiSw " + version + "(used to be SLPAw) is for weighted (directed) networks, version=" + version + "\n" + "Usage: java -jar GANXiSw.jar -i networkfile\n" + "Options:\n" + "  -i input network file\n" + "  -d output director (default: output)\n" + "  -L set to 1 to use only the largest connected component\n" + "  -t maximum iteration (default: 100)\n" + "  -run number of repetitions\n" + "  -r a specific threshold in [0,0.5]\n" + "  -ov set to 0 to perform disjoint detection\n" + "  -W treat the input as a weighted network, set 0 to ignore the weights(default 1)\n" + "  -Sym set to 1 to make the edges symmetric/bi-directional (default 0)\n" + "  -seed user specified seed for random generator\n" + "  -help to display usage info\n" + " -----------------------------Advanced Parameters---------------------------------\n" + "  -v weighted version in {1,2,3}, default=3\n" + "  -Oov set to 1 to output overlapping file, default=0\n" + "  -Onc set to 1 to output <nodeID communityID> format, 2 to output <communityID nodeID> format\n" + "  -minC min community size threshold, default=2\n" + "  -maxC max community size threshold\n" + "  -ev embedded SLPAw\'s weighted version in {1,2,3}, default=1\n" + "  -loopfactor determine the num of loops for depomposing each large com, default=1.0\n" + "  -Ohis1 set to 1 to output histgram Level1\n" + "  -Ohis2 set to 1 to output histgram Level2\n\n" + "  -OMem1 set to 1 to output each node\'s memory content at Level 1\n" + "  -EC evolution cutoff, a real value > 1.0 \n" + "NOTE: 1. more parameters refer to Readme.pdf\n" + "      2. parameters are *CASE-SENSITIVE*, e.g., -Onc is not -onc\n";
        ArrayList THRS = pre_load_THRS();
        int maxRun = 1;
        int maxT = 100;
        boolean isOverlapping = true;
        boolean isUseLargestComp = false;
        boolean isSymmetrize = false;
        String inputFileName = "";
        String outputDir = "output";
        Hashtable argTable = new Hashtable();

        String v;
        for (int key = 0; key < args.length - 1; ++key) {
            v = new String(args[key]);
            String tb0 = new String(args[key + 1]);
            if (!v.equals("-t") && !v.equals("-ov") && !v.equals("-r") && !v.equals("-run") && !v.equals("-i") && !v.equals("-d") && !v.equals("-L") && !v.equals("-v") && !v.equals("-ev") && !v.equals("-loopfactor") && !v.equals("-minC") && !v.equals("-maxC") && !v.equals("-Oov") && !v.equals("-Onc") && !v.equals("-Ocn") && !v.equals("-Ohis1") && !v.equals("-Ohis2") && !v.equals("-OMem1") && !v.equals("-help") && !v.equals("-EC") && !v.equals("-U") && !v.equals("-W") && !v.equals("-seed") && !v.equals("-Sym") && !v.equals("-help")) {
                System.out.println("No such a parameter:" + v);
                System.out.println(usage);
                System.exit(1);
            } else if (!tb0.startsWith("-")) {
                argTable.put(v, tb0);
                ++key;
            }
        }

        String var39 = "-help";
        if (argTable.containsKey(var39)) {
            System.out.println(usage);
            System.exit(1);
        }

        var39 = "-t";
        if (argTable.containsKey(var39)) {
            v = (String) argTable.get(var39);
            maxT = Integer.parseInt(v);
        }

        var39 = "-ov";
        if (argTable.containsKey(var39)) {
            v = (String) argTable.get(var39);
            if (v.equals("0")) {
                isOverlapping = false;
            }
        }

        var39 = "-run";
        if (argTable.containsKey(var39)) {
            v = (String) argTable.get(var39);
            maxRun = Integer.parseInt(v);
        }

        var39 = "-r";
        if (argTable.containsKey(var39)) {
            v = (String) argTable.get(var39);
            THRS.clear();
            THRS.add(Double.valueOf(Double.parseDouble(v)));
        }

        var39 = "-v";
        if (argTable.containsKey(var39)) {
            v = (String) argTable.get(var39);
            WeigthVersion = Integer.parseInt(v);
        }

        var39 = "-ev";
        if (argTable.containsKey(var39)) {
            v = (String) argTable.get(var39);
            embslpaw_WeigthVersion = Integer.parseInt(v);
        }

        var39 = "-minC";
        if (argTable.containsKey(var39)) {
            v = (String) argTable.get(var39);
            thr_commsize_min = Integer.parseInt(v);
        }

        var39 = "-maxC";
        if (argTable.containsKey(var39)) {
            v = (String) argTable.get(var39);
            thr_commsize_max = Integer.parseInt(v);
        }

        var39 = "-loopfactor";
        if (argTable.containsKey(var39)) {
            v = (String) argTable.get(var39);
            embslpaw_loopcn = Double.parseDouble(v);
        }

        var39 = "-Oov";
        if (argTable.containsKey(var39)) {
            v = (String) argTable.get(var39);
            if (v.equals("1")) {
                isWriteOvfile = true;
            }
        }

        var39 = "-Onc";
        if (argTable.containsKey(var39)) {
            v = (String) argTable.get(var39);
            if (v.equals("1")) {
                writeNODECOMFlag = 1;
            } else if (v.equals("2")) {
                writeNODECOMFlag = 2;
            } else {
                System.err.println("ERROR: invalid parameter value for Onc!");
                System.out.println(usage);
                System.exit(1);
            }
        }

        var39 = "-Ohis1";
        if (argTable.containsKey(var39)) {
            v = (String) argTable.get(var39);
            if (v.equals("1")) {
                isOhis1 = true;
            }
        }

        var39 = "-Ohis2";
        if (argTable.containsKey(var39)) {
            v = (String) argTable.get(var39);
            if (v.equals("1")) {
                isOhis2 = true;
            }
        }

        var39 = "-OMem1";
        if (argTable.containsKey(var39)) {
            v = (String) argTable.get(var39);
            if (v.equals("1")) {
                isOMem1 = true;
            }
        }

        var39 = "-U";
        if (argTable.containsKey(var39)) {
            v = (String) argTable.get(var39);
            if (v.equals("0")) {
                isUsage = false;
            }
        }

        var39 = "-W";
        if (argTable.containsKey(var39)) {
            v = (String) argTable.get(var39);
            if (v.equals("0")) {
                isWeighted = false;
            }
        }

        if (!isOverlapping) {
            THRS.clear();
            THRS.add(Double.valueOf(0.5D));
        }

        var39 = "-seed";
        if (argTable.containsKey(var39)) {
            v = (String) argTable.get(var39);
            seed = Long.parseLong(v);
            System.out.println("Info: Using user defined seed=" + seed);
        }

        var39 = "-Sym";
        if (argTable.containsKey(var39)) {
            v = (String) argTable.get(var39);
            if (v.equals("1")) {
                isSymmetrize = true;
            }
        }

        var39 = "-i";
        if (argTable.containsKey(var39)) {
            v = (String) argTable.get(var39);
            inputFileName = v;
        }

        var39 = "-d";
        if (argTable.containsKey(var39)) {
            v = (String) argTable.get(var39);
            outputDir = v;
        }

        var39 = "-L";
        if (argTable.containsKey(var39)) {
            v = (String) argTable.get(var39);
            if (v.equals("1")) {
                isUseLargestComp = true;
            }
        }

        var39 = "-EC";
        if (argTable.containsKey(var39)) {
            v = (String) argTable.get(var39);
            EC = Double.parseDouble(v);
        }

        fileOpts.createDirectory("output");
        Random rndGlobal = new Random(seed);
        if (inputFileName.length() != 0 && fileOpts.isFileExist(inputFileName)) {
            long var40 = System.currentTimeMillis();
            SLPAw slpaw = new SLPAw(inputFileName, THRS, maxRun, maxT, outputDir, isUseLargestComp);
            slpaw.isUseLargestComp = Boolean.valueOf(false);
            slpaw.isSymmetrize = isSymmetrize;
            slpaw.WeigthVersion = WeigthVersion;
            slpaw.thr_commsize_min = thr_commsize_min;
            slpaw.thr_commsize_max = thr_commsize_max;
            slpaw.embslpaw_WeigthVersion = embslpaw_WeigthVersion;
            slpaw.embslpaw_loopcn = embslpaw_loopcn;
            slpaw.isWriteOvfile = isWriteOvfile;
            slpaw.writeNODECOMFlag = writeNODECOMFlag;
            slpaw.isOhis1 = isOhis1;
            slpaw.isOhis2 = isOhis2;
            slpaw.isOMem1 = isOMem1;
            slpaw.isRELESE = isRELESE;
            slpaw.EC = EC;
            slpaw.isWeighted = isWeighted;
            slpaw.ht_name_ID = ht_name_ID;
            slpaw.rndGlobal = rndGlobal;
            System.out.println("Info: Reading in your network......");
            slpaw.start();
            if (!isRELESE) {
                System.out.println("WeigthVersion=" + WeigthVersion);
                System.out.println("thr_commsize_min=" + thr_commsize_min);
                System.out.println("thr_commsize_max=" + thr_commsize_max);
                System.out.println("isWriteOvfile=" + isWriteOvfile);
            }

            if (isUsage) {
                printUsage(usage);
                printCitation();
            }

            System.out.println("Info: GANXiS " + version + " completed! Take :" + (double) (System.currentTimeMillis() - var40) / 1000.0D + " seconds.");
            System.out.println("      Random Seed:" + seed + " (you can save it to reproduce these results)");
        } else {
            System.err.println("ERROR: input network " + inputFileName + " not found!");
            System.out.println(usage);
            System.exit(1);
        }

    }

    public SLPAw embededSLPAw_on_OneCOM_start(double thrp, ArrayList<Long> oneComIDS, Hashtable<String, Long> ht_name_ID) {
        if (!this.isRELESE) {
            System.out.println(">>>>start embedded SLPAw....");
        }

        ArrayList embTHRS = new ArrayList();
        embTHRS.add(Double.valueOf(thrp));
        byte embmaxRun = 1;
        String emboutputDir = "";
        String inputFileName = this.fileName_net;
        SLPAw embslpaw = new SLPAw(inputFileName, embTHRS, embmaxRun, this.maxT, emboutputDir, this.isUseLargestComp.booleanValue());
        embslpaw.isUseLargestComp = this.isUseLargestComp;
        embslpaw.isSymmetrize = this.isSymmetrize;
        embslpaw.thr_commsize_min = this.thr_commsize_min;
        embslpaw.thr_commsize_max = this.thr_commsize_max;
        embslpaw.isRELESE = this.isRELESE;
        embslpaw.EC = this.EC;
        embslpaw.isWeighted = this.isWeighted;
        embslpaw.WeigthVersion = this.embslpaw_WeigthVersion;
        embslpaw.embslpaw_loopcn = this.embslpaw_loopcn;
        embslpaw.isWriteOvfile = false;
        embslpaw.writeNODECOMFlag = 0;
        embslpaw.isOhis1 = false;
        embslpaw.isOhis2 = false;
        embslpaw.isOMem1 = false;
        embslpaw.ht_name_ID = ht_name_ID;
        embslpaw.rndGlobal = this.rndGlobal;
        embslpaw.net.subNetIDSTable.clear();

        for (int i = 0; i < oneComIDS.size(); ++i) {
            Long ID = (Long) oneComIDS.get(i);
            embslpaw.net.subNetIDSTable.put(ID, ID);
        }

        embslpaw.isEmbeddedSLPAw = true;
        embslpaw.start();
        if (!this.isRELESE) {
            System.out.println("WeigthVersion=" + embslpaw.WeigthVersion);
            System.out.println("thr_commsize_min=" + embslpaw.thr_commsize_min);
            System.out.println("thr_commsize_max=" + embslpaw.thr_commsize_max);
            System.out.println("isWriteOvfile=" + embslpaw.isWriteOvfile);
            System.out.println("<<<<finish embedded SLPAw");
        }

        return embslpaw;
    }
}
