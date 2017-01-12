% nohup /p/matlab-7.14/bin/matlab -nodisplay -nodesktop -nojvm -nosplash -r experimentpprgrow > smallppr.txt &
% MUST COMPLETE experimenthkgrow.m BEFORE RUNNING THIS

addpath ..;

data_directory = '../data/';
output_directory = '../results/';
load([output_directory 'smalldata']);

numtrials = size(indices,2);
numfiles = numel(filename);
times = zeros(numfiles,numtrials,4);
conds = zeros(numfiles,numtrials,4);
cuts = zeros(numfiles,numtrials,4);
vols = zeros(numfiles,numtrials,4);
setsizes = zeros(numfiles,numtrials,4);

    fprintf('numtrials = %i  ,  number of datasets = %i \n', numtrials, numfiles);

for fileid=1:numfiles
    dataset = char(filename(fileid));
    fprintf('file number = %i  ,  graph = %s \n', fileid, dataset);
    A = load_graph(dataset);
    n = size(A,1);

% randseed
    etype = 1;
    for trial_num=1:numtrials
        tic; [dummy,conds(fileid,trial_num,etype),cuts(fileid,trial_num,etype),vols(fileid,trial_num,etype)] = pprgrow(A,indices(fileid,trial_num,etype));
        times(fileid,trial_num,etype) = toc;
        setsizes(fileid,trial_num,etype) = min(length(dummy), n - length(dummy));
    end
    fprintf('\t pprrandseed done\n');

% heavyseed
    etype = 2;
    for trial_num=1:numtrials
        tic; [dummy,conds(fileid,trial_num,etype),cuts(fileid,trial_num,etype),vols(fileid,trial_num,etype)] = pprgrow(A,indices(fileid,trial_num,etype));
        times(fileid,trial_num,etype) = toc;
        setsizes(fileid,trial_num,etype) = min(length(dummy), n - length(dummy));
    end
    fprintf('\t pprheavyseed done\n');

% randhood
    etype = 3;
    for trial_num=1:numtrials
        tic; [dummy,conds(fileid,trial_num,etype),cuts(fileid,trial_num,etype),vols(fileid,trial_num,etype)] = pprgrow(A,indices(fileid,trial_num,etype),'neighborhood',true);
        times(fileid,trial_num,etype) = toc;
        setsizes(fileid,trial_num,etype) = min(length(dummy), n - length(dummy));
    end
    fprintf('\t pprrandhood done\n');

% heavyhood
    etype = 4;
    for trial_num=1:numtrials
        tic; [dummy,conds(fileid,trial_num,etype),cuts(fileid,trial_num,etype),vols(fileid,trial_num,etype)] = pprgrow(A,indices(fileid,trial_num,etype),'neighborhood',true);
        times(fileid,trial_num,etype) = toc;
        setsizes(fileid,trial_num,etype) = min(length(dummy), n - length(dummy));
    end
    fprintf('\t pprheavyhood done\n');

clear A;
end

outputname = strcat('smallppr'); dataname = 'ppr';
save([output_directory outputname '.mat'], 'dataname', 'setsizes',...
 'vols', 'cuts', 'indices', 'times', 'conds', 'filename','-v7.3');
clear
exit;


