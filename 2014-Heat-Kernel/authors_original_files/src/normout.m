function P = normout(A)
% NORMOUT Normalize the outdegrees of the matrix A.  
%
% P = normout(A)
%
%   P has the same non-zero structure as A, but is normalized such that the
%   sum of each row is 1, assuming that A has non-negative entries. 
%

%
% normout.m
% David Gleich
%
% Revision 1.00
% 19 Octoboer 2005
%

% compute the row-sums/degrees
d = full(sum(A,2));

% invert the non-zeros in the data
id = spfun(@(x) 1./x, d);

% scale the rows of the matrix
P = diag(sparse(id))*A;
