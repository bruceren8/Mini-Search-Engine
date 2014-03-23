#include "searchSys.h"

/***********************************/
//系统类构造函数(读取配置文件)
/***********************************/
SearchSys::SearchSys(const char *conf_path)
{
	FileOperator fr(conf_path);
	FILE * fp = fr.open_read();
	int conf_size = fr.getSize();

	char *buf = new char[conf_size];
	memset(buf, 0, conf_size);
	int nread = fread(buf, 1, conf_size, fp);

	string str_temp, str_1, str_2;
	int i;
	for (i = 0; i < nread; i++)
	{
		switch (buf[i])
		{
			case '<':
				while (buf[i] != '\n')
					++i;
				break;
			case '=':
				str_1 = str_temp;
				str_temp.clear();
				break;
			case '\n':
			case '\0':
				str_2 = str_temp;
				if (!strcmp(str_1.c_str(), "index_file"))
				{
					index_file = str_2;
				}
				else if (!strcmp(str_1.c_str(), "page_file"))
				{
					page_file = str_2;
				}
				else if (!strcmp(str_1.c_str(), "query_file"))
				{
					query_file = str_2;
				}
				else if (!strcmp(str_1.c_str(), "stopword_file"))
				{
					stopword_file = str_2;
				}
				else if (!strcmp(str_1.c_str(), "ip"))
				{
					//ip = str_2.c_str();
					strcpy(ip, str_2.c_str());
				}
				else if (!strcmp(str_1.c_str(), "port"))
				{
					port = atoi(str_2.c_str());
				}
				else if (!strcmp(str_1.c_str(), "top10EquelNum"))
				{
					top10EquelNum = atoi(str_2.c_str());
				}
				else if (!strcmp(str_1.c_str(), "flagEV"))
				{
					flagEV = atoi(str_2.c_str());
				}
				else if (!strcmp(str_1.c_str(), "groupLimen"))
				{
					groupLimen = atof(str_2.c_str());
				}
				str_1.clear();
				str_2.clear();
				str_temp.clear();
				break;
			default:
				str_temp += buf[i];
		}
	}
	FileOperator fr_sw(stopword_file);
	FILE *fp_sw = fr_sw.open_read();
	string stopWord;
	while (getline(fp_sw,stopWord) != -1)
	{
		//cout << stopWord << " ";
		excluded.insert(stopWord);
	}
	fclose(fp_sw);//error
	fclose(fp);
}

/***********************************/
//初始化系统
/***********************************/
int SearchSys::init()
{
	///*
	//cout << "Is it need to build index?(Y)";
	//char ch;
	//cin >> ch;
	//if (ch == 'y' || ch == 'Y')
	//{
		cout << "build index..." << endl;
		time_t time1, time2;
		double diff1;
		time(&time1);

		buildIndex();

		time(&time2);
		diff1 = difftime(time2, time1);
		cout << "build index OK! run time:" << diff1 << " sec." << endl << endl;
	//}
	//*/
	///*
	cout << "uniquePage..." << endl;
	time_t time3, time4;
	double diff2;
	time(&time3);

	uniquePage(uniPage);

	time(&time4);
	diff2 = difftime(time4, time3);
	cout << "uniquePage ...OK! run time:" << diff2 << " sec." << endl << endl;
	//*/

	
	cout << "group page..." << endl;
	time_t time5, time6;
	double diff3;
	time(&time5);

	groupPage(uniPage);

	time(&time6);
	diff3 = difftime(time6, time5);
	cout << "group page ...OK! run time:" << diff3 << " sec." << endl << endl;
	
	return 0;
}

