#include "header.hpp"

void receive_filename(
    char *filename,
    int from)
{
    MPI_Recv(
        filename,
        MAX_FILENAME,
        MPI_CHAR,
        from,
        0,
        MPI_COMM_WORLD,
        MPI_STATUS_IGNORE);
}

void receive_num_segments(
    int *num_segments,
    int from)
{
    MPI_Recv(
        num_segments,
        1,
        MPI_INT,
        from,
        0,
        MPI_COMM_WORLD,
        MPI_STATUS_IGNORE);
}

void receive_segments(
    vector<string> &segments_owned,
    int from,
    int num_segments)
{
    for (int j = 0; j < num_segments; j++)
    {
        MPI_Status status;
        MPI_Probe(from, 0, MPI_COMM_WORLD, &status);

        int segment_size;
        MPI_Get_count(&status, MPI_CHAR, &segment_size);

        char *segment = new char[segment_size];

        MPI_Recv(
            segment,
            segment_size,
            MPI_CHAR,
            from,
            0,
            MPI_COMM_WORLD,
            MPI_STATUS_IGNORE);

        string segment_string(segment, segment_size - 1);
        segments_owned.push_back(segment_string);
        delete[] segment;
    }
}

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
            0,
            MPI_COMM_WORLD);
    }
}

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
        MPI_ANY_TAG,
        MPI_COMM_WORLD,
        &status);

    // Return the source of the action
    return status.MPI_SOURCE;
}

void send_segments(
    vector<string> segments_owned,
    int dest,
    int num_segments_owned)
{
    for (auto &segment : segments_owned)
    {
        // Send each segment to the source
        MPI_Send(
            segment.c_str(),
            segment.size() + 1,
            MPI_CHAR,
            dest,
            0,
            MPI_COMM_WORLD);
    }
}

void handle_request(
    int source,
    tracker_info *tracker_info_local)
{
    // Receive the filename
    char *filename = (char *)malloc(MAX_FILENAME * sizeof(char));
    receive_filename(filename, source);
    string filename_string(filename);

    cout << "filename: " << filename_string << endl;

    // Send back to the source the swarm info for the requested file
    map<string, swarm_info> file_to_peers_owning_it = tracker_info_local->get_file_to_peers_owning_it();
    swarm_info swarm = file_to_peers_owning_it[filename_string];
    map<int, vector<string>> client_list_and_segments_owned = swarm.get_client_list_and_segments_owned();

    // Send the number of peers owning the file
    int num_peers_owning_file = client_list_and_segments_owned.size();
    MPI_Send(
        &num_peers_owning_file,
        1,
        MPI_INT,
        source,
        0,
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
            0,
            MPI_COMM_WORLD);

        // Send the number of segments owned
        int num_segments_owned = client.second.size();
        MPI_Send(
            &num_segments_owned,
            1,
            MPI_INT,
            source,
            0,
            MPI_COMM_WORLD);

        // Send all segments owned by the respective client
        send_segments(client.second, source, num_segments_owned);
    }
}

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
            0,
            MPI_COMM_WORLD,
            MPI_STATUS_IGNORE);

        // For each file owned
        for (int k = 0; k < num_files_owned; k++)
        {
            // Receive the file name
            char *filename = (char *)malloc(MAX_FILENAME * sizeof(char));
            receive_filename(filename, i);
            string filename_string(filename);

            // Receive the number of segments
            int num_segments;
            receive_num_segments(&num_segments, i);

            // Receive each segment
            vector<string> segments_owned;
            receive_segments(segments_owned, i, num_segments);

            // Add segments to tracker info based on the pairing filename-swarm
            add_to_tracker_info(tracker_info_local, i, filename_string, segments_owned);
        }
    }
}

void tracker(
    int numtasks,
    int rank)
{
    // Declare tracker info
    tracker_info *tracker_info_local = new tracker_info();

    // Receive segments from peers
    receive_initial_holders(numtasks, tracker_info_local);

    // After receiving from all the clients, send ACK to all of them
    // to allow them to continue with downloading or uploading
    send_acks(numtasks);

    // Receive actions from peers using recv any
    // and send back the appropriate response
    int action;
    int source = recv_command(action);

    switch (action)
    {
    case action::REQUEST:
        handle_request(source, tracker_info_local);
        break;

    case action::UPDATE:
        break;

    case action::FINALIZE:
        break;

    default:
        cout << "Error: action not recognized" << endl;
        break;
    }

    // Print tracker info to out file
    // ofstream out_file;
    // out_file.open("out.txt");

    // map<string, swarm_info> file_to_peers_owning_it = tracker_info_local->get_file_to_peers_owning_it();
    // for (auto &file : file_to_peers_owning_it)
    // {
    //     out_file << file.first << endl;
    //     for (auto &swarm : file.second.get_client_list_and_segments_owned())
    //     {
    //         out_file << swarm.first << endl;
    //         for (auto &segment : swarm.second)
    //         {
    //             out_file << segment << endl;
    //         }
    //     }
    // }

    // out_file.close();
}
