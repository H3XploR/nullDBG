// Debugger.cpp
#include "Debugger.hpp"
#include <iostream>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

Debugger::Debugger(const std::string& prog_name) : program_name(prog_name) {
	std::cout << "[+] Debugger initialized for program: " << program_name << "\n";
}

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

    DEBUGING();

    ptrace(PTRACE_CONT, child_pid, nullptr, nullptr);
    waitpid(child_pid, &status, 0);

    std::cout << "[+] Child process exited.\n";
}

void Debugger::breakpoint_list() {
    std::cout << "Breakpoints:\n";
    for (const auto& bp : breakpoints) {
	std::cout << bp << std::endl;
    }
}

void Debugger::set_breakpoint(pid_t pid, std::intptr_t addr) {
    Breakpoint bp(pid, addr);
    bp.enable();
    breakpoints.push_back(bp);
    std::cout << "[+] Breakpoint set at address: " << std::hex << addr << std::dec << "\n";
}

void Debugger::remove_breakpoint(pid_t pid, std::intptr_t addr) {
    auto it = std::remove_if(breakpoints.begin(), breakpoints.end(),
			     [pid, addr](const Breakpoint& bp) {
				 return bp.get_address() == addr && bp.is_enabled();
			     });
    if (it != breakpoints.end()) {
	it->disable();
	breakpoints.erase(it, breakpoints.end());
	std::cout << "[+] Breakpoint removed at address: " << std::hex << addr << std::dec << "\n";
    } else {
	std::cout << "[-] No breakpoint found at address: " << std::hex << addr << std::dec << "\n";
    }
}

void Debugger::DEBUGING() {
    std::cout << "[+] Entering debugging loop. Type 'exit' to quit.\n";
    std::string command;
    while (true) {
	std::cout << "debugger> ";
	std::getline(std::cin, command);
	if (command == "exit") {
	    break;
	} else if (command == "list") {
	    breakpoint_list();
	} else if (command.substr(0, 3) == "set") {
	    std::intptr_t addr = std::stol(command.substr(4), nullptr, 16);
	    set_breakpoint(child_pid, addr);
	} else if (command.substr(0, 6) == "remove") {
	    std::intptr_t addr = std::stol(command.substr(7), nullptr, 16);
	    remove_breakpoint(child_pid, addr);
	} else {
	    std::cout << "Unknown command: " << command << "\n";
	}
    }
}
