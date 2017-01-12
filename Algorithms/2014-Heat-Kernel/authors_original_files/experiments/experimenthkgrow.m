% BEFORE RUNNING, check "output_directory" and "data_directory" to make sure they're accurate
%
% nohup /p/matlab-7.14/bin/matlab -nodisplay -nodesktop -nojvm -nosplash -r experimenthkgrow > exphk.txt &


output_directory = '../results/';
data_directory = '../data';

addpath('..') ;

filename={'pgp-cc', 'ca-AstroPh-cc', 'marvel-comics-cc', 'as-22july06', 'rand-ff-25000-0.4',...
 'cond-mat-2003-cc', 'email-Enron-cc','cond-mat-2005-fix-cc', 'soc-sign-epinions-cc',...
  'itdk0304-cc', 'dblp-cc', 'flickr-bidir-cc'};
numtrials = 200;


%   'indices(i,j,k)' contains the seed node's index for graph i during trial j and experiment type j
%   'cuts' contains the number of boundary edges in the cut of best conductance
%   'setsizes' contains the number of vertices on the small side of the cut of best conductance
%   'vols' contains the edges inside the cut of best conductance

indices = zeros(numel(filename),numtrials,4);
times = zeros(numel(filename),numtrials,4);
conds = zeros(numel(filename),numtrials,4);
cuts = zeros(numel(filename),numtrials,4);
vols = zeros(numel(filename),numtrials,4);
setsizes = zeros(numel(filename),numtrials,4);

gsize = zeros(numel(filename),2);

for fileid=1:numel(filename)
    dataset = char(filename(fileid));
    fprintf('file number = %i  ,  graph = %s \n', fileid, dataset);
    A = load_graph(dataset);

    gsize(fileid,1) = size(A,1);
    gsize(fileid,2) = nnz(A);

        % randseed
        etype = 1;
            [setsizes(fileid,:,etype) vols(fileid,:,etype) cuts(fileid,:,etype)...
             conds(fileid,:,etype) times(fileid,:,etype) indices(fileid,:,etype)] = randseed(A,numtrials);
        fprintf('\t randseed done\n');

        % heavyseed
        etype = 2;
        [setsizes(fileid,:,etype) vols(fileid,:,etype) cuts(fileid,:,etype)...
         conds(fileid,:,etype) times(fileid,:,etype) indices(fileid,:,etype)] = heavyseed(A,numtrials);
        fprintf('\t heavyseed done\n');

        % randhood
        etype = 3;
        [setsizes(fileid,:,etype) vols(fileid,:,etype) cuts(fileid,:,etype)...
         conds(fileid,:,etype) times(fileid,:,etype) indices(fileid,:,etype)] = randhood(A,numtrials);
        fprintf('\t randhood done\n');

        % heavyhood
        etype = 4;
        [setsizes(fileid,:,etype) vols(fileid,:,etype) cuts(fileid,:,etype)...
         conds(fileid,:,etype) times(fileid,:,etype) indices(fileid,:,etype)] = heavyhood(A,numtrials);
        fprintf('\t heavyhood done\n');
    clear A;
end

outputname = strcat('smalldata'); dataname = 'hk';
save([output_directory outputname '.mat'], 'dataname', 'gsize', 'setsizes', 'vols',...
 'cuts', 'indices', 'times', 'conds', 'filename','-v7.3');
clear
exit;