/***********************************/
//建立索引
/***********************************/
int SearchSys::buildIndex()
{
	FileOperator fr_query(query_file);
	FileOperator fr_page(page_file);
	FileOperator fw_index(index_file);
	FILE *fp_query = fr_query.open_read();
	FILE *fp_page = fr_page.open_read();
	FILE *fp_index = fw_index.open_write();

	string query;
	long offset = 0;
	while (getline(fp_query, query) != -1)
	{
		int flag = 1;
		while (flag)
		{
			if ((offset = getFlagLoc(fp_page, "<query>", "", offset)) == -1)
			{
				flag = 0;//找不到时退出循环
				continue;
			}
			string queryCmp;//用于比较查询词，不同时继续循环向下找
			if (getline(fp_page, queryCmp) == -1 || queryCmp.compare(query) != 0)
				continue;
			offset = ftell(fp_page);//获得此时偏移
			break;
		}
		if (offset == -1)
			continue;

		cout << "query:" << query << "\t" << offset << endl;
		saveIndex(fp_index, query, offset);
		query.clear();
		//offset = 0;//每查询一个词就回到文件头
	}

	fclose(fp_query);
	fclose(fp_page);
	fclose(fp_index);
	return 0;
}

/***********************************/
//获取标记flag的位置
//返回值:成功时返回flag后面的位置，读到stopflag或找不到标记时返回-1
/***********************************/
long getFlagLoc(FILE *fp, string flag, string stopflag, long offset)
{
	fseek(fp, offset, SEEK_SET);
	char ch;
	char *flagCmp = new char[flag.size()];
	memset(flagCmp, 0, sizeof(flagCmp));
	char *stopflagCmp;
	if (stopflag.size() > 1)//当结束标记>1时才有必要
	{
		stopflagCmp = new char[stopflag.size()];
		memset(stopflagCmp, 0, sizeof(stopflagCmp));
	}
	while (fread(&ch, 1, 1, fp) == 1)
	{
		if (ch == flag[0])//比较标记
		{
			int nread = fread(flagCmp, 1, flag.size()-1, fp);//读取标签的字节数
			flagCmp[flag.size()-1] = '\0';
			string flag2;
			flag2.assign(flag, 1, flag.size()-1);
			if (!strcmp(flagCmp, flag2.c_str()))//当标签匹配时
			{
				return ftell(fp);
			}
			fseek(fp, -nread, SEEK_CUR);
		}
		if (stopflag.size() > 0 && ch == stopflag[0])//比较结束标记
		{
			if (stopflag.size() == 1)//当结束标记只有一个字符时，此时已经匹配了
				return -1;
			int nread = fread(stopflagCmp, 1, stopflag.size()-1, fp);//读取结束标签的字节数
			stopflagCmp[stopflag.size()-1] = '\0';

			string stopflag2;
			stopflag2.assign(stopflag, 1, stopflag.size()-1);
			if (!strcmp(stopflagCmp, stopflag2.c_str()))//当结束标签匹配时
			{
				return -1;
			}
			fseek(fp, -nread, SEEK_CUR);
		}
	}
	return -1;
}

/***********************************/
//保存索引文件
/***********************************/
int SearchSys::saveIndex(FILE *fp, string query, long offset)
{
	string index(query);
	index += "\t";
	char temp[20];
	sprintf(temp, "%ld", offset);
	index += temp;
	index += "\n";
	if (fwrite(index.c_str(), 1, index.size(), fp) != index.size())
	{
		cerr << "err: write index!" << endl;
		exit(1);
	}
	return 0;
}

/***********************************/
//网页去重
/***********************************/

