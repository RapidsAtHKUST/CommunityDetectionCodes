function [H] = growclusters(nworkers,A,method,seeds,varargin)
% GROWCLUSTERS Expand seed sets into clusters based on the PPR method
%
% [H,setdata] = growclusters(A,seeds,...)
% takes a vector of seeds, or a cell array of seed sets.
% as well as options for the ppr cluster routine
% and returns the best cluster found by exploring around
% the seed.  Note that this does not neessarily contain
% the seed itself.

n = size(A,1);
if ~iscell(seeds), seeds=num2cell(seeds); end
ns = numel(seeds);

%fprintf('Computing %sgrow clustering ... for %i seeds\n', method,ns);
H = sparse(n,ns);

%% single thread
if nworkers==1
    for i=1:ns
        seed = seeds{i};
        switch method
            case 'ppr'
                [curset,~,~,~,~] = pprgrow(A,seed,varargin{:});
            case 'vppr'
                [curset,~,~,~,~] = vpprgrow(A,seed,varargin{:});
            case 'hk'
                [curset,~,~,~,~] = hkgrow(A,seed);
            otherwise
                error('need a method name')
        end
        H(curset,i) = 1;
    end
%% multiple workers
else
    h = cell(ns,1);
    parpool(nworkers)
    parfor i=1:ns
        seed = seeds{i};
        switch method
            case 'ppr'
                [h{i},~,~,~,~] = pprgrow(A,seed,varargin{:});
            case 'vppr'
                [h{i},~,~,~,~] = vpprgrow(A,seed,varargin{:});
            case 'hk'
                [h{i},~,~,~,~] = hkgrow(A,seed);
            otherwise
                error('need a method name')
        end
    end
    delete(gcp);
    tic
    for i=1:ns
        H(h{i},i)=1;
    end
    toc
end
