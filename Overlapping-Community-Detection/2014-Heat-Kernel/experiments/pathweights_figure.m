%% Create the pathweights figure for the paper
% The point of this figure is to illustrate the difference between the path
% weighting in PageRank and in the heat-kernel as we vary alpha and t.

clf; hold all;

% Draw two alphas
legtext = {};
alphas = [0.85,0.99];
ts = [1, 5, 15];


for ai=1:numel(alphas)
    alpha = alphas(ai);

    prweight0 = 1-alpha;

    ks = 100;
    prweight = zeros(ks,1);
    prweight(1) = prweight0;
    for k=2:ks
        prweight(k) = alpha*prweight(k-1);
    end
    plot(prweight,'b--','LineWidth',1.2,'Color',[0,0,0.8]);
    legtext{end+1} = sprintf('pr - alpha=%.2f',alpha);
end

for ti=1:numel(ts)
    t = ts(ti);
    hkweight0 = exp(-t);
    hkweight = zeros(ks,1);
    hkweight(1) = hkweight0;
    for k=2:ks
        hkweight(k) = t/(k-1)*hkweight(k-1);
    end
    plot(hkweight,'r-','LineWidth',1.2,'Color',[0.8,0,0]);
    legtext{end+1} = sprintf('hk - t=%g',t);
end
text(10,1e-7,'t=1','VerticalAlignment','bottom','HorizontalAlignment','right');
text(20,1e-7,'t=5','VerticalAlignment','bottom','HorizontalAlignment','right');
text(38,1e-7,'t=15','VerticalAlignment','bottom','HorizontalAlignment','right');
text(87,1e-7,'\alpha=0.85','VerticalAlignment','bottom');
text(87,5e-3,'\alpha=0.99','VerticalAlignment','bottom');
ylabel('Weight');
xlabel('Length');
set(gca,'YScale','log');
set(gca,'LineWidth',0.6);
ylim([1e-8,1]);
%legend(legtext{:});

set_figure_size([4,2]);
print('pathweights.eps','-depsc2');