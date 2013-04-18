#include <fstream>
#include <iostream>
#include <unistd.h>
#include <ios>
#include <iostream>
#include <fstream>
#include <string>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <proc/readproc.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "Snap.h"
 
// Benchmarking prototypes:
template <class PGraph> void BenchmarkGraphEdgeI(PGraph Graph, std::ofstream& file, bool isDefrag);
template <class PGraph> void BenchmarkGraphNodeI(PGraph Graph, std::ofstream& file, bool isDefrag);
template <class PGraph> void BenchmarkGraphDegTrav(PGraph Graph, std::ofstream& file, bool isDefrag);
template <class PGraph> void Benchmark(PGraph G, std::ofstream& file);
 
// Added to pass the assert condition, which overflows for large node values
template <class PGraph> PGraph GenRndGnm(const int& Nodes, const int& Edges, const bool& IsDir=true, TRnd& Rnd=TInt::Rnd);
template <class PGraph>
PGraph GenRndGnm(const int& Nodes, const int& Edges, const bool& IsDir, TRnd& Rnd) {
  PGraph GraphPt = PGraph::New();
  typename PGraph::TObj& Graph = *GraphPt;
  Graph.Reserve(Nodes, Edges);
  for (int node = 0; node < Nodes; node++) {
    IAssert(Graph.AddNode(node) == node);
  }
  for (int edge = 0; edge < Edges; ) {
    const int SrcNId = Rnd.GetUniDevInt(Nodes);
    const int DstNId = Rnd.GetUniDevInt(Nodes);
    if (SrcNId != DstNId && Graph.AddEdge(SrcNId, DstNId) != -2) {
      if (! IsDir) { Graph.AddEdge(DstNId, SrcNId); }
      edge++;
    }
  }
  return GraphPt;
}
 
template <class PGraph>
void BenchmarkGraphEdgeI(PGraph Graph, std::ofstream& file, bool isDefrag) {
  int ECount = 0;
  int i = 0;
  clock_t start = clock();
 
  for (i = 0; i < 50; i++) {
    ECount = 0;
    for (typename PGraph::TObj::TEdgeI EI = Graph->BegEI(); EI < Graph->EndEI(); EI++) {
      ECount++;
    }
  }
 
  double msec = (clock() - start) * 1000.0 / CLOCKS_PER_SEC;
  printf("Nodes: %d Edges: %d Freq: %f ops\n", Graph->GetNodes(), ECount, 50000/msec);
  file << 50000/msec << " ";
}
 
template <class PGraph>
void BenchmarkGraphDegTrav(PGraph Graph, std::ofstream& file, bool isDefrag) {
  int ECount = 0;
  int i = 0;
  clock_t start = clock();
 
  for (i = 0; i < 50; i++) {
    ECount = 0;
    for (typename PGraph::TObj::TNodeI NI = Graph->BegNI(); NI < Graph->EndNI(); NI++) {
      for (int e = 0; e < NI.GetOutDeg(); e++) {
        ECount++;
      }
    }
  }
 
  double msec = (clock() - start) * 1000.0 / CLOCKS_PER_SEC;
  printf("Nodes: %d Edges: %d Freq: %f ms\n", Graph->GetNodes(), ECount, 50000/msec);
  file << 50000/msec << " ";
}
 
template <class PGraph>
void BenchmarkGraphNodeI(PGraph Graph, std::ofstream& file, bool isDefrag) {
  int NCount = 0;
  int i = 0;
  clock_t start = clock();
 
  for (i = 0; i < 50; i++) {
    NCount = 0;
    for (typename PGraph::TObj::TNodeI NI = Graph->BegNI(); NI < Graph->EndNI(); NI++) {
      NCount++;
    }
  }
 
  double msec = (clock() - start) * 1000.0 / CLOCKS_PER_SEC;
  printf("Nodes: %d Edges: %d Freq: %f ops\n", NCount, Graph->GetEdges(), 50000/msec);
  file << 50000/msec << " ";
}
 
