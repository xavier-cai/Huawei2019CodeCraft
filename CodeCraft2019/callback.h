#ifndef CALLBACK_H
#define CALLBACK_H

class NullBase
{ };

template <typename _T, typename _B = NullBase>
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
    template <typename _R, typename _A>
    class Invoker : public RefCount< Invoker<_R, _A> >
    {
    public:
        virtual ~Invoker() { };
        virtual _R Invoke(_A a) const = 0;
    };

    template <typename _R, typename _A>
    class Handle
    {
    private:
        Invoker<_R, _A>* m_invoker;

        void ReleaseInvoker() 
        {
            if (m_invoker != 0 && m_invoker->UnRef() <= 0)
                delete m_invoker;
            m_invoker = 0;
        }

        void PeekInvoker()
        {
            if (m_invoker != 0)
                m_invoker->Ref();
        }

    public:
        Handle(Invoker<_R, _A>* invoker = 0)
            : m_invoker(invoker)
        {
            PeekInvoker();
        }

        Handle(const Handle& o)
        {
            *this = o;
        }

        Handle& operator = (const Handle& o)
        {
            ReleaseInvoker();
            m_invoker = o.m_invoker;
            PeekInvoker();
            return *this;
        }

        ~Handle()
        {
            ReleaseInvoker();
        }

        bool IsNull() const
        {
            return m_invoker == 0;
        }

        _R Invoke(_A a) const
        {
            return m_invoker->Invoke(a);
        }
    };

    template <typename _R, typename _A>
    static Handle<_R, _A> Create(_R (*fun)(_A))
    {
        class InvokerImpl : public Invoker<_R, _A>
        {
        private:
            _R (*m_fun)(_A);
        public:
            InvokerImpl(_R (*fun)(_A))
                : m_fun(fun)
            { }
            virtual _R Invoke(_A a) const
            {
                return (*m_fun)(a);
            }
        } *instance = new InvokerImpl(fun);
        return Handle<_R, _A>(instance);
    }

    template <typename _C, typename _O, typename _R, typename _A>
    static Handle<_R, _A> Create(_R (_C::*fun)(_A), _O* o)
    {
        class InvokerImpl : public Invoker<_R, _A>
        {
        private:
            _R (_C::*m_fun)(_A);
            _O* m_o;
        public:
            InvokerImpl(_R (_C::*fun)(_A), _O* o)
                : m_fun(fun), m_o(o) { }
            virtual _R Invoke(_A a) const
            {
                _C& c = *m_o;
                return (c.*m_fun)(a);
            }
        } *instance = new InvokerImpl(fun, o);
        return Handle<_R, _A>(instance);
    }

    template <typename _C, typename _O, typename _R, typename _A>
    static Handle<_R, _A> Create(_R (_C::*fun)(_A) const, const _O* o)
    {
        class InvokerImpl : public Invoker<_R, _A>
        {
        private:
            _R (_C::*m_fun)(_A) const;
            const _O* m_o;
        public:
            InvokerImpl(_R (_C::*fun)(_A) const, const _O* o)
                : m_fun(fun), m_o(o)
            { }
            virtual _R Invoke(_A a) const
            {
                const _C& c = *m_o;
                return (c.*m_fun)(a);
            }
        } *instance = new InvokerImpl(fun, o);
        return Handle<_R, _A>(instance);
    }

};//class Callback

#endif