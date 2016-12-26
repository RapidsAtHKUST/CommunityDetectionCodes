
% first load heavytwitterp, or pprheavytwitterp



load ../results/hktwitterpheavy;

k = 1;

% size of cluster
clf;
plot(log10(setsizes(k:length(setsizes))),(conds(k:length(conds)) ),'r+','MarkerSize',5);

hold all;


load ../results/pprtwitterpheavy;

plot(log10(setsizes(k:length(setsizes))),(conds(k:length(conds)) ),'bo','Markersize',5);

xlabel('log10(clustersize)');
ylabel('conductance');
hleg = legend('hk','ppr','Location','Southeast');
xlim([0,5])
ylim([0.1,1]);
%%
hc = get(hleg,'Children');
set_figure_size([3,3]);
print(gcf,strcat('twitcluscond','.eps'),'-depsc2');

%%
clf; hold all;
load hktwitterpheavy;
ksdensity(conds,'support',[0,1])
load pprtwitterpheavy;
ksdensity(conds,'support',[0,1])
xlabel('conductance');
ylabel('density');
set(gca,'YTick',[]);
xlim([0,1])
hline = findobj(gcf,'type','line');
set(hline(1),'Color','b');
set(hline(2),'Color','r');
set(hline(1),'LineStyle','--');
legend('hk','ppr','Location','northwest');
legend boxoff;

set_figure_size([3,1]);
print(gcf,'twitter-cond-density.eps','-depsc2');