#include"graph.h"
#include<fstream>
#include<sstream>
#include<time.h>

graph::graph(){}

graph::~graph(){}

void 
graph::init()
{
	//边集
	vector<pair<int,int> >tmp;
	edge.push_back(tmp);
	edgeOfMultiPre.push_back(tmp);

	//三元组
	entityTriples.push_back(0);


	IDToEntity.push_back("");
	IDToPredicate.push_back("");
	preType=entityCnt=triples=invalidEdgeCnt=0;
}

int 
graph::getSvCnt()
{
	svCnt = preType + validResultCnt;
	for (int i = 1; i <= preType; ++ i)
		if (edge_weight[IDToPredicate[i]] == 0)
			edge_weight[IDToPredicate[i]] += 0.1;
	for (int i = preType + 1; i <= svCnt; ++ i)
		edge_weight[IDToPredicate[i]] = 1000;
	return svCnt;
}

void 
graph::loadGraph(string txt_name, string tag, string weight)
{
	string line;
	ifstream in1(txt_name.data());

	vector<pair<int, int> >tmp;
	cout << txt_name << "========" << endl;
	while(getline(in1, line))
	{
		if(triples % 10000 == 0)
			cout << "loading triples : " << triples << endl;
		triples ++;
		line.resize(line.length() - 2);
		vector<string> s;
		s = split(line, tag);
		
		//＜a, pre, b＞　中间那个就是谓词
		predicate.insert(s[1]);

		//取a和b,确保在unordered_map中加入映射
		for(int i = 0; i < 3; i += 2)
			//如果s[0] 或 s[2]是 entity 并且 entity - ID 映射中没有
			if((s[i][0] == '<' || s[i][0] == '_') && entityToID.count(s[i]) == 0)
			{
				entityToID[s[i]] = ++entityCnt;
				IDToEntity.push_back(s[i]);

				entityTriples.push_back(0);
			}
		
		edge_cnt[s[1]] ++;
		int a = entityToID[s[0]];

		//a实体 对应的三元组数量 ++
		entityTriples[a] ++;

		//a 和 b同时为entity 谓词才能算preType
		if((s[0][0] == '<' || s[0][0] == '_') && (s[2][0] == '<' || s[2][0] == '_'))
		{
			if(predicateToID.count(s[1]) == 0)
			{
				predicateToID[s[1]] = ++ preType;
				IDToPredicate.push_back(s[1]);
				edge.push_back(tmp);
			}		
			int b = entityToID[s[2]];
			//加到谓词所属的边集
			edge[predicateToID[s[1]]].push_back( {a, b} );	

			//b实体 对应的三元组数量 ++
			entityTriples[b]++;
		}
	}
	in1.close();

	ifstream in2(weight.data());
	string pred, freq_str;
	int freq;
	while(getline(in2, line))
	{
		auto sep = line.find(" ");
		pred = line.substr(0, sep);
		freq_str = line.substr(sep + 1);
		freq = atoi(freq_str.data());
		// cout << pred << " " << freq << endl;
		edge_weight[pred] = (double)freq;
	}
	in2.close();

	// for(auto it : edge_weight)
	// 	cout << it->first << "\t" << predicateToID[it->first] << "\t" << it->second << endl;

	limit = entityCnt / part / 1.2;
	printf("limit: %lld\n", limit);
	printf("triples: %lld\n", triples);
	printf("entityCnt: %lld\n", entityCnt);
	printf("predicate: %lu\n", predicate.size());

	printf("entity->preType: %d\n", preType);

	// for (int i = 1; i <= preType; ++ i)		
	// 	printf("sizeof edge_cnt[%d] : %ld\n", i, edge_cnt[IDToPredicate[i]]);
}

//在line中由标签tag 分割出 entity
vector<string> 
graph::split(string textline,string tag)
{
	vector<string> res;
	std::size_t pre_pos = 0;
	std::size_t pos = textline.find(tag);

	//依次处理line中的每个tag数据
	while (pos != std::string::npos)
	{
		string curStr = textline.substr(pre_pos, pos - pre_pos);
		curStr.erase(0, curStr.find_first_not_of("\r\t\n "));
		curStr.erase(curStr.find_last_not_of("\r\t\n ") + 1);


		if(strcmp(curStr.c_str(), "") != 0)
			res.push_back(curStr);
		pre_pos = pos + tag.size();
		pos = textline.find(tag, pre_pos);
	}

	string curStr = textline.substr(pre_pos, pos - pre_pos);
	curStr.erase(0, curStr.find_first_not_of("\r\t\n "));
	curStr.erase(curStr.find_last_not_of("\r\t\n ") + 1);
	if(strcmp(curStr.c_str(), "") != 0)
		res.push_back(curStr);

	return res;
}