int SearchSys::uniquePage(map<string, vector<Page> > &result)
{
	FileOperator fr_index(index_file);
	FileOperator fr_page(page_file);
	FILE *fp_index = fr_index.open_read();
	FILE *fp_page = fr_page.open_read();

	string query;
	long queryLoc;//<query>后下一行的开头位置
	long docLoc;//<doc>一行开头的位置
	long docIdLoc;//<docid>的后一个位置
	int docId;
	long urlLoc;//<url>的后一个位置
	long contentLoc;//<content>标记的第一个位置(<的位置)
	string content;
	vector<string> eV1;//每个网页的特征值eigenValue
	map<long, vector<string> > allEV1;//一个查询词对应所有网页的分词top10:<<docid,top10>>
	string eV2;
	map<long, string > allEV2;//一个查询词对应所有网页的分词top10:<<docid,top10>>
	//multimap<string, pair<int,long> > result;//结果集:<query, <docid,offset> >
	//map<string, vector<Page> > result;
	int idCnt = 0;

	while (!readIndex(fp_index, query, queryLoc))//从索引文件中读取
	{
		cout << query << "..." << endl;
		docLoc = queryLoc;//网页在文件中的偏移
		//生成所有网页的top10
		while ((docLoc = getFlagLoc(fp_page, "<doc>", "</query>", docLoc)) != -1)//读取查询词对应的所有的网页
		{
			getStrByFlag(fp_page, "<content>", "</content>", content, ftell(fp_page));//获取内容
			//cout << "content:" << content << endl;
			//cout << "docid:" << idCnt << endl;
			if (flagEV == 1)
			{
				eV1 = getTop10(content);//获取top10
				content.clear();
				pair<long, vector<string> > p(docLoc, eV1);
				if (eV1.size() == 10)
					allEV1.insert(p);//加入集合
				eV1.clear();
			}
			else
			{
				eV2 = getEigenStr(content);//获取每个","前后的5个字符
				pair<long, string> p(docLoc, eV2);
				//cout << "eV:" << eV2 << endl;
				if (eV2.size() != 0)
					allEV2.insert(p);//加入集合
				eV2.clear();
			}
			++idCnt;
		}

		//网页去重
		//方法1(top10):
		if (flagEV == 1)
		{
			map< long, vector<string> >::iterator iter1;//用于对网页遍历
			map< long, vector<string> >::iterator iter2;//用于遍历iter1之后的网页进行重复判断
			bool uniqueFlag = true;//标记是否唯一
			for (iter1=allEV1.begin(); iter1!=allEV1.end(); ++iter1)//对每一个网页判断是否重复
			{
				for (iter2 = ++iter1, --iter1; iter2 != allEV1.end(); ++iter2)
				{
					if ((uniqueFlag = isUniqueTop10(iter1->second, iter2->second)) == false)
						break;//iter1重复时跳出iter2的循环比较
				}
				if (iter2 == allEV1.end())//iter到最后一个网页时直接判断不重复
					uniqueFlag = true;
				if (uniqueFlag)//iter1唯一时加入结果集
				{
					docIdLoc = getFlagLoc(fp_page, "<docid>", "", iter1->first);
					string docIdStr;
					getline(fp_page, docIdStr);
					docId = (int)strtol(docIdStr.c_str(), NULL, 10);
					//cout << "insert id: " << docId << endl;
					urlLoc = getFlagLoc(fp_page, "<url>", "", docIdLoc);
					string url;
					getline(fp_page, url);
					getFlagLoc(fp_page, "<title>", "", urlLoc);
					string title;
					getline(fp_page, title);
					contentLoc = getFlagLoc(fp_page, "<content>", "", urlLoc) - 9;
					Page page(docId, url, title, contentLoc);

					map<string, vector<Page> >::iterator it = result.find(query);
					if (it != result.end())//若结果集map中已有query对应的项
					{
						it->second.push_back(page);
					}
					else
					{
						vector<Page> vec;
						vec.push_back(page);
						pair<string, vector<Page> > p(query, vec);
						result.insert(p);
					}
				}
			}
		}
		//方法2(LCS):
		else
		{
			map< long, string>::iterator iter1;//用于对网页遍历
			map< long, string>::iterator iter2;//用于遍历iter1之后的网页进行重复判断
			bool uniqueFlag = true;//标记是否唯一
			for (iter1=allEV2.begin(); iter1!=allEV2.end(); ++iter1)//对每一个网页判断是否重复
			{
				for (iter2 = ++iter1, --iter1; iter2 != allEV2.end(); ++iter2)
				{
					if ((uniqueFlag = isUniqueStr(iter1->second, iter2->second)) == false)
						break;//iter1重复时跳出iter2的循环比较
				}
				if (iter2 == allEV2.end())//iter到最后一个网页时直接判断不重复
					uniqueFlag = true;
				//uniqueFlag = true;//不去重，调试用
				if (uniqueFlag)//iter1唯一时加入结果集
				{
					docIdLoc = getFlagLoc(fp_page, "<docid>", "", iter1->first);
					string docIdStr;
					getline(fp_page, docIdStr);
					docId = (int)strtol(docIdStr.c_str(), NULL, 10);
					//cout << "insert id: " << docId << endl;
					long urlLoc = getFlagLoc(fp_page, "<url>", "", docIdLoc);
					string url;
					getline(fp_page, url);
					getFlagLoc(fp_page, "<title>", "", urlLoc);
					string title;
					getline(fp_page, title);
					long contentLoc = getFlagLoc(fp_page, "<content>", "", urlLoc) - 9;
					Page page(docId, url, title, contentLoc);

					map<string, vector<Page> >::iterator it = result.find(query);
					if (it != result.end())//若结果集map中已有query对应的项
					{
						it->second.push_back(page);
					}
					else
					{
						vector<Page> vec;
						vec.push_back(page);
						pair<string, vector<Page> > p(query, vec);
						result.insert(p);
					}
				}
			}
		}
				
		allEV1.clear();
		allEV2.clear();

		cout << "handle query: " << query << "...OK!"<< endl;
	}

	cout << "\nbefore unique: " << endl;
	cout << "old size: " << idCnt << endl;
	cout << "unique result: " << endl;
	int newSize = 0;
	map<string,vector<Page> >::iterator iter;
	for (iter = result.begin(); iter != result.end(); ++iter)
	{
		/*
		   cout << "query: " << iter->first << endl;
		   vector<Page>::iterator i;
		   for (i = iter->second.begin(); i != iter->second.end(); ++i)
		   {
		   cout << "id: " << i->id << endl;
		   cout << "title: " << i->title << endl;
		   cout << "url: " << i->url << endl;
		   cout << "content: " << i->content_addr << endl;
		   }
		   */
		newSize += iter->second.size();
	}
	cout << "new size: " << newSize << endl;


	fclose(fp_index);
	fclose(fp_page);
	return 0;
}


