#ifndef _FILEMANAGER_H_
#define _FILEMANAGER_H_

#include <dirent.h>
#include <string>
#include <cstring>
#include <vector>

using namespace std;

string getUpperText(string _src);

typedef struct {
    string name;
    bool isDir;
} entry;

class FileManager{
        bool isASaki(entry *a, entry *b);
        void bubbleSortEntries(void);
    public:
        bool valid;
        string curPath;
        vector<entry> entries;
        FileManager(string);
        bool OpenPath(string);
        bool Reload(void);
        bool Back(void);
        string getFullPath(int);
};

#endif//_FILEMANAGER_H_