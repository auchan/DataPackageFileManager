#ifndef _SCOPE_FILE_H_
#define _SCOPE_FILE_H_

#include <cstdio>
#include <string>

class ScopeFile
{

public:

	ScopeFile(const std::string& fileName, const std::string& mode = "rb")
	{
		_fp = fopen(fileName.c_str(), mode.c_str());
	}

	~ScopeFile()
	{
		if (NULL != _fp)
		{
			fclose(_fp);
			_fp = NULL;
		}
	}

	FILE* _fp;
};

#endif