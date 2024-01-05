#include "logger.hpp"

logger::logger()
{
    this->log_file.open("log.txt");
}

logger::logger(string filename)
{
    this->log_file.open(filename);
}

logger::~logger()
{
    this->log_file.close();
}

void logger::log(string message)
{
    this->log_file << message << endl;
}