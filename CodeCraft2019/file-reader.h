#ifndef FILE_READER_H
#define FILE_READER_H

#include <iostream>

class FileReader
{
private:
    class Invoker
    {
    public:
        virtual bool Invoke(std::istream& is) const = 0;
    };//class Invoker
    
    bool Read(const char* file, const Invoker& invoker) const;

public:
    FileReader();
    
    template <typename _T, typename _OBJ>
    bool Read(const char* file, bool (_T::*cb)(std::istream&), _OBJ* obj) const
    {
        class InvokerImpl : public Invoker
        {
        private:
            bool (_T::*m_cb)(std::istream&);
            _OBJ* m_obj;
        public:
            InvokerImpl(bool (_T::*cb)(std::istream&), _OBJ* obj)
                : m_cb(cb), m_obj(obj)
            { }
            virtual bool Invoke(std::istream& is) const override
            {
                return (m_obj->*m_cb)(is);
            }
        };
        return Read(file, InvokerImpl(cb, obj)); 
    }
    
};//class FileReader



#endif
