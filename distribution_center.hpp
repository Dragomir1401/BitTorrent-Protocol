#pragma once

#include <map>
using namespace std;

class distribution_center
{
public:
    distribution_center(int nr_of_clients);
    ~distribution_center();
    void add_request(
        int client_id);
    void remove_request(
        int client_id);
    int get_number_of_requests(
        int client_id);

private:
    int nr_of_clients;
    map<int, int> number_of_requests_per_client;
};