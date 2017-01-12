% nohup /p/matlab-7.14/bin/matlab -nodisplay -nodesktop -nojvm -nosplash -r symmattwit > symmattwit.txt &
A = load_graph('twitter-2010','/scratch/dgleich/kyle/data');
A=A|A';
save(['/scratch2/dgleich/kyle/symmats/' 'twitter' '.mat'], 'A','-v7.3');
exit;