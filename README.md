# Delegate
That library implement delegate for C++.

--------Exemple-------

#include <iostream>

#include "Delegate.h"

class A
{
public:
    void Foo1(int var) { std::cout << "A::Foo1(" << var <<");\n"; }
    void Foo2(int var) { std::cout << "A::Foo2(" << var <<");\n"; }
};

int main(int argc, char* argv[])
{
    Delegate<int> delegate;
    A a;
    delegate.bind(&a, &A::Foo1);
    delegate.bind(&a, &A::Foo2);
    delegate.unbind(&a, &A::Foo1);
    delegate.Call(321);
    return 0;
}
