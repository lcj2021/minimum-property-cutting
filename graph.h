#include<iostream>
#include<stdio.h>
#include<vector>
#include<unordered_map>
#include<set>
#include<string.h>
#include<string>
#include<map>
#include<queue>
#include<algorithm>
#include<stdlib.h> 
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
using namespace std;

class graph
{
public:
	graph();
	~graph();
	void init();
	void loadGraph(string txt_name,string tag, string weight);
	vector <string> split(string textline,string tag);
	int getParent(int son,vector<int>& fa);
	int getParentMap(int son,unordered_map<int,int>& fa);
	void coarsening();
	void unionEdgeForEnum();
	void enumPre(int preID,long long choice,vector<int> &curParent,vector<int> &curSoncnt,vector<int> &curRank,bool *inValid);
	void unionEdgeForGreed();
	void greed1(vector<int> &choice,vector<int> &curParent,vector<int> &curSonCnt,vector<int> &curRank,vector<bool> &invalid);
	int cal(long long cur);
	void compareCrossingEdgeCnt(long long cur);
	void greed2();
	void greed3();
	void unionBlock(vector<int> &choice,int goal);
	void randEntity(string txt_name,string tag);
	void randPre(string txt_name,string tag);
	void metis(string txt_name,string tag);
	void generateWeight();
	void partition(string txt_name, string tag, string out_file);
	int getPreNum(); 
	// void update();

	string RDF;	
	string WEIGHT;

	int part;

private:
	set<string> predicate;
	unordered_map<string,int> entityToID;
	vector<string> IDToEntity;
	unordered_map<string,int> predicateToID;
	vector<string> IDToPredicate;
	vector<vector<pair<int,int> > >edge;
	vector<pair<int,string> >otherEdge;

	//coarseningPoint[preID]表示preID的谓词
	vector<unordered_map<int,int> >coarseningPoint;

	//每个实体对应三元组数量
	vector<int> entityTriples;
	set<pair<string,int> >finalResult;


	unordered_map<string,int> edge_cnt;

	//每个谓词对应边的数量
	// edge_cnt : the key is the property and the value is the count of the property
	unordered_map<string, double> edge_weight;

	unordered_map<string, int> group;
	vector<vector<pair<pair<string,string>, string > > > edgeGroup;

	
	vector<bool> invalid;
	long long triples;

	// entityCnt : entity count
	long long entityCnt;

	//谓词种类数
	int preType;

	long long limit;	
	long long ans;	
	long long crossEgdeCnt;
	long long invalidEdgeCnt;																																		
};