#include "Debugger.hpp"
#include <iomanip>
#include <iostream>
#include <linux/elf.h>
#include <signal.h>
#include <sys/ptrace.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <bitset>
#include <stdint.h>
#include <stdio.h>

Debugger::Debugger(const std::string &prog_name) : program_name(prog_name) {
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
  ptrace(static_cast<__ptrace_request>(PTRACE_TRACEME), 0, nullptr, nullptr);
  execl(program_name.c_str(), program_name.c_str(), nullptr);
}

void Debugger::run_debugger() {
  int status;
  waitpid(child_pid, &status, 0);
  std::cout << "[+] Debugger attached to PID: " << child_pid << "\n";
  ptrace(PTRACE_SINGLESTEP, child_pid, nullptr, nullptr);
  waitpid(child_pid, &status, 0);
  go_until_x0_filled();
  print_registers(child_pid);
  get_opcode();
  single_step();
  print_registers(child_pid);
  get_opcode();
  std::cout << "[+] Child process exited.\n";
}

void Debugger::single_step(void){
	int	status;

	std::cout << "[+] Single stepping...\n";
	ptrace(PTRACE_SINGLESTEP, child_pid, nullptr, nullptr);
	waitpid(child_pid, &status, 0);
}

void Debugger::go_until_x0_filled(void)
{
	while (1)
	{
		if (get_x0() != 0)
			break ;
		single_step();
	}
}

uint32_t Debugger::get_opcode(void){
  struct user_pt_regs {
    uint64_t regs[31];  // x0-x30
    uint64_t sp;        // Stack Pointer
    uint64_t pc;        // Program Counter
    uint64_t pstate;    // Processor State
  } regs;
  struct iovec io;
  io.iov_base = &regs;
  io.iov_len = sizeof(regs);
  if (ptrace(PTRACE_GETREGSET, child_pid, (void*)NT_PRSTATUS, &io) == -1) {
    perror("ptrace(PTRACE_GETREGSET)");
    return 0;
  }
  std::cout << "[+] Current PC(HEX): " << std::hex << regs.pc << std::dec << "\n";
  std::cout << "[+] Current PC(BIN): ";
  std::cout << "[+] regs.pc (big-endian, 8-bit chunks): ";
    for (int i = 56; i >= 0; i -= 8) {
        uint8_t byte = (regs.pc >> i) & 0xFF;
        for (int j = 7; j >= 0; --j) {
            std::cout << ((byte >> j) & 1);
        }
        std::cout << " ";
    }
    std::cout << std::endl;
  return ptrace(PTRACE_PEEKTEXT, child_pid, regs.pc, nullptr);
}

uint64_t Debugger::get_x0(void) {
  struct user_pt_regs {
    uint64_t regs[31];  // x0-x30
    uint64_t sp;        // Stack Pointer
    uint64_t pc;        // Program Counter
    uint64_t pstate;    // Processor State
  } regs;
  struct iovec io;
  io.iov_base = &regs;
  io.iov_len = sizeof(regs);
  if (ptrace(PTRACE_GETREGSET, child_pid, (void*)NT_PRSTATUS, &io) == -1) {
    perror("ptrace(PTRACE_GETREGSET)");
    return 0;
  }
  return regs.regs[0];  // x0 is the first register
}

uint64_t Debugger::get_pc(void) {
  struct user_pt_regs {
    uint64_t regs[31];  // x0-x30
    uint64_t sp;        // Stack Pointer
    uint64_t pc;        // Program Counter
    uint64_t pstate;    // Processor State
  } regs;
  struct iovec io;
  io.iov_base = &regs;
  io.iov_len = sizeof(regs);
  if (ptrace(PTRACE_GETREGSET, child_pid, (void*)NT_PRSTATUS, &io) == -1) {
    perror("ptrace(PTRACE_GETREGSET)");
    return 0;
  }
  return regs.pc;  // Return the program counter
}

void Debugger::breakpoint_list() {
  std::cout << "Breakpoints:\n";
  for (const auto &bp : breakpoints) {
    std::cout << bp << std::endl;
  }
}

void Debugger::set_breakpoint(pid_t pid, std::intptr_t addr) {
  Breakpoint bp(pid, addr);
  bp.enable();
  breakpoints.push_back(bp);
  std::cout << "[+] Breakpoint set at address: " << std::hex << addr << std::dec
            << "\n";
}

void Debugger::remove_breakpoint(pid_t pid, std::intptr_t addr) {
  auto it = std::remove_if(breakpoints.begin(), breakpoints.end(),
                           [pid, addr](const Breakpoint &bp) {
                             return bp.get_address() == addr && bp.is_enabled();
                           });
  if (it != breakpoints.end()) {
    it->disable();
    breakpoints.erase(it, breakpoints.end());
    std::cout << "[+] Breakpoint removed at address: " << std::hex << addr
              << std::dec << "\n";
  } else {
    std::cout << "[-] No breakpoint found at address: " << std::hex << addr
              << std::dec << "\n";
  }
}

  
// Attention: ce code est spécifique à l'architecture AArch64
void Debugger::print_registers(pid_t pid) {
    struct user_pt_regs {
        uint64_t regs[31];  // x0-x30
        uint64_t sp;        // Stack Pointer
        uint64_t pc;        // Program Counter
        uint64_t pstate;    // Processor State
    } regs;

    struct iovec io;
    io.iov_base = &regs;
    io.iov_len = sizeof(regs);

    if (ptrace(PTRACE_GETREGSET, pid, (void*)NT_PRSTATUS, &io) == -1) {
        perror("ptrace(PTRACE_GETREGSET)");
        return;
    }

    std::ios old_fmt(nullptr);
    old_fmt.copyfmt(std::cout);

    std::cout << std::hex << std::setfill('0');

    std::cout << "==== Register Dump (AArch64) ====" << std::endl;

    for (int i = 0; i < 31; i++) {
		std::cout << "x" << std::dec << std::setw(2) << std::right << i
			<< " = 0x" << std::hex << std::setw(16) << regs.regs[i] << "\n";
    }

    std::cout << " sp = 0x" << std::setw(16) << regs.sp << std::endl;
    std::cout << " pc = 0x" << std::setw(16) << regs.pc << std::endl;
    std::cout << " pstate = 0x" << std::setw(16) << regs.pstate << std::endl;

    std::cout.copyfmt(old_fmt);
}

