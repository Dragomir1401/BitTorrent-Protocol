#include "header.hpp"

int recv_request(
    int &action)
{
    // Receive action from the other clients excpet the tracker and itself
    MPI_Status status;
    MPI_Recv(
        &action,
        1,
        MPI_INT,
        MPI_ANY_SOURCE,
        tag::DOWNLOAD_REQUEST,
        MPI_COMM_WORLD,
        &status);

    // Return the source of the action
    return status.MPI_SOURCE;
}

bool check_if_received_kill()
{
    // Probe to see if tracker sent a kill message
    MPI_Status status;
    int flag;
    MPI_Iprobe(
        TRACKER_RANK,
        tag::KILL,
        MPI_COMM_WORLD,
        &flag,
        &status);

    if (!flag)
    {
        return false;
    }

    // Receive the kill message
    int kill_message;
    MPI_Recv(
        &kill_message,
        1,
        MPI_INT,
        TRACKER_RANK,
        tag::KILL,
        MPI_COMM_WORLD,
        MPI_STATUS_IGNORE);

    if (kill_message == action::KILL_UPLOAD_THREAD)
    {
        return true;
    }

    return false;
}

void upload_thread_func(int rank, peer_info *input, distribution_center *dc)
{
    // Receive in loop requests
    while (true)
    {
        if (check_if_received_kill())
        {
            // Close the thread
            return;
        }

        // Receive request from other peers
        int action;
        int source = recv_request(action);

        // If the action is a request
        if (action == action::REQUEST)
        {
            // Add a request to the distribution center for the current client
            dc->add_request(rank);

            MPI_Status status;
            MPI_Probe(source, tag::DOWNLOAD, MPI_COMM_WORLD, &status);

            int segment_size;
            MPI_Get_count(&status, MPI_CHAR, &segment_size);

            char *segment = new char[segment_size];

            MPI_Recv(
                segment,
                segment_size,
                MPI_CHAR,
                source,
                tag::DOWNLOAD,
                MPI_COMM_WORLD,
                MPI_STATUS_IGNORE);

            delete segment;

            // Send an ACK to the source meaning that the segment is being sent
            char *ack = (char *)malloc(4 * sizeof(char));
            strcpy(ack, "ACK");
            MPI_Send(
                ack,
                4,
                MPI_CHAR,
                source,
                tag::UPLOAD,
                MPI_COMM_WORLD);

            free(ack);
        }
    }

    // Close the thread
    return;
}