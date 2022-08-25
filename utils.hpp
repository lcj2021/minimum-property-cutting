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

#include <string> 
#include <vector> 
#include <cstring> 
#include <unordered_map> 


class Utils
{
    public: 
    static std::vector<std::string> split(std::string textline, std::string tag);
    static int count1InBinary(long long decimal);
    static int getParent(int son, std::vector<int> &fa);
    static int getParentMap(int son, std::unordered_map<int, int> &fa);
    static void trim(std::string &textline);
};