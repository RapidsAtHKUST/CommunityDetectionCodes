function [time_hk cond_hk bestset_hk setup_time] = test_hkgrow_mex(filename,numtrials,tol,alphat,debugflag)
% [times conductances cut_sets setup_time] = test_hkgrow_mex(filename,numtrials,tol,alphat,debugflag)
% set debugflag to 1 to turn on messages in the matlab and mex code.
tic;

% check inputs
if nargin < 1, filename = 'netscience-cc'; end
if nargin < 2, numtrials = 1; end
if nargin < 3, tol  = 1e-5; end
if nargin < 4, alphat = 1; end
if nargin < 5, debugflag = 0; end

assert(tol > 0 && tol <= 1, 'tol violates 0<tol<=1');
assert(alphat>0, 'alphat violates alphat>0');
assert(numtrials>=1, 'numtrials must be positive integer');

% setup inputs
A = load_graph(filename); n = size(A,1);
setup_time = toc;

if debugflag == 1, fprintf('test_hkgrow_mex: setup time=%f \n', setuptime); end

indices = randi(n,numtrials,1);
time_hk = zeros(numtrials,1);
cond_hk = zeros(numtrials,1);
bestset_hk = zeros(n,numtrials);

curexpand = 1000;

% Test on random seeds
for trial_num=1:numtrials

if debugflag==1, fprintf('test_hkgrow_mex:  start rand trial=%i \n', trial_num); end

    tic; [dummy cond_hk(trial_num) cut_hk vol_hk hkvec npushes] = hkgrow_mex(A,indices(trial_num),alphat, tol, debugflag);
    time_hk(trial_num) = toc;

if debugflag == 1, fprintf('test_hkgrow_mex:  end rand trial=%i \n', trial_num); end

    bestset_hk(1:size(dummy,1),trial_num) = dummy;
end

