function [Abc,p,Af,fcc] = biconncore(A,allones)
% this file is updated by David on Apr. 12, 2013
% BICONNCORE
%
% [Abc,p,Af,fcc] = biconncore(A)
%   Abc - the biconnected core 
%   p - the selector vector for the biconnected core
%   Af - the filtered graph with the biconnected core
%     seperated from the rest of the graph
%   fcc - the 
%
% The default option only removes edges in a biconnected component of 
% size one connected to the largest biconnected component.  
%
% ... biconncore(A,1) will also remove ANY biconnected component of 
% of size one.
%

if nargin < 2, allones = 0; end

n = size(A,1);
[a,C] = biconnected_components(A);
%[x,n] = dhist(nonzeros(C));
%[maxval,maxind] = max(n);
bccsizes=accumarray(nonzeros(C),1)/2;
[maxval maxind] = max(bccsizes);
%fprintf('Largest biconnected component: %i edges\n',maxval);
[i,j,c] = find(C);
edges = [i(c==maxind) j(c==maxind)];
biconverts = unique(edges(:));
bigbcc = zeros(n,1);
bigbcc(biconverts) = 1;
% find all bccs of size one.
sizeone = bccsizes==1;
% edges to remove are in a size one bcc and connected to the bigbcc
filt = sizeone(c) & (bigbcc(i) | bigbcc(j));
if allones
    filt = sizeone(c);
else     
    assert(~any(filt & (bigbcc(i) & bigbcc(j))));
end
% but we can't have any edge with both ends in the bcc
bccf = ~filt;

Af = sparse(i(bccf),j(bccf),1,size(A,1),size(A,2));
[Abc,p,fcc] = largest_component(Af);
