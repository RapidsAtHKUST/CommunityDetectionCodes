/* 
 * File:   main.cpp
 * Author: T.S.Evans, Physics Dept., Imperial College London
 * These line graphs are defined in the paper by T.S.Evans and R.Lambiotte,
 * Line Graphs, Link Partitions and Overlapping Communities,
 * Phys.Rev.E <b>80</b> (2009) 016105 [arXiv:0903.2181].
 * Please cite this paper and acknowledge use of this code if you use this code.
 * Created on 14 August 2009, 15:25
 */

#include <stdlib.h>
#include <iostream>
#include <ostream>
//#include <vector>
#include "TseGraph.h"

using namespace std;

const double MINNORMALISATION=1e-10;
const char *VERSION = "090822";
char *infile = NULL;
char *outfile = NULL;
bool inGraphWeighted;
bool infoOn=true;
int lgType=2;

void 
printLineGraphTypes(ostream& os){
 os << "--- Line Graph Types (see Phys.Rev.E 80 (2009) 016105)" <<endl;
 os << "0\tUnweighted and no self loops (C of eqn 8)" <<endl;
 os << "1\tUnweighted with self-loops (\\tilde{C} of footnote 2)" <<endl;
 os << "2\tWeighted and no self loops (D of eqn 11)" <<endl;
 os << "3\tWeighted with self-loops (E of eqn 14)" <<endl;
}

void 
printFileFormats(ostream& os){
 os << "--- File Formats" <<endl;
 os << "Files have vertices numbered from 0." <<endl;
 os << "Each line corresponds to one edge in the graph and" <<endl;
 os << "has two or three entries separated by white space" <<endl;
 os << "(e.g. using spaces, or by using a tab separated format from a spreadsheet)." <<endl;
 os << "The first two entries are the index of the source then target" <<endl;
 os << "vertices at the two ends of an edge." <<endl;
 os << "The third column is only used if graph is weighted and is the edge weight." <<endl;
 os << "No extra entries or lines are tolerated.  In particular" <<endl;
 os << "you must specify the input graph is weighted if the input file has a third column" <<endl;
 os << "or if its unweighted there must not be a third column." <<endl;
 os << "Note also that the number of vertices created in the graph is one more" <<endl;
 os << "than the largest vertex index in the list.  They are numbered from 0 to" <<endl;
 os << "this largest index. If an index in this range is not listed then it is" <<endl;
 os << "created as a vertex of degree 0." <<endl;
 os << "Self-loops may be created but it is not clear if the code works for these." <<endl;
 os << "Multiple edges between the same two vertices may also be created but" <<endl;
 os << "again it is not clear if the code will work properly in this case." <<endl;
 }



/**
 *Prints usage to an output stream such as cout.
 */
void
usage(char *prog_name, char *more, ostream& os) {
    os << "Version " << VERSION << endl;
    os << more;
  os << "usage: " << prog_name << " -i input_file -o output_file [options]" << endl << endl;
  os << "-t n\tcreate line graph of type 0<=n<=3. Default is 2." << endl;
  os << "-w\tread the graph as a weighted one. Otherwise graph is unweighted." << endl;
  os << "-h\tshow this usage message." << endl;
  printLineGraphTypes(os);
  printFileFormats(os);
}



/**
 *Prints usage to cout then execution halted.
 */
void
usage(char *prog_name, char *more) {
  usage(prog_name, more, cout);   
  exit(0);
}



