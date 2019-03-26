#ifndef FILE_READER_H
#define FILE_READER_H

#include <iostream>
#include "callback.h"

class FileReader
{
public:
    FileReader();
    bool Read(const char* file, Callback::Handle<bool, std::istream&> callback) const;
    
};//class FileReader



#endif
