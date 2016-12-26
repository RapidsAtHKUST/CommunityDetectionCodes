function [B,data] = graphprep(A)
% This file is updated by Joyce on Apr. 25, 2013
% only considering biconnected core, i.e., only removing bridges

% GRAPHPREP Clean up a graph before running clustering procedures.
%
% 1. Remove the 1-core from the graph
% 2. Remove the biconnected whiskers from what's left.
% 3. See how large the bigcore that's left is...
% 4. Remove the two-core from the bicore
% 5. 
%
% [B,data] = graphprep(A);
%
% returns B, the biconnected core of the graph after removing the one
% core, and separating the largest biconnected component from what's left.
%
% data.original = starting graph A
% data.core_numbers = core numbers of A
% data.two_core_filter = indicator over vertices in the two core
% data.two_core = the graph restricted to the largest two core
% data.bicore = the two core, further restricted to the largest biconnected
%   core
% data.bicore_filter = indicator over data.two_core graph for the largest
%   biconnected core
% data.bicore_separated = the two core, where each edge connecting the
%   largest biconnected core to the rest of the graph ahs been removed
% data.bicore_components = the connected ccomponent map for the graph
%   where the bicore has been separated
% data.bicore_numbers = the core numbers of the bicore
% data.bicore3_filter = the filter for the 3-core of the bicore
% data.bicore3 = the 3-core of the bicore


% %% compute core numbers
% cn = core_numbers(A);
% data.original = A;
% data.core_numbers = cn;
% 
% %% form 2-core
% data.two_core_filter = cn > 1;
% A2 = A(data.two_core_filter,data.two_core_filter);
% data.two_core = A2;

%% remove all the bridges
[data.bicore data.bicore_filter data.bicore_separated data.bicore_components] ...
    = biconncore(A,1);
B = data.bicore;

% %% form 3-core
% data.bicore_numbers = core_numbers(B);
% data.bicore3_filter = data.bicore_numbers >= 3;
% data.bicore3 = B(data.bicore3_filter, data.bicore3_filter);
% B = data.bicore3;

%% now build the map back to the original graph
vid = 1:size(A,1);
% vid = vid(data.two_core_filter); data.two_core_vid = vid;
vid = vid(data.bicore_filter); data.bicore_vid = vid;
data.vid = vid;
% vid = vid(data.bicore3_filter); data.bicore3_vid = vid;

% TODO, eliminate the bicore3 vertices properly by leaving in a single
% connection for 
