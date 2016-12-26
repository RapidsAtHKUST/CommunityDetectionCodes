function [cond cut vol cc t teind]=triangleclusters(A)
% TRIANGLECLUSTERS Clustering metrics for clusters of vertex neighborhoods.
%
% This function studies clusters which are given by vertex neighborhoods.
% Let v be a vertex in a graph, then the cluster associated with v is just
% the set of all neighbors of v and v itself.  We return the clustering
% metrics associated with these clusters for all vertices in the grah.
%
% [cond cut vol cc t]=triangleclusters(A) returns 
%   
%   cond[uctance] of each cluster of a vertex neighborhood
%   cut of each cluster
%   vol[ume] of each cluster
%   cc - clustering coefficient of each vertex 
%   t - number of triangles centered at each vertex
%
% Example:
%   load_graph('dolphins');
%   d = sum(A,2);
%   cond = triangleclusters(A);
%   plot(d,cond,'.'); % a fake network community plot

% David F. Gleich
% Copyright, Purdue University, 2011

% History
% 2011-10-05: Based on clustercoeffs.m from gaimc

try
     [cond cut vol cc t teind] = triangleclusters_mex(A);
     return;
catch me
    if isempty(me.identifier),
        rethrow(me)
    end
    warning('triangleclusters:mexFailed','this could be slow');
end

usew = 0;

if isstruct(A)
    rp=A.rp; ci=A.ci; %ofs=A.offset;
else
    if ~isequal(A,A'), error('gaimc:triangleclusters',...
            'only undirected (symmetric) inputs allowed: see dirclustercoeffs');
    end
    [rp ci]=sparse_to_csr(A); 
end
n=length(rp)-1;

Gvol = 0;
cc=zeros(n,1); ind=false(n,1); t=zeros(n,1); 
cut=zeros(n,1); vol=zeros(n,1); 
for v=1:n
    % index the neighbors
    for rpi=rp(v):rp(v+1)-1
        w=ci(rpi); 
        if v~=w, ind(w)=1; end
    end
    curcc=0; d=rp(v+1)-rp(v);
    % run two steps of bfs to try and find triangles and cuts
    curvol = 0;
    curcut = 0;
    for rpi=rp(v):rp(v+1)-1
        w=ci(rpi); if v==w, d=d-1; continue; end % discount self-loop
        curvol = curvol+1;
        Gvol = Gvol+1;
        for rpi2=rp(w):rp(w+1)-1
            x=ci(rpi2); if x==w, continue; end
            curvol = curvol+1;
            if ind(x)
                curcc=curcc+1;
            else
                if x~=v, curcut=curcut+1; end
            end
        end
    end
    
    vol(v) = curvol;
    cut(v) = curcut;
    if d>1, cc(v) = curcc/(d*(d-1)); 
    else cc(v) = 0; 
    end
    t(v) = curcc/2;
    for rpi=rp(v):rp(v+1)-1, w=ci(rpi); ind(w)=0; end % reset indicator
end

cond = cut./min(vol,Gvol-vol);


