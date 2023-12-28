#pragma once

#include <map>
#include <string>
#include <vector>

using namespace std;

class swarm_info
{
public:
    swarm_info();
    swarm_info(
        map<int, vector<string>> client_list_and_segments_owned);
    ~swarm_info();
    map<int, vector<string>> get_client_list_and_segments_owned();
    void add_client(
        int client_id,
        vector<string> segments_owned);
    void remove_client(
        int client_id);

private:
    map<int, vector<string>> client_list_and_segments_owned;
};