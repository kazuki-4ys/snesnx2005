#include "filemanager.h"

string getUpperText(string _src){//大文字にする
    const char *src = _src.c_str();
    int srcLen = strlen(src);
    char *dest = (char*)calloc(sizeof(char), srcLen + 1);
    for(int i = 0;i < srcLen;i++){
        if(src[i] >= 'a' && src[i] <= 'z'){
            dest[i] = src[i] - ('a' - 'A');
        }else{
            dest[i] = src[i];
        }
    }
    string destString = dest;
    free(dest);
    return destString;
}

bool FileManager::isASaki(entry *a, entry *b){//aを先にする場合はtrue
    if(a->isDir != b->isDir){
        if(a->isDir)return false;
        return true;
    }
    if(strcmp(getUpperText(a->name).c_str(), getUpperText(b->name).c_str()) < 0){
        return true;
    }
    return false;
}

void FileManager::bubbleSortEntries(void){
    if(entries.size() < 2)return;
    for(int i = 0;i < entries.size();i++){
        for(int j = i + 1; j < entries.size(); j++){
            entry tmpEntry;
            if(!isASaki(&(entries[i]), &(entries[j]))){
                tmpEntry = entries[i];
                entries[i] = entries[j];
                entries[j] = tmpEntry;
            }
        }
    }
}

bool FileManager::OpenPath(string s){
    struct dirent *d;
    DIR *pdir = opendir(s.c_str());
    char *tmpName;
    entry tmpEntry;
    if(!pdir)return false;
    curPath = s;
    entries.clear();
    entries.shrink_to_fit();
    while(1){
        d = readdir(pdir);
        if(!d)break;
        tmpName = d->d_name;
        if(!(strcmp(tmpName,".") && strcmp(tmpName,"..")))continue;
        tmpEntry.name = string(tmpName);
        if(d->d_type == DT_DIR){
            tmpEntry.isDir = true;
        }else{
            tmpEntry.isDir = false;
        }
        entries.push_back(tmpEntry);
    }
    closedir(pdir);
    bubbleSortEntries();
    return true;
}

FileManager::FileManager(string s){
    if(!OpenPath(s)){
        valid = true;
    }else{
        valid = false;
    }
}

bool FileManager::Reload(){
    if(!OpenPath(curPath))return false;
    return true;
}

bool FileManager::Back(){
    int rootIndex = 0;
    int index = curPath.size() - 1;
    if(curPath[index] == '/')index--;
    while(curPath[index] != '/'){
        if(index < 1)return false;
        index--;
    }
    while(curPath[rootIndex] != '/'){
        rootIndex++;
    }
    if(rootIndex == index){
        return OpenPath(curPath.erase(index + 1));
    }
    return OpenPath(curPath.erase(index));
}

string FileManager::getFullPath(int i){
    if(curPath[curPath.size() - 1] == '/'){
        return curPath + (entries[i]).name;
    }
    return curPath + "/" + (entries[i]).name;
}