/***********************************/
//读索引文件
//读到文件尾时返回-1
/***********************************/
int SearchSys::readIndex(FILE *fp, string &query, long &queryloc)
{
	string line;
	if (getline(fp, line) == -1)
		return -1;
	string word;
	size_t i;
	for (i = 0; i != line.size(); ++i)
	{
		if (line[i] == '\t')
		{
			query = word;
			word.clear();
			continue;
		}
		word.push_back(line[i]);
	}
	queryloc = strtol(word.c_str(), NULL, 10);
	return 0;
}

/***********************************/
//读取文件中flag1到flag2之间的内容到string中
/***********************************/
int getStrByFlag(FILE *fp, string flag1, string flag2, string &result, long offset)
{
	fseek(fp, offset, SEEK_SET);
	long begin = getFlagLoc(fp, flag1, "", offset);
	long end = getFlagLoc(fp, flag2, "", begin) - flag2.size();
	if (end <= begin)
		return 1;

	int len = end - begin;
	char *str = new char[len + 1];
	memset(str, 0, sizeof(str));

	fseek(fp, begin, SEEK_SET);

	fread(str, 1, len, fp);
	str[len] = '\0';
	result = str;
	delete str;
	return 0;
}

bool sortFunc(pair<string, int> i, pair<string, int> j)
{
	return (i.second > j.second);
}

