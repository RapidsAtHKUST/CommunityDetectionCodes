function [] = displayLatex(distances,distRatio,prt)

fprintf('===== distances =====\n');
for i=1:length(distances)
    fprintf('%d & %d & %1.6f & %1.6f & %1.6f & %1.6f \\\\ \\hline \n'...
        ,i,prt(i),distances(1,i),distances(2,i),distances(3,i),distances(4,i));
end


fprintf('\n\n===== distRatio =====\n');
for i=1:length(distRatio)
    fprintf('%d & %d & %1.6f & %1.6f & %1.6f & %1.6f \\\\ \\hline \n'...
        ,i,prt(i),distRatio(1,i),distRatio(2,i),distRatio(3,i),distRatio(4,i));
end

end

