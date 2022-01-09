#ifndef python_h__
#define python_h__

#include <exception>
#include <memory>

#include <Python.h>

namespace renpy::python
{
    class NullReferenceError : public std::exception {};

    class Context
    {
    public:
        Context();
        ~Context();
    };

    class WeakReference
    {
    public:
        WeakReference(PyObject* ptr);
        virtual ~WeakReference();
        virtual PyObject* Raw();
    protected:
        PyObject* raw_;
    };

    class StrongReference : public WeakReference
    {
    public:
        StrongReference(PyObject* ptr);
        virtual ~StrongReference();
    };

    std::unique_ptr<WeakReference> MakeWeakRef(PyObject* ptr);
    std::unique_ptr<StrongReference> MakeStrongRef(PyObject* ptr);
    PyObject* RawPtr(std::unique_ptr<WeakReference>& ref);
    PyObject* RawPtr(std::unique_ptr<StrongReference>& ref);
}

#endif