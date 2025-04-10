#pragma once

#include <mutex>

template<typename TypeParameter>
class Delegate
{
    struct DelegateDataBase
    {
        virtual ~DelegateDataBase() = default;
        DelegateDataBase* next = nullptr;
        long long Hash = 0;
        virtual void Call(TypeParameter) = 0;
    };
        
    template<class T>
    struct DelegateDataForClass : DelegateDataBase
    {
        T* object = nullptr;

        typedef void(T::*Method)(TypeParameter);
        Method method = nullptr;

        void Call(TypeParameter data) override
        {
            (object->*method)(data);
        }
    };

    struct DelegateDataForFunction : DelegateDataBase
    {
        typedef void(*Func)(TypeParameter);
        Func func = nullptr;

        void Call(TypeParameter data) override
        {
            func(data);
        }
    };
    
    DelegateDataBase* firstElement = nullptr;

    std::mutex mDelegateList;

    static constexpr unsigned int FNV1a(const char* str, unsigned int n, unsigned int hash = 2166136261U) {
        return n == 0 ? hash : FNV1a(str + 1, n - 1, (hash ^ str[0]) * 19777619U);
    }
public:
    template<typename T>
    void bind(T* object, void(T::*method)(TypeParameter))
    {
        char* str = new char[sizeof(T*) + sizeof(void(T::*)(TypeParameter))];
        const char* ptrToObjectRef = reinterpret_cast<const char*>(&object);
        for (long unsigned int i = 0; i < sizeof(T*); ++i)
            str[i] = ptrToObjectRef[i];
     
        const char* ptrToMethodRef = reinterpret_cast<const char*>(&method);
        for (long unsigned int i = sizeof(T*); i < sizeof(T*) + sizeof(void(T::*)(TypeParameter)); ++i)
            str[i] = ptrToMethodRef[i - sizeof(T*)];
        
        DelegateDataForClass<T>* data = new DelegateDataForClass<T>;
        data->next = firstElement;
        data->object = object;
        data->method = method;
        data->Hash = FNV1a(str, sizeof(T*) + sizeof(void(T::*)(TypeParameter)));

        delete[] str;
        
        mDelegateList.lock();
        firstElement = data;
        mDelegateList.unlock();
    }

    void bind(void(*func)(TypeParameter))
    {
        char* str = new char[sizeof(void(*)(TypeParameter))];
        const char* ptrToFuncRef = reinterpret_cast<const char*>(&func);
        for (int i = 0; i < sizeof(void(*)(TypeParameter)); ++i)
            str[i] = ptrToFuncRef[i];
    
        DelegateDataForFunction* data = new DelegateDataForFunction;
        data->next = firstElement;
        data->func = func;
        data->Hash = FNV1a(str, sizeof(func));
        delete[] str;

        mDelegateList.lock();
        firstElement = data;
        mDelegateList.unlock();
    }

    template<typename T>
    void unbind(T* object, void(T::*method)(TypeParameter))
    {
        if (firstElement == nullptr)
            return;

        char* str = new char[sizeof(T*) + sizeof(void(T::*)(TypeParameter))];
        const char* ptrToObjectRef = reinterpret_cast<const char*>(&object);
        for (long unsigned int i = 0; i < 8; ++i)
            str[i] = ptrToObjectRef[i];
     
        const char* ptrToMethodRef = reinterpret_cast<const char*>(&method);
        for (long unsigned int i = sizeof(T*); i < sizeof(T*) + sizeof(void(T::*)(TypeParameter)); ++i)
            str[i] = ptrToMethodRef[i - sizeof(T*)];

        unsigned int Hash = FNV1a(str, sizeof(T*) + sizeof(void(T::*)(TypeParameter)));
        
        DelegateDataBase* iterator = firstElement;
        DelegateDataBase* temp = nullptr;
        while (iterator != nullptr)
        {
            if (iterator->Hash == Hash)
            {
                if (temp != nullptr)
                    temp->next = iterator->next;
                else
                    firstElement = iterator->next;

                delete static_cast<DelegateDataForClass<T>*>(iterator);
                break;
            }

            temp = iterator;
            iterator = iterator->next;
        }
    }

    void unbind(void(*func)(TypeParameter))
    {
        char* str = new char[sizeof(void(*)(TypeParameter))];
        const char* ptrToFuncRef = reinterpret_cast<const char*>(&func);
        for (int i = 0; i < sizeof(void(*)(TypeParameter)); ++i)
            str[i] = ptrToFuncRef[i];
    
        unsigned int Hash = FNV1a(str, sizeof(void(*)(TypeParameter)));
        delete[] str;

        DelegateDataBase* iterator = firstElement;
        DelegateDataBase* temp = nullptr;
        while (iterator != nullptr)
        {
            if (iterator->Hash == Hash)
            {
                if (temp != nullptr)
                    temp->next = iterator->next;
                else
                    firstElement = iterator->next;

                delete static_cast<DelegateDataForFunction*>(iterator);
                break;
            }

            temp = iterator;
            iterator = iterator->next;
        }
    }

    void Call(TypeParameter data) const
    {
        DelegateDataBase* iterator = firstElement;
        while (iterator != nullptr)
        {
            iterator->Call(data);
            iterator = iterator->next;
        }
    }
};

template<>
class Delegate<void>
{
    struct DelegateDataBase
    {
        virtual ~DelegateDataBase() = default;
        DelegateDataBase* next = nullptr;
        long long Hash = 0;
        virtual void call() = 0;
    };
        
    template<class T>
    struct DelegateDataForClass : DelegateDataBase
    {
        T* object = nullptr;

        typedef void(T::*Method)();
        Method method = nullptr;

        void call() override
        {
            (object->*method)();
        }
    };

    struct DelegateDataForFunction : DelegateDataBase
    {
        typedef void(*Func)();
        Func func = nullptr;

        void call() override
        {
            func();
        }
    };
    
    DelegateDataBase* firstElement = nullptr;

    std::mutex mDelegateList;

    static constexpr unsigned int FNV1a(const char* str, unsigned int n, unsigned int hash = 2166136261U) {
        return n == 0 ? hash : FNV1a(str + 1, n - 1, (hash ^ str[0]) * 19777619U);
    }
public:
    template<typename T>
    void bind(T* object, void(T::*method)())
    {
        char* str = new char[sizeof(T*) + sizeof(void(T::*)())];
        const char* ptrToObjectRef = reinterpret_cast<const char*>(&object);
        for (int i = 0; i < sizeof(T*); ++i)
            str[i] = ptrToObjectRef[i];
     
        const char* ptrToMethodRef = reinterpret_cast<const char*>(&method);
        for (int i = sizeof(T*); i < sizeof(T*) + sizeof(void(T::*)()); ++i)
            str[i] = ptrToMethodRef[i - sizeof(T*)];
        
        DelegateDataForClass<T>* data = new DelegateDataForClass<T>;
        data->next = firstElement;
        data->object = object;
        data->method = method;
        data->Hash = FNV1a(str, sizeof(T*) + sizeof(void(T::*)()));

        delete[] str;
        
        mDelegateList.lock();
        firstElement = data;
        mDelegateList.unlock();
    }

    void bind(void(*func)())
    {
        char* str = new char[sizeof(func)];
        const char* ptrToFuncRef = reinterpret_cast<const char*>(&func);
        for (int i = 0; i < sizeof(void(*)()); ++i)
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

    template<typename T>
    void unbind(T* object, void(T::*method)())
    {
        if (firstElement == nullptr)
            return;

        char* str = new char[sizeof(T*) + sizeof(void(T::*)())];
        const char* ptrToObjectRef = reinterpret_cast<const char*>(&object);
        for (int i = 0; i < sizeof(T*); ++i)
            str[i] = ptrToObjectRef[i];
     
        const char* ptrToMethodRef = reinterpret_cast<const char*>(&method);
        for (int i = sizeof(T*); i < sizeof(T*) + sizeof(void(T::*)()); ++i)
            str[i] = ptrToMethodRef[i - sizeof(T*)];

        unsigned int Hash = FNV1a(str, sizeof(T*) + sizeof(void(T::*)()));
        
        DelegateDataBase* iterator = firstElement;
        DelegateDataBase* temp = nullptr;
        while (iterator != nullptr)
        {
            if (iterator->Hash == Hash)
            {
                if (temp != nullptr)
                    temp->next = iterator->next;
                else
                    firstElement = iterator->next;

                delete static_cast<DelegateDataForClass<T>*>(iterator);
                break;
            }

            temp = iterator;
            iterator = iterator->next;
        }
    }

    void unbind(void(*func)())
    {
        char* str = new char[sizeof(void(*)())];
        const char* ptrToFuncRef = reinterpret_cast<const char*>(&func);
        for (int i = 0; i < sizeof(void(*)()); ++i)
            str[i] = ptrToFuncRef[i];
    
        unsigned int Hash = FNV1a(str, sizeof(void(*)()));
        delete[] str;

        DelegateDataBase* iterator = firstElement;
        DelegateDataBase* temp = nullptr;
        while (iterator != nullptr)
        {
            if (iterator->Hash == Hash)
            {
                if (temp != nullptr)
                    temp->next = iterator->next;
                else
                    firstElement = iterator->next;

                delete static_cast<DelegateDataForFunction*>(iterator);
                break;
            }

            temp = iterator;
            iterator = iterator->next;
        }
    }

    void Call() const
    {
        DelegateDataBase* iterator = firstElement;
        while (iterator != nullptr)
        {
            iterator->call();
            iterator = iterator->next;
        }
    }
};