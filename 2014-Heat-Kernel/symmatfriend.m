% nohup /p/matlab-7.14/bin/matlab -nodisplay -nodesktop -nojvm -nosplash -r symmatfriend > symmatfriend.txt &
A = load_graph('com-friendster.ungraph','/scratch/dgleich/friendster');
A=A|A';
save(['/scratch2/dgleich/kyle/symmats/' 'friendster' '.mat'], 'A','-v7.3');
exit;