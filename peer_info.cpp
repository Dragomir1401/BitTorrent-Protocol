#include "peer_info.hpp"

peer_info::peer_info(
    map<string, vector<string>> files_owned,
    vector<string> files_wanted,
    map<string, vector<string>> segments_downloaded)
{
    this->files_owned = files_owned;
    this->files_wanted = files_wanted;
    this->segments_downloaded = segments_downloaded;
}

peer_info::~peer_info()
{
}

map<string, vector<string>> peer_info::get_files_owned()
{
    return this->files_owned;
}

vector<string> peer_info::get_files_wanted()
{
    return this->files_wanted;
}

void peer_info::add_file_owned(
    string filename,
    vector<string> chunks)
{
    this->files_owned[filename] = chunks;
}

void peer_info::add_file_wanted(
    string filename)
{
    this->files_wanted.push_back(filename);
}

void peer_info::remove_file_owned(
    string filename)
{
    this->files_owned.erase(filename);
}

void peer_info::remove_file_wanted(
    string filename)
{
    for (int i = 0; i < (int)(this->files_wanted.size()); i++)
    {
        if (this->files_wanted[i] == filename)
        {
            this->files_wanted.erase(this->files_wanted.begin() + i);
            break;
        }
    }
}

void peer_info::add_segment_downloaded(
    string filename,
    string segment)
{
    this->segments_downloaded[filename].push_back(segment);
}

vector<string> peer_info::get_segments_downloaded(
    string filename)
{
    return this->segments_downloaded[filename];
}
