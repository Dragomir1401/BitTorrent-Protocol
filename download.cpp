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

void receive_file_structure(vector<string> &segments_contained)
{
    // Receive the number of files which consist the target file
    int num_segments;
    MPI_Recv(
        &num_segments,
        1,
        MPI_INT,
        TRACKER_RANK,
        0,
        MPI_COMM_WORLD,
        MPI_STATUS_IGNORE);

    // Receive each segment
    for (int i = 0; i < num_segments; i++)
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
        segments_contained.push_back(string(segment));
    }
}
map<int, vector<string>> handle_response_to_request(vector<string> &segments_contained)
{
    // Receive the file structure
    receive_file_structure(segments_contained);

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

    return client_list_and_segments_owned;
}

int find_best_client(
    vector<string> segments_contained,
    map<int, vector<string>> client_list_and_segments_owned,
    string file,
    peer_info *peer_info_local,
    distribution_center *dc)
{
    vector<string> segmentsDownloaded = peer_info_local->get_segments_downloaded(file);
    int min_workload = INT_MAX;
    int best_client_id = -1;

    // For each segment that the client wants and does not own
    // Find the client with the least workload to download from
    // Send a request to that client
    for (auto &segment : segments_contained)
    {
        // If the segment is not already downloaded
        if (find(segmentsDownloaded.begin(), segmentsDownloaded.end(), segment) == segmentsDownloaded.end())
        {
            // For each client that has the segment
            for (auto &client : client_list_and_segments_owned)
            {
                int client_id = client.first;
                vector<string> segments_owned = client.second;

                // If the client has the segment
                if (find(segments_owned.begin(), segments_owned.end(), segment) != segments_owned.end())
                {
                    // Get nr of requests of client from the distribution center
                    int nr_requests = dc->get_number_of_requests(client_id);

                    // If the client has a lower workload
                    if (nr_requests < min_workload)
                    {
                        min_workload = nr_requests;
                        best_client_id = client_id;
                    }
                }
            }
        }
    }

    return best_client_id;
}

void download_thread_func(
    int rank,
    peer_info *input,
    distribution_center *dc)
{
    // For each wanted file
    for (auto &file : input->get_files_wanted())
    {
        // Send a request to the tracker
        request(file);

        // Handle response from tracker
        vector<string> segments_contained;
        map<int, vector<string>> client_list_and_segments_owned =
            handle_response_to_request(segments_contained);

        // Find the best client to download from
        int best_client_id = find_best_client(
            segments_contained,
            client_list_and_segments_owned,
            file,
            input,
            dc);
    }
}
