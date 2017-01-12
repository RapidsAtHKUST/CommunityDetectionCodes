function [bestset,bestcond,bestcut,bestvol] = hkgrow(A,vert,varargin)
% HKGROW Grow a cluster around a vertex using a heatkernel-pagerank algorithm
%
% [bestset,cond,cut,vol] = hkgrow(A,vert) computes exp(t*P)*v where
% v is either a single column of the identity or a group of columns of the
% identity and then extract a cluster. The algorithm uses various values of
% t and returns the best conductance cluster among any of them. 
%
% ... hkgrow(A,verts,'key',value,'key',value) specifies optional argument
%
%    'neighborhood' : [false | true] to use the neighborhood of the given
%    vertex as the seed. The default is false.
%
%    'debug' : [false | true] to enable debugging info. The default is
%    false.

% Kyle Kloster
% Purdue University, 2014

p = inputParser;
p.addOptional('debug',false,@islogical);
p.addOptional('neighborhood',false,@islogical);
p.parse(varargin{:});

debugflag = p.Results.debug;

t_vals = [10 20 40 80];
eps_vals = [1e-4 1e-3 5*1e-3 1e-2];


if p.Results.neighborhood
    neighs = find(A(:,vert));
    vert = union(vert,neighs);
end

bestcond = Inf;
bestset = [];
for ei=1:numel(t_vals)

    if debugflag==1, fprintf('hkgrow.m: Called hkgrow_mex on set of size=%i with t=%f  ;  eps=%f \n', ...
            numel(vert), t_vals(ei), eps_vals(ei)); end

    [curset cond cut vol hk npushes] = hkgrow_mex(A, vert, t_vals(ei), eps_vals(ei), debugflag);

    if debugflag==1, fprintf('hkgrow.m: hkgrow_mex done on set of size=%i with t=%f  ;  eps=%f \n', ...
            numel(vert), t_vals(ei), eps_vals(ei)); end

    if cond < bestcond
        bestcond = cond;
        bestset = curset;
        bestvol = vol;
        bestcut = cut;
    end
end