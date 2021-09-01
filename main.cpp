#include"graph.cpp"
#include<ctime>
clock_t st,ed;
double endtime;
int
main(int argc, char* argv[])
{	
	string txt_name=argv[1];
	string name=argv[2];
	string sign=(string(argv[3])=="1")?" ":"\t";
	int part=atoi(argv[4]);
	string weight = argv[5];
	
	st = clock();
		graph* test=new graph();
		test->init();
		test->RDF=name;
		test->WEIGHT=weight;
		test->part=part;
		
		
		test->loadGraph(txt_name,sign, weight);
		int predNum = test->getPreNum();
		if(predNum < 20)
			// if the number of properties is smaller than 20, we find the optimal partitioning results.
			test->unionEdgeForEnum();
			// test->greed2();
		else if(predNum >= 20 && predNum < 120)
			test->unionEdgeForGreed();
		else
			test->greed3();

		test -> partition(txt_name, sign, name);

	ed = clock();
	endtime = (double)(ed-st) / CLOCKS_PER_SEC;
	cout << "partition : " << endtime << " s" << endl;

	delete test;

	return 0;
}
//g++ main.cpp -std=c++11 -o mpc
//./mpc watdiv100K.nt MPC_watdiv100K_data 2 8 watdiv100K_predicate_frequery.txt
//./mpc watdiv100M MPC_watdiv100M_data 2 8 watdiv100_predicate_frequery.txt