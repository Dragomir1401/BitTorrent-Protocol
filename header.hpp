#pragma once

#include <mpi.h>
#include <iostream>
#include <thread>
#include <vector>
#include <map>
#include <fstream>
#include <string>
#include <sstream>
#include <thread>
#include <algorithm>
#include <climits>
#include "peer_info.hpp"
#include "tracker_info.hpp"
#include "swarm_info.hpp"
#include "distribution_center.hpp"

using namespace std;

#define TRACKER_RANK 0
#define MAX_FILES 10
#define MAX_FILENAME 15
#define HASH_SIZE 32
#define MAX_CHUNKS 100

// Define enum for actions get, update, finalize
enum action
{
    REQUEST,
    UPDATE,
    FINALIZE
};

void tracker(
    int numtasks,
    int rank);

void peer(
    int numtasks,
    int rank,
    distribution_center *dc);

void download_thread_func(
    int rank,
    peer_info *input,
    distribution_center *dc);

void upload_thread_func(
    int rank,
    peer_info *input);

peer_info *read_peer_input(
    int rank);