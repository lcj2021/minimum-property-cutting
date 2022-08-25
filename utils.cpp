/*=============================================================================
# Filename: Util.h
# Author: ChanningLcj
# Mail: 1012889296@qq.com
# Last Modified: 2022-1-22 10:43
# Description: 
1. 
2. common macros, functions, classes, etc
# Notice: 
=============================================================================*/

#include "utils.hpp"

using namespace std;


// split entity from textline by seperate tag 
vector<string> 
Utils::split(string textline, string tag)
{
    // textline = textline.substr(0, textline.find_last_of(" "));
	vector<string> res;
	if (tag == "\t")
	{
		size_t pre_pos = 0;
		size_t pos = textline.find(tag);

		//依次处理line中的每个tag数据
		while (pos != string::npos)
		{
			string curStr = textline.substr(pre_pos, pos - pre_pos);
			curStr.erase(0, curStr.find_first_not_of("\r\t\n "));
			curStr.erase(curStr.find_last_not_of("\r\t\n ") + 1);

			if (strcmp(curStr.c_str(), "") != 0)
				res.push_back(curStr);
			pre_pos = pos + tag.size();
			pos = textline.find(tag, pre_pos);
		}

		string curStr = textline.substr(pre_pos, pos - pre_pos);
		curStr.erase(0, curStr.find_first_not_of("\r\t\n "));
		curStr.erase(curStr.find_last_not_of("\r\t\n ") + 1);
		if (strcmp(curStr.c_str(), "") != 0)
			res.push_back(curStr);
	}
	else
	{
		string curStr = textline;
		string subjectStr = curStr.substr(0, curStr.find(" "));
		curStr = curStr.substr(curStr.find(" ") + 1);
		string predicateStr = curStr.substr(0, curStr.find(" "));
		curStr = curStr.substr(curStr.find(" ") + 1);
		string objectStr = curStr;
		// cout << objectStr << 1 << endl;

		res.push_back(subjectStr);
		res.push_back(predicateStr);
		res.push_back(objectStr);
	}

	return res;
}

// count the cnt of 1 in binary form for decimal
int 
Utils::count1InBinary(long long decimal)
{
	int ret = 0;
	while (decimal)
		ret += (decimal & 1), decimal >>= 1;
	return ret;
}

int 
Utils::getParent(int son, vector<int> &fa)
{
	// return fa[son]==son?son:fa[son]=getParent(fa[son],fa);
	int i, j, k;
	k = son;
	while (k != fa[k])
		k = fa[k];
	i = son;
	while (i != k)
	{
		j = fa[i];
		fa[i] = k;
		i = j;
	}
	return k;
}

int 
Utils::getParentMap(int son, unordered_map<int, int> &fa)
{
	int i, j, k;
	k = son;
	while (k != fa[k])
		k = fa[k];
	i = son;
	while (i != k)
	{
		j = fa[i];
		fa[i] = k;
		i = j;
	}
	return k;
}

void 
Utils::trim(string &textline)
{
	if (textline.empty())	return ;
	textline.erase(0, textline.find_first_not_of(" "));
	textline.erase(textline.find_last_not_of(" ") + 1);
}
