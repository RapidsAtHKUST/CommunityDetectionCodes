function [bestset,bestcond,bestcut,bestvol,hkvec,npushes] = hkgrow1(A,vert,varargin)
% HKGROW Grow a cluster around a vertex using a heatkernel-pagerank algorithm
%
% [bestset,cond,cut,vol,hkvec,npushes] = hkgrow1(A,vert)
% computes the vector hkvec = exp(t*P)*v using npushes number of adds where
% v is either a single column of the identity or a group of columns of the
% identity, and then extract a cluster. The algorithm uses various values of
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
p.addOptional('t',20);
p.addOptional('eps',1e-4);
p.parse(varargin{:});

debugflag = p.Results.debug;

if p.Results.neighborhood
    neighs = find(A(:,vert));
    vert = union(vert,neighs);
end

bestcond = Inf;
bestset = [];

if debugflag==1, fprintf('hkgrow1.m: Called hkgrow_mex on set of size=%i with t=%f  ;  eps=%f \n', ...
        numel(vert), t_vals(ei), eps_vals(ei)); end

[curset cond cut vol hkvec npushes] = hkgrow_mex(A, vert, p.Results.t, p.Results.eps, debugflag);

if debugflag==1, fprintf('hkgrow1.m: hkgrow_mex done on set of size=%i with t=%f  ;  eps=%f \n', ...
        numel(vert), t_vals(ei), eps_vals(ei)); end

% ripped code from hkgrow.m hence the strange extra work
% TODO elimiate this and simplify.
if cond < bestcond
    bestcond = cond;
    bestset = curset;
    bestvol = vol;
    bestcut = cut;
end
