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
        tag::COMMANDS,
        MPI_COMM_WORLD);

    // Then send the filename for the requested file
    MPI_Send(
        file.c_str(),
        file.size() + 1,
        MPI_CHAR,
        TRACKER_RANK,
        tag::COMMANDS,
        MPI_COMM_WORLD);
}

void receive_segments_owned(
    map<int, vector<string>> &client_list_and_segments_owned,
    int client,
    tag tag)
{
    // Receive the number of segments owned
    int num_segments_owned;
    MPI_Recv(
        &num_segments_owned,
        1,
        MPI_INT,
        TRACKER_RANK,
        tag,
        MPI_COMM_WORLD,
        MPI_STATUS_IGNORE);

    // Receive each segment
    for (int i = 0; i < num_segments_owned; i++)
    {
        MPI_Status status;
        MPI_Probe(TRACKER_RANK, tag, MPI_COMM_WORLD, &status);

        int segment_size;
        MPI_Get_count(&status, MPI_CHAR, &segment_size);

        char *segment = new char[segment_size];

        MPI_Recv(
            segment,
            segment_size,
            MPI_CHAR,
            TRACKER_RANK,
            tag,
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
        tag::COMMANDS,
        MPI_COMM_WORLD,
        MPI_STATUS_IGNORE);

    // Receive each segment
    for (int i = 0; i < num_segments; i++)
    {
        MPI_Status status;
        MPI_Probe(TRACKER_RANK, tag::COMMANDS, MPI_COMM_WORLD, &status);

        int segment_size;
        MPI_Get_count(&status, MPI_CHAR, &segment_size);

        char *segment = new char[segment_size];

        MPI_Recv(
            segment,
            segment_size,
            MPI_CHAR,
            TRACKER_RANK,
            tag::COMMANDS,
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
        tag::COMMANDS,
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
            tag::COMMANDS,
            MPI_COMM_WORLD,
            MPI_STATUS_IGNORE);

        // Receive the segments owned
        receive_segments_owned(client_list_and_segments_owned, client_id, tag::COMMANDS);
    }

    return client_list_and_segments_owned;
}

void request_segment_from_best_client(
    int best_client_id,
    string segment,
    set<string> requested_segments)
{

    if (requested_segments.find(segment) == requested_segments.end())
    {
        requested_segments.insert(segment);

        // Request the segment
        // Send an MPI message with a GET action
        int action = action::REQUEST;
        MPI_Send(
            &action,
            1,
            MPI_INT,
            best_client_id,
            tag::DOWNLOAD_REQUEST,
            MPI_COMM_WORLD);
    }
}

bool receive_requested_segment(int best_client_id)
{
    MPI_Request recvReq;
    char *ack = (char *)malloc(4 * sizeof(char));

    MPI_Irecv(
        ack,
        4,
        MPI_CHAR,
        best_client_id,
        tag::ACKNOWLEDGEMENT,
        MPI_COMM_WORLD,
        &recvReq);

    // Wait for the ACK to be received or test periodically
    int flag = 0;
    while (!flag)
    {
        MPI_Test(&recvReq, &flag, MPI_STATUS_IGNORE);
    }

    if (strcmp(ack, "ACK") == 0)
    {
        free(ack);
        return true;
    }

    free(ack);
    return false;
}

void send_update_to_tracker(
    vector<string> segments_downloaded,
    string filename)
{
    // Send an update command to tracker
    int action = action::UPDATE;
    MPI_Send(
        &action,
        1,
        MPI_INT,
        TRACKER_RANK,
        tag::COMMANDS,
        MPI_COMM_WORLD);

    // Send the filename
    MPI_Send(
        filename.c_str(),
        filename.size() + 1,
        MPI_CHAR,
        TRACKER_RANK,
        tag::UPDATE_COMMAND,
        MPI_COMM_WORLD);

    // Send number of segments downloaded from the filename
    int num_segments_downloaded = segments_downloaded.size();
    MPI_Send(
        &num_segments_downloaded,
        1,
        MPI_INT,
        TRACKER_RANK,
        tag::UPDATE_COMMAND,
        MPI_COMM_WORLD);

    // Send each segment downloaded to tracker
    for (auto &segment : segments_downloaded)
    {
        MPI_Send(
            segment.c_str(),
            segment.size() + 1,
            MPI_CHAR,
            TRACKER_RANK,
            tag::UPDATE_COMMAND,
            MPI_COMM_WORLD);
    }
}

map<int, vector<string>> handle_update_response()
{
    // Handle response from tracker to the update command
    // Receive number of peers owning the file
    int num_peers_owning_file;
    MPI_Recv(
        &num_peers_owning_file,
        1,
        MPI_INT,
        TRACKER_RANK,
        tag::UPDATE_COMMAND,
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
            tag::UPDATE_COMMAND,
            MPI_COMM_WORLD,
            MPI_STATUS_IGNORE);

        // Receive the segments owned
        receive_segments_owned(client_list_and_segments_owned, client_id, tag::UPDATE_COMMAND);
    }

    return client_list_and_segments_owned;
}

