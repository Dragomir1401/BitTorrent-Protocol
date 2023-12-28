#include <map>
#include <string>
#include <vector>
using namespace std;

class peer_info
{
public:
    peer_info(
        map<string, vector<string>> filesOwned,
        vector<string> filesWanted);
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

private:
    map<string, vector<string>> filesOwned;
    vector<string> filesWanted;
};