void
parse_args(int argc, char **argv) {
  if (argc==1)  cout<< "!!! No arguments given, default settings will be used.\n    Use "<<argv[0]<<" -h for usage" << endl;;
  inGraphWeighted=false;
  for (int i = 1; i < argc; i++) {
    if(argv[i][0] == '-') {
      char c=argv[i][1];
      switch(c) {
      case 'i':
	if (++i==argc)
	  usage(argv[0], "Infile missing\n");
	infile = argv[i];
	break;
      case 'o':
	if (++i==argc)
	  usage(argv[0], "Outfile missing\n");
        outfile = argv[i];
	break;
      case 't':
	if (++i==argc)
	  usage(argv[0], "Line graph type number missing\n");
    //if (argv[i].length!=1) usage(argv[0], "Line graph type number incorrect\n");
        /* Declaring a variable in one case causes proplems in some compiliers
         * with errors such as "jump to case label" and
         * "crosses initialization of ...".  Switches are treated as 
         * computed got labels hence the label comment.  Solution is to restrict
         * the scope of variables only needed within one case.
         */
        {int n=argv[i][0]-48;
        if ((n<0) || (n>3)) usage(argv[0], "Line graph type number incorrect\n");
         lgType=n;}
	break;
      case 'w' :
	inGraphWeighted=true;
	break;
      case 'h' :
	usage(argv[0], "Options\n");
	break;
      default:
	usage(argv[0], "Unknown option\n");
      }
    } else {
      usage(argv[0], "Options start with a -\n");
    }
  }
  if (infile==NULL || outfile==NULL)
    usage(argv[0], "In or outfile missing\n");
}

void
display_time(const char *str) {
  time_t rawtime;
  time ( &rawtime );
  cout << str << " : " << ctime (&rawtime);
}



/*
 *  For details and notation see 
 * T.S.Evans and R.Lambiotte, 
 * Line graphs, link partitions, and overlapping communities, 
 * Phys.Rev.E 80 (2009) 016105  [arXiv:0903.2181]
 *   
 * Line Graph Types:-
 * 0 Unweighted and no self loops (C of eqn 8)
 * 1 Unweighted with self-loops (\tilde{C} of footnote 2)
 * 2 Weighted and no self loops (D of eqn 11)
 * 3 Weighted with self-loops (E of eqn 14)
 */
void
makeLineGraph(TseGraph& tg, TseGraph& lg, int newType, bool infoOn){

        int type=2;
        if ((newType>-1)&&(newType<4)) type = newType;
        else  {cout << "*** makeLineGraph needs type to be between 0 and 3, was given " << newType << endl; exit(0);}

        cout << "--- Making Line Graph of type  " << newType << endl; 
        int noselfloops = 1; // don't include self loops
        bool includeSelfLoops = false;
        int countfactor = -1;
        if ((type==1) || (type==3)) {// do include self-loops
            countfactor=1;
            noselfloops=0;
            includeSelfLoops = true;
        }


        int lgMaximumVertices = (tg.getNumberStubs() /2); // remember this is the stub number

        int lgMaximumStubs = 0;
        for (int v = 0; v < tg.getNumberVertices(); v++) {
                int k = tg.getVertexDegree(v);
                lgMaximumStubs += k * (k + countfactor); // remember this is stub number
                }

        // this creates the space for the vertices but does nothing for the edges
        lg.setSize(lgMaximumVertices,lgMaximumStubs);
        lg.setDirected( tg.isDirected() || tg.isWeighted() );
        lg.setWeighted( tg.isWeighted() || (type>1) );
        // add blank vertices
        for (int v=0; v<lgMaximumVertices; v++) lg.addVertex();

        if (infoOn) cout << "Predicting " <<lgMaximumVertices << " vertices and " << lgMaximumStubs << " stubs in line graph " << endl;
        const int dotIncrement = lgMaximumVertices/100;
        const int newLineIncrement = lgMaximumVertices/10;
        int nextDot = dotIncrement;
        int nextNewLine = newLineIncrement;
        time_t time_begin, time_end;
        time(&time_begin);
        if (infoOn) display_time("time started");
  
        

        // Now case where tg is undirected and lg is undirected
        // This excludes the weighted input tg to type 2 C LG case
        // which always gives Directed graphs
            int kv = -1;
            int stub1, stub2=-1;
            //double w=1; // this is the weight of an lg edge
            double norm=1; // normalisation
            double s=-1;
            bool lgweighted = lg.isWeighted();
            for (int v = 0; v < tg.getNumberVertices(); v++) {
                if (infoOn){
                    if (v>=nextDot) {
                        cout << "."; 
                        fflush(stdout);
                        nextDot += dotIncrement; 
                    }
                    if (v>=nextNewLine) {
                        time(&time_end);
                        int percentageStubs = (int)(0.5+(100*((double) lg.getNumberStubs()) / ((double) lgMaximumStubs)));
                        cout << " "<<percentageStubs <<"%, " << (time_end-time_begin) << "s" << endl;
                        fflush(stdout);
                        nextNewLine+=newLineIncrement;
                    }
                } // eo if infoOn
//                if (infoOn && counting.increment()){
//                        System.out.println(" : "+timing.elapsedTimeString()+": "+timing.estimateRemainingTimeString(v/dnv));
//                        System.out.println("           : "+memory.StringAllValues());
//                }
                kv = tg.getVertexDegree(v);
                if (kv <= noselfloops) continue;
                if (lgweighted){
                    s=tg.getVertexStrength(v); // this is degree when input tg is unweighted
                    if (s<MINNORMALISATION) continue;
                    norm=s;
                }
                for (int ni = 0; ni < tg.getVertexDegree(v); ni++) {
                    stub1 = tg.getStub(v, ni);
                    if(lgweighted && !includeSelfLoops) norm = (s-tg.getStubWeight(stub1));
                    if (norm<MINNORMALISATION) continue;
                    for (int no = ni+noselfloops; no < tg.getVertexDegree(v); no++) {
                      stub2 = tg.getStub(v, no);
                      // use edge index=stub/2 for the vertex indices in the line graph
                      if (lgweighted) {
                          lg.increaseWeight(stub1>>1, stub2>>1, tg.getStubWeight(stub2)/norm); // note that e and e2 are really stub indices
                      }
                      else lg.addEdgeUnique(stub1>>1, stub2>>1);
                    }// eo for no
                } //eo for ni
                

            } //eo for v
            time(&time_end);
            if (infoOn )cout << "\nFinished makeLineGraph, time taken "<< (time_end-time_begin) << "s" << endl;

            //return lg;
    } // eo old undirected unweighted case



