% nohup /p/matlab-7.14/bin/matlab -nodisplay -nodesktop -nojvm -nosplash -r pprtwitter > pprtwit.txt &
%

filename = 'twitterp';

data_directory = '../data/';
output_directory = '../results/';

addpath ..;

load([data_directory filename]);
load([data_directory filename 'trials']);

numtrials = size(indices,1);

times = zeros(numtrials,4);
conds = zeros(numtrials,4);
cuts = zeros(numtrials,4);
vols = zeros(numtrials,4);
setsizes = zeros(numtrials,4);
n = size(A,1);

fprintf('starting graph = %s\n', filename);

% randseed
etype = 1;
for trial_num=1:numtrials
tic; [dummy,conds(trial_num,etype),cuts(trial_num,etype),vols(trial_num,etype)] = pprgrow(A,indices(trial_num,etype));
times(trial_num,etype) = toc;
setsizes(trial_num,etype) = min(length(dummy), n - length(dummy));
end
fprintf('\t pprrandseed done\n');

% heavyseed
etype = 2;
for trial_num=1:numtrials
tic; [dummy,conds(trial_num,etype),cuts(trial_num,etype),vols(trial_num,etype)] = pprgrow(A,indices(trial_num,etype));
times(trial_num,etype) = toc;
setsizes(trial_num,etype) = min(length(dummy), n - length(dummy));
end
fprintf('\t pprheavyseed done\n');

% randhood
etype = 3;
for trial_num=1:numtrials
tic; [dummy,conds(trial_num,etype),cuts(trial_num,etype),vols(trial_num,etype)] = pprgrow(A,indices(trial_num,etype),'neighborhood',true);
times(trial_num,etype) = toc;
setsizes(trial_num,etype) = min(length(dummy), n - length(dummy));
end
fprintf('\t pprrandhood done\n');

% heavyhood
% etype = 4;
% for trial_num=1:numtrials
% tic; [dummy,conds(trial_num,etype),cuts(trial_num,etype),vols(trial_num,etype)] = pprgrow(A,indices(trial_num,etype),'neighborhood',true);
% times(trial_num,etype) = toc;
% setsizes(trial_num,etype) = min(length(dummy), n - length(dummy));
% end
% fprintf('\t pprheavyhood done\n');


avecond = sum(conds(:,1))./numtrials;
avetime = sum(times(:,1))./numtrials;
fprintf('avecond=%f  avetime=%f \n', avecond, avetime);

dataname = 'ppr';
outputname = strcat(dataname,filename,'trials');
save([output_directory outputname '.mat'], 'gsize', 'setsizes', 'vols', 'cuts',...
 'indices', 'times', 'conds', 'dataname', 'filename','-v7.3');
exit;