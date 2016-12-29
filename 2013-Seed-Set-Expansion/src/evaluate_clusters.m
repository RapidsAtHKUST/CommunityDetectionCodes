function setdata=evaluate_clusters(A,C)
% EVALUATE_CLUSTERS Evaluate clustering metrics for a set of clusters.
% 
% data = evaluate_clusters(A,C) uses the columns of C as indicator vectors
% for the clusters.
% data = evaluate_clusters(A,map) uses a partition vector to define the
% clusters instead.
%
% data.C the cluster indicator matrix
% data.cond the conductance of each cluster
% data.vol the volume of each cluster
% data.size the size of each cluster
% data.cut the cut of each cluster
% 
%

n = size(A,1);

assert(size(C,1) == n);

if size(C,2) == 1
    % this is a partition vector
    p = C;
    cids = unique(p);
    nclusters = numel(cids);
    C = sparse([],[],[],n,nclusters,n); % preallocate n non-zeros
    for i=1:nclusters
        C(:,i) = p==cids(i);
    end
end 

nsets = size(C,2);
    
setdata = struct();
setdata.cond = zeros(nsets,1);
setdata.cut = zeros(nsets,1);
setdata.vol = zeros(nsets,1);
setdata.size = zeros(nsets,1);

for si=1:nsets
    curset = find(C(:,si));
    [setdata.cond(si) setdata.cut(si) setdata.vol(si)] = ...
        cutcond_mex(A,curset,nnz(A));
    setdata.vol(si) = min(nnz(A)-setdata.vol(si),setdata.vol(si));
    setdata.size(si) = min(size(A,1) - numel(curset),numel(curset));
end
%[ignore p] = sort(setdata.cond,'descend');
[ignore p] = sort(setdata.cond); 
setdata.cond = setdata.cond(p);
setdata.cut = setdata.cut(p);
setdata.vol = setdata.vol(p);
setdata.size = setdata.size(p);
setdata.C = C(:,p);

setdata = add_extra_community_data(setdata.C, setdata);
