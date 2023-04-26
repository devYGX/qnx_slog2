#include "qnx_slog2.hpp"
#include <thread>
void thread_fun1() {
    SLOG_E("%s:%d", "Thread1 Hello World!", 123);
    SLOG_W("%s:%d", "Thread1 Hello World!", 123);
    SLOG_D("%s:%d", "Thread1 Hello World!", 123);
    SLOG_I("%s:%d", "Thread1 Hello World!", 123);    
}
void thread_fun2() {
    SLOG_E("%s:%d", "Thread2 Hello World!", 123);
    SLOG_W("%s:%d", "Thread2 Hello World!", 123);
    SLOG_D("%s:%d", "Thread2 Hello World!", 123);
    SLOG_I("%s:%d", "Thread2 Hello World!", 123);    
}
int main() {
    if (false == SLOG_INIT()) {
        printf("slog init failed!\n");
        return -1;
    }
    SLOG_E("%s:%d", "Hello World!", 123);
    SLOG_W("%s:%d", "Hello World!", 123);
    SLOG_D("%s:%d", "Hello World!", 123);
    SLOG_I("%s:%d", "Hello World!", 123);

    std::thread thread1(thread_fun1);
    std::thread thread2(thread_fun2);
    thread1.join();
    thread2.join();
    
    return 0;
}