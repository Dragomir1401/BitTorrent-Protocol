#pragma once

#include <mpi.h>
#include <iostream>
#include <thread>
#include <vector>
#include <map>
#include <set>
#include <fstream>
#include <string>
#include <sstream>
#include <thread>
#include <algorithm>
#include <climits>
#include <queue>
#include "peer_info.hpp"
#include "tracker_info.hpp"
#include "swarm_info.hpp"
#include "logger.hpp"

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
    FINALIZE,
    KILL_UPLOAD_THREAD,
    GET_WORKLOAD
};

// Enum for download and upload tags
enum tag
{
    INIT,
    COMMANDS,
    UPDATE_COMMAND,
    DOWNLOAD_REQUEST,
    KILL,
    WORKLOAD,
    ACKNOWLEDGEMENT
};

/// @brief  Function that encapsulates the tracker's functionality
/// @param numtasks - Number of MPI tasks
/// @param rank - Rank of the current task
/// @param log - Pointer to the logger instance
void tracker(
    int numtasks,
    int rank,
    logger *log);

/// @brief  Function that encapsulates the peer's functionality
/// @param numtasks - Number of MPI tasks
/// @param rank - Rank of the current task
/// @param log - Pointer to the logger instance
void peer(
    int numtasks,
    int rank,
    logger *log);

/// @brief  Function that encapsulates the download thread's functionality
/// @param rank - Rank of the current task
/// @param input - Pointer to the peer_info instance
/// @param numtasks - Number of MPI tasks
/// @param log - Pointer to the logger instance
void download_thread_func(
    int rank,
    peer_info *input,
    int numtasks,
    logger *log);

/// @brief  Function that encapsulates the upload thread's functionality
/// @param rank - Rank of the current task
/// @param input - Pointer to the peer_info instance
/// @param log - Pointer to the logger instance
void upload_thread_func(
    int rank,
    peer_info *input,
    logger *log);

peer_info *read_peer_input(
    int rank);