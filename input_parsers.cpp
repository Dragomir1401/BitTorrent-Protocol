#include "header.hpp"

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

    // Create result
    peer_info *result = new peer_info(filesOwned, filesWanted);

    // Close file
    in.close();

    return result;
}