/***********************************/
//拆分字符串
/***********************************/
void stringSplit(const string &str, const char split, vector<string> &result)
{
	int begin = 0, end = 0;
	string word;
	string::const_iterator it;
	for (it = str.begin(); it != str.end(); ++it)
	{
		if (*it != split)
		{
			word.push_back(*it);
			++end;
		}
		else
		{
			if (word.size() == 0)
				continue;
			result.push_back(word);
			word.clear();
			begin = ++end;
		}
	}
	if (begin != end)
		result.push_back(word);
}

/***********************************/
//获取top10
/***********************************/
vector<string> SearchSys::getTop10(string &content)
{
	vector<string> top10;//top10
	string word;
	map<string, int> wordCnt;//对单词计数

	string result;
	segStr(content, result);

	stringstream stream(result);
	while (stream >> word)
	{
		//检测停用词
		if (!excluded.count(word))
		{
			//cout << word << " "; 
			++wordCnt[word];
		}
	}
	if (wordCnt.size() == 0)//当正文此时没有统计到单词时,退出
		return top10;
	vector< pair<string, int> > vecSort;//用于对map排序
	map<string, int>::iterator iter;
	for (iter = wordCnt.begin(); iter != wordCnt.end(); ++iter)
	{
		vecSort.push_back(*iter);
	}
	partial_sort(vecSort.begin(), vecSort.begin() + 10, vecSort.end(), sortFunc);
	int i;
	for (i = 0; i < 10; ++i)
	{
		//cout << vecSort[i].first << " > " << vecSort[i].second << endl;
		top10.push_back(vecSort[i].first);
	}
	//cout << endl;
	return top10;
}

/***********************************/
//
/***********************************/
int SearchSys::segStr(string &content, string &result)
{
	CCodeTransformer codeTransformer;
	wstring wstr;
	wstring wresult;
	CELUSSegOption::SetSEG_TAG("SingleSpace");
	wstr = codeTransformer.utf2wstr(content);
	CELUSSeg::BMMSeg(wstr, wresult);
	result = codeTransformer.wstr2utf(wresult);
	return 0;
}
/***********************************/
//判断top10是否唯一
/***********************************/
bool SearchSys::isUniqueTop10(vector<string> vec1, vector<string> vec2)
{
	int cnt = 0;
	//方法1:
	vector<string>::iterator iter1 = vec1.begin();
	vector<string>::iterator iter2 = vec2.begin();
	for (; iter1 != vec1.end(); ++iter1)
	{
		for (; iter2 != vec2.end(); ++iter2)
		{
			if (*iter1 == *iter2)
			{
				++cnt;
				break;
			}
		}
	}
	/*//方法2:更慢!
	  sort(vec1.begin(), vec1.end());
	  sort(vec2.begin(), vec2.end());
	  vector<string>::iterator iter1 = vec1.begin();
	  vector<string>::iterator iter2 = vec2.begin();
	  while (iter1 != vec1.end() && iter2 != vec2.end())
	  {
	  if (*iter1 == *iter2)
	  {
	  ++cnt;
	  ++iter1;
	  ++iter2;
	  }
	  else if (*iter1 < *iter2)
	  ++iter1;
	  else
	  ++iter2;
	  }
	  */
	if (cnt >= top10EquelNum)
		return false;
	else
		return true;
}

/***********************************/
//取网页中","前后各5个字节为特征值
/***********************************/
string SearchSys::getEigenStr(string &content)
{
	string eigenStr;
	size_t flagPos = -1;
	int cnt = 0;//限制长度用
	while ((flagPos = content.find("，", flagPos + 1)) != string::npos && cnt < 3)
	{
		eigenStr.append(content, flagPos-5, 5);
		eigenStr.append(content, flagPos+1, 5);
		++cnt;
	}
	return eigenStr;
}

