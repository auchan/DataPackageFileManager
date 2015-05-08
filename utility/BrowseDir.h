/************************************************** 
　　 这是CBrowseDir的类定义文件 BrowseDir.h 

**************************************************/ 
#include <cstdlib> 
#include <string>

class CBrowseDir 
{ 
public: 

	CBrowseDir(); 
	~CBrowseDir();

	bool setBrowseDir(const std::string& dir); 

	bool beginBrowse(const std::string& filespec); 

protected: 

	bool browseDir(const std::string& dir, const std::string& filespec); 

	virtual bool processFile(const std::string& filename); 

	virtual void processDir(const std::string& currentdir, const std::string& parentdir); 

private: 
	std::string _initDir; 

private:
	std::string formatDir(const std::string &dirName);
}; 