int main(int argc, char **argv) {

  for (int i = 1; i < argc; i++) cout << "arg "<<i <<" = " << argv[i] <<endl;
  srand(time(NULL));
  infile ="input/BowTieinputEL.dat";
  outfile ="output/BowTieWLGoutputEL.dat";
  lgType =2;


  parse_args(argc, argv);
  cout << "  Input file " << infile << (inGraphWeighted?" weighted graph ":" unweighted graph ") << endl;
  cout << "  Output file " << outfile << ", line graph type "<< lgType << endl;
  
  TseGraph tg = TseGraph();
  time_t time_begin, time_end;
  time(&time_begin);
  display_time("time started");
  tg.read(infile,inGraphWeighted);
  time(&time_end);
  display_time("finished reading file");
  cout <<  " time taken for file reading " << (time_end-time_begin) << endl;

  if (tg.getNumberVertices()<20) tg.write();


  cout << "input network : "
       << tg.getNumberVertices() << " vertices, "
       << tg.getNumberStubs() << " stubs, "
       << (tg.isWeighted()?"weighted":"unweighted") 
       << endl;

  //if (lgType>-1) exit(0);
  
  // make line graph
  time(&time_begin);
  TseGraph lg;
  makeLineGraph(tg,lg,lgType, infoOn);
  time(&time_end);
  display_time("finished making line graph");
  cout <<  " time taken to make line graph " << (time_end-time_begin) << "s" << endl;
  if (lg.getNumberVertices()<20) lg.write();
   cout << "line graph: "
       << lg.getNumberVertices() << " vertices, "
       << lg.getNumberStubs() << " stubs, "
       << (lg.isWeighted()?"weighted":"unweighted") 
       << endl;
  time(&time_begin);
  cout <<  " time taken to make line graph " << (time_end-time_begin) << "s"  << endl;

  //output line graph file
  time(&time_begin);
  lg.write(outfile);
  time(&time_end);
  display_time("finished making line graph file ");
  cout <<  " time taken to write line graph to file "<< outfile << " " << (time_end-time_begin) << "s"  << endl;

}

