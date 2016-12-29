function [C] = nise(A, k, seeding, ego, expansion, nworkers, fid)
%% Overlapping Community Detection Using Seed Set Expansion,
%% J. J. Whang, D. F. Gleich, and I. S. Dhillon,
%% ACM International Conference on Information and Knowledge Management (CIKM), October 2013.
%% Contact: Joyce Whang, The University of Texas at Austin, joyce@cs.utexas.edu

% A: adjacency matrix
% k: the number of communities
% seeding: seeding strategy ('hrc_graclus' or 'sphub')
% ego: neighborhood inflation (true or false)
% expansion: expansion method ('ppr' or 'vppr')
% nworkers: the number of threads (parallel expansion of the seeds)
% fid: output file ID
% C: assignment matrix (no. of nodes x no. of clusters)

addpath('./GraclusSeeds/graclus1.2/matlab');
addpath('./matlab_bgl');

assert(nnz(diag(A))==0,'Diagonal entries should be zeros.');
if nargin<3
    seeding='sphub';
end
if nargin<4
    ego=true;
end
if nargin<5
    expansion='ppr';
end
if nargin<6
    nworkers=1;
end
if nargin<7
   fid = fopen('nise_log.txt','w'); 
end

time_total = tic;
%% generate bicore
time_prep = tic;
[~,pdata] = graphprep(A); % A: input graph
fprintf(fid,'--- filtering phase: %0.2f seconds\n',toc(time_prep));
fprintf('filtering phase done...\n');

%% graph type
G = pdata.bicore; % bicore graph

%% seeding 
runs = 13;
time_seeding = tic;
if strcmp(seeding, 'hrc_graclus')
    minSize=20;
    [center] = get_hrc_graclus_seeds(G,k,minSize);
    seeds=center;
elseif strcmp(seeding, 'sphub')
    seeds=spHubSeeds(G,k);
else
    error('Unknown seeding!');
end
fprintf(fid,'--- seeding phase: %0.2f seconds\n',toc(time_seeding));
fprintf('seeding phase done...\n');

%% expand
time_expand = tic;
seeddata = seed_report_expand(nworkers,G,seeds,ego,expansion,'nruns',runs,'maxexpand',nnz(G)/2);
fprintf(fid,'--- expansion phase: %0.2f seconds\n',toc(time_expand));
fprintf('expansion phase done...\n');

%% Assign remaining nodes
time_post = tic;
[C_full] = Assign_bi(A,G,seeddata,pdata);
fprintf(fid,'--- propagation phase: %0.2f seconds\n',toc(time_post));
fprintf(fid,'------ total run time: %0.2f seconds\n',toc(time_total));
fprintf('propagation phase done...\n');

%% Flip assignment for super large clusters
[flipped_C] = flip_C(C_full',A);
C = flipped_C;
fprintf(fid, 'returned no. of clusters: %d, graph coverage: %3.2f (%%) \n', size(C,2), nnz(sum(C,2))/size(C,1)*100);

end
