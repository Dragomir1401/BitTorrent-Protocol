#include "header.hpp"

/// @brief  Function that receives the filename from a peer
/// @param filename - Pointer to the filename
/// @param from - Source of the filename (peer)
/// @param tag - Tag of the message
void receive_filename(
    char *filename,
    int from,
    tag tag)
{
    MPI_Recv(
        filename,
        MAX_FILENAME,
        MPI_CHAR,
        from,
        tag,
        MPI_COMM_WORLD,
        MPI_STATUS_IGNORE);
}

/// @brief  Function that receives the number of segments of a file from a peer
/// @param num_segments - Pointer to the number of segments of the file
/// @param from - Source of the segments (peer)
/// @param tag - Tag of the message
void receive_num_segments(
    int *num_segments,
    int from,
    tag tag)
{
    MPI_Recv(
        num_segments,
        1,
        MPI_INT,
        from,
        tag,
        MPI_COMM_WORLD,
        MPI_STATUS_IGNORE);
}

/// @brief  Function that receives the segments of a file from a peer
/// @param segments_owned - Vector of strings containing the segments of the file
/// @param from - Source of the segments (peer)
/// @param num_segments - Number of segments of the file
/// @param tag - Tag of the message
void receive_segments(
    vector<string> &segments_owned,
    int from,
    int num_segments,
    tag tag)
{
    for (int j = 0; j < num_segments; j++)
    {
        MPI_Status status;
        MPI_Probe(from, tag, MPI_COMM_WORLD, &status);

        int segment_size;
        MPI_Get_count(&status, MPI_CHAR, &segment_size);

        char *segment = new char[segment_size];

        MPI_Recv(
            segment,
            segment_size,
            MPI_CHAR,
            from,
            tag,
            MPI_COMM_WORLD,
            MPI_STATUS_IGNORE);

        string segment_string(segment, segment_size - 1);
        segments_owned.push_back(segment_string);
        delete[] segment;
    }
}

/// @brief  Function that adds the segments to the tracker info
/// @param tracker_info_local - Pointer to the tracker info object
/// @param i - Id of the peer
/// @param filename_string - Name of the file
/// @param segments_owned - Vector of strings containing the segments of the file
void add_to_tracker_info(
    tracker_info *tracker_info_local,
    int i,
    string filename_string,
    vector<string> segments_owned)
{
    map<string, swarm_info> file_to_peers_owning_it = tracker_info_local->get_file_to_peers_owning_it();

    if (file_to_peers_owning_it.find(filename_string) == file_to_peers_owning_it.end())
    {
        // If the file is not in the tracker info, add it
        swarm_info swarm;
        swarm.add_client(i, segments_owned);
        tracker_info_local->add_file(filename_string, swarm);
    }
    else
    {
        // If the file is in the tracker info, add the segments to the swarm
        file_to_peers_owning_it[filename_string].add_client(i, segments_owned);
    }
}

/// @brief  Function that sends acks to all peers
/// @param numtasks - Number of MPI tasks
void send_acks(
    int numtasks)
{
    // Create ack message with null terminator
    char ack[4] = "ACK";

    for (int i = 1; i < numtasks; i++)
    {
        // Send ack to all peers
        // Include null terminator in the length
        MPI_Send(
            ack,
            4,
            MPI_CHAR,
            i,
            tag::INIT,
            MPI_COMM_WORLD);
    }
}

/// @brief  Function that receives a command from a peer
/// @param action - Action to be received
/// @return - Source of the action
int recv_command(
    int &action)
{
    // Receive action from clients using recv any
    // And store the action in a int
    MPI_Status status;
    MPI_Recv(
        &action,
        1,
        MPI_INT,
        MPI_ANY_SOURCE,
        tag::COMMANDS,
        MPI_COMM_WORLD,
        &status);

    // Return the source of the action
    return status.MPI_SOURCE;
}

/// @brief  Function that sends the segments of a file to a peer
/// @param segments_owned - Vector of strings containing the segments of the file
/// @param dest - Destination of the segments (peer)
/// @param num_segments_owned - Number of segments owned by the peer
/// @param tag - Tag of the message
void send_segments(
    vector<string> segments_owned,
    int dest,
    int num_segments_owned,
    tag tag)
{
    for (auto &segment : segments_owned)
    {
        // Send each segment to the destination
        MPI_Send(
            segment.c_str(),
            segment.size() + 1,
            MPI_CHAR,
            dest,
            tag,
            MPI_COMM_WORLD);
    }
}

