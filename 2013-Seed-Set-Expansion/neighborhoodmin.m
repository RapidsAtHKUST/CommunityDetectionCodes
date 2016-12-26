function [minverts,minvals] = neighborhoodmin(A,vals,strict)
% NEIGHBORHOODMIN Find extrema in a graph based on neighborhoods
%
% minverts = neighborhoodmin(A,vals) find a set of vertices where
% vals(i) <= vals(j) for all neighbors N(j)
% i.e. local minima in the space of the graph
%
% minverts = neighborhoodmin(A,vals,1) uses strict inequalities
%   vals(i) < vals(j) for all neighbors N(j)
% i.e. local minima in the space of the graph
%

if nargin<3, strict=0; end

if any(diag(A))
    A = A-diag(diag(A));
end

minverts = zeros(size(A,2),1);

for i=1:size(A,2)
    neigh = logical(A(:,i));
    neighmin = min(vals(neigh));
    if strict
        if vals(i) < neighmin
            minverts(i) = 1;
        end
    else
        if vals(i) <= neighmin
            minverts(i) = 1;
        end
    end
end

minverts = find(minverts);
if nargout>1
    minvals = vals(minverts);
end
    
    