template <class PGraph>
void Benchmark(PGraph G, std::ofstream& file) {
  printf("NodeI ");
  BenchmarkGraphNodeI(G, file, false);
  printf("EdgeI ");
  BenchmarkGraphEdgeI(G, file, false);
  printf("NodeE ");
  BenchmarkGraphDegTrav(G, file, false);
}
 
 
int main(int argc, char* argv[]) {
  // benchmark graph creation
  int OneK = 1000;
  // Stores nodes from 1k to 10m nodes
  int NodeArr[5];
  // Stores edge multipliers for counting nodes (10, 50, 100)
  int EdgeArr[3];
  int i = 0;
  int j = 0;
  int k = 0;
  int TenP = 1;
  int NNodes = 0;
  int NEdges = 0;
  double msec = 0;
  PNGraph G1;
  PUNGraph G2;
  PNEGraph G3;
  PNEAGraph G4;
  struct proc_t usage;
  double mu0, mu1;
  
  std::ofstream file;
  using std::cout;
  using std::endl;

  if (argc < 3) {
    printf("Usage: ./demo-topology-benchmark <0-3> <1 for defrag/0 for no defrag>\n0:TNGraph, 1:TUNGraph, 2:TNEGraph, 3:TNEAGraph\n");
    return -1;
  }
 
  k = atoi(argv[1]);
  for (i = 0; i < 5; i++) {
    TenP *= 10;
    NodeArr[i] = TenP * OneK;
  }
  EdgeArr[0] = 10;
  EdgeArr[1] = 50;
  EdgeArr[2] = 100;
 
  //  for (k = 0; k < 4; k++) {
    printf("Starting Benchmarking for ");
    switch (k) {
    case 0:
      file.open("tngraph.dat", std::ios_base::app);
      printf("TNGraph\n\n");
      break;
    case 1:
      file.open("tungraph.dat", std::ios_base::app);
      printf("TUNGraph\n\n");
      break;
    case 2:
      file.open("tnegraph.dat", std::ios_base::app);
      printf("TNEGraph\n\n");
      break;
    default:
      file.open("tneagraph.dat", std::ios_base::app);
      printf("TNEAGraph\n\n");
      break;
    }
    int counters = 0;
    for (i = 1; i < 3; i++) {
      for (j = 1; j < 3; j++) {
        clock_t start = clock();
        NNodes = NodeArr[i];
        NEdges = EdgeArr[j]*NodeArr[i];
        switch (k) {
      case 0:
	printf("\nGenerating Graph...\n");
	printf("GrGen ");
	look_up_our_self(&usage);
	mu0 = (double) usage.vsize / 1000000;
	G1 = GenRndGnm<PNGraph>(NNodes, NEdges, true);
	msec = (clock() - start) * 1000 / CLOCKS_PER_SEC;
	printf("Nodes: %d Edges: %d Time: %.3f ms\n", NNodes, NEdges, msec);
        file << NNodes << " " << NEdges << " " << msec << " ";
	if (atoi(argv[2])) {
	  printf("Defragmenting...\n");
	  G1->Defrag();
	}
	look_up_our_self(&usage);
	mu1 = (double) usage.vsize / 1000000;
	Benchmark(G1, file);
	G1->Clr();
	printf("Memory Usage: %.3f\n", mu1-mu0);
        file << NNodes << " " << NEdges << " " << msec << " ";
	break;
      case 1:
	printf("\nGenerating Graph...\n");
	printf("GrGen ");
	G2 = GenRndGnm<PUNGraph>(NNodes, NEdges, false);
	msec = (clock() - start) * 1000 / CLOCKS_PER_SEC;
	printf("Nodes: %d Edges: %d Time: %.3f ms\n", NNodes, NEdges, msec);
        file << NNodes << " " << NEdges << " " << msec << " ";
	//process_mem_usage(vm, rss);
	Benchmark(G2, file);    
	printf("Defragmenting...\n");
	G2->Defrag();
	//process_mem_usage(vm, rss);
	//cout << "VM: " << vm << "; RSS: " << rss << endl;
	Benchmark(G2, file);
	G2->Clr();
	break;
      case 2:
	printf("\nGenerating Graph...\n");
	printf("GrGen ");
	G3 = GenRndGnm<PNEGraph>(NNodes, NEdges, true);
	msec = (clock() - start) * 1000 / CLOCKS_PER_SEC;
	printf("Nodes: %d Edges: %d Time: %.3f ms\n", NNodes, NEdges, msec);
        //file << NNodes << " " << NEdges << " " << msec << " ";
	//process_mem_usage(vm, rss);
	Benchmark(G3, file);    
	printf("Defragmenting...\n");
	//process_mem_usage(vm, rss);
	//cout << "VM: " << vm << "; RSS: " << rss << endl;
	G3->Defrag();
	Benchmark(G3, file);
	G3->Clr();
	break;
      default:
	printf("\nGenerating Graph...\n");
	printf("GrGen ");
	G4 = GenRndGnm<PNEAGraph>(NNodes, NEdges, true);
	msec = (clock() - start) * 1000 / CLOCKS_PER_SEC;
	printf("Nodes: %d Edges: %d Time: %.3f ms\n", NNodes, NEdges, msec);
        file << NNodes << " " << NEdges << " " << msec << " ";
	//process_mem_usage(vm, rss);
	//cout << "VM: " << vm << "; RSS: " << rss << endl;
	Benchmark(G4, file);    
	printf("Defragmenting...\n");
	G4->Defrag();
	//	process_mem_usage(vm, rss);
	//cout << "VM: " << vm << "; RSS: " << rss << endl;
	Benchmark(G4, file);
	G4->Clr();
	break;
	}
        file << "\n";
      }
    }
 
    file.close();
    //  }
}