/// @brief  Function that sends the segments of a file to a peer
/// @param number_of_segments - Number of segments of the file
/// @param segment - Vector of strings containing the segments of the file
/// @param dest - Destination of the segments (peer)
void send_info_about_file_structure(int number_of_segments, vector<string> segment, int dest)
{
    // Send number of segments which the file is divided into
    MPI_Send(
        &number_of_segments,
        1,
        MPI_INT,
        dest,
        tag::COMMANDS,
        MPI_COMM_WORLD);

    // Send all segments that compose the file
    send_segments(segment, dest, number_of_segments, tag::COMMANDS);
}

/// @brief  Function that handles the request command from a peer
/// @param source - Source of the request command (peer)
/// @param tracker_info_local - Pointer to the tracker info object
void handle_request(
    int source,
    tracker_info *tracker_info_local)
{
    // Receive the filename
    char *filename = (char *)malloc(MAX_FILENAME * sizeof(char));
    receive_filename(filename, source, tag::COMMANDS);
    string filename_string(filename);

    // Send back to the source the swarm info for the requested file
    map<string, swarm_info> file_to_peers_owning_it = tracker_info_local->get_file_to_peers_owning_it();
    swarm_info swarm = file_to_peers_owning_it[filename_string];
    map<int, vector<string>> client_list_and_segments_owned = swarm.get_client_list_and_segments_owned();

    // Send the file structure
    int number_of_segments = tracker_info_local->get_segments(filename_string).size();
    vector<string> segments = tracker_info_local->get_segments(filename_string);
    send_info_about_file_structure(number_of_segments, segments, source);

    // Send the number of peers owning the file
    int num_peers_owning_file = client_list_and_segments_owned.size();
    MPI_Send(
        &num_peers_owning_file,
        1,
        MPI_INT,
        source,
        tag::COMMANDS,
        MPI_COMM_WORLD);

    // Send the client list and segments owned
    for (auto &client : client_list_and_segments_owned)
    {
        // Send the client id
        int client_id = client.first;
        MPI_Send(
            &client_id,
            1,
            MPI_INT,
            source,
            tag::COMMANDS,
            MPI_COMM_WORLD);

        // Send the number of segments owned
        int num_segments_owned = client.second.size();
        MPI_Send(
            &num_segments_owned,
            1,
            MPI_INT,
            source,
            tag::COMMANDS,
            MPI_COMM_WORLD);

        // Send all segments owned by the respective client
        send_segments(client.second, source, num_segments_owned, tag::COMMANDS);
    }
}

/// @brief  Receives the initial holders of the files from all peers
/// @param numtasks - Number of MPI tasks
/// @param tracker_info_local - Pointer to the tracker info object
void receive_initial_holders(
    int numtasks,
    tracker_info *tracker_info_local)
{
    for (int i = 1; i < numtasks; i++)
    {
        // Firstly receive number of files owned
        int num_files_owned;
        MPI_Recv(
            &num_files_owned,
            1,
            MPI_INT,
            i,
            tag::INIT,
            MPI_COMM_WORLD,
            MPI_STATUS_IGNORE);

        // For each file owned
        for (int k = 0; k < num_files_owned; k++)
        {
            // Receive the file name
            char *filename = (char *)malloc(MAX_FILENAME * sizeof(char));
            receive_filename(filename, i, tag::INIT);
            string filename_string(filename);

            // Receive the number of segments
            int num_segments;
            receive_num_segments(&num_segments, i, tag::INIT);

            // Receive each segment
            vector<string> segments_owned;
            receive_segments(segments_owned, i, num_segments, tag::INIT);

            // Add segments to tracker info based on the pairing filename-swarm
            add_to_tracker_info(tracker_info_local, i, filename_string, segments_owned);

            // Add the segment to the tracker info
            tracker_info_local->add_segments(filename_string, segments_owned);
        }
    }
}

/// @brief  Function that handles the finalize command from a peer
/// @param source - Source of the finalize command (peer)
/// @param finished_downloading - Map that stores which peers have finished downloading
void handle_finalize(
    int source,
    map<int, bool> &finished_downloading)
{
    // The peer source has finished downloading all the files wanted
    finished_downloading[source] = true;
}

/// @brief  Function that checks if all peers have finished downloading
/// @param finished_downloading - Map that stores which peers have finished downloading
/// @param numtasks  - Number of MPI tasks
/// @return - True if all peers have finished downloading, false otherwise
bool all_peers_finalized(
    map<int, bool> finished_downloading,
    int numtasks)
{
    for (int i = 1; i < numtasks; i++)
    {
        if (!finished_downloading[i])
        {
            return false;
        }
    }

    return true;
}

