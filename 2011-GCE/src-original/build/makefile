communityfinder: 
	g++ -O3 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"Seed.d" -MT"Seed.d" -o"Seed.o" "../Seed.cpp"
	g++ -O3 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"Community_Finder.d" -MT"Community_Finder.d" -o"Community_Finder.o" "../Community_Finder.cpp"
	g++ -O3 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"aaron_utils.d" -MT"aaron_utils.d" -o"aaron_utils.o" "../aaron_utils.cpp"
	g++ -O3 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"cliques.d" -MT"cliques.d" -o"cliques.o" "../cliques.cpp"
	g++ -O3 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"find_communities.d" -MT"find_communities.d" -o"find_communities.o" "../find_communities.cpp"
	g++ -O3 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"graph_loading.d" -MT"graph_loading.d" -o"graph_loading.o" "../graph_loading.cpp"
	g++ -O3 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"graph_representation.d" -MT"graph_representation.d" -o"graph_representation.o" "../graph_representation.cpp"
	g++  -o "GCECommunityFinder"  ./Community_Finder.o ./Seed.o ./aaron_utils.o ./cliques.o ./find_communities.o ./graph_loading.o ./graph_representation.o   

profile:
	g++ -p -O3 -Wall -c -MMD -MP -MF"Seed.d" -MT"Seed.d" -o"Seed.o" "../Seed.cpp"
	g++ -p -O3 -Wall -c -MMD -MP -MF"Community_Finder.d" -MT"Community_Finder.d" -o"Community_Finder.o" "../Community_Finder.cpp"
	g++ -p -O3 -Wall -c -MMD -MP -MF"aaron_utils.d" -MT"aaron_utils.d" -o"aaron_utils.o" "../aaron_utils.cpp"
	g++ -p -O3 -Wall -c -MMD -MP -MF"cliques.d" -MT"cliques.d" -o"cliques.o" "../cliques.cpp"
	g++ -p -O3 -Wall -c -MMD -MP -MF"find_communities.d" -MT"find_communities.d" -o"find_communities.o" "../find_communities.cpp"
	g++ -p -O3 -Wall -c -MMD -MP -MF"graph_loading.d" -MT"graph_loading.d" -o"graph_loading.o" "../graph_loading.cpp"
	g++ -p -O3 -Wall -c -MMD -MP -MF"graph_representation.d" -MT"graph_representation.d" -o"graph_representation.o" "../graph_representation.cpp"
	g++ -p -o "GCECommunityFinder"  ./Community_Finder.o ./Seed.o ./aaron_utils.o ./cliques.o ./find_communities.o ./graph_loading.o ./graph_representation.o   


release:
	g++ -O3 -Wall -c -MMD -MP -MF"Seed.d" -MT"Seed.d" -o"Seed.o" "../Seed.cpp"
	g++ -O3 -Wall -c -MMD -MP -MF"Community_Finder.d" -MT"Community_Finder.d" -o"Community_Finder.o" "../Community_Finder.cpp"
	g++ -O3 -Wall -c -MMD -MP -MF"aaron_utils.d" -MT"aaron_utils.d" -o"aaron_utils.o" "../aaron_utils.cpp"
	g++ -O3 -Wall -c -MMD -MP -MF"cliques.d" -MT"cliques.d" -o"cliques.o" "../cliques.cpp"
	g++ -O3 -Wall -c -MMD -MP -MF"find_communities.d" -MT"find_communities.d" -o"find_communities.o" "../find_communities.cpp"
	g++ -O3 -Wall -c -MMD -MP -MF"graph_loading.d" -MT"graph_loading.d" -o"graph_loading.o" "../graph_loading.cpp"
	g++ -O3 -Wall -c -MMD -MP -MF"graph_representation.d" -MT"graph_representation.d" -o"graph_representation.o" "../graph_representation.cpp"
	g++ -o "GCECommunityFinder"  ./Community_Finder.o ./Seed.o ./aaron_utils.o ./cliques.o ./find_communities.o ./graph_loading.o ./graph_representation.o   


clean:
	rm *.o
	rm *.d
	rm GCECommunityFinder
