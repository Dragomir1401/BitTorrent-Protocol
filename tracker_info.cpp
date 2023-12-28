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
