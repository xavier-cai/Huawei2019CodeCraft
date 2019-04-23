#ifndef COPY_OBJECT_H
#define COPY_OBJECT_H

#include "assert.h"
#include <typeinfo>

#ifdef TYPE_ID_NAME
	#error TYPE_ID_NAME is alreay defined
#endif //#ifdef TYPE_ID_NAME
#define TYPE_ID_NAME(item) (std::string(typeid(item).name()))

/*
 * base type to perform COPY function
 */
class CopyableObject {
public:
	virtual ~CopyableObject() { }
	virtual CopyableObject* Copy() const = 0;

protected:
	virtual void DoCopyTo(CopyableObject* copy) const { };

	/*
	 * copy [src](type _SRC) to create a new variable with type _TARGET
	 * even it is is shallow copy, it's enough for classes that don't include any pointer
	 */
	template <class _T_TARGET, class _T_SRC>
	static _T_TARGET* ShallowCopy(const _T_SRC* src)
	{
		_T_TARGET* pointer = dynamic_cast<_T_TARGET*>(const_cast<_T_SRC*>(src));
        /*
		ASSERT_MSG(pointer != NULL, "can not convert " << TYPE_ID_NAME(_T_SRC)
			<< " to " << TYPE_ID_NAME(_T_TARGET)
			<< ", please check the first template type of CopyableTemplate when using");
            */
		_T_TARGET* copy = new _T_TARGET(*pointer);
		return copy;
	}

public:
	template <class _T>
	_T* CopyWithType() const
	{
		_T* instance = dynamic_cast<_T*>(Copy());
		ASSERT(instance != NULL);
		return instance;
	}
};//class CopyableObject

#undef TYPE_ID_NAME

/*
 * the template to define API of COPY function
 * _T_THIS means your new class' name
 * _T_BASE means your new class' parent class type (default to CopyableObject)
 */
template <class _T_THIS, class _T_BASE = CopyableObject>
class CopyableTemplate : public _T_BASE
{
protected:
	virtual void DoCopy(_T_THIS* copy) const { }
	virtual void DoCopyTo(CopyableObject* copy) const override
	{
		_T_BASE::DoCopyTo(copy);
		DoCopy(dynamic_cast<_T_THIS*>(copy));
	}

public:
	virtual ~CopyableTemplate() { }
	friend _T_THIS;
	
	virtual CopyableObject* Copy() const override
	{
		//_T_THIS* instance = new _THIS();
		_T_THIS* instance = CopyableObject::ShallowCopy<_T_THIS>(this);
		CopyableTemplate* copy = dynamic_cast<CopyableTemplate*>(instance);
		ASSERT(instance != NULL);
		DoCopyTo(copy);
		return copy;
	}

	_T_THIS* CopyObject() const
	{
		return CopyableObject::CopyWithType<_T_THIS>();
	}
};//class CopyableTemplate

#endif
