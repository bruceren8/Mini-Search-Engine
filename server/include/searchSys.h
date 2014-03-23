#ifndef _H_SYS_
#define _H_SYS_

#include <iostream>
#include <sstream>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <cmath>
#include <unistd.h>
#include <string>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include "SegInterface_Use.h"
#include "PosInterface_Use.h"
#include "codetransformer.h"
#include "JTCharCodeConvert.h"

using namespace std;

/***********************************/
//读取一行字符串(删掉换行符)到str中,
//成功返回读取的总个数，失败返回-1.
/**********************************/
int getline(FILE *fp, string &str);

/***********************************/
//获取标记flag的位置
//返回值:成功时返回flag后面的位置，读到stopflag或找不到标记时返回-1
/***********************************/
long getFlagLoc(FILE *fp, string flag, string stopflag, long offset = 0);

/***********************************/
//读取文件中flag1到flag2之间的内容到string中
/***********************************/
int getStrByFlag(FILE *fp, string flag1, string flag2, string &result, long offset = 0);

bool sortFunc(pair<string, int> i, pair<string, int> j);

/***********************************/
//拆分字符串
/***********************************/
void stringSplit(const string &str, const char split, vector<string> &result);

/***********************************/
//文件操作类
/***********************************/
class FileOperator
{
	private:
		string filePath;
		int fileSize;
	public:
		FileOperator(string path):filePath(path){}
		FILE* open_read();//以读的方式打开
		FILE* open_write();//以写的方式打开
		int getSize();//读文件时获取文件大小
};

/***********************************/
//word类
/***********************************/
struct Word
{
	string str;
	int tf;
	int df;
	float weight;
public:
	Word(string s, int t, int d, float w)
		:str(s), tf(t), df(d), weight(w){}
};
/***********************************/
//网页类
/***********************************/
struct Page
{
	int id;
	string url;
	string title;
	long content_addr;//<content>标记的第一个位置(<的位置)
	vector<Word> words;
	map<string, float> eigenVec;
	int groupNum;
public:
	Page(int i, string u, string t, long c)
	:id(i), url(u), title(t), content_addr(c){}
};

/***********************************/
//搜索引擎系统类
/***********************************/
class SearchSys
{
public:
	string index_file;
	string page_file;
	string query_file;
	string stopword_file;
	set<string> excluded;
	char ip[16];
	int port;
	int flagEV;
	int top10EquelNum;
	float groupLimen;

	map<string, vector<Page> > uniPage;	//去重后的结果集 
	map<string, vector<vector<Page> > > cluster;//每个查询词对应的分组
public:
	SearchSys(const char *conf_path);

	int init();//初始化系统

	int buildIndex();//建立索引
	//long getQueryLoc(FILE *fp, string query);//求查询词的偏移量
	int saveIndex(FILE *fp, string query, long offset);//保存索引文件

	int uniquePage(map<string, vector<Page> > &result);//网页去重
	int readIndex(FILE *fp, string &query, long &queryloc);//读索引文件
	vector<string> getTop10(string &content);//获取top10
	int segStr(string &content, string &result);
	string getEigenStr(string &content);//获取top10
	bool isUniqueTop10(vector<string> vec1, vector<string> vec2);
	bool isUniqueStr(string &str1, string &str2);

	int groupPage(map<string, vector<Page> > &uniPage);
	int setWeight(FILE *fp_page, vector<Page> &pages);
	int getMatrix(vector<Page> &pages, float *matrix);
	float getEigenVec(Page &p1, Page &p2);
	int getCluster(vector<Page> &pages, vector<vector<Page> > &group);
	int getClassCnt(string query);
	string search(string query, int tag, int classNum);
	void printPage(Page p, string &result);
};

#endif
