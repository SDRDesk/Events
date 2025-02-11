/*/
#include "../include/StackFunction.h"
#include <iostream>

void freeFunction(int x) {
    std::cout << "Free function called with " << x <<
std::endl;
}

class MyClass {
public:
    void memberFunction(int x) {
        std::cout << "Member function called with " << x <<
std::endl;
    }

    void constMemberFunction(int x) const {
        std::cout << "Const member function called with " <<
x << std::endl;
    }

    bool operator==(const MyClass&) const {
        return true; // Simplified for demonstration
purposes
    }
};

int main() {
    StackFunction<void(int)> func1 = freeFunction;
    func1(42);

    StackFunction<void(int)> func2 = [](int x) {
        std::cout << "Lambda called with " << x <<
std::endl;
    };
    func2(43);

    MyClass obj;
    StackFunction<void(int)> func3 = [&obj](int x) {
        obj.memberFunction(x);
    };
    func3(44);

    StackFunction<void(int)> func4 = [&obj](int x) {
        obj.constMemberFunction(x);
    };
    func4(45);

    StackFunction<void(int)> func5;
    if (!func5) {
        std::cout << "func5 is empty" << std::endl;
    }

    func5 = func1;
    func5(46);

    swap(func1, func2);
    func1(47);
    func2(48);

    if (func1 == func2) {
        std::cout << "func1 and func2 are equal" <<
std::endl; } else { std::cout << "func1 and func2 are not
equal" << std::endl;
    }

    return 0;
}
/*/