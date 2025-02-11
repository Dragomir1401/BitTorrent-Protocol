#include "header.hpp"

/// @brief Sends through MPI the number of files owned by the peer
/// @param num_files_owned  - Number of files owned
void send_num_files(
    int num_files_owned)
{
    MPI_Send(
        &num_files_owned,
        1,
        MPI_INT,
        TRACKER_RANK,
        tag::INIT,
        MPI_COMM_WORLD);
}

/// @brief Sends through MPI the segments of a file
/// @param segments - Vector of strings containing the segments
void send_segments(
    vector<string> segments)
{
    for (auto &segment : segments)
    {
        // Send each segment to tracker
        MPI_Send(
            segment.c_str(),
            segment.size() + 1,
            MPI_CHAR,
            TRACKER_RANK,
            tag::INIT,
            MPI_COMM_WORLD);
    }
}

/// @brief Sends through MPI the filews owned by the peer to the tracker
/// @param input - Pointer to the peer_info object containing the information
void send_each_file_owned(
    peer_info *input)
{
    for (auto &file : input->get_files_owned())
    {
        // Firstly send through MPI the name of the file owned
        MPI_Send(
            file.first.c_str(),
            file.first.size() + 1,
            MPI_CHAR,
            TRACKER_RANK,
            tag::INIT,
            MPI_COMM_WORLD);

        // Then send the number of segments
        int num_segments = file.second.size();
        MPI_Send(
            &num_segments,
            1,
            MPI_INT,
            TRACKER_RANK,
            tag::INIT,
            MPI_COMM_WORLD);

        // Then send each segment
        send_segments(file.second);
    }
}

/// @brief - Receives ack from tracker to know to start downloading
/// @param ack - Pointer to the ack string
void receive_ack(
    char *ack)
{
    // Expecting 4 characters including null terminator
    MPI_Recv(
        ack,
        4,
        MPI_CHAR,
        TRACKER_RANK,
        tag::INIT,
        MPI_COMM_WORLD,
        MPI_STATUS_IGNORE);

    ack[3] = '\0';
}

/// @brief  Function that encapsulates the peer's functionality
/// @param numtasks - Number of MPI tasks
/// @param rank - Rank of the current task
/// @param log - Pointer to the logger instance
void peer(
    int numtasks,
    int rank,
    logger *log)
{
    // Read input for peer
    peer_info *input = read_peer_input(rank);

    // Firstly send number of files owned to tracker
    int num_files_owned = input->get_files_owned().size();
    send_num_files(num_files_owned);

    // Send through MPI each segment of each file owned to the tracker
    send_each_file_owned(input);

    // Receive ack from tracker to know that it has received all the segments
    char *ack = (char *)malloc(4 * sizeof(char));
    receive_ack(ack);

    // See if ack is correct
    if (strcmp(ack, "ACK") != 0)
    {
        cout << "Error: ack not received correctly" << endl;
        exit(1);
    }

    thread download_thread(download_thread_func, rank, input, numtasks, log);
    thread upload_thread(upload_thread_func, rank, input, log);

    download_thread.join();
    upload_thread.join();
}
