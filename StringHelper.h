//
// Created by 袁健 on 2017/12/14.
//

#ifndef LITEHTTPSERVER_STRINGHELPER_H
#define LITEHTTPSERVER_STRINGHELPER_H

#include <string>
#include <vector>

using namespace std;


inline vector<string> split_str(string str, char delimiter){
    vector<string> r;
    string tmpstr;
    while (!str.empty()){
        int ind = str.find_first_of(delimiter);
        if (ind == -1){
            r.push_back(str);
            str.clear();
        }
        else{
            r.push_back(str.substr(0, ind));
            str = str.substr(ind + 1, str.size() - ind - 1);
        }
    }
    return r;
}

inline std::string& trim_str(std::string &s)
{
    if (s.empty())
    {
        return s;
    }

    s.erase(0,s.find_first_not_of(" "));
    s.erase(s.find_last_not_of(" ") + 1);
    return s;
}

#endif //LITEHTTPSERVER_STRINGHELPER_H
