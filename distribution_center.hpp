#pragma once

#include <map>
#include <atomic>
#include "../../../../../../../../usr/include/c++/9/bits/atomic_base.h"
#include <mutex>
#include <condition_variable>
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