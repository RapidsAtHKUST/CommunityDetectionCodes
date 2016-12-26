%%
addpath ../.. % because normout.m and hkgrow1.m are used

%% Generate a network.
Nx = 512;
Ny = 512;
k = 0.22;
T = 10;
eta = 0.99;
Nc = 1000;

rng(0);
G = chirikov_operator(Nx,Ny,T,k,eta,Nc);

%%
A = G|G';

%% Compute PageRank on the standard map
n = size(G,1);
P = normout(A);
v = zeros(n,1);
v(randi(n,1)) = 1;
alpha = 0.85;
x = v;
for i=1:500 % sufficies for convergence
    x = alpha*(P'*x) + (1-alpha)*v;
end
x = x/sum(x);

%%
imagesc(reshape(log(x),Nx,Ny)'); colormap(1-hot);
axis tight;
axis square;
colorbar;


%% Compare
v = randi(n,1)
alpha1 = 0.85;
fval = @(x) exp(x)/x - 1/(1-alpha1);
t = fsolve(fval,-log(1-alpha1));
t = t;
%t = -2*log(1-alpha1)


subplot(2,1,1);
[bestsethk,hkcond,~,~,hkvec,~] = hkgrow1(A,v,'t',t);
hkvec = hkvec*exp(-t);
sethk = zeros(n,1); sethk(bestsethk) = 1;
imagesc(reshape(log(hkvec),Nx,Ny)'); colormap(1-hot);
axis tight;
axis square;
colorbar;

subplot(2,1,2);
[bestsetpr,prcond,~,~,prvec] = pprgrow_mex(A,v,100000,alpha1);
setpr = zeros(n,1); setpr(bestsetpr) = 1;
imagesc(reshape(log(prvec),Nx,Ny)'); colormap(1-hot);
axis tight;
axis square;
colorbar;

%%
subplot(2,1,1);
imagesc(reshape(sethk,Nx,Ny)');
axis tight;
axis square;
title(sprintf('hkcond = %f, nnz = %i', hkcond, nnz(sethk)));

subplot(2,1,2);
imagesc(reshape(setpr,Nx,Ny)');
axis tight;
axis square;
title(sprintf('prcond = %f, nnz=%i', prcond, nnz(setpr)));

%% Final figures
% There are all saved as images.
cmap = 1-hot(255);
v = 88183;
alpha1 = 0.85;
fval = @(x) exp(x)/x - 1/(1-alpha1);
tsol = fsolve(fval,-log(1-alpha1))
t = 3; 
assert(abs(t - tsol) < 0.25);

[bestsethk,hkcond,~,~,hkvec,~] = hkgrow1(A,v,'t',t);
hkvec = hkvec*exp(-t);
sethk = zeros(n,1); sethk(bestsethk) = 1;

[bestsetpr,prcond,~,~,prvec] = pprgrow_mex(A,v,100000,alpha1);
setpr = zeros(n,1); setpr(bestsetpr) = 1;

%% Write images
imwrite_scaled(reshape(log(x),Nx,Ny)',cmap,'chirikov-pr.png');
imwrite_scaled(reshape(log(hkvec+1e-8),Nx,Ny)',cmap,'chirikov-hk-vec.png');
imwrite_scaled(reshape(log(prvec+1e-10),Nx,Ny)',cmap,'chirikov-pr-vec.png');
imwrite_scaled(reshape(sethk,Nx,Ny)',cmap,'chirikov-hk-set.png');
imwrite_scaled(reshape(setpr,Nx,Ny)',cmap,'chirikov-pr-set.png');
sprintf('prcond = %f, nnz=%i', prcond, nnz(setpr))
sprintf('hkcond = %f, nnz = %i', hkcond, nnz(sethk))

%% Old experiments
% There were moved here after we decided on the final figures.
%% Compute PPR
alpha1 = 0.85;
[~,~,~,~,prvec] = pprgrow_mex(A,randi(n,1),100000,alpha1);
imagesc(reshape(log(prvec),Nx,Ny)'); colormap(hot);
axis tight;
axis square;
colorbar;

%%
[~,~,~,~,hkvec,~] = hkgrow1(A,randi(n,1),'t',30);
imagesc(reshape(log(hkvec),Nx,Ny)'); colormap(hot);
axis tight;
axis square;
colorbar;