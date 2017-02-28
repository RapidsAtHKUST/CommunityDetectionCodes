//
// Source code recreated from a .class file by IntelliJ IDEA
// (powered by Fernflower decompiler)
//

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.Iterator;

import org.apache.commons.collections.map.MultiKeyMap;

public class Net {
    public int N;
    public ArrayList<NODE> NODES = new ArrayList();
    public Hashtable<Long, NODE> NODESTABLE = new Hashtable();
    public String networkPath;
    public String netName;
    public MultiKeyMap EdgesTable = new MultiKeyMap();
    public Hashtable<Long, Long> subNetIDSTable = new Hashtable();
    public boolean isRELESE = false;
    static final Comparator<ArrayList<Long>> ComCPMsizedec = new Comparator<ArrayList<Long>>() {
        public int compare(ArrayList<Long> o1, ArrayList<Long> o2) {
            Integer i1 = Integer.valueOf(o1.size());
            Integer i2 = Integer.valueOf(o2.size());
            return i2.compareTo(i1);
        }
    };

    public Net(String networkPath, String netName) {
        this.networkPath = networkPath;
        this.netName = netName;
    }

    public void readNetwork_EdgesList(String fileName, boolean isUseLargestComp, boolean isSymmetrize, Hashtable<String, Long> ht, boolean isWeighted) {
        boolean isshow = true;
        if (!this.isRELESE) {
            System.out.println("reading network....");
        }

        this.EdgesTable.clear();
        if (!fileOpts.isFileExist(this.networkPath + fileName)) {
            System.err.println("networkPath=" + this.networkPath);
            System.err.println(this.networkPath + fileName + " not found(read)!");
            System.exit(1);
        }

        this.NODES.clear();
        this.NODESTABLE.clear();
        String delims = "[ \t]+";
        File file = new File(this.networkPath + fileName);
        BufferedReader reader = null;

        try {
            reader = new BufferedReader(new FileReader(file));
            String e = "";

            label200:
            while (true) {
                String fromID_s;
                String toID_s;
                Long fromID;
                Long toID;
                double w;
                do {
                    do {
                        do {
                            if ((e = reader.readLine()) == null) {
                                break label200;
                            }

                            e = e.trim();
                        } while (e.startsWith("#"));
                    } while (e.startsWith("%"));

                    String[] tokens = e.split(delims);
                    fromID_s = tokens[0].trim();
                    toID_s = tokens[1].trim();
                    fromID = CollectionFuns.Lookup_Hashtable_getputID_Str_Long(ht, fromID_s);
                    toID = CollectionFuns.Lookup_Hashtable_getputID_Str_Long(ht, toID_s);
                    w = 1.0D;
                    if (isWeighted && tokens.length == 3) {
                        w = Double.parseDouble(tokens[2].trim());
                    }

                    if (this.subNetIDSTable.isEmpty()) {
                        break;
                    }

                    if (isshow) {
                        if (!this.isRELESE) {
                            System.out.println("->readin subnet...");
                        }

                        isshow = false;
                    }
                } while (!this.subNetIDSTable.containsKey(fromID) || !this.subNetIDSTable.containsKey(toID));

                if (fromID != toID) {
                    this.pre_ReadInOneEdge(fromID, toID, fromID_s, toID_s);
                    this.EdgesTable.put(fromID, toID, Double.valueOf(w));
                    if (isSymmetrize) {
                        this.pre_ReadInOneEdge(toID, fromID, toID_s, fromID_s);
                        this.EdgesTable.put(toID, fromID, Double.valueOf(w));
                    }
                }
            }
        } catch (FileNotFoundException var28) {
            var28.printStackTrace();
        } catch (IOException var29) {
            var29.printStackTrace();
        } finally {
            try {
                if (reader != null) {
                    reader.close();
                }
            } catch (IOException var27) {
                var27.printStackTrace();
            }

        }

        this.pre_convert_nbSet2_nbList();
        this.N = this.NODES.size();
        if (isUseLargestComp) {
            if (!this.isRELESE) {
                System.out.println("Using largest connected component only(Only for unweighted undirected network)........\n");
            }

            this.post_UseLargestComponent();
        } else if (!this.isRELESE) {
            System.out.println("INFO: use all the network components.");
        }

    }

