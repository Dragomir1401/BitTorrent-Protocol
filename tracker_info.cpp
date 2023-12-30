#include "tracker_info.hpp"

tracker_info::tracker_info()
{
}

tracker_info::tracker_info(
    map<string, swarm_info> file_to_peers_owning_it)
{
    this->file_to_peers_owning_it = file_to_peers_owning_it;
}

tracker_info::~tracker_info()
{
}

map<string, swarm_info> tracker_info::get_file_to_peers_owning_it()
{
    return this->file_to_peers_owning_it;
}

void tracker_info::add_file(
    string filename,
    swarm_info swarm)
{
    this->file_to_peers_owning_it[filename] = swarm;
}

void tracker_info::remove_file(
    string filename)
{
    this->file_to_peers_owning_it.erase(filename);
}

void tracker_info::add_segment(
    string filename,
    string segment)
{
    this->segments_contained_in_file[filename].push_back(segment);
}

void tracker_info::remove_segment(
    string filename,
    string segment)
{
    for (int i = 0; i < (int)(this->segments_contained_in_file[filename].size()); i++)
    {
        if (this->segments_contained_in_file[filename][i] == segment)
        {
            this->segments_contained_in_file[filename].erase(this->segments_contained_in_file[filename].begin() + i);
            break;
        }
    }
}

vector<string> tracker_info::get_segments(
    string filename)
{
    return this->segments_contained_in_file[filename];
}

void tracker_info::add_segments(
    string filename,
    vector<string> segments)
{
    for (int i = 0; i < (int)(segments.size()); i++)
    {
        // If the segment is not in the map, add it
        if (find(this->segments_contained_in_file[filename].begin(),
                 this->segments_contained_in_file[filename].end(), segments[i]) ==
            this->segments_contained_in_file[filename].end())
        {
            this->segments_contained_in_file[filename].push_back(segments[i]);
        }
    }
}

void tracker_info::to_file()
{
    ofstream fout;
    // Open with append mode
    fout.open("tracker_info.txt", ios::app);

    fout << "Tracker info:" << endl;
    for (auto &file : this->file_to_peers_owning_it)
    {
        fout << "File: " << file.first << ">>>>";

        swarm_info swarm = file.second;
        map<int, vector<string>> client_list_and_segments_owned = swarm.get_client_list_and_segments_owned();

        fout << "Peers owning it: ";
        for (auto it = client_list_and_segments_owned.begin(); it != client_list_and_segments_owned.end(); it++)
        {
            fout << " Client " << it->first << " with " << it->second.size() << " segments; ";
        }
        fout << endl;
    }

    fout << "END OF TRACKER INFO" << endl
         << endl;

    fout.close();
}