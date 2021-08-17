#include "graph.cpp"
#include <ctime>
clock_t st,ed;
double endtime;
int
main(int argc, char* argv[])
{	
	string rdf_name = argv[1];
	string name = argv[2];
	string sign = (string(argv[3]) == "1") ? " " : "\t";
	int part = atoi(argv[4]);
	string weight = argv[5];
	string template_path = argv[6];
	string result_path = argv[7];

	st = clock();
		graph* test = new graph();
		test -> init();
		test -> RDF = name;
		test -> WEIGHT = weight;
		test -> part = part;
		
		test -> loadGraph(rdf_name, sign, weight);
		test -> getFileList(template_path, result_path);
		test -> readQueryResult(template_path, result_path, sign);

		int svCnt = test -> getSvCnt();
		if(svCnt < 20)
			// if the number of properties is smaller than 20, we find the optimal partitioning results.
			test -> unionEdgeForEnum();
			// test->greed2();
		else if(svCnt >= 20 && svCnt < 120)
			test -> unionEdgeForGreed();
		else
			test -> greed3();

		test -> partition(rdf_name, sign, name);

	ed = clock();
	endtime = (double)(ed-st) / CLOCKS_PER_SEC;
	cout << "partition : " << endtime << " s" << endl;

	delete test;
	return 0;
}
//g++ main.cpp -std=c++11 -o mpc2
//.\mpc2 watdiv100K.nt MPC_watdiv100K_data 2 8 watdiv100K_predicate_frequery.txt ./origin_query ./warpInput/100k
//./mpc2 watdiv100M.txt MPC_watdiv100M_data 2 8 watdiv100M_predicatecd_frequery.txt ./origin_query ./warpInput/100m
//./mpc2 watdiv100M MPC_watdiv100M_data 2 8 watdiv100_predicate_frequery.txt