/// @brief  Function that sends a message to all peers to kill the upload thread
/// @param numtasks - Number of MPI tasks
/// @param action - Action to be sent
void send_message_to_upload(
    int numtasks,
    int action)
{
    for (int i = 1; i < numtasks; i++)
    {
        MPI_Send(
            &action,
            1,
            MPI_INT,
            i,
            tag::KILL,
            MPI_COMM_WORLD);
    }
}

/// @brief  Function that handles the update command from a peer
/// @param source - Source of the update command (peer)
/// @param tracker_info_local - Pointer to the tracker info object
void handle_update(int source, tracker_info *tracker_info_local)
{
    // Receive the filename
    char *filename = (char *)malloc(MAX_FILENAME * sizeof(char));
    receive_filename(filename, source, tag::UPDATE_COMMAND);
    string filename_string(filename);

    // Receive the number of segments
    int num_segments;
    receive_num_segments(&num_segments, source, tag::UPDATE_COMMAND);

    // Receive each segment
    vector<string> segments_owned;
    receive_segments(segments_owned, source, num_segments, tag::UPDATE_COMMAND);

    // Add segments to tracker info based on the pairing filename-swarm
    swarm_info swarm = tracker_info_local->get_file_to_peers_owning_it()[filename_string];
    map<int, vector<string>> client_list_and_segments_owned = swarm.get_client_list_and_segments_owned();

    if (client_list_and_segments_owned.find(source) == client_list_and_segments_owned.end())
    {
        vector<string> new_segments;
        for (int i = 0; i < (int)(segments_owned.size()); i++)
        {
            new_segments.push_back(segments_owned[i]);
        }
        client_list_and_segments_owned[source] = new_segments;
    }
    // If the client has some segments of the file, add the new ones
    else
    {
        for (int i = 0; i < (int)(segments_owned.size()); i++)
        {
            // If the client does not have the segment, add it
            if (find(client_list_and_segments_owned[source].begin(),
                     client_list_and_segments_owned[source].end(), segments_owned[i]) ==
                client_list_and_segments_owned[source].end())
            {
                client_list_and_segments_owned[source].push_back(segments_owned[i]);
            }
        }
    }

    // Add the swarm to the tracker info
    swarm_info new_swarm(client_list_and_segments_owned);
    tracker_info_local->add_file(filename_string, new_swarm);

    // Send the number of peers owning the file
    int num_peers_owning_file = client_list_and_segments_owned.size();
    MPI_Send(
        &num_peers_owning_file,
        1,
        MPI_INT,
        source,
        tag::UPDATE_COMMAND,
        MPI_COMM_WORLD);

    // Send the client list and segments owned
    for (auto &client : client_list_and_segments_owned)
    {
        // Send the client id
        int client_id = client.first;
        MPI_Send(
            &client_id,
            1,
            MPI_INT,
            source,
            tag::UPDATE_COMMAND,
            MPI_COMM_WORLD);

        // Send the number of segments owned
        int num_segments_owned = client.second.size();
        MPI_Send(
            &num_segments_owned,
            1,
            MPI_INT,
            source,
            tag::UPDATE_COMMAND,
            MPI_COMM_WORLD);

        // Send all segments owned by the respective client
        send_segments(client.second, source, num_segments_owned, tag::UPDATE_COMMAND);
    }
}

/// @brief  Function that implements the tracker logic
/// @param numtasks - Number of MPI tasks
/// @param rank - Rank of the current task
/// @param log - Pointer to the logger instance
void tracker(
    int numtasks,
    int rank,
    logger *log)
{
    // Declare tracker info
    tracker_info *tracker_info_local = new tracker_info();

    // Receive segments from peers
    receive_initial_holders(numtasks, tracker_info_local);

    // Declare map to store which peers have finished downloading
    map<int, bool> finished_downloading;
    for (int i = 1; i < numtasks; i++)
    {
        finished_downloading[i] = false;
    }

    // After receiving from all the clients, send ACK to all of them
    // to allow them to continue with downloading or uploading
    send_acks(numtasks);

    // Receive actions from peers using recv any
    // and send back the appropriate response
    while (!all_peers_finalized(finished_downloading, numtasks))
    {
        int action;
        int source = recv_command(action);

        switch (action)
        {
        case action::REQUEST:
            handle_request(source, tracker_info_local);
            break;

        case action::UPDATE:
            handle_update(source, tracker_info_local);
            break;

        case action::FINALIZE:
            handle_finalize(source, finished_downloading);
            break;

        default:
            cout << "Error: action not recognized" << endl;
            break;
        }
    }

    // Send a KILL message on the KILL_UPLOAD_THREAD tag to all peers
    send_message_to_upload(numtasks, action::KILL_UPLOAD_THREAD);

    return;
}
