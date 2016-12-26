function [Acc,p,ci] = largest_component(A,sym)
% LARGEST_COMPONENT Return the largest connected component of A
%
% Acc = largest_component(A) returns the largest connected component
% of the graph A.  If A is directed, this returns the largest
% strongly connected component.
%
% Acc = largest_component(A,1) returns the largest connected piece of
% a directed graph where connectivity is undirected.  Algorithmically,
% this takes A, drops the directions, then components the largest component
% and returns just this piece of the original _directed_ network.  So the
% output Acc is directed in this case.
%
% [Acc,p,ci] = largest_component(A,...) also returns a logical vector
% indicating which vertices in A were chosen, along with the vector of
% connected components in the original graph.
%
% See also COMPONENTS
%
% Example:
%   load_mbgl_graph('dfs_example')
%   [Acc p] = largest_component(A); % compute the largest component
%   xy2 = xy(p,:); labels2 = labels(p); % get component metadata
%   % draw original graph
%   subplot(1,2,1); graph_draw(A,xy,'labels',labels); title('Original');
%   % draw component
%   subplot(1,2,2); graph_draw(Acc,xy2,'labels',labels2); title('Component');

% David F. Gleich
% Copyright, Stanford University, 2008-2009
% Copyright, 2011 Sandia National Labs

% History
% 2009-04-29: Initial coding
% 2011-03-11: Added to MatlabBGL
% 2011-10-06: Added component output

% TODO
% Add example to test cases

if ~exist('sym','var') || isempty(sym), sym=0; end

if sym
    As = A|A';
    [ci sizes] = components(As);
else
    [ci sizes] = components(A);
end
[csize cind] = max(sizes);
p = ci==cind;
Acc = A(p,p);
    
    