int 
graph::getParent(int son,vector<int> &fa)
{
	// return fa[son]==son?son:fa[son]=getParent(fa[son],fa);
	int i,j,k;
	k=son;
	while(k!=fa[k])k=fa[k];
	i=son;
	while(i!=k)
	{
		j=fa[i];
		fa[i]=k;
		i=j;
	}
	return k;
}

int 
graph::getParentMap(int son,unordered_map<int,int> &fa)
{
	int i,j,k;
	k=son;
	while(k!=fa[k])k=fa[k];
	i=son;
	while(i!=k)
	{
		j=fa[i];
		fa[i]=k;
		i=j;
	}
	return k;
}

void 
graph::coarsening()
{
	// cout << preType << " ? " << edge.size() - 1 << endl;
	// cout << validResultCnt << " ? " << edgeOfMultiPre.size() - 1<< endl;
	invalid = vector<bool>(svCnt + 1, 0);
	coarseningPoint = vector<unordered_map<int, int> >(svCnt + 1, unordered_map<int, int>());
	for(int svID = 1; svID <= svCnt; svID ++)
	{
		vector<int> sonCnt = vector<int>(entityCnt + 1, 1);
		vector<int> rank = vector<int>(entityCnt + 1, 0);

		//遍历 标签为 svID 的所有边
		for(int p = 0; p < edge[svID].size(); ++ p)
		{
			int A = edge[svID][p].first, B = edge[svID][p].second;

			//孤立点, 以自环形式加入 标签为 svID 的WCC
			if(coarseningPoint[svID].count(A) == 0)
				coarseningPoint[svID].insert(make_pair(A, A));
			if(coarseningPoint[svID].count(B) == 0)
				coarseningPoint[svID].insert(make_pair(B, B));

			//找到 A B 所属 WCC 中 父节点
			int parentA = getParentMap(A, coarseningPoint[svID]);
			int parentB = getParentMap(B, coarseningPoint[svID]);

			//令A的color点权重 > B
			if(rank[parentA] < rank[parentB])		swap(parentA, parentB);

			//A B在两个 WCC树 里面
			if(parentA != parentB)
			{
				//把B 归入 A的WCC
				coarseningPoint[svID][parentB] = parentA;
				sonCnt[parentA] += sonCnt[parentB];
				rank[parentA] = max(rank[parentA], rank[parentB] + 1);

				//某个WCC规模超过了limit
				if(sonCnt[parentA] > limit)
				{
					invalid[svID] = 1;
					invalidEdgeCnt ++;
					string preName;
					preName = IDToPredicate[svID];
					
					printf("invalid: %d %s\n", svID, preName.data());
					break;
				}
			}
		}
	}
}

void 
graph::unionEdgeForEnum()
{
	//缩点
	coarsening();

	//判断当前选择 WCC规模是否合法
	bool *invalidST = new bool[1 << preType]();
	for(int i=1;i<=preType;i++)if(invalid[i])invalidST[1<<(i-1)]=1;

	//ans中 1代表内部属性 0代表交叉边
	ans = 0;
	vector<int> choice(preType + 1, 0);

	vector<int> parent = vector<int>(entityCnt + 1);
	for(int i = 1; i <= entityCnt; i ++)	parent[i] = i;
	vector<int> sonCnt = vector<int>(entityCnt + 1, 1);
	vector<int> rank = vector<int>(entityCnt + 1, 0);

	crossEgdeCnt = preType;
	printf("enumPre\n");
	enumPre(0, 0, parent, sonCnt, rank, invalidST);

	printf("crossEgdeCnt: %d\n", preType - cal(ans));
	for(int i=1;i<=preType;i++)
	{
		//ans记录当前选择
		choice[i]=((1<<(i-1))&ans)?1:0;
		// if(choice[i]==0)cout<<i<<"	"<<IDToPredicate[i]<<endl;
	}
	printf("\n");
	unionBlock(choice, part);
	delete[] invalidST;
}