/***********************************/
//判断两个string特征值是否重复（通过最长公共子序列）
/***********************************/
bool SearchSys::isUniqueStr(string &a, string &b)
{
	string max;//最长公共子序列
	string temp;
	int alen = a.size();
	int blen = b.size();
	for(int i = 0; i < alen; ++i){
		for(int j = 0; j < blen; ++j){
			if(a[i] == b[j]){
				int len = (alen-i < blen-j)? alen-i: blen-j;//可能的最大长度
				if(len < (int)max.size())//当可能的最大值也没有当前的大时
					continue;
				temp = a[i];
				for(int t = 1; t < len; ++t){
					if(a[i+t] != b[j+t])
						break;
					temp += a[i+t];
				}
				if(temp.size() > max.size())//实际求出的没有当前的大时
					max = temp;
				temp.clear();
			}
		}
	}

	int v = alen < blen ? alen : blen;//判断重复的参数
	if ( (v = max.size() / v) > 0.6)
		return false;
	else
		return true;
}

/***********************************/
//根据网页相似度,网页分类
/***********************************/
int SearchSys::groupPage(map<string, vector<Page> > &uniPage)
{
	FileOperator fr_page(page_file);
	FILE *fp_page = fr_page.open_read();
	map<string, vector<Page> >::iterator it;
	for (it = uniPage.begin(); it != uniPage.end(); ++it)
	{
		cout << it->first <<  "..." << endl;
		cout << "set tf df weight..." << endl;
		setWeight(fp_page, it->second);
		/*
		cout << "word\t\ttf\tdf\tweight" << endl;
		vector<Page>::iterator itPage;
		for (itPage = it->second.begin(); itPage != it->second.end(); ++itPage)
		{
			vector<Word>::iterator itWord;
			for (itWord = itPage->words.begin(); itWord != itPage->words.end(); ++itWord)
			{
				cout << itWord->str << ":\t\t" << itWord->tf << "\t" << itWord->df << "\t" << itWord->weight << endl;
			}
			cout << endl;
		}
		*/
		//float *matrix = new float[it->second.size() * (it->second.size() - 1) / 2];
		//cout << "get matrix..." << endl;
		//getMatrix(it->second, matrix);
		//cout << "get cluster..." << endl;
		vector< vector<Page> > group;
		getCluster(it->second, group);
		cout << "group OK! have:" << group.size() << " groups" << endl;
		pair<string, vector<vector<Page> > > p(it->first, group);
		cluster.insert(p);
		//delete matrix;
		cout << "handle query:" << it->first << "...OK!" << endl;
	}
	return 0;
}
/***********************************/
//求文档中每个词的权重
/***********************************/
int SearchSys::setWeight(FILE *fp_page, vector<Page> &pages)
{
	map<string, int> dfMap;//map<word, df>
	map<string, int>::iterator itDf;

	//遍历所有网页得到map<word, df>
	vector<string> vecNowWord;//当前网页出现的词的集合
	vector<string>::iterator itNowWord;
	vector<Page>::iterator itPage;
	for (itPage = pages.begin(); itPage != pages.end(); ++itPage)
	{
		string content;
		getStrByFlag(fp_page, "<content>", "</content>", content, itPage->content_addr);//获取内容
		string word;
		string result;
		segStr(content, result);
		stringstream stream(result);
		while (stream >> word)
		{
			if (!excluded.count(word))//检测停用词
			{
				//求tf
				//cout << "tf..." << endl;
				vector<Word>::iterator itWord = itPage->words.begin();
				for (; itWord != itPage->words.end(); ++itWord)
				{
					if(itWord->str == word)
						break;
				}
				if (itWord != itPage->words.end())//word已存在时,tf加1
				{
					++(itWord->tf);//tf+1
				}
				else//word不存在时,加入,tf=1
				{
					//cout << "push_back word: " <<  word << endl;
					Word w(word, 1, 0, 0);
					itPage->words.push_back(w);
				}
				
				//求df
				//cout << "df..." << endl;
				itDf = dfMap.find(word);
				if (itDf == dfMap.end())//该词在dfMap第一次出现
				{
					vecNowWord.push_back(word);
					pair<string, int> p(word, 1);
					dfMap.insert(p);
				}
				else
				{
					for (itNowWord = vecNowWord.begin(); itNowWord != vecNowWord.end(); ++itNowWord)
					{
						if (*itNowWord == word)
							break;
					}
					if (itNowWord == vecNowWord.end())//该词在该网页第一次出现
					{
						++(itDf->second);//df+1
						vecNowWord.push_back(word);
					}
				}
			}
		}
		vecNowWord.clear();
	}
	//把df存储到每个网页的每个单词中,并计算权重weight
	//cout << "weight..." << endl;
	for (itPage = pages.begin(); itPage != pages.end(); ++itPage)
	{
		float weightSum = 0.0;
		vector<Word>::iterator itWords = itPage->words.begin();
		for (;itWords != itPage->words.end(); ++itWords)
		{
			itDf = dfMap.find(itWords->str);
			itWords->df = itDf->second;
			itWords->weight = itWords->tf/(float)itWords->df;
			weightSum += (itWords->weight * itWords->weight);
		}
		weightSum = sqrt(weightSum);
		for (itWords = itPage->words.begin(); itWords != itPage->words.end(); ++itWords)
		{
			itWords->weight /= weightSum;//权重归一化
			pair<string, float> p(itWords->str, itWords->weight);
			itPage->eigenVec.insert(p);//加入网页的特征向量
		}

	}

	return 0;
}

