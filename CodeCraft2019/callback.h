#ifndef CALLBACK_H
#define CALLBACK_H

class NullClass { };

template <typename _T, typename _B = NullClass>
class RefCount : public _B
{
private:
    int m_counter;
public:
    RefCount()
        : m_counter(0)
    { }

    virtual ~RefCount()
    { }
    //const int& Ref() const { return ++const_cast<RefCount*>(this)->m_counter; }
    //const int& UnRef() const { return --const_cast<RefCount*>(this)->m_counter; }
    const int& Ref()
    {
        return ++m_counter;
    }

    const int& UnRef()
    {
        return --m_counter;
    }

};//class RefCount

class Callback
{
private:
    virtual ~Callback() = 0;

public:

#define CALLBACK_DEFINE(id, TypenameList, TypeList, ArgumentsIn, ArgumentsOut) \
    template <typename _R, TypenameList> \
    class Invoker##id : public RefCount< Invoker##id <_R, TypeList> > \
    { \
    public: \
        virtual ~Invoker##id () { }; \
        virtual _R Invoke (ArgumentsIn) const = 0; \
    }; \
    \
    template <typename _R, TypenameList> \
    class Handle##id \
    { \
    private: \
        Invoker##id <_R, TypeList>* m_invoker; \
        \
        void ReleaseInvoker () \
        { \
            if (m_invoker != 0 && m_invoker->UnRef() <= 0) \
                delete m_invoker; \
            m_invoker = 0; \
        } \
        \
        void PeekInvoker () \
        { \
            if (m_invoker != 0) \
                m_invoker->Ref(); \
        } \
        \
    public: \
        Handle##id (Invoker##id <_R, TypeList>* invoker = 0) \
            : m_invoker(invoker) \
        { \
            PeekInvoker(); \
        } \
        \
        Handle##id (const Handle##id & o) \
        { \
            *this = o; \
        } \
        \
        Handle##id & operator = (const Handle##id & o) \
        { \
            ReleaseInvoker(); \
            m_invoker = o.m_invoker; \
            PeekInvoker(); \
            return *this; \
        } \
        \
        ~Handle##id () \
        { \
            ReleaseInvoker(); \
        } \
        \
        bool IsNull () const \
        { \
            return m_invoker == 0; \
        } \
        \
        _R Invoke (ArgumentsIn) const \
        { \
            return m_invoker->Invoke(ArgumentsOut); \
        } \
    }; \
    \
    template <typename _R, TypenameList> \
    static Handle##id <_R, TypeList> Create(_R (*fun)(TypeList)) \
    { \
        class InvokerImpl : public Invoker##id <_R, TypeList> \
        { \
        private: \
            _R (*m_fun)(TypeList); \
        public: \
            InvokerImpl (_R (*fun)(TypeList)) \
                : m_fun(fun) \
            { } \
            virtual _R Invoke (ArgumentsIn) const \
            { \
                return (*m_fun)(ArgumentsOut); \
            } \
        } *instance = new InvokerImpl(fun); \
        return Handle##id <_R, TypeList>(instance); \
    } \
    \
    template <typename _C, typename _O, typename _R, TypenameList> \
    static Handle##id <_R, TypeList> Create(_R (_C::*fun)(TypeList), _O* o) \
    { \
        class InvokerImpl : public Invoker##id <_R, TypeList> \
        { \
        private: \
            _R (_C::*m_fun)(TypeList); \
            _O* m_o; \
        public: \
            InvokerImpl (_R (_C::*fun)(TypeList), _O* o) \
                : m_fun(fun), m_o(o) { } \
            virtual _R Invoke (ArgumentsIn) const \
            { \
                _C& c = *m_o; \
                return (c.*m_fun)(ArgumentsOut);\
            } \
        } *instance = new InvokerImpl(fun, o); \
        return Handle##id <_R, TypeList>(instance); \
    } \
    \
    template <typename _C, typename _O, typename _R, TypenameList> \
    static Handle##id <_R, TypeList> Create(_R (_C::*fun)(TypeList) const, const _O* o) \
    { \
    class InvokerImpl : public Invoker##id <_R, TypeList> \
        { \
        private: \
            _R (_C::*m_fun)(TypeList) const; \
            const _O* m_o; \
        public: \
            InvokerImpl (_R (_C::*fun)(TypeList) const, const _O* o) \
                : m_fun(fun), m_o(o) \
            { } \
            virtual _R Invoke (ArgumentsIn) const \
            { \
                const _C& c = *m_o; \
                return (c.*m_fun)(ArgumentsOut); \
            } \
        } *instance = new InvokerImpl(fun, o); \
        return Handle##id <_R, TypeList>(instance); \
    }

#define COMMA ,
CALLBACK_DEFINE(1, typename _A, _A, _A a, a)
CALLBACK_DEFINE(2, typename _A1 COMMA typename _A2, _A1 COMMA _A2, _A1 a1 COMMA _A2 a2, a1 COMMA a2)
CALLBACK_DEFINE(3, typename _A1 COMMA typename _A2 COMMA typename _A3, _A1 COMMA _A2 COMMA _A3, _A1 a1 COMMA _A2 a2 COMMA _A3 a3, a1 COMMA a2 COMMA a3)
#undef COMMA

#undef CALLBACK_DEFINE

};//class Callback

#endif