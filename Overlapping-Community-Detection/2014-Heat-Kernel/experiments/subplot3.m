function [a p]=subplot3(m,n,i,j,border,xpad,ypad)

if ~exist('border','var'), border=0.05; end
if ~exist('xpad','var'), xpad=0.5; end
if ~exist('ypad','var'), ypad=xpad; end

wf = (1-2*border)/n;
w = wf - xpad*wf/n;
hf = (1-2*border)/m;
h = hf - ypad*hf/m;
i=i-1; j=j-1;
p = [j*wf+border i*hf+border w h];
a=axes('position',p);