% nohup /p/matlab-7.14/bin/matlab -nodisplay -nodesktop -nojvm -nosplash -r trials_friendster > hkfriend.txt &

filename = 'friendster';

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

gsize(1) = size(A,1);
gsize(2) = nnz(A);

fprintf('starting graph = %s\n', filename);

% randseed
etype = 1;
[setsizes(:,etype) vols(:,etype) cuts(:,etype) conds(:,etype) times(:,etype) indices(:,etype)] = randseed(A,numtrials);
fprintf('randseed done  \n');

% heavyseed
etype = 2;
[setsizes(:,etype) vols(:,etype) cuts(:,etype) conds(:,etype) times(:,etype) indices(:,etype)] = heavyseed(A,numtrials);
fprintf('heavyseed done  \n');

% randhood
etype = 3;
[setsizes(:,etype) vols(:,etype) cuts(:,etype) conds(:,etype) times(:,etype) indices(:,etype)] = randhood(A,numtrials);
fprintf('randhood done  \n');

%heavyhood
etype = 4;
[setsizes(:,etype) vols(:,etype) cuts(:,etype) conds(:,etype) times(:,etype) indices(:,etype)] = heavyhood(A,numtrials);

avecond = sum(conds(:,1))./numtrials;
avetime = sum(times(:,1))./numtrials;
fprintf('avecond=%f  avetime=%f \n', avecond, avetime);
dataname = 'hk';
outputname = strcat(filename,'trials');
save([output_directory outputname '.mat'], 'dataname', 'gsize', 'setsizes', 'vols', 'cuts', 'indices', 'times', 'conds', 'filename','-v7.3');
exit;