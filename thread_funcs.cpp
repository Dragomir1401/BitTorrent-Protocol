#include "header.hpp"

void request(
    string file)
{
    // Send an MPI message with a GET action
    int action = action::REQUEST;
    MPI_Send(
        &action,
        1,
        MPI_INT,
        TRACKER_RANK,
        0,
        MPI_COMM_WORLD);

    // Then send the filename for the requested file
    MPI_Send(
        file.c_str(),
        file.size() + 1,
        MPI_CHAR,
        TRACKER_RANK,
        0,
        MPI_COMM_WORLD);
}

void receive_segments_owned(
    map<int, vector<string>> &client_list_and_segments_owned,
    int client)
{
    // Receive the number of segments owned
    int num_segments_owned;
    MPI_Recv(
        &num_segments_owned,
        1,
        MPI_INT,
        TRACKER_RANK,
        0,
        MPI_COMM_WORLD,
        MPI_STATUS_IGNORE);

    // Receive each segment
    for (int i = 0; i < num_segments_owned; i++)
    {
        MPI_Status status;
        MPI_Probe(TRACKER_RANK, 0, MPI_COMM_WORLD, &status);

        int segment_size;
        MPI_Get_count(&status, MPI_CHAR, &segment_size);

        char *segment = new char[segment_size];

        MPI_Recv(
            segment,
            segment_size,
            MPI_CHAR,
            TRACKER_RANK,
            0,
            MPI_COMM_WORLD,
            MPI_STATUS_IGNORE);

        // Add the segment to the vector
        client_list_and_segments_owned[client].push_back(string(segment));
    }
}

void handle_response_to_request()
{
    // Receive the number of peers owning the file from the tracker
    int num_peers_owning_file;
    MPI_Recv(
        &num_peers_owning_file,
        1,
        MPI_INT,
        TRACKER_RANK,
        0,
        MPI_COMM_WORLD,
        MPI_STATUS_IGNORE);

    // Receive the client list and segments owned
    map<int, vector<string>> client_list_and_segments_owned;
    for (int i = 0; i < num_peers_owning_file; i++)
    {
        // Receive the client id
        int client_id;
        MPI_Recv(
            &client_id,
            1,
            MPI_INT,
            TRACKER_RANK,
            0,
            MPI_COMM_WORLD,
            MPI_STATUS_IGNORE);

        // Receive the segments owned
        receive_segments_owned(client_list_and_segments_owned, client_id);
    }
}

void download_thread_func(
    int rank,
    peer_info *input)
{
    // For each wanted file
    for (auto &file : input->get_files_wanted())
    {
        // Send a request to the tracker
        request(file);

        // Handle response from tracker
        handle_response_to_request();
    }
}

void upload_thread_func(int rank)
{
    // Thread function logic here
}