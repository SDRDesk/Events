#ifdef NDEBUG
#undef NDEBUG
#endif
#include "../include/myEventing.hpp"

using namespace Eventing;

struct Event1 {};
struct Event2 {};
struct Event3 {};
struct Event4 {};

class MyClass {
    public:
    static inline int static_counter = 0;

    int eventHandler1(const char* msg) const {
        printf("MyClass Event 1: %s\n", msg);
        ++static_counter;
        printf("MyClass::static_counter after Event1: %d\n",
            static_counter);
        ++event1Counter;
        return event1Counter;
    }

    int eventHandler2(const char* msg) const {
        printf("MyClass Event 2: %s\n", msg);
        ++static_counter;
        printf("MyClass::static_counter after Event2: %d\n",
            static_counter);
        ++event2Counter;
        return event2Counter;
    }

    int eventHandler3(const char* msg) {
        printf("MyClass Event 3: %s\n", msg);
        ++static_counter;
        printf("MyClass::static_counter after Event3: %d\n",
            static_counter);
        ++event3Counter;
        return event3Counter;
    }
};

void freeFunction(const char* msg) {
    printf("Free Function: %s\n", msg);
    ++event3Counter;
}

using SingleThreadedDispatcher
    = EventDispatcher<SingleThreaded, const char*>;
using MultiThreadedDispatcher
    = EventDispatcher<MultiThreaded, int, std::string>;

SingleThreadedDispatcher singleThreadDispatcher;

void signalHandler(int) {
    printf("\n\nSignal received: CTRL+C - Stopping event "
           "dispatcher\n");
    singleThreadDispatcher.stop();
}

class AnotherClass {
    public:
    static inline int static_counter = 0;

    int eventHandler1(
        int num, const std::string& msg) const {
        printf("AnotherClass Event 1: %d, %s\n", num,
            msg.c_str());
        ++static_counter;
        printf("AnotherClass::static_counter after Event1: "
               "%d\n",
            static_counter);
        ++event1Counter;
        return static_counter;
    }

    int eventHandler2(int num, const std::string& msg) {
        printf("AnotherClass Event 2: %d, %s\n", num,
            msg.c_str());
        ++static_counter;
        printf("AnotherClass::static_counter after Event2: "
               "%d\n",
            static_counter);
        ++event2Counter;
        return static_counter;
    }
};

MultiThreadedDispatcher multiThreadDispatcher;

int testEventing() {
    std::signal(SIGINT, signalHandler);

    MyClass myClassInstance;

    singleThreadDispatcher.registerEvent<MyClass, Event1>(
        &myClassInstance, &MyClass::eventHandler1);
    singleThreadDispatcher.registerEvent<MyClass, Event1>(
        &myClassInstance, &MyClass::eventHandler1);
    puts("NOTE: Registered MyClass::eventHandler1 twice");
    singleThreadDispatcher.registerEvent<MyClass, Event1>(
        &myClassInstance, &MyClass::eventHandler2);
    singleThreadDispatcher.registerEvent<MyClass, Event2>(
        &myClassInstance, &MyClass::eventHandler3);

    singleThreadDispatcher.registerEvent<Event3>(
        freeFunction);

    singleThreadDispatcher.registerEvent<Event4>(
        [](const char* msg) {
            printf("Lambda Function: %s\n", msg);
            ++event4Counter;
        });

    singleThreadDispatcher.postEvent<Event1>(
        "Testing event 1");
    singleThreadDispatcher.postEvent<Event2>(
        "Testing event 2");
    singleThreadDispatcher.postEvent<Event3>(
        "Testing event 3");
    singleThreadDispatcher.postEvent<Event4>(
        "Testing event 4");

    unsigned long counter = 0;
    bool checked_mem_fns = false;

    singleThreadDispatcher.dispatchLoop([&]() {
        std::this_thread::sleep_for(
            std::chrono::milliseconds(100));
        if (counter % 100 == 0) {
            singleThreadDispatcher.postEvent<Event1>(
                "Every 100mS: tick!");
        }
        if (counter == 0 && !checked_mem_fns) {
            assert(MyClass::static_counter == 4);
            MyClass::static_counter
                = 3; // to keep the correct count for all
                     // the other assertions.
            checked_mem_fns = true;
        }
        counter += 100;
        if (counter > 500) {
            singleThreadDispatcher.stop();
        }
    });

    AnotherClass anotherClassInstance;

    multiThreadDispatcher
        .registerEvent<AnotherClass, Event1>(
            &anotherClassInstance,
            &AnotherClass::eventHandler1);
    multiThreadDispatcher
        .registerEvent<AnotherClass, Event2>(
            &anotherClassInstance,
            &AnotherClass::eventHandler2);

    multiThreadDispatcher.registerEvent<Event3>(
        [](int num, const std::string& msg) {
            printf("Lambda Function: %d, %s\n", num,
                msg.c_str());
            ++event3Counter;
        });

    multiThreadDispatcher.postEvent<Event1>(
        42, "Testing event 1");
    multiThreadDispatcher.postEvent<Event2>(
        43, "Testing event 2");
    multiThreadDispatcher.postEvent<Event3>(
        44, "Testing event 3");

    std::thread dispatcherThread([&]() {
        multiThreadDispatcher.dispatchLoop([]() {
            std::this_thread::sleep_for(
                std::chrono::milliseconds(1));
            if (multiThreadDispatcher.empty()) {
                multiThreadDispatcher.stop();
            }
        });
    });

    dispatcherThread.join();

    assert(event1Counter == 13);
    assert(event2Counter == 7);
    assert(event3Counter == 3);
    assert(event4Counter == 1);

    const auto val = MyClass::static_counter;
    assert(val == 18);
    int* pVal = &anotherClassInstance.static_counter;

    assert(anotherClassInstance.eventHandler1(
               42, "Testing event 1")
        == *pVal);
    assert(*pVal == 3);

    assert(anotherClassInstance.eventHandler2(
               43, "Testing event 2")
        == *pVal);
    assert(*pVal == 4);

    printf("All tests passed!\n");

    return 0;
}

int main() {
    return testEventing();
}