//dfs枚举所有2^preID中选择
void 
graph::enumPre(int preID, long long choice, vector<int> &curParent, vector<int> &curSonCnt, vector<int> &curRank, bool *invalidST)
{
	//最优剪枝
	if(cal(ans) >= cal(choice) + preType - preID - invalidEdgeCnt)	return;

	//边界
	if(preID == preType)
	{
		printf("choice(Binary) when preID == preType: %lld\n",choice);
		compareCrossingEdgeCnt(choice);
		return;
	}
	preID ++;

	//选择加上当前 谓词
	long long nextchoice = choice | (1LL<<(preID-1));

	//flag为1: 选择非法
	bool flag=invalidST[nextchoice]|invalidST[1LL<<(preID-1)];

	if(!flag)
	{
		vector<int> nextFa(curParent);
		vector<int> nextSonCnt(curSonCnt);
		vector<int> nextRank(curRank);
		unordered_map<int,int>::iterator it;
		for(it=coarseningPoint[preID].begin();it!=coarseningPoint[preID].end();it++)
		{
			int point=it->first;
			int parentA=getParent(point,nextFa),parentB=getParent(getParentMap(point,coarseningPoint[preID]),nextFa);
			if(nextRank[parentA]<nextRank[parentB])swap(parentA,parentB);
			if(parentA!=parentB)
			{
				nextFa[parentB]=parentA;
				nextSonCnt[parentA]+=nextSonCnt[parentB];
				nextRank[parentA]=max(nextRank[parentA],nextRank[parentB]+1);
				if(nextSonCnt[parentA]>limit)
				{
					flag=1;
					break;
				}
			}
		}

		if(!flag)enumPre(preID,nextchoice,nextFa,nextSonCnt,nextRank,invalidST);
	}

	//如果不合法 就跳过当前谓词,选下一个
	if(flag)
	{
		for(int i=1;i<=preType;i++)invalidST[(1LL<<(i-1))|nextchoice]=1;
	}
	enumPre(preID,choice,curParent,curSonCnt,curRank,invalidST);
}

void 
graph::unionEdgeForGreed()
{
	coarsening();
	ans = 0;
	vector<int> choice(svCnt + 1, 0);
	vector<int> parent = vector<int>(entityCnt + 1);
	for(int i = 1; i <= entityCnt; i ++)		parent[i] = i;
	vector<int> sonCnt = vector<int>(entityCnt + 1, 1);
	vector<int> rank = vector<int>(entityCnt + 1, 1);
	printf("greed1\n");

	invalid = vector<bool>(svCnt + 1, 0);
	int threshold = entityCnt * 0.0001;

	int optim=0;

    for(int preID = 1; preID <= svCnt; preID ++)

		//如果谓词的边规模 < 门槛 
	    if(edge[preID].size() <threshold)
	    {
	    	for(int p = 0; p < edge[preID].size(); p ++)
	        {

	        	int A = edge[preID][p].first, B = edge[preID][p].second;
	            int parentA = getParent(A, parent), parentB = getParent(B, parent);

	            if(rank[parentA]<rank[parentB])swap(parentA,parentB);
	            if(parentA!=parentB)
	            {
                    parent[parentB]=parentA;
                    sonCnt[parentA]+=sonCnt[parentB];
                    rank[parentA]=max(rank[parentA],rank[parentB]+1);
	            }
	        }
	        choice[preID] = 1; optim ++;
	    }
	printf("opt: %d\n", optim);
	greed1(choice, parent, sonCnt, rank, invalid);

	int crossEdge = 0;
	for(int preID = 1; preID <= svCnt; preID ++)
		if(choice[preID] == 0)	cout << preID << " " << IDToPredicate[preID] << endl, crossEdge++;
	printf("crossEdge: %d\n",crossEdge);
	printf("\n");
	unionBlock(choice,part);
}