void update_client_list_and_segments_owned(
    map<int, vector<string>> client_list_and_segments_owned,
    map<int, vector<string>> &client_list_and_segments_owned_update)
{
    // Update the client list and segments owned with entries from the update that are not already in the client list and segments owned
    for (auto &client : client_list_and_segments_owned)
    {
        int client_id = client.first;
        vector<string> segments_owned = client.second;

        // If the client is not already in the client list and segments owned
        if (client_list_and_segments_owned_update.find(client_id) == client_list_and_segments_owned_update.end())
        {
            client_list_and_segments_owned_update[client_id] = segments_owned;
        }
        // If the client is already in the client list and segments owned
        else
        {
            int segment_counter = 0;
            // For each segment owned by the client
            for (auto &segment : segments_owned)
            {
                // If the segment is not already in the client list and segments owned
                if (find(client_list_and_segments_owned_update[client_id].begin(), client_list_and_segments_owned_update[client_id].end(), segment) == client_list_and_segments_owned_update[client_id].end())
                {
                    segment_counter++;
                    // Add the segment to the client list and segments owned
                    client_list_and_segments_owned_update[client_id].push_back(segment);
                }
            }
        }
    }
}

void find_best_client(
    vector<string> segments_contained,
    map<int, vector<string>> client_list_and_segments_owned,
    string file,
    peer_info *peer_info_local,
    int &download_counter)
{
    vector<string> segments_downloaded = peer_info_local->get_segments_downloaded(file);
    set<string> requested_segments;

    // For each segment that the client wants and does not own
    // Find the client with the least workload to download from
    // Send a request to that client
    for (auto &segment : segments_contained)
    {
        // If the download counter is bigger than 9
        if (download_counter > 9)
        {
            // Reset the download counter
            download_counter = 0;

            // Send an update to the tracker
            send_update_to_tracker(peer_info_local->get_segments_downloaded(file), file);

            // Handle request response from tracker similar to how it is done to a real response to request to tracker
            map<int, vector<string>> client_list_and_segments_owned_extra = handle_update_response();

            // Update the client list and segments owned
            update_client_list_and_segments_owned(client_list_and_segments_owned_extra, client_list_and_segments_owned);
        }

        // If the segment is not already downloaded
        if (find(segments_downloaded.begin(), segments_downloaded.end(), segment) == segments_downloaded.end())
        {
            int min_workload = INT_MAX;
            int best_client_id = -1;

            // For each client that has the segment
            for (auto &client : client_list_and_segments_owned)
            {
                int client_id = client.first;
                vector<string> segments_owned = client.second;

                // If the client has the segment
                if (find(segments_owned.begin(), segments_owned.end(), segment) != segments_owned.end())
                {
                    // Get nr of requests of client from the distribution center
                    int nr_requests = 0;

                    // Send a request to client to get the number of requests
                    int action = action::GET_WORKLOAD;
                    MPI_Send(
                        &action,
                        1,
                        MPI_INT,
                        client_id,
                        tag::WORKLOAD,
                        MPI_COMM_WORLD);

                    // Receive the number of requests
                    MPI_Recv(
                        &nr_requests,
                        1,
                        MPI_INT,
                        client_id,
                        tag::WORKLOAD,
                        MPI_COMM_WORLD,
                        MPI_STATUS_IGNORE);

                    cout << "Client " << client_id << " has " << nr_requests << " requests" << endl;

                    // If the client has a lower workload
                    if (nr_requests < min_workload)
                    {
                        min_workload = nr_requests;
                        best_client_id = client_id;
                    }
                }
            }

            // Request the segment from the best client
            request_segment_from_best_client(best_client_id, segment, requested_segments);

            // Receive the segment from the best client
            bool res = receive_requested_segment(best_client_id);

            if (res)
            {
                // Increase the download counter
                download_counter++;

                // Add the segment to the downloaded segments
                peer_info_local->add_segment_downloaded(file, segment);
            }
        }
    }
}

void save_downloaded_file(
    vector<string> segments_downloaded,
    int rank,
    string file)
{
    // Save segments to a file with the name client<R>_filename
    string filename = "client" + to_string(rank) + "_" + file;

    // Create a txt file
    ofstream output_file(filename);

    // For each segment
    for (auto &segment : segments_downloaded)
    {
        // Write the segment to the file
        output_file << segment << endl;
    }

    // Close the file
    output_file.close();
}

void check_if_file_was_downloaded(
    int rank,
    string file,
    peer_info *peer_info_local,
    vector<string> segments_contained)
{
    // Get the segments downloaded
    vector<string> segments_downloaded = peer_info_local->get_segments_downloaded(file);

    // If the size of the segments downloaded is equal to the size of the segments contained
    if (segments_downloaded.size() == segments_contained.size())
    {
        // Check each tag to see the order match
        bool order_match = true;
        for (int i = 0; i < (int)(segments_downloaded.size()); i++)
        {
            // If the order does not match
            if (segments_downloaded[i] != segments_contained[i])
            {
                order_match = false;
                break;
            }
        }

        if (order_match)
        {
            // Remove the file from the list of wanted files
            peer_info_local->remove_file_wanted(file);

            // Save the file
            save_downloaded_file(segments_downloaded, rank, file);
        }
    }
}

bool all_files_are_downloaded(peer_info *peer_info_local)
{
    // Check if all files are downloaded
    vector<string> files_wanted = peer_info_local->get_files_wanted();

    // If all files are downloaded
    if (files_wanted.size() == 0)
    {
        // Send an MPI message with a finalize action
        int action = action::FINALIZE;
        MPI_Send(
            &action,
            1,
            MPI_INT,
            TRACKER_RANK,
            tag::COMMANDS,
            MPI_COMM_WORLD);

        return true;
    }

    return false;
}

void download_thread_func(
    int rank,
    peer_info *input)
{
    int download_counter = 0;
    while (!all_files_are_downloaded(input))
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
            find_best_client(
                segments_contained,
                client_list_and_segments_owned,
                file,
                input,
                download_counter);

            // Check if the file was downloaded
            check_if_file_was_downloaded(
                rank,
                file,
                input,
                segments_contained);
        }
    }

    // Close the download thread if all files are downloaded
    return;
}
