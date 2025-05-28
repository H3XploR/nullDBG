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
    void print_registers(pid_t pid);
    void DEBUGING(void);
    void go_until_x0_filled(void);
    void single_step(void);
    uint64_t get_x0(void); 
    uint64_t get_pc(void);
    uint32_t get_opcode(void);

private:
    void run_target();
    void run_debugger();
    
    std::string program_name;
    pid_t child_pid;
    std::vector<Breakpoint> breakpoints;
};

