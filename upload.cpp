#include "header.hpp"

int recv_request(
    queue<MPI_Request> &requestQueue)
{
    // Receive action from the other clients excpet the tracker and itself
    MPI_Request recvReq;
    int action;
    MPI_Irecv(
        &action,
        1,
        MPI_INT,
        MPI_ANY_SOURCE,
        tag::DOWNLOAD_REQUEST,
        MPI_COMM_WORLD,
        &recvReq);
    requestQueue.push(recvReq);

    /// Test the recvReq
    int flag;
    MPI_Status status;
    MPI_Test(&recvReq, &flag, &status);

    return status.MPI_SOURCE;
}

bool check_if_received_kill()
{
    // Receive the kill message
    int kill_message;
    MPI_Request recvReq;
    MPI_Irecv(
        &kill_message,
        1,
        MPI_INT,
        TRACKER_RANK,
        tag::KILL,
        MPI_COMM_WORLD,
        &recvReq);

    if (kill_message == action::KILL_UPLOAD_THREAD)
    {
        return true;
    }

    return false;
}

void upload_thread_func(int rank, peer_info *input, distribution_center *dc)
{
    std::queue<MPI_Request> requestQueue;
    MPI_Status status;
    int flag;

    while (true)
    {
        // if (check_if_received_kill())
        // {
        //     cout << "Proccess with rank " << rank << " received kill message" << endl;
        //     // Process any remaining requests before exiting
        //     while (!requestQueue.empty())
        //     {
        //         // Handle pending requests
        //         auto req = requestQueue.front();
        //         // Cancel the request
        //         MPI_Cancel(&req);
        //         // Remove the request from the queue
        //         requestQueue.pop();
        //     }

        //     // Close the thread
        //     return;
        // }

        // Non-blocking receive of requests from other peers
        int action;
        MPI_Request recvReq;
        MPI_Irecv(
            &action,
            1,
            MPI_INT,
            MPI_ANY_SOURCE,
            tag::DOWNLOAD_REQUEST,
            MPI_COMM_WORLD,
            &recvReq);

        requestQueue.push(recvReq);

        // Check and process completed requests
        while (!requestQueue.empty())
        {
            MPI_Test(&requestQueue.front(), &flag, &status);
            if (flag)
            {
                // Handle the request
                int source = status.MPI_SOURCE;

                // Send back ack to the source
                char ack[4] = "ACK";
                MPI_Send(
                    ack,
                    4,
                    MPI_CHAR,
                    source,
                    tag::INIT,
                    MPI_COMM_WORLD);

                // Remove the completed request from the queue
                requestQueue.pop();
            }
        }
    }

    // Close the thread
    return;
}