void graph::greed1(vector<int> &choice,vector<int> &curParent,vector<int> &curSonCnt,vector<int> &curRank,vector<bool> &invalid)
{
	//越小越好
	int nextMinCost = 0x3f3f3f3f;

	if(true)
	{
		vector<int> nextBestParent;
		vector<int> nextBestSonCnt;
		vector<int> nextBestRank;
		vector<int> nextBestChoice(choice);
		for(int preID = 1; preID <= svCnt; preID ++)
			//如果 当前谓词作为交叉边 且 当前谓词有效
			if(choice[preID]==0&&!invalid[preID])
			{
				vector<int> nextParent(curParent);
				vector<int> nextSonCnt(curSonCnt);
				vector<int> nextRank(curRank);
				int nextBlockNum=0;
				bool flag=0;
				unordered_map<int, int>::iterator it;
				unordered_map<int, int> cost;
				int curMax = -1;

				//同一谓词森林 下 树之间的合并
				for(it = coarseningPoint[preID].begin(); it != coarseningPoint[preID].end(); it ++)
				{
					int point = it -> first;
					int parentA = getParent(point, nextParent), parentB=getParent(getParentMap(point,coarseningPoint[preID]),nextParent);
					if(nextRank[parentA]<nextRank[parentB])swap(parentA,parentB);
					if(parentA!=parentB)
					{
						nextParent[parentB]=parentA;
						nextSonCnt[parentA]+=nextSonCnt[parentB];
						nextRank[parentA]=max(nextRank[parentA],nextRank[parentB]+1);
						if(nextSonCnt[parentA] > limit)
						{
							flag=1;
							break;
						}
						curMax = max(nextSonCnt[parentA] , curMax);
					}
				}
				
				if(flag)continue;

				double curCost = (double) curMax / edge_weight[IDToPredicate[preID]];
				// cout << "cost of ID:" << preID << " " << curCost << endl;
				// cout << preID << " " << IDToPredicate[preID] << " " << edge_weight[IDToPredicate[preID]] << endl;

				//全部森林中 树的数量
				for(int p = 1; p <= entityCnt; p ++)	if(getParent(p, nextParent) == p)	nextBlockNum++;

				if(!nextMinCost || curCost < nextMinCost)
				{
					nextMinCost = curCost;	
					nextBestParent.assign(nextParent.begin(),nextParent.end());
					nextBestSonCnt.assign(nextSonCnt.begin(),nextSonCnt.end());
					nextBestRank.assign(nextRank.begin(),nextRank.end());
					nextBestChoice.assign(choice.begin(),choice.end());
					nextBestChoice[preID]=1;
					// cout << preID << endl;
				}
			}

		choice.assign(nextBestChoice.begin(),nextBestChoice.end());
		curParent.assign(nextBestParent.begin(),nextBestParent.end());
		curSonCnt.assign(nextBestSonCnt.begin(),nextBestSonCnt.end());
		curRank.assign(nextBestRank.begin(),nextBestRank.end());
		// for(int i = 1; i <= preType; ++ i)
		// 	cout << "choice of " << i << "is " << choice[i] << endl;
		// cout << endl;
	}

	if(nextMinCost != 0x3f3f3f3f)			greed1(choice, curParent, curSonCnt, curRank, invalid);
}

//对cur中的1 计数
int 
graph::cal(long long cur)
{
	int ret=0;
	while(cur)ret+=(cur&1),cur>>=1;
	return ret;
}

//最大化内部属性的个数,判断当前选择的内部属性个数是否>现有情况
void 
graph::compareCrossingEdgeCnt(long long cur)
{
	//c cur中1的个数,即选择的内部属性的个数  cnt cur中0对应的 pre标签的边集规模的和
	int c=0,cnt=0;
	for(int i=0;i<preType;i++)
	{
		if(cur&(1LL<<i))c++;
		else cnt+=edge[i+1].size();
	}
	if(cal(ans)<c)ans=cur,crossEgdeCnt=cnt;
	else if(cal(ans)==c&&crossEgdeCnt>cnt)ans=cur,crossEgdeCnt=cnt;
}

