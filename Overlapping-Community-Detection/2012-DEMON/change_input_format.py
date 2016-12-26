fin = open('/home/cheyulin/gitrepos/SocialNetworkAnalysis/Codes-Yche/karate_edges_input.csv')
fout = open('/home/cheyulin/gitrepos/SocialNetworkAnalysis/Codes-Yche/karate_edges_input_modified.csv', 'w')
for l in fin:
    l = l.rstrip().split(" ")
    tmpstr = l[0] + '|' + l[1] + '|1'
    fout.write(tmpstr + '\n')
