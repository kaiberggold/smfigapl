#ifndef SMTESTMISC
#define SMTESTMISC

#include <vector>
#include <filesystem>
#ifdef SM_CLANG
#include <dirent.h>
#endif

static std::vector<std::string> folderList(std::string path)
{
	std::vector<std::string> temp;
	temp.resize(0);

//workaround for not compiling with clang 10.0.1
#ifdef SM_CLANG
if (auto folder = opendir(path.c_str())) {
    while (auto f = readdir(folder))
    {
        if (f->d_name[0] == '.' || f->d_name[0]=='s')
            continue;
        temp.push_back(path+"/"+std::string(f->d_name));
    }
    closedir(folder);
}
#else
	for(auto& p: std::filesystem::directory_iterator(path))
	{
		if (p.path().extension().string()==".toml" && p.path().filename().string().substr(0, 1)=="0")
			temp.push_back(p.path().string());

	}
	sort(temp.begin(), temp.end()); // iterator is not ordered
#endif
	return temp;
}




#endif /* SMTESTMISC */
