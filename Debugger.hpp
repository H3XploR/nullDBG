// Debugger.hpp
#pragma once
#include <string>
#include <sys/types.h>

class Debugger {
public:
    Debugger(const std::string& prog_name);
    void run();

private:
    void run_target();
    void run_debugger();

    std::string program_name;
    pid_t child_pid;
};

