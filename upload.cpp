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
        tag::DOWNLOAD,
        MPI_COMM_WORLD,
        &status);

    // Return the source of the action
    return status.MPI_SOURCE;
}

void upload_thread_func(int rank, peer_info *input, distribution_center *dc)
{
    // Receive in loop requests
    while (!dc->get_all_clients_finished_downloading())
    {
        // Receive request from other peers
        int action;
        int source = recv_request(action);

        // If the action is a request
        if (action == REQUEST)
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
        }
    }

    // Close the thread
    return;
}