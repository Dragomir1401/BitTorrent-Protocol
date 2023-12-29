#include "header.hpp"

bool check_if_received_kill()
{
    MPI_Status status;
    int flag;
    int kill_message;

    // Non-blocking probe to check if a kill message has been sent by the tracker
    MPI_Iprobe(
        TRACKER_RANK,
        tag::KILL,
        MPI_COMM_WORLD,
        &flag,
        &status);

    if (flag)
    {
        // A message is available, receive the kill message
        MPI_Request killReq;
        MPI_Irecv(
            &kill_message,
            1,
            MPI_INT,
            TRACKER_RANK,
            tag::KILL,
            MPI_COMM_WORLD,
            &killReq);

        // Check if the received message is the kill signal
        if (kill_message == action::KILL_UPLOAD_THREAD)
        {
            return true; // Kill signal received
        }
    }

    return false; // No kill signal received
}

void get_request_for_workload(int workload)
{
    // Receive the request_workload message
    int request_workload_message;
    MPI_Status status;
    MPI_Recv(
        &request_workload_message,
        1,
        MPI_INT,
        MPI_ANY_SOURCE,
        tag::WORKLOAD,
        MPI_COMM_WORLD,
        &status);

    // Send the workload to the source
    MPI_Send(
        &workload,
        1,
        MPI_INT,
        status.MPI_SOURCE,
        tag::WORKLOAD,
        MPI_COMM_WORLD);
}

void upload_thread_func(int rank, peer_info *input, distribution_center *dc)
{
    std::queue<MPI_Request> requestQueue;
    MPI_Status status;
    int flag;

    while (true)
    {
        if (check_if_received_kill())
        {
            cout << "Proccess with rank " << rank << " received kill message" << endl;
            // Process any remaining requests before exiting
            while (!requestQueue.empty())
            {
                // Handle pending requests
                auto req = requestQueue.front();
                // Cancel the request
                MPI_Cancel(&req);
                // Remove the request from the queue
                requestQueue.pop();
            }

            // Close the thread
            return;
        }

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
            get_request_for_workload(requestQueue.size());

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