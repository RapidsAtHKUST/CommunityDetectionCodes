% nohup /p/matlab-7.14/bin/matlab -nodisplay -nodesktop -nojvm -nosplash -r trials_twitter > hktwit.txt &

filename = 'twitterp';

output_directory = '../results/';
data_directory = '../data/';

addpath('..') ;
load([data_directory filename]);

numtrials = 1000;

indices = zeros(numtrials,4);
times = zeros(numtrials,4);
conds = zeros(numtrials,4);
cuts = zeros(numtrials,4);
vols = zeros(numtrials,4);
setsizes = zeros(numtrials,4);
gsize = zeros(1,2);

gsize(1) = size(A,1); n =gsize(1);
gsize(2) = nnz(A);

fprintf('starting graph = %s\n', filename);

% randseed
etype = 1;
[setsizes(:,etype) vols(:,etype) cuts(:,etype) conds(:,etype) times(:,etype) indices(:,etype)] = randseed(A,numtrials);
fprintf('randseed done  \n');

% heavyseed
etype = 2;
indices(:,etype) = perms(1:numtrials);
for trial_num=1:numtrials
tic; [dummy,conds(trial_num,etype),cuts(trial_num,etype),vols(trial_num,etype)] = hkgrow(A,indices(trial_num,etype),'debug',false);
times(trial_num,etype) = toc;
setsizes(trial_num,etype) = min(length(dummy), n - length(dummy));
end
fprintf('heavyseed done  \n');

% randhood
etype = 3;
[setsizes(:,etype) vols(:,etype) cuts(:,etype) conds(:,etype) times(:,etype) indices(:,etype)] = randhood(A,numtrials);
fprintf('randhood done  \n');

%heavyhood
% etype = 4;
% indices(:,etype) = perms(1:numtrials);
% for trial_num=1:numtrials
% fprintf('hh trial=%i  \t',trial_num);
% tic; [dummy,conds(trial_num,etype),cuts(trial_num,etype),vols(trial_num,etype)] = hkgrow(A,indices(trial_num,etype),'neighborhood', true,'debug',false);
% times(trial_num,etype) = toc;
% setsizes(trial_num,etype) = min(length(dummy), n - length(dummy));
% end


avecond = sum(conds(:,1))./numtrials;
avetime = sum(times(:,1))./numtrials;
fprintf('\n avecond=%f  avetime=%f \n', avecond, avetime);
dataname = 'hk';
outputname = strcat(filename,'trials');
save([output_directory outputname '.mat'], 'dataname', 'gsize', 'setsizes', 'vols', 'cuts', 'indices', 'times', 'conds', 'filename','-v7.3');
exit;