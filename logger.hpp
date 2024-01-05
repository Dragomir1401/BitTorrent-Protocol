#include <iostream>
#include <fstream>

using namespace std;

class logger
{
private:
    ofstream log_file;

public:
    logger();
    logger(string filename);
    ~logger();
    void log(string message);
};