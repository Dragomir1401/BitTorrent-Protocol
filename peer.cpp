#include "header.hpp"

void peer(int numtasks, int rank)
{
    // Read input for peer
    peer_info *input = read_peer_input(rank);

    // Firstly send number of files owned to tracker
    int num_files_owned = input->get_files_owned().size();
    MPI_Send(
        &num_files_owned,
        1,
        MPI_INT,
        TRACKER_RANK,
        0,
        MPI_COMM_WORLD);

    // Send through MPI each segment of each file owned to the tracker
    // The tracker is rank 0
    for (auto &file : input->get_files_owned())
    {
        // Firstly send through MPI the name of the file owned
        MPI_Send(
            file.first.c_str(),
            file.first.size() + 1,
            MPI_CHAR,
            TRACKER_RANK,
            0,
            MPI_COMM_WORLD);

        // Then send the number of segments
        int num_segments = file.second.size();
        MPI_Send(
            &num_segments,
            1,
            MPI_INT,
            TRACKER_RANK,
            0,
            MPI_COMM_WORLD);

        for (auto &segment : file.second)
        {
            // Send each segment to tracker
            MPI_Send(
                segment.c_str(),
                segment.size() + 1,
                MPI_CHAR,
                TRACKER_RANK,
                0,
                MPI_COMM_WORLD);
        }
    }

    // thread download_thread(download_thread_func, rank);
    // thread upload_thread(upload_thread_func, rank);

    // download_thread.join();
    // upload_thread.join();
}
