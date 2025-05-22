// Debugger.cpp
#include "Debugger.hpp"
#include <iostream>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

Debugger::Debugger(const std::string& prog_name) : program_name(prog_name) {}

void Debugger::run() {
    child_pid = fork();
    if (child_pid == 0) {
        run_target();
    } else if (child_pid > 0) {
        run_debugger();
    } else {
        std::cerr << "[!] fork() failed.\n";
    }
}

void Debugger::run_target() {
    std::cout << "[+] Child process started.\n";
    ptrace(PTRACE_TRACEME, 0, nullptr, nullptr);
    execl(program_name.c_str(), program_name.c_str(), nullptr);
}

void Debugger::run_debugger() {
    int status;
    waitpid(child_pid, &status, 0);
    std::cout << "[+] Debugger attached to PID: " << child_pid << "\n";

    ptrace(PTRACE_CONT, child_pid, nullptr, nullptr);
    waitpid(child_pid, &status, 0);

    std::cout << "[+] Child process exited.\n";
}

