#include "Delegate.h"

void Delegate::DelegateDataForFunction::Call(void* data)
{
    func(data);
}

void Delegate::bind(void(*func)(void*))
{
    char* str = new char[8];
    const char* ptrToFuncRef = reinterpret_cast<const char*>(&func);
    for (int i = 0; i < 8; ++i)
        str[i] = ptrToFuncRef[i];
    
    DelegateDataForFunction* data = new DelegateDataForFunction;
    data->next = firstElement;
    data->func = func;
    data->Hash = FNV1a(str, 8);
    delete[] str;

    mDelegateList.lock();
    firstElement = data;
    mDelegateList.unlock();
}

void Delegate::Call(void* data) const
{
    DelegateDataBase* iterator = firstElement;
    while (iterator != nullptr)
    {
        iterator->Call(data);
        iterator = iterator->next;
    }
}
