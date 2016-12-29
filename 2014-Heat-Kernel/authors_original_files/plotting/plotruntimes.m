% first load runtimes

load ../results/runtimes;
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

title('Runtime: hk vs. ppr');
xlabel('log10(|V|+|E|)');
ylabel('Runtime (s)');
legend(hs,'hkgrow 50%', '25%', '75%', 'pprgrow 50%', '25%', '75%');
legend boxoff;
xlim([4.5,9.75]);
set_figure_size([5,3]);
print(gcf,strcat('runtimes','.eps'),'-depsc2');