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
#include "peer_info.hpp"
#include "tracker_info.hpp"
#include "swarm_info.hpp"

using namespace std;

#define TRACKER_RANK 0
#define MAX_FILES 10
#define MAX_FILENAME 15
#define HASH_SIZE 32
#define MAX_CHUNKS 100

void tracker(
    int numtasks,
    int rank);

void peer(
    int numtasks,
    int rank);

void download_thread_func(
    int rank);

void upload_thread_func(
    int rank);

peer_info *read_peer_input(
    int rank);