void 
graph::greed2()
{
	printf("greed2\n");
	invalid=vector<bool>(preType + 1,0);
	int threshold = entityCnt * 0.0001;
	vector<int> fa(entityCnt+1);
	vector<int> FA(entityCnt+1);
    for(int i=1;i<=entityCnt;i++)fa[i]=FA[i]=i;
    vector<int> RANK(entityCnt+1,0);

	//SONCNT record the size of each weakly connected component in final result
	vector<int> SONCNT(entityCnt+1,1);

	//choice is used to determine if a property is selected as internal, 1 means internal
    vector<int> choice(preType+1,0);
    vector<pair<int,int> >arr;

    for(int preID=1;preID<=preType;preID++){
		//如果preID对应的边集规模小于门槛
	    if(edge_cnt[IDToPredicate[preID]]<threshold)
	    {
			//枚举preID对应边集的所有边
	    	for(int p=0;p<edge[preID].size();p++)
	        {

	        	int A=edge[preID][p].first,B=edge[preID][p].second;
	            int parentA=getParent(A,FA),parentB=getParent(B,FA);
				
				//令A对应color的权重 > B
	            if(RANK[parentA]<RANK[parentB])swap(parentA,parentB);
	            if(parentA!=parentB)
	            {
                    FA[parentB]=parentA;
                    SONCNT[parentA]+=SONCNT[parentB];
                    RANK[parentA]=max(RANK[parentA],RANK[parentB]+1);
	            }
	        }

			//把preID这个作为内部属性
	        choice[preID]=1;
	    }
		//如果preID对应的边集规模 >= 门槛
	    else
	    {
	        vector<int> parent(fa);
	        vector<int> sonCnt=vector<int>(entityCnt+1,1);
			
			//int MaxCntSize = 0;
	        for(int p=0;p<edge[preID].size();p++)
	        {

	        	int A=edge[preID][p].first,B=edge[preID][p].second;
	            int parentA=getParent(A,parent),parentB=getParent(B,parent);

	            if(parentA!=parentB)
	            {
                    parent[parentB]=parentA;
					//sonCnt record the size of each weakly connected component in intermediate result
                    sonCnt[parentA]+=sonCnt[parentB];
					//if(sonCnt[parentA]>MaxCntSize){
					//	MaxCntSize = sonCnt[parentA];
					//}
                    if(sonCnt[parentA]>limit)
                    {
                        invalid[preID]=1;
                        printf("invalid: %d\n",preID);
                        break;
                    }
	            }
	        }
	        if(invalid[preID])continue;
	        
			//sorting properties by the numbers of weakly connected components
	        int SonCntNum=0;

			//确定preID森林中 树根的数量
	        for(int p=1;p<=entityCnt;p++)if(getParent(p,parent)==p)SonCntNum++;
			arr.push_back(make_pair(SonCntNum,preID));

	        //arr.push_back(make_pair(MaxCntSize,preID));
	    }
	}

	sort(arr.begin(),arr.end());
	/*put as many properties as possbile into the internal properties*/
    for(int i=arr.size()-1;i>=0;i--)
	//for(int i=0;i < arr.size();i++)
    {
        int preID=arr[i].second;
        // cout<<IDToPredicate[preID]<<" "<<arr[i].first<<endl;
        for(int p=0;p<edge[preID].size();p++)
        {

        	int A=edge[preID][p].first,B=edge[preID][p].second;
            int parentA=getParent(A,FA),parentB=getParent(B,FA);

            if(RANK[parentA]<RANK[parentB])swap(parentA,parentB);
            if(parentA!=parentB)
            {
                FA[parentB]=parentA;
                SONCNT[parentA]+=SONCNT[parentB];
                RANK[parentA]=max(RANK[parentA],RANK[parentB]+1);

				//cost(preID) > limit 选不了了
                if(SONCNT[parentA]>limit)
                {
                    invalid[preID]=1;
                    break;
                }
            }
        }
        if(invalid[preID])break;
        choice[preID]=1;
    }

	int crossEdge=0;
	// for(int preID=1;preID<=preType;preID++)
	// 	if(choice[preID]==0)cout<<preID<<"	"<<IDToPredicate[preID]<<endl,crossEdge++;
	printf("crossEdge: %d\n",crossEdge);
	printf("\n");
	unionBlock(choice,part);
}

