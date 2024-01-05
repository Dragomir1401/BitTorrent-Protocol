#include "header.hpp"

int main(
    int argc,
    char *argv[])
{
    int numtasks, rank;
    int provided;

    // Create a logger instance
    logger *log = new logger("log.txt");

    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    if (provided < MPI_THREAD_MULTIPLE)
    {
        cerr << "MPI does not have support for multi-threading\n";
        exit(-1);
    }

    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == TRACKER_RANK)
    {
        tracker(numtasks, rank, log);
    }
    else
    {
        peer(numtasks, rank, log);
    }

    // Call the log destructor
    delete log;

    MPI_Finalize();
    return 0;
}
