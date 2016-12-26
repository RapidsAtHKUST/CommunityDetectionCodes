

function [cond_hk time_hk setuptime] = trials_hkgrow(filename,numtrials,saveasname,tol,alphat)
% setuptime = trials_hkgrow(filename,numtrials,saveasname,tol,alphat)

output_directory = '../results/';
data_directory = '../data';

addpath('..') ;

% col stochastic version
% A = load_graph(filename,'~/data'); P = colnormout(A); clear A; n = size(P,1);

% row stochastic version
tic;

A = load_graph(filename); n = size(A,1);
degrees = zeros(n,1);
for ind = 1:n
    degrees(ind) = nnz(A(:,ind));
end
P = A; %normout(A); clear A;

setuptime = toc;

if (nargin<2)
    numtrials = 5;
    tol = 1e-5;
    alphat = 1;
    saveasname = 'test';
end

if (nargin<4)
    tol = 1e-5;
    alphat = 1;
end
                                                  
                                                  %% First do random seeds
indices = randi(n,numtrials,1);

time_hk = zeros(numtrials,1);
cond_hk = zeros(numtrials,1);
bestset_hk = zeros(n,numtrials);

%    fprintf('seeds set\n');

for trial_num=1:numtrials

%fprintf('start rand trial=%i \n', trial_num);
tic; [dummy,cond_hk(trial_num),cut_hk,vol_hk] = hkgrow(P,indices(trial_num),tol,alphat);
%fprintf('end rand trial=%i \n', trial_num);
    bestset_hk(1:size(dummy,1),trial_num) = dummy;
    time_hk(trial_num) = toc;
end

outputname = strcat(filename,'-',saveasname,'-randseed-hkgrow');
 save([output_directory outputname '.mat'], 'indices', 'time_hk', 'cond_hk', 'bestset_hk' ,'-v7.3');

%% Now do degree sorted seeds

[vals, perm] = sort(degrees,'descend');

%    fprintf('heavy seeds \n');

indices = perm(1:numtrials);

time_hk = zeros(numtrials,1);
cond_hk = zeros(numtrials,1);
bestset_hk = zeros(n,numtrials);
for trial_num=1:numtrials
%fprintf('start heavy trial=%i \n', trial_num);
    tic; [dummy,cond_hk(trial_num),cut_hk,vol_hk] = hkgrow(P,indices(trial_num),tol,alphat);
%fprintf('end rand trial=%i \n', trial_num);
    bestset_hk(1:size(dummy,1),trial_num) = dummy;
    time_hk(trial_num) = toc;
end

outputname = strcat(filename,'-',saveasname,'-heavyseed-hkgrow');
save([output_directory outputname '.mat'], 'indices', 'time_hk', 'cond_hk', 'bestset_hk' ,'-v7.3');
