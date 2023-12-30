#include <map>
#include <string>
#include <vector>
#include <algorithm>
using namespace std;

class peer_info
{
public:
    peer_info(
        map<string, vector<string>> files_owned,
        vector<string> files_wanted,
        map<string, vector<string>> segments_downloaded);
    ~peer_info();
    map<string, vector<string>> get_files_owned();
    vector<string> get_files_wanted();
    void add_file_owned(
        string filename,
        vector<string> chunks);
    void add_file_wanted(
        string filename);
    void remove_file_owned(
        string filename);
    void remove_file_wanted(
        string filename);
    void add_segment_downloaded(
        string filename,
        string segment);
    vector<string> get_segments_downloaded(
        string filename);

private:
    map<string, vector<string>> files_owned;
    vector<string> files_wanted;
    map<string, vector<string>> segments_downloaded;
};
