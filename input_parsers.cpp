#include "header.hpp"

/// @brief  Reads the input from the file in<rank>.txt
/// @param rank - The rank of the peer
/// @return - A pointer to a peer_info object with the information read from the file
peer_info *read_peer_input(
    int rank)
{
    // Read from file in<rank>.txt
    string filename = "in" + to_string(rank) + ".txt";
    ifstream in(filename);

    // Read number of files owned
    int num_files;
    in >> num_files;

    // Create the fileOwned map
    map<string, vector<string>> filesOwned;

    for (int i = 0; i < num_files; i++)
    {
        // Read filenames
        string filename;
        in >> filename;

        // Read number of chunks in file
        int num_chunks;
        in >> num_chunks;

        // Read chunks
        vector<string> chunks;
        for (int j = 0; j < num_chunks; j++)
        {
            string chunk;
            in >> chunk;
            chunks.push_back(chunk);
        }

        // Add to result
        filesOwned[filename] = chunks;
    }

    // Read number of files wanted
    int num_files_wanted;
    in >> num_files_wanted;

    // Read files wanted
    vector<string> filesWanted;

    for (int i = 0; i < num_files_wanted; i++)
    {
        string filename;
        in >> filename;
        filesWanted.push_back(filename);
    }

    // For each file wanted create an entry in the segmentsDownloaded map with an empty vector
    map<string, vector<string>> segmentsDownloaded;
    for (auto &file : filesWanted)
    {
        segmentsDownloaded[file] = vector<string>();
    }

    // Create result
    peer_info *result = new peer_info(filesOwned, filesWanted, segmentsDownloaded);

    // Close file
    in.close();

    return result;
}
