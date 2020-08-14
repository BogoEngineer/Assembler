#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <iostream>
#include <string.h>
#include <fstream>
#include <vector>

using namespace std;

class FileManager{
    private:
        fstream file;
        vector<string> content;
    public:
        FileManager();
        vector<string> getContent(string fname);
        void setContent(string output, string fname);
};

#endif