    public void post_UseLargestComponent() {
        new ArrayList();
        ArrayList cpm = new ArrayList();
        ArrayList oneCom = new ArrayList();

        for (int allcomponents_cpm = 0; allcomponents_cpm < this.NODES.size(); ++allcomponents_cpm) {
            oneCom.add(Long.valueOf(((NODE) this.NODES.get(allcomponents_cpm)).ID));
        }

        cpm.add(oneCom);
        ArrayList var9 = this.pre_findAllConnectedComponents_InOneCluster_CPM(cpm);
        Collections.sort(var9, ComCPMsizedec);
        ArrayList largestCom = (ArrayList) var9.get(0);
        ArrayList newNODES = new ArrayList();

        for (int i = 0; i < this.NODES.size(); ++i) {
            NODE v = (NODE) this.NODES.get(i);
            Long vid = Long.valueOf(v.ID);
            if (largestCom.contains(vid)) {
                newNODES.add(v);
            } else {
                this.NODESTABLE.remove(vid);
            }
        }

        this.NODES = newNODES;
        this.N = this.NODES.size();
    }

    public void pre_ReadInOneEdge(Long fromID, Long toID, String fromID_s, String toID_s) {
        NODE v;
        if (this.NODESTABLE.containsKey(fromID)) {
            ((NODE) this.NODESTABLE.get(fromID)).nbSet.add(toID);
            ((NODE) this.NODESTABLE.get(fromID)).nbSet_Out.add(toID);
        } else {
            v = new NODE(fromID);
            v.name = fromID_s;
            v.nbSet.add(toID);
            v.nbSet_Out.add(toID);
            this.NODES.add(v);
            this.NODESTABLE.put(fromID, v);
        }

        if (this.NODESTABLE.containsKey(toID)) {
            ((NODE) this.NODESTABLE.get(toID)).nbSet_In.add(fromID);
        } else {
            v = new NODE(toID);
            v.name = toID_s;
            v.nbSet_In.add(fromID);
            this.NODES.add(v);
            this.NODESTABLE.put(toID, v);
        }

    }

    public void pre_convert_nbSet2_nbList() {
        for (int i = 0; i < this.NODES.size(); ++i) {
            NODE v = (NODE) this.NODES.get(i);
            v.nbList = new ArrayList(v.nbSet);
            v.numNbs = v.nbList.size();
            Collections.sort(v.nbList);
            Long toID;
            if (v.nbSet_Out.size() > 0) {
                v.nbList_Out = new ArrayList(v.nbSet_Out);
                v.numNbs_Out = v.nbList_Out.size();
                Collections.sort(v.nbList_Out);
                toID = Long.valueOf(v.ID);
            }

            if (v.nbSet_In.size() > 0) {
                v.nbList_In = new ArrayList(v.nbSet_In);
                v.numNbs_In = v.nbList_In.size();
                Collections.sort(v.nbList_In);
                toID = Long.valueOf(v.ID);
                v.nbweightList_In.clear();
                Double sum = Double.valueOf(0.0D);

                int j;
                for (j = 0; j < v.nbList_In.size(); ++j) {
                    Long fromID = (Long) v.nbList_In.get(j);
                    Double w = (Double) this.EdgesTable.get(fromID, toID);
                    v.nbweightList_In.add(w);
                    sum = Double.valueOf(sum.doubleValue() + w.doubleValue());
                }

                v.nbweightList_In_norm.clear();

                for (j = 0; j < v.nbList_In.size(); ++j) {
                    v.nbweightList_In_norm.add(Double.valueOf(((Double) v.nbweightList_In.get(j)).doubleValue() / sum.doubleValue()));
                }
            }
        }

    }

    public void pre_convert_nbList2_nbSet() {
        for (int i = 0; i < this.NODES.size(); ++i) {
            NODE v = (NODE) this.NODES.get(i);
            v.nbSet = new HashSet(v.nbList);
        }

    }

    public ArrayList<ArrayList<Long>> pre_findAllConnectedComponents_InOneCluster_CPM(ArrayList<ArrayList<Long>> cpm) {
        ArrayList newcpm = new ArrayList();

        for (int i = 0; i < cpm.size(); ++i) {
            HashSet Com = new HashSet((Collection) cpm.get(i));

            while (Com.size() > 0) {
                HashSet exploredSet = new HashSet();
                HashSet unexploredSet = new HashSet();
                Long vid = this.getFirstElemnetInSet(Com);
                HashSet nbSet = ((NODE) this.NODESTABLE.get(vid)).nbSet;
                HashSet newnbSet = (HashSet) CollectionFuns.interSet(nbSet, Com);
                unexploredSet.addAll(newnbSet);
                Com.removeAll(newnbSet);
                exploredSet.add(vid);
                Com.remove(vid);

                while (unexploredSet.size() > 0) {
                    vid = this.getFirstElemnetInSet(unexploredSet);
                    nbSet = ((NODE) this.NODESTABLE.get(vid)).nbSet;
                    newnbSet = (HashSet) CollectionFuns.interSet(nbSet, Com);
                    unexploredSet.addAll(newnbSet);
                    Com.removeAll(newnbSet);
                    unexploredSet.remove(vid);
                    exploredSet.add(vid);
                }

                ArrayList oneComponent = new ArrayList(exploredSet);
                newcpm.add(oneComponent);
            }
        }

        Collections.sort(newcpm, ComCPMsizedec);
        if (newcpm.size() != cpm.size()) {
            if (!this.isRELESE) {
                System.out.println("before post_sameLabelDisconnectedComponents() K=" + cpm.size());
            }

            if (!this.isRELESE) {
                System.out.println("after post_sameLabelDisconnectedComponents() K=" + newcpm.size());
            }
        }

        return newcpm;
    }

    public Long getFirstElemnetInSet(HashSet<Long> set) {
        Long v = Long.valueOf(-1L);
        Iterator iter = set.iterator();
        if (iter.hasNext()) {
            v = (Long) iter.next();
        }

        return v;
    }

    public ArrayList<ArrayList<Long>> readCPM(String cpmPath, String fileName, Boolean isTrueCom) {
        ArrayList cpm = new ArrayList();
        if (!fileOpts.isFileExist(cpmPath + fileName)) {
            System.err.println("cpmPath=" + cpmPath);
            System.err.println(cpmPath + fileName + " not found(read)!");
            System.exit(1);
        }

        String delims = "[ \t]+";
        File file = new File(cpmPath + fileName);
        BufferedReader reader = null;

        try {
            reader = new BufferedReader(new FileReader(file));
            String e = "";
            long comInd = -1L;

            while (true) {
                do {
                    do {
                        if ((e = reader.readLine()) == null) {
                            return cpm;
                        }

                        e = e.trim();
                    } while (e.startsWith("#"));
                } while (e.startsWith("%"));

                ++comInd;
                ArrayList oneCom = new ArrayList();
                cpm.add(oneCom);
                String[] tokens = e.split(delims);

                for (int i = 0; i < tokens.length; ++i) {
                    Long nodeID = Long.valueOf(Long.parseLong(tokens[i].trim()));
                    oneCom.add(nodeID);
                    NODE v = (NODE) this.NODESTABLE.get(nodeID);
                    if (isTrueCom.booleanValue()) {
                        v.truecomTable.put(Long.valueOf(comInd), Double.valueOf(1.0D));
                    } else {
                        v.foundcomTable.put(Long.valueOf(comInd), Double.valueOf(1.0D));
                    }
                }
            }
        } catch (FileNotFoundException var26) {
            var26.printStackTrace();
        } catch (IOException var27) {
            var27.printStackTrace();
        } finally {
            try {
                if (reader != null) {
                    reader.close();
                }
            } catch (IOException var25) {
                var25.printStackTrace();
            }

        }

        return cpm;
    }

    public void write2Txt_WQ_nameS(String fileName, int st) {
        ArrayList data = new ArrayList();
        StringBuffer oneLine = new StringBuffer();
        Collections.sort(this.NODES, NODE.ComNodeIDInc);

        for (int i = 0; i < this.N; ++i) {
            NODE v = (NODE) this.NODES.get(i);
            oneLine.setLength(0);
            oneLine.append(v.name);

            for (int t = st; t < v.WQueue.size(); ++t) {
                oneLine.append(" ");
                oneLine.append(v.WQueue.get(t));
            }

            data.add(oneLine.toString());
        }

        fileOpts.writeToTxt(fileName, false, data);
    }

    public void applyEvolutionCutoff(int T, double EC) {
        int ect = -1;
        double cnT = this.calNumDistinctLabelAtT(T - 1);
        if (EC >= 0.0D) {
            if (EC < 1.0D) {
                System.out.println("EC should be larger than 1");
                System.exit(1);
            }

            System.out.println("applying evolution cutoff " + EC);

            for (int v = 0; v < T - 1; ++v) {
                if (this.calNumDistinctLabelAtT(v) <= EC * cnT) {
                    ect = v;
                    break;
                }
            }

            if (ect != -1) {
                for (int i = 0; i < this.NODES.size(); ++i) {
                    NODE var10 = (NODE) this.NODES.get(i);

                    for (int j = 0; j < ect; ++j) {
                        var10.WQueue.remove(0);
                    }
                }
            }

        }
    }

    public double calNumDistinctLabelAtT(int t) {
        double cn = 0.0D;
        Hashtable distTable = new Hashtable();

        for (int i = 0; i < this.NODES.size(); ++i) {
            NODE v = (NODE) this.NODES.get(i);
            distTable.put((Long) v.WQueue.get(t), Integer.valueOf(0));
        }

        cn = (double) distTable.size();
        return cn;
    }

    public static void main(String[] args) {
    }
}
