#include "stdlib.h" 
#include "direct.h" 
#include "string.h" 
#include "io.h" 

#include "browsedir.h" 

CBrowseDir::CBrowseDir() 
{ 

} 

CBrowseDir::~CBrowseDir() 
{ 
	
}

bool CBrowseDir::setBrowseDir(const std::string& dir) 
{ 
#ifdef WIN32
	//�ж�Ŀ¼�Ƿ���� K

	_initDir = formatDir(dir);


	return true;
#endif
} 

bool CBrowseDir::beginBrowse(const std::string& filespec) 
{ 
	processDir(_initDir, ""); 
	return browseDir(_initDir, filespec); 
} 

bool CBrowseDir::browseDir(const std::string& dir, const std::string& filespec) 
{ 
#ifdef WIN32
	//_chdir(dir.c_str()); 

	//���Ȳ���dir�з���Ҫ����ļ� 
	long hFile; 
	_finddata_t fileinfo; 
	std::string filePath = dir + filespec;
	if ((hFile=_findfirst(filePath.c_str(), &fileinfo)) != -1) 
	{ 
		do 
		{ 
			//����ǲ���Ŀ¼ 
			//�������,����д��� 
			if (!(fileinfo.attrib & _A_SUBDIR)) 
			{ 
				std::string filename = dir +  fileinfo.name;
				if (!processFile(filename)) 
					return false; 
			} 

		} while (_findnext(hFile,&fileinfo) == 0); 
		_findclose(hFile); 
	} 

	//����Ŀ¼
	filePath = dir + "*.*";
	if ((hFile=_findfirst(filePath.c_str(), &fileinfo)) != -1) 
	{
		do 
		{ 
			//����ǲ���Ŀ¼ 
			if (fileinfo.attrib & _A_SUBDIR) 
			{ 
				 if (strcmp(fileinfo.name,".") != 0 && strcmp 
						(fileinfo.name,"..") != 0) 
				{ 
					std::string subdir = dir + fileinfo.name + "/";
					processDir(subdir, dir); 
					if (!browseDir(subdir,filespec)) 
						return false; 
				} 
			}

		} while (_findnext(hFile,&fileinfo) == 0); 
		_findclose(hFile); 
	}

	return true; 
#endif
} 

bool CBrowseDir::processFile(const std::string& filename) 
{ 
	printf("%s\n", filename.c_str());
	return true; 
} 

void CBrowseDir::processDir(const std::string& currentdir, const std::string& parentdir) 
{ 
} 

std::string CBrowseDir::formatDir(const std::string &dirName)
{
	if (dirName.length() == 0)
	{
		return dirName;
	}

	std::string newdirName = dirName;
	std::string::iterator cur, end = newdirName.end();
	for (cur = newdirName.begin(); cur != end; ++cur)
	{
		if (*cur == '\\')
		{
			*cur = '/';
		}
	}

	if (newdirName.at(newdirName.length() - 1) != '/')
	{
		newdirName += '/';
	}

	return newdirName;
}