void 
graph::greed3()
{
	printf("greed3\n");
	invalid = vector<bool>(svCnt + 1, 0);
	int threshold = entityCnt * 0.0001;
	vector<int> fa(entityCnt + 1);
	vector<int> FA(entityCnt + 1);
    for(int i = 1; i <= entityCnt; i ++)		fa[i] = FA[i] = i;
    vector<int> RANK(entityCnt + 1, 0);

	//SONCNT record the size of each weakly connected component in final result
	vector<int> SONCNT(entityCnt + 1, 1);

	//choice is used to determine if a property is selected as internal, 1 means internal
    vector<int> choice(svCnt + 1, 0);
    vector<pair<double, int> >arr;

    for(int preID = 1; preID <= svCnt; preID ++)
	{
		//如果preID对应的边集规模小于门槛
	    if(edge[preID].size() < threshold)
	    {
			//枚举preID对应边集的所有边
	    	for(int p = 0; p < edge[preID].size(); p ++)
	        {

	        	int A = edge[preID][p].first, B = edge[preID][p].second;
	            int parentA = getParent(A, FA), parentB = getParent(B, FA);
				
				//令A对应color的权重 > B
	            if(RANK[parentA] < RANK[parentB])	swap(parentA, parentB);
	            if(parentA != parentB)
	            {
                    FA[parentB] = parentA;
                    SONCNT[parentA] += SONCNT[parentB];
                    RANK[parentA] = max(RANK[parentA], RANK[parentB] + 1);
	            }
	        }

			//把preID这个作为内部属性
	        choice[preID] = 1;
	    }
		//如果preID对应的边集规模 >= 门槛
	    else
	    {
	        vector<int> parent(fa);
	        vector<int> sonCnt = vector<int>(entityCnt + 1, 1);
			
			//int MaxCntSize = 0;
	        for(int p = 0; p < edge[preID].size(); p ++)
	        {

	        	int A = edge[preID][p].first, B = edge[preID][p].second;
	            int parentA = getParent(A, parent), parentB = getParent(B, parent);

	            if(parentA != parentB)
	            {
                    parent[parentB] = parentA;
					//sonCnt record the size of each weakly connected component in intermediate result
                    sonCnt[parentA] += sonCnt[parentB];
					//if(sonCnt[parentA]>MaxCntSize){
					//	MaxCntSize = sonCnt[parentA];
					//}
                    if(sonCnt[parentA] > limit)
                    {
                        invalid[preID] = 1;
                        printf("invalid: %d\n",preID);
                        break;
                    }
	            }
	        }
	        if(invalid[preID])		continue;
	        
			//sorting properties by the numbers of weakly connected components
	        int SonCntNum = 0;

			//确定preID森林中 树根的数量
	        for(int p = 1; p <= entityCnt; p ++)	
				if(getParent(p, parent) == p)	SonCntNum ++;
			arr.push_back( {SonCntNum * edge_weight[IDToEntity[preID]], preID} );

	        //arr.push_back(make_pair(MaxCntSize,preID));
	    }
	}

	sort(arr.begin(), arr.end());
	/*put as many properties as possbile into the internal properties*/

	// for(auto it = arr.begin(); it != arr.end(); ++ it)
	// 	cout << it->first << " " << it->second << endl;


    for (int i = arr.size() - 1; i >= 0; i --)
	//for(int i=0;i < arr.size();i++)
    {
        int preID = arr[i].second;
        // cout<<IDToPredicate[preID]<<" "<<arr[i].first<<endl;
        for(int p = 0; p < edge[preID].size(); p ++)
        {

        	int A = edge[preID][p].first, B = edge[preID][p].second;
            int parentA = getParent(A, FA), parentB = getParent(B, FA);

            if(RANK[parentA] < RANK[parentB])		swap(parentA, parentB);
            if(parentA != parentB)
            {
                FA[parentB] = parentA;
                SONCNT[parentA] += SONCNT[parentB];
                RANK[parentA] = max(RANK[parentA], RANK[parentB] + 1);

				//cost(preID) > limit 选不了了
                if(SONCNT[parentA] > limit)
                {
                    invalid[preID] = 1;
                    break;
                }
            }
        }
        if(invalid[preID])		break;
        choice[preID] = 1;
    }

	int crossEdge = 0;
	// for(int preID=1;preID<=preType;preID++)
	// 	if(choice[preID]==0)cout<<preID<<"	"<<IDToPredicate[preID]<<endl,crossEdge++;
	printf("crossEdge: %d\n", crossEdge);
	printf("\n");
	unionBlock(choice, part);
}

