function [setsizes vols cuts conds times indices] = randseed(A,numtrials)
% [setsizes vols cuts conds times indices] = randseed(A,numtrials)

n = size(A,1);

indices = randi(n,numtrials,1);
times = zeros(numtrials,1);
conds = zeros(numtrials,1);
cuts = zeros(numtrials,1);
vols = zeros(numtrials,1);
setsizes = zeros(numtrials,1);

for trial_num=1:numtrials
    tic; [dummy,conds(trial_num),cut(trial_num),vols(trial_num)] = hkgrow(A,indices(trial_num));
    times(trial_num) = toc;
    setsizes(trial_num) = min(length(dummy), n - length(dummy));
end