/***********************************/
//求相似度矩阵
/***********************************/
int SearchSys::getMatrix(vector<Page> &pages, float *matrix)
{
	int n = pages.size();
	//float *matrix = new float[n * (n - 1) / 2];
	//用压缩矩阵存储,大小为n(n-1)/2, &a[i][j]==&a[0][0]+n*i-i*(i+1)/2+(j-i+1)
	//	0	1	2	3	4
	//0		*	*	*	*
	//1			*	*	*
	//2				*	*
	//3					*
	//4

	cout << "matrix:" << endl;
	int i, j;
	for (i = 0; i < n - 1; ++i)//一行一行遍历
	{
		for (j = i + 1; j < n; ++j)//一行对应的每一列
		{
			int pos = n*i - i*(i+1)/2 + (j-i-1); 
			matrix[pos] = getEigenVec(pages[i], pages[j]);
			cout << matrix[pos] << "\t";
		}
		cout << endl;
	}
	return 0;
}

/***********************************/
//求两个网页的特征向量的乘积（余弦相似度）
/***********************************/
float SearchSys::getEigenVec(Page &p1, Page &p2)
{
	float result = 0.0;
	/*//双层循环速度太慢
	vector<Word>::iterator it1, it2;
	for (it1 = p1.words.begin(); it1 != p1.words.end(); ++it1)
	{
		for (it2 = p2.words.begin(); it2 != p2.words.end(); ++it2)
		{
			if (it1->str == it2->str)
			{
				result += (it1->weight * it2->weight);
				break;
			}
		}
	}
	*/
	//改为map数据结构后速度明显加快:
	map<string, float>::iterator itEV1;
	map<string, float>::iterator itEV2;
	for (itEV1 = p1.eigenVec.begin(); itEV1 != p1.eigenVec.end(); ++itEV1)
	{
		itEV2 = p2.eigenVec.find(itEV1->first);
		if (itEV2 != p2.eigenVec.end())
			result += (itEV1->second * itEV2->second);
	}
	return result;
}

