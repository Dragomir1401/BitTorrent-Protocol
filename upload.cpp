#include "header.hpp"
MPI_Request killReq = MPI_REQUEST_NULL;
int kill_message = 0;
MPI_Request workloadReq = MPI_REQUEST_NULL;
int request_workload_message = 0;

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

bool check_if_received_kill()
{
    int flag;
    MPI_Test(&killReq, &flag, MPI_STATUS_IGNORE);

    return flag && kill_message == action::KILL_UPLOAD_THREAD;
}

void initialize_workload_request()
{
    if (workloadReq == MPI_REQUEST_NULL)
    {
        MPI_Irecv(
            &request_workload_message,
            1,
            MPI_INT,
            MPI_ANY_SOURCE,
            tag::WORKLOAD,
            MPI_COMM_WORLD,
            &workloadReq);
    }
}

bool check_and_handle_workload_request(int workload)
{
    int flag;
    MPI_Status status;

    MPI_Test(&workloadReq, &flag, &status);

    if (flag)
    {
        // Send the workload to the source
        MPI_Send(
            &workload,
            1,
            MPI_INT,
            status.MPI_SOURCE,
            tag::WORKLOAD,
            MPI_COMM_WORLD);

        // Reinitialize for next workload request
        initialize_workload_request();
        return true;
    }

    return false;
}

void upload_thread_func(int rank, peer_info *input)
{
    std::queue<MPI_Request> requestQueue;
    MPI_Status status;
    int flag;

    // Initialize the workload request
    initialize_workload_request();

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
            check_and_handle_workload_request(requestQueue.size());
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