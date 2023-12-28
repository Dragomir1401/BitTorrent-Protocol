#pragma once

#include <map>
#include <string>
#include <vector>
#include "swarm_info.hpp"
using namespace std;

class tracker_info
{
public:
    tracker_info();
    tracker_info(
        map<string, swarm_info> file_to_peers_owning_it);
    ~tracker_info();
    map<string, swarm_info> get_file_to_peers_owning_it();
    void add_file(
        string filename,
        swarm_info swarm);
    void remove_file(
        string filename);
    void add_segment(
        string filename,
        string segment);
    void add_segments(
        string filename,
        vector<string> segments);
    void remove_segment(
        string filename,
        string segment);
    vector<string> get_segments(
        string filename);

private:
    map<string, swarm_info> file_to_peers_owning_it;
    map<string, vector<string>> segments_contained_in_file;
};
