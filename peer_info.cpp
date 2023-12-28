#include "peer_info.hpp"

peer_info::peer_info(
    map<string, vector<string>> filesOwned,
    vector<string> filesWanted)
{
    this->filesOwned = filesOwned;
    this->filesWanted = filesWanted;
}

peer_info::~peer_info()
{
}

map<string, vector<string>> peer_info::get_files_owned()
{
    return this->filesOwned;
}

vector<string> peer_info::get_files_wanted()
{
    return this->filesWanted;
}

void peer_info::add_file_owned(
    string filename,
    vector<string> chunks)
{
    this->filesOwned[filename] = chunks;
}

void peer_info::add_file_wanted(
    string filename)
{
    this->filesWanted.push_back(filename);
}

void peer_info::remove_file_owned(
    string filename)
{
    this->filesOwned.erase(filename);
}

void peer_info::remove_file_wanted(
    string filename)
{
    for (int i = 0; i < (int)(this->filesWanted.size()); i++)
    {
        if (this->filesWanted[i] == filename)
        {
            this->filesWanted.erase(this->filesWanted.begin() + i);
            break;
        }
    }
}
