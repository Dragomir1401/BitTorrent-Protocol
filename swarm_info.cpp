#include "swarm_info.hpp"

swarm_info::swarm_info()
{
}

swarm_info::swarm_info(
    map<int, vector<string>> client_list_and_segments_owned)
{
    this->client_list_and_segments_owned = client_list_and_segments_owned;
}

swarm_info::~swarm_info()
{
}

map<int, vector<string>> swarm_info::get_client_list_and_segments_owned()
{
    return this->client_list_and_segments_owned;
}

void swarm_info::add_client(
    int client_id,
    vector<string> segments_owned)
{
    this->client_list_and_segments_owned[client_id] = segments_owned;
}

void swarm_info::remove_client(
    int client_id)
{
    this->client_list_and_segments_owned.erase(client_id);
}