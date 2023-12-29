#include "swarm_info.hpp"

swarm_info::swarm_info()
{
}

swarm_info::swarm_info(
    swarm_info *swarm)
{
    // Deep copy of the swarm
    for (auto it = swarm->client_list_and_segments_owned.begin(); it != swarm->client_list_and_segments_owned.end(); it++)
    {
        vector<string> segments_owned;
        for (int i = 0; i < (int)(it->second.size()); i++)
        {
            segments_owned.push_back(it->second[i]);
        }
        this->client_list_and_segments_owned[it->first] = segments_owned;
    }
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

void swarm_info::add_new_downloaded_segments(
    int client_id,
    vector<string> segments)
{
    // If the client does not have any segments of the file, add it to the swarm
    if (this->client_list_and_segments_owned.find(client_id) == this->client_list_and_segments_owned.end())
    {
        vector<string> new_segments;
        for (int i = 0; i < (int)(segments.size()); i++)
        {
            new_segments.push_back(segments[i]);
        }
        this->client_list_and_segments_owned[client_id] = new_segments;
    }
    // If the client has some segments of the file, add the new ones
    else
    {
        for (int i = 0; i < (int)(segments.size()); i++)
        {
            // If the client does not have the segment, add it
            if (find(this->client_list_and_segments_owned[client_id].begin(),
                     this->client_list_and_segments_owned[client_id].end(), segments[i]) ==
                this->client_list_and_segments_owned[client_id].end())
            {
                this->client_list_and_segments_owned[client_id].push_back(segments[i]);
            }
        }
    }
}