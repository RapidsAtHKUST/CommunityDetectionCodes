% first load conductances

load ../results/conductances;

% newindexing is the subset we want
newindexing = [newindexing 13:15];

clf;
hold all;
hs = [];
colors = 'rb';
for id=1:2
    plotstr = [colors(id) 'o'];
    scatter(log10(inputsize),percdata(:,2,id),plotstr);
    %scatter(log10(inputsize),percdata(:,1,id),[colors(id) '.']);
    %scatter(log10(inputsize),percdata(:,3,id),[colors(id) '.']);

    subset = inputsize(newindexing);
    subperc = percdata(newindexing,:,:);
    [~,perm] = sort(subset);
    hs(end+1) = plot(log10(subset(perm)), subperc(perm,2,id),[colors(id) '.-']);
    hs(end+1) = plot(log10(subset(perm)), subperc(perm,1,id),[colors(id) '.--']);
    hs(end+1) = plot(log10(subset(perm)), subperc(perm,3,id),[colors(id) '.--']);
end
set(gca,'YScale','log');
ylim([5e-3,1]);

title('Conductances: hk vs. ppr');
xlabel('log10(|V|+|E|)');
ylabel('log10(Conductances)');
legend(hs,'hkgrow 50%', '25%', '75%', 'pprgrow 50%', '25%', '75%','Location','Southeast');
legend boxoff
xlim([4.5,9.75]);
set_figure_size([5,3]);
print(gcf,strcat('conductances','.eps'),'-depsc2');