void 
graph::unionBlock(vector<int> &choice,int goal)
{
	cout << "unionBlock: " << endl;
	vector<int>parent(entityCnt + 1);
	for(int p = 1; p <= entityCnt; p ++)		parent[p] = p;
	vector<int> rank = vector<int>(entityCnt + 1, 0);
	for(int preID = 1; preID <= svCnt; preID ++)	
		if(choice[preID] == 1)
		{
			for(int p = 0; p < edge[preID].size(); p ++)
			{
				int parentA = getParent(edge[preID][p].first,parent);
				int parentB = getParent(edge[preID][p].second, parent);
				if(rank[parentA] < rank[parentB])	swap(parentA, parentB);
				if(parentA != parentB)
				{
					rank[parentA] = max(rank[parentA], rank[parentB] + 1);
					parent[parentB] = parentA;
					entityTriples[parentA] += entityTriples[parentB];
				}
			}
		}
	vector<pair<int,int> >block;
	int blockNum = 0;
	for(int p = 1; p <= entityCnt; p ++)	
		if(p == getParent(p, parent))
			block.push_back( {entityTriples[p], p} ), ++ blockNum;
	printf("blockNum: %d\n", blockNum);
	
	sort(block.begin(), block.end());

	//小根堆	<entityTriples, 加入序号>
	priority_queue<pair<int,int>, vector<pair<int,int> >, greater<pair<int,int> > > Q;

	for(int i = 1; i <= goal; i ++)		Q.push(make_pair(0, i));
	vector<int> blockTogoal(entityCnt + 1, 0);
	for(int i = block.size() - 1; i >= 0; i --) 
	{
		pair<int,int> tmp = Q.top();
		Q.pop();
		tmp.first += block[i].first;

		//tmp = make_pair(block[i].first, 加入序号)
		blockTogoal[block[i].second] = tmp.second;
		Q.push(tmp);
	}

	//相当于给block加了序号
	while(!Q.empty())
	{
		printf("%d %d\n", Q.top().first, Q.top().second);
		Q.pop();
	}

	vector<int>CNT(goal + 1, 0);

	// if(access(RDF.c_str(), 0)==-1)
	// 	mkdir(RDF.c_str(),0777);
	// ofstream outFile("/opt/workspace/PCP/"+RDF+"InternalPoints.txt"); 
	ofstream outFile(RDF + "InternalPoints.txt");   
	for(int pos, p = 1; p <= entityCnt; p ++)
	{
		string t = IDToEntity[p];
		pos = blockTogoal[getParent(p, parent)];
		int groupID = pos - 1;
		group[t] = groupID;
		outFile << t << "	" << groupID << endl ;
		CNT[pos] ++;
	}
	outFile.close();
	printf("\n");
	for(int i = 1; i <= goal; i ++)	printf("%d %d\n", i, CNT[i]);

	// ofstream File("/opt/workspace/PCP/"+RDF+"crossingEdges.txt"); 
	ofstream File(RDF+"crossingEdges.txt"); 
	File << "Predicate" << "\t" << "isCrossEgde" << "\t" << "weight" << endl << endl;
	for(unordered_map<string,int>::iterator it=edge_cnt.begin();it!=edge_cnt.end();it++)
	{
		File<<it->first<<"\t"<<it->second<<"\t";
		if(predicateToID.count(it->first))
			File << (!choice[predicateToID[it->first]]);
		else File << "0";
		File << '\t' << edge_weight[it->first] << endl;
	}

	for (int i = preType + 1; i <= svCnt; ++ i)
	{
		File << IDToPredicate[i] << "\t" << edge[i].size() << "\t";
		File << (!choice[i]) << "\t";
		File << edge_weight[IDToPredicate[i]] << endl;
	}

	File.close();
	// update();
}

void 
graph::metis(string txt_name,string tag)
{
	ifstream in(txt_name.data());
	string str;
	entityCnt=0;
	vector<vector<int> > EDGE;
	EDGE.push_back(vector<int>());
	triples=0;
	int edge_count = 0;
	while(getline(in,str))
	{
		triples++;
		if(triples % 10000 == 0)
			cout << "loading triples : " << triples << endl;
        str.resize(str.length()-2);
        vector<string> s;
        s=split(str,tag);
        predicate.insert(s[1]);
        for(int i=0;i<3;i+=2)if((s[i][0]=='<'||s[i][0]=='_')&& entityToID.count(s[i])==0)
        {
                entityToID[s[i]]=++entityCnt;
                IDToEntity.push_back(s[i]);
                EDGE.push_back(vector<int>());
        }
        if(entityToID.count(s[0]) != 0 && entityToID.count(s[2]) != 0){
            EDGE[entityToID[s[0]]].push_back(entityToID[s[2]]);
            EDGE[entityToID[s[2]]].push_back(entityToID[s[0]]);
            edge_count++;
        }
	}
	
	ofstream out((RDF + ".tmp").data());
	out << entityCnt << " " << edge_count << endl;
	for(int i=1;i<EDGE.size();i++)
	{
		for(int j = 0;j < EDGE[i].size();j++)
			out << EDGE[i][j] << " ";
        out << endl;
	}

	stringstream cmd_ss;
    cmd_ss << "./gpmetis " << RDF << ".tmp " << part;
    cout << cmd_ss.str().c_str() << endl;
	system(cmd_ss.str().c_str());
	stringstream metis_ss;
    metis_ss << "./" << RDF << ".tmp.part." << part;
	cout << metis_ss.str().c_str() << endl;
	ifstream In(metis_ss.str().c_str());
	ofstream Out(("./"+RDF+"METISInternalPoints.txt").data());
	int idx=1;
	while(getline(In,str))
	{
        Out<<IDToEntity[idx++]<<"\t"<<atoi(str.c_str())<<endl;
	}
}

