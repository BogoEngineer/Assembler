#include "FileManager.h"
#include "INCLUDES.h"

FileManager::FileManager(){}

vector<string> FileManager::getContent(string fname){
    content = {};
    string line;
    file.open(fname, ios::in | ios::out);
    if(file.is_open()==false){
        cout<<"File "<<fname<< " cannot be oppened"<<endl;
        return content;
    }
    while(getline(file, line)){
        content.push_back(line);
    }
    file.close();
    return content;
}

void FileManager::setContent(vector<string> content, string fname){
    this->content = content;
    file.open(fname, ios::in | ios::out);
    if(file.is_open()==false){
        cout<<"File "<<fname<< " cannot be oppened"<<endl;
        return;
    }
    for(string n: content){
        file<<n + "\n";
    }
    file.close();
}
