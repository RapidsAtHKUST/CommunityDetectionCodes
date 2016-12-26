


#version 2.4, September 28, 2011

Main_folder=Sources_2_5



source_folder=$Main_folder/OSLOM_files
visual_folder=$Main_folder/visualSources


echo "Compiling OSLOM undirected (oslom_undir) ..."
echo "g++ -o oslom_undir $source_folder/main_undirected.cpp -O3 -Wall"
g++ -o oslom_undir $source_folder/main_undirected.cpp -O3 -Wall

echo ""
echo "Compiling OSLOM directed (oslom_dir) ..."
echo "g++ -o oslom_dir $source_folder/main_directed.cpp -O3 -Wall"
g++ -o oslom_dir $source_folder/main_directed.cpp -O3 -Wall


echo ""
echo "Compiling program to write pajek format (pajek_write_undir) ..."
echo "g++ -o pajek_write_undir $visual_folder/main_pajek.cpp -O3"
g++ -o pajek_write_undir $visual_folder/main_pajek.cpp -O3


echo ""
echo "Compiling program to write pajek format (pajek_write_dir) ..."
echo "g++ -o pajek_write_dir $visual_folder/main_pajek_directed.cpp -O3"
g++ -o pajek_write_dir $visual_folder/main_pajek_directed.cpp -O3




echo ""
echo "Compiling infomap_undirected ..."
cd $Main_folder/infomap_undir/
make clean
make
g++ -o infomap_scr infomap_scr.cpp -O3
cd ../..
mv $Main_folder/infomap_undir/infomap infomap_undir
mv $Main_folder/infomap_undir/infomap_scr infomap_undir_script


echo ""
echo "Compiling infomap_directed ..."
cd $Main_folder/infomap_dir/
make clean
make
g++ -o infomap_scr infomap_scr.cpp -O3
cd ../..
mv $Main_folder/infomap_dir/infomap infomap_dir
mv $Main_folder/infomap_dir/infomap_scr infomap_dir_script


echo ""
echo "Compiling louvain  method ..."
cd $Main_folder/louvain/
g++ script_to_compile.cpp
./a.out
g++ -o louvain_script order.cpp -O3
cd ../..
mv $Main_folder/louvain/louvain_script .
mv $Main_folder/louvain/community .
mv $Main_folder/louvain/convert .
mv $Main_folder/louvain/hierarchy .





echo ""
echo "***********************************************"
echo "type \"./oslom_undir\" (or \"./oslom_dir\") to check if the program is compiled and works"

