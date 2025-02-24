#pragma once

#include <mutex>

class Delegate
{
    struct DelegateDataBase
    {
        DelegateDataBase* next = nullptr;
        long long Hash = 0;
        virtual void Call(void*) = 0;
    };
        
    template<class T>
    struct DelegateDataForClass : DelegateDataBase
    {
        T* object = nullptr;

        typedef void(T::*Method)(void*);
        Method method = nullptr;

        void Call(void* data) override
        {
            (object->*method)(data);
        }
    };

    struct DelegateDataForFunction : DelegateDataBase
    {
        typedef void(*Func)(void*);
        Func func = nullptr;

        void Call(void* data) override;
    };
    
    DelegateDataBase* firstElement = nullptr;

    std::mutex mDelegateList;

    static constexpr unsigned int FNV1a(const char* str, unsigned int n, unsigned int hash = 2166136261U) {
        return n == 0 ? hash : FNV1a(str + 1, n - 1, (hash ^ str[0]) * 19777619U);
    }
public:
    template<typename T>
    void bind(T* object, void(T::*method)(void*))
    {
        char* str = new char[16];
        const char* ptrToObjectRef = reinterpret_cast<const char*>(&object);
        for (int i = 0; i < 8; ++i)
            str[i] = ptrToObjectRef[i];
     
        const char* ptrToMethodRef = reinterpret_cast<const char*>(&method);
        for (int i = 8; i < 16; ++i)
            str[i] = ptrToMethodRef[i - 8];
        
        DelegateDataForClass<T>* data = new DelegateDataForClass<T>;
        data->next = firstElement;
        data->object = object;
        data->method = method;
        data->Hash = FNV1a(str, 16);

        delete[] str;
        
        mDelegateList.lock();
        firstElement = data;
        mDelegateList.unlock();
    }

    void bind(void(*func)(void*));

    template<typename T>
    void unbind(T* object, void(T::*method)(void*))
    {
        if (firstElement == nullptr)
            return;

        char* str = new char[16];
        const char* ptrToObjectRef = static_cast<const char*>(&object);
        for (int i = 0; i < 8; ++i)
            str[i] = ptrToObjectRef[i];
     
        const char* ptrToMethodRef = static_cast<const char*>(&method);
        for (int i = 8; i < 16; ++i)
            str[i] = ptrToMethodRef[i - 8];

        unsigned int Hash = FNV1a(str, 16);
        
        DelegateDataBase* iterator = firstElement;
        DelegateDataBase* temp = nullptr;
        while (iterator->next != nullptr)
        {
            if (iterator->Hash == Hash)
            {
                mDelegateList.lock();
                
                if (temp != nullptr)
                    temp->next = iterator->next;
                else if (iterator->next == nullptr)
                    firstElement = nullptr;
                
                DelegateDataForClass<T>* data = static_cast<DelegateDataForClass<T>*>(*iterator);
                delete data;
                
                mDelegateList.unlock();
                return;
            }

            temp = iterator;
        }
    }

    void Call(void* data) const;
};
