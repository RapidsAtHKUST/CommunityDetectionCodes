function [center] = get_hrc_graclus_seeds(A,k,minSize)

ncs=2;
lvl=round(log2(k));
[prt] = hrcGraclus(A,ncs,lvl, minSize);
partition=prt{end};
[center] = ComputeCenterNodes(A, partition, 0);

end

