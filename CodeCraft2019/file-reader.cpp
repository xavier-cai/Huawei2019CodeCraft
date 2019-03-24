#include "file-reader.h"
//#include "assert.h"
#include <fstream>
#include <sstream>

FileReader::FileReader()
{ }

bool FileReader::Read(const char* file, const FileReader::Invoker& invoker) const
{
    std::ifstream ifs(file);
    if (!ifs.is_open())
        return false;
    char line[1000];
    while(ifs.getline(line, sizeof(line)).gcount())
    {
        std::stringstream ss;
        ss << line;
        if (!invoker.Invoke(ss))
            break;
    }
    ifs.clear();
    ifs.close();
    return true;
}
