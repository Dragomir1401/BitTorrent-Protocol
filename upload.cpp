#include "header.hpp"
MPI_Request killReq = MPI_REQUEST_NULL;
int kill_message = 0;

/// @brief Function that initializes the kill signal request
void initialize_kill_signal_request()
{
    if (killReq == MPI_REQUEST_NULL)
    {
        MPI_Irecv(
            &kill_message,
            1,
            MPI_INT,
            MPI_ANY_SOURCE,
            tag::KILL,
            MPI_COMM_WORLD,
            &killReq);
    }
}

/// @brief  Function that checks if the kill signal has been received
/// @return - True if the kill signal has been received, false otherwise
bool check_if_received_kill()
{
    int flag;
    MPI_Test(&killReq, &flag, MPI_STATUS_IGNORE);

    return flag && kill_message == action::KILL_UPLOAD_THREAD;
}

/// @brief  Function that encapsulates the upload thread's functionality
/// @param rank - Rank of the current task
/// @param input - Pointer to the peer_info instance
/// @param log - Pointer to the logger instance
void upload_thread_func(int rank, peer_info *input, logger *log)
{
    std::queue<MPI_Request> requestQueue;
    MPI_Status status;
    int flag;

    // Initialize the kill signal request
    initialize_kill_signal_request();

    while (true)
    {
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
            if (check_if_received_kill())
            {
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
                    tag::ACKNOWLEDGEMENT,
                    MPI_COMM_WORLD);

                // Remove the completed request from the queue
                requestQueue.pop();
            }
        }
    }

    // Close the thread
    return;
}