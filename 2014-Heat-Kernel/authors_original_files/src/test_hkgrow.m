function [time_hk cond_hk bestset_hk setup_time] = test_hkgrow(filename,numtrials,debugflag)
% [times conductances cut_sets setup_time] = TEST_HKGROW(filename,numtrials,debugflag)
% set debugflag to 1 to turn on messages in the matlab and mex code.
tic;

if nargin < 1,
filename = 'netscience-cc';
end

if nargin < 2,
numtrials = 1;
end

if nargin < 3,
debugflag = false;
end

assert(numtrials>=1, 'numtrials must be positive integer');


%A = load_graph(filename,'/data'); n = size(A,1);
A = load_graph(filename); n = size(A,1);
degrees = zeros(n,1);
for ind = 1:n
degrees(ind) = nnz(A(:,ind));
end
setup_time = toc;

if debugflag == 1,
fprintf('test_hkgrow: setup time=%f \n', setuptime);
end

%% First do random seeds
indices = randi(n,numtrials,1);

time_hk = zeros(numtrials,1);
cond_hk = zeros(numtrials,1);
bestset_hk = zeros(n,numtrials);

for trial_num=1:numtrials

if debugflag==1, fprintf('test_hkgrow:  start rand trial=%i \n', trial_num); end
[neighborhood, ~, ~] = find(A(:,indices(trial_num)));
tic; [dummy,cond_hk(trial_num),cut_hk,vol_hk] = hkgrow(A,neighborhood,'debug',debugflag);

if debugflag == 1, fprintf('test_hkgrow:  end rand trial=%i \n', trial_num); end

bestset_hk(1:size(dummy,1),trial_num) = dummy;
time_hk(trial_num) = toc;
end

