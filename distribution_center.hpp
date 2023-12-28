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
    void set_all_clients_finished_downloading();
    bool get_all_clients_finished_downloading();

private:
    int nr_of_clients;
    map<int, int> number_of_requests_per_client;
    bool all_clients_finished_downloading;
};