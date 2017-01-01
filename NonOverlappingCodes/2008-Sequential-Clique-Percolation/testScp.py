import os
executable="./k_clique"
tempDir="./"
testDataDir="testData/"

def listComp(l1,l2):
    if len(l1)!=len(l2):
        return cmp(len(l1),len(l2))    
    for e1,e2 in zip(l1,l2):
        if e1!=e2:
            return cmp(e1,e2)
    return 0

def compareCommunities(scpComms,cfComms):
    l1=[]
    l2=[]
    for line in open(scpComms,'r'):
        comms=map(int,line.split())
        comms.sort()
        if len(comms)>0:
            l1.append(comms)

    for line in open(cfComms,'r'):
        comms=map(int,line.split()[1:])
        comms.sort()
        l2.append(comms)
    
    l1.sort(cmp=listComp)
    l2.sort(cmp=listComp)

    l1=tuple(map(tuple,l1))
    l2=tuple(map(tuple,l2))
    #print l1
    #print l2
    return l1==l2


testNets=filter(lambda x:x.endswith(".edg"),os.listdir(testDataDir))

for net in testNets:
    print "Testing ",
    print net,
    print "..."

    #find the k-values
    kvalues=[]
    if os.path.exists(testDataDir+net+"_files/"):
        for file in os.listdir(testDataDir+net+"_files/"):
            if file.startswith("k="):
                kvalues.append(int(file[2:]))
    kvalues.sort()

    for k in kvalues:
        print "k="+str(k)+": ",
        #remove old temp file
        if os.path.exists(tempDir+"temp.txt"):
            os.remove(tempDir+ "temp.txt")

        #run the k-clique percolator
        list(os.popen(executable+ " "+testDataDir+net+" -k="+str(k)+" -o="+tempDir+"temp.txt"))

        if compareCommunities(tempDir+"temp.txt",testDataDir+net+"_files/k="+str(k)+"/communities"):
            print "OK."
        else:
            print "FAILED!"
            break



#remove old temp file
if os.path.exists(tempDir+"temp.txt"):
    os.remove(tempDir+ "temp.txt")