void
graph::partition(string txt_name, string tag, string out_file)
{
	string line;
	ifstream readGraph(txt_name.data());
	triples = 0;

	vector<pair<pair<string,string>, string> > tmp;
	for(int i = 1; i <= part; ++ i)
		edgeGroup.push_back(tmp);

	while(getline(readGraph, line))
	{
		triples++;
		if(triples % 10000 == 0)
			cout << "grouping triples : " << triples << endl;
        line.resize(line.length()-2);
        vector<string> s;
        s = split(line, tag);

		int u = entityToID[s[0]], v = entityToID[s[2]], p = predicateToID[s[1]];
		// // if(u == 0)	continue;
		int uID = group[s[0]], vID = group[s[2]];
		// cout << triples << ":" << u << " " << v << " ";
		edgeGroup[uID].push_back({{s[0], s[2]}, s[1]});
		if(uID != vID && v != 0)
			edgeGroup[vID].push_back({{s[0], s[2]}, s[1]});
	}

	int partCnt = part;
	
	for(int i = 0; i < partCnt; ++ i)
	{
		string SubGraphName = out_file + to_string(i) + ".txt";
		ofstream out(SubGraphName);
		int groupSize = edgeGroup[i].size();
		cout << groupSize << endl;
		for(int j = 0; j < groupSize; ++ j)
			out << edgeGroup[i][j].first.first << " " << edgeGroup[i][j].second << " " << edgeGroup[i][j].first.second << " ." << endl;
		out.close();
	}	
	// cout << "ID : " << entityToID[""] << endl;
}

void 
graph::getFileList(string template_path, string result_path)
{
	if (template_path.find_last_of("/") != template_path.size() - 1)
        template_path += "/";
	if (result_path.find_last_of("/") != result_path.size() - 1)
		result_path += "/";

	DIR * dir_1, * dir_2;
	struct dirent * ptr;
	int i = 0, cnt = 1;
	dir_1 = opendir(template_path.data()); 
	while((ptr = readdir(dir_1)) != NULL) 
	{
		if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0)
            continue;
		string filename = ptr->d_name;
		query_template.push_back(filename);
		queryID[filename] = cnt ++;
	}
	closedir(dir_1);
	i = 0;
	dir_2 = opendir(result_path.data()); 
	while((ptr = readdir(dir_2)) != NULL) 
	{
		if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0)
            continue;
		string result = ptr->d_name;
		query_result.push_back(result);
	}
	closedir(dir_2);
	// cout << queryID["F5.txt"] << endl;
	queryCnt = -- cnt;
	for (auto it : query_template)	cout << it << endl;
	cout << endl;
	for (auto it : query_result)	cout << it << endl;
}

void 
graph::readQueryResult(string template_path, string result_path, string tag)
{
	//initialize patternQueryNode
	vector<string> tmp;
	for(int i = 0; i < queryCnt + 1; ++ i)
		patternQueryNode.push_back(tmp);
	validResultCnt = 0;
	// mkdir("./tmp/");

	string str, t;
	size_t pos = 0, pos_v0 = 0;
	int id;

	for(auto & filename : query_result)
	{
		id = queryID[filename];
		
		ifstream in_result((result_path + "/" + filename).data());

		getline(in_result, str);
		if (str == "")		continue;
		++ validResultCnt;

		stringstream ss(str);
		while(ss >> t)		patternQueryNode[id].push_back(t);

		ofstream out(("./tmp/" + filename + "_tmp.txt").data());
		//先读结果 

		vector<pair<int, int> > resultTriples;

		while(getline(in_result, str))
		{
			int lineLen = str.size();
			s = split(str, tag);

			//匹配到查询pattern

			ifstream in_template((template_path + "/" + filename).data());
			getline(in_template, str);
			//逐行替换
			while(getline(in_template, str))
			{
				if(str == "}")	break;

				s0 = split(str, "\t");

				int vCnt = patternQueryNode[id].size();
				for(int i = 0; i < vCnt; ++ i)
				{
					string v = patternQueryNode[id][i];
					pos = str.find(v);
					if(pos != string::npos)
					{
						str = str.replace(pos, v.size(), s[i]);
					}
				}
				str.resize(str.size() - 3);
				out << str << endl;

				s1 = split(str, "\t");
				int a = entityToID[s1[0]], b = entityToID[s1[2]];
				if (a && b)	resultTriples.push_back( {a, b} );
			}
			in_template.close();
		}
		in_result.close();
		out.close();
		edge.push_back(resultTriples);
		IDToPredicate.push_back("multiPre_" + filename);
	}
	
	// puts("-----------------------------------");
	// for (int i = 1; i <= validResultCnt; ++ i)
	// {
	// 	cout << i << "'s size is : " << edgeOfMultiPre[i].size() << endl;
	// }
}