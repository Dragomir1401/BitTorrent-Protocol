#include "distribution_center.hpp"

using namespace std;

distribution_center::distribution_center(int nr_of_clients)
{
    this->nr_of_clients = nr_of_clients;
    for (int i = 0; i < nr_of_clients; i++)
    {
        this->number_of_requests_per_client[i] = 0;
    }
}

distribution_center::~distribution_center()
{
}

void distribution_center::add_request(
    int client_id)
{
    this->number_of_requests_per_client[client_id]++;
}

void distribution_center::remove_request(
    int client_id)
{
    this->number_of_requests_per_client[client_id]--;
}

int distribution_center::get_number_of_requests(
    int client_id)
{
    return this->number_of_requests_per_client[client_id];
}
