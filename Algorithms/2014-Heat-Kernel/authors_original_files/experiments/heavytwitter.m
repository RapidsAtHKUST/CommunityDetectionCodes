% nohup /p/matlab-7.14/bin/matlab -nodisplay -nodesktop -nojvm -nosplash -r heavytwitter > heavtwit.txt &

% hkgrow and pprgrow both took far too long on the largest degree nodes of twitter,
% so this file has them run, not on the 1000 largest degree nodes, but a "log uniform
% sampling" of the top 10% largest degree nodes. This starts with the largest degree node,
% then skips the next 2000 largest degree nodes, and repeats for 2000 total trials.

filename = 'twitterp';
dataname = 'hk';

output_directory = '../results/';
data_directory = '../data';

addpath('..') ;
load([data_directory filename]);


numtrials = 2000;

indices = zeros(numtrials,1);
times = zeros(numtrials,1);
conds = zeros(numtrials,1);
cuts = zeros(numtrials,1);
vols = zeros(numtrials,1);
setsizes = zeros(numtrials,1);
gsize = zeros(1,2);

gsize(1) = size(A,1); n =gsize(1);
gsize(2) = nnz(A);

fprintf('starting graph = %s\n', filename);

%heavyhood

for j=1:numtrials
 indices(j) = perms(1+(j-1)*2000);
end
 for trial_num=1:numtrials
 fprintf('hh trial=%i  \t',trial_num);
 tic; [dummy,conds(trial_num),cuts(trial_num),vols(trial_num)] = hkgrow(A,indices(trial_num),'neighborhood', true,'debug',false);
 times(trial_num) = toc;
 setsizes(trial_num) = min(length(dummy), n - length(dummy));
 end


avecond = sum(conds)./numtrials;
avetime = sum(times)./numtrials;
fprintf('\n avecond=%f  avetime=%f \n', avecond, avetime);

outputname = strcat(dataname,filename,'heavy');
save([output_directory outputname '.mat'], 'perms','dataname', 'gsize', 'setsizes',...
 'vols', 'cuts', 'indices', 'times', 'conds', 'filename','-v7.3');
exit;