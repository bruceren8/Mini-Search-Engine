#include "searchSys.h"

int getline(FILE* fp, string &str)
{
	char *line = NULL;
	size_t len;
	int nget;
	if ((nget = getline(&line, &len, fp)) == -1)//读取失败或读到文件尾
		return -1;

	str = line;
	if (line)//释放空间
		free(line);

	//去除换行符：
	size_t pos = 0;
	for (pos = str.find('\n', pos); pos != string::npos; pos = str.find('\n', pos))
	{
		str.erase(pos, 1);
	}
	//str.erase(str.size()-1, 1);
	return nget;
}

FILE *FileOperator::open_read()
{
	FILE *fp = fopen(filePath.c_str(), "r");
	if (fp == NULL)
	{
		cerr << "err: open " << filePath << endl;
		exit(1);
	}

	struct stat statbuf;
	memset(&statbuf, 0, sizeof(struct stat));
	if (lstat(filePath.c_str(), &statbuf) == 1)
	{
		cerr << "err: stat!" << endl;
	}
	fileSize = (int)statbuf.st_size;
	return fp;
}

FILE *FileOperator::open_write()
{
	FILE *fp = fopen(filePath.c_str(), "w");
	if (fp == NULL)
	{
		cerr << "err: open " << filePath << endl;
		exit(1);
	}
	return fp;
}

int FileOperator::getSize()
{
	return fileSize;
}

