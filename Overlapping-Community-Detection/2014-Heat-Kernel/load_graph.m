function [A varargout]=load_graph(graphname,path)
% LOAD_GRAPH Loads a graph from the data directory
%
% load_graph is a helper function to load a graph provided with the
% regardless of the current working directory.  
%
% Example:
%   A = load_graph('cond-mat-2005-fix-cc');
%
% If you call it with two outputs ...
%   [A,xy] = load_graph('...')
% then it will load a coordinates file if it exists, or compute one
% via a fructerman_reingold_force_directed_layout (that requires MatlabBGL)

% David F. Gleich
% Copyright, Purdue University, 2013

% History
% 2011-10-02: Initial coding based on load_gaimc_graph
% 2013-01-28: Added option to load coordinates
if nargin < 2
	path=fileparts(mfilename('fullpath'));
	A=readSMAT(fullfile(path,'data',[graphname '.smat']));
	A=spones(A); % todo, make this an option?
	A = A - diag(diag(A));
	return;
end	

%	path=fileparts(mfilename('fullpath'));
	A=readSMAT(fullfile(path,[graphname '.smat']));
	A=spones(A); % todo, make this an option?
	A = A - diag(diag(A));

if nargout > 1
    xyfile = fullfile(path,'data', [graphname '.xy']);
    if exist(xyfile, 'file')
        varargout{1} = load(xyfile);
    else
        %varargout{1} = fruchterman_reingold_force_directed_layout(A);
        varargout{1} = kamada_kawai_spring_layout(A);
    end
end