/***********************************/
//得到网页分类簇
/***********************************/
int SearchSys::getCluster(vector<Page> &pages, vector< vector<Page> > &group)
{
	vector<Page> vecGroup;

	//将第一的网页放进第一个分类
	if (pages.size() != 0)
	{
		vecGroup.push_back(*pages.begin());
		group.push_back(vecGroup);
		vecGroup.clear();
		pages.begin()->groupNum = 0;
	}

	vector<Page>::iterator itPage;
	for (itPage = pages.begin() + 1; itPage != pages.end(); ++itPage)
	{
		float maxVal = 0.0;//当前网页与之前网页最大的相似度
		float nowVal;
		vector<Page>::iterator itMax;
		vector<Page>::iterator itPre;
		for (itPre = pages.begin(); itPre != itPage; ++itPre)
		{
			nowVal = getEigenVec(*itPage, *itPre);
			//cout << nowVal << endl;
			if (nowVal > maxVal)
			{
				maxVal = nowVal;
				itMax = itPre;
			}
		}
		if (maxVal >= groupLimen)//当大于阈值时加入该类
		{
			group[itMax->groupNum].push_back(*itPage);
			itPage->groupNum = itMax->groupNum;
			/*
			vector<vector<Page> >::iterator itGp;
			for (itGp = group.begin(); itGp != group.end(); ++itGp)//遍历每个分类
			{
				vector<Page>::iterator it;
				for (it = itGp->begin(); it != itGp->end(); ++it)
				{
					if (it->id == itMax->id)
						break;
				}
				if (it != itGp->end())
					break;
			}
			if (itGp != group.end())//找到分类时
			{
				itGp->push_back(*itPage);
			}
			*/
		}
		else//否则创建新类
		{
			vecGroup.push_back(*itPage);
			group.push_back(vecGroup);
			vecGroup.clear();
			itPage->groupNum = group.size() - 1;
		}
		//cout << itPage->id << " group OK!" << endl;
	}
	return 0;
}

/***********************************/
//将查询到的类个数返回客户端
/***********************************/
int SearchSys::getClassCnt(string query)
{
	return cluster.find(query)->second.size();
}

/***********************************/
//将查询结果返回客户端
/***********************************/
string SearchSys::search(string query, int tag, int classNum)
{
	string result;
	if (tag == 0)//通用搜索
	{
		map<string, vector<Page> >::const_iterator itMap;
		itMap = uniPage.find(query);
		if (itMap == uniPage.end())
			result =  "该关键字无结果!";
		else
		{
			int cnt = 0;
			vector<Page>::const_iterator it;
			for (it = itMap->second.begin(); it != itMap->second.end() && cnt < 10; ++it, ++cnt)
			{
				printPage(*it, result);
			}
		}
	}
	else//聚类搜索
	{
		map<string, vector<vector<Page> > >::const_iterator itMap;
		itMap = cluster.find(query);
		if (itMap == cluster.end())
			result = "该关键字无结果!";
		else
		{
			if (classNum > (int)itMap->second.size()-1 || classNum < 0)
				result = "没有该类!";
			else
			{
				int cnt = 0;
				vector<Page>::const_iterator it;
				for (it = itMap->second[classNum].begin(); it != itMap->second[classNum].end() && cnt < 10; ++it, ++cnt)
				{
					printPage(*it, result);
				}
			}
		}
	}
	return result;
}


/***********************************/
//显示页面信息
/***********************************/
void SearchSys::printPage(Page p, string &result)
{
	FileOperator fr(page_file);
	FILE *fp = fr.open_read();
	//string content;
	//getStrByFlag(fp, "<content>", "</content>", content, p.content_addr);
	fseek(fp, p.content_addr + 9, SEEK_SET);	
	char content[256] = {'\0'};
	fread(content, 1, sizeof(content), fp);

	char docid[6];
	sprintf(docid, "%d", p.id);

	result += docid;
	result += "--";
	result += ("<a href=\"" + p.url + "\" target=\"_blank\" style=\"font-weight:bold; color:#036; text-decoration: none;\">" + p.title + "</a>" + "</br>");
	//result += "</br>";
	result += ("<dir style=\"padding: 0px; margin: 0px; color: #999; font-style: italic;\">" + p.url + "</dir>");
	result += content;
	result += "...</br></br>";

	fclose(fp);
}









