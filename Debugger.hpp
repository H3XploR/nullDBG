// Debugger.hpp
#pragma once
#include <string>
#include <sys/types.h>
#include <vector>
#include <algorithm>
#include "Breakpoint.hpp"

class Debugger {
public:
    Debugger(const std::string& prog_name);
    ~Debugger() {std::cout << "Debugger destroyed." << std::endl;}
    void breakpoint_list();
    void set_breakpoint(pid_t pid, std::intptr_t addr);
    void remove_breakpoint(pid_t pid, std::intptr_t addr);
    void run();
    void DEBUGING(void);

private:
    void run_target();
    void run_debugger();
    
    std::string program_name;
    pid_t child_pid;
    std::vector<Breakpoint> breakpoints;
};

