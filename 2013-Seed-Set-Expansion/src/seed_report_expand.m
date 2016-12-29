function seeddata = seed_report_expand(nworkers,A,seeds,expand,method,varargin)
% seeddata = seed_report(A,seeds,expand,varargin);
%   if expand is true, then the seed sets are expanded via their
%     neighborhood

if ~exist('expand','var') || isempty(expand), expand = true; end

%if ~iscell(seeds), seeds=num2cell(seeds); end
eseeds = cell(numel(seeds),1);
for i=1:numel(seeds)
    si = seeds(i);
    if expand
        eseeds{i} = unique([si; find(A(:,si))]); % egonet
    else
        eseeds{i} = si; % only seed
    end 
end
%fprintf('seed_report... growclusters... start.. no. of egonets passed: %d\n',length(eseeds));
[H] = growclusters(nworkers,A,method,eseeds,varargin{:});
%fprintf('growclusters done...\n');

[~,uc] = unique(H','rows','first'); % unique communities
H = H(:,uc);
%fprintf('seed report done...');
seeddata.C = H;