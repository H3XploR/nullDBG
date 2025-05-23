// Breakpoint.cpp
#include "Breakpoint.hpp"
#include <sys/ptrace.h>
#include <unistd.h>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <cerrno>
#include <cstdio>

Breakpoint::Breakpoint(pid_t pid, std::intptr_t addr)
    : m_pid(pid), m_addr(addr) {}

void Breakpoint::enable() {
    if (m_addr % 4 != 0) {
        throw std::runtime_error("[!] Breakpoint address is not 4-byte aligned (AArch64 requires alignment).");
    }

    errno = 0;
    long data = ptrace(PTRACE_PEEKDATA, m_pid, m_addr, nullptr);
    if (data == -1 && errno != 0) {
        perror("ptrace PEEKDATA failed");
        throw std::runtime_error("ptrace PEEKDATA failed");
    }

    std::memcpy(&m_saved_data, &data, sizeof(uint32_t));

    uint32_t brk_instr = 0xD4200000;
    long data_with_brk;
    std::memcpy(&data_with_brk, &brk_instr, sizeof(uint32_t));
    data_with_brk |= (data & ~0xFFFFFFFF);

    if (ptrace(PTRACE_POKEDATA, m_pid, m_addr, data_with_brk) == -1) {
        perror("ptrace POKEDATA failed");
        throw std::runtime_error("ptrace POKEDATA failed");
    }

    m_enabled = true;
}

void Breakpoint::disable() {
    errno = 0;
    long data = ptrace(PTRACE_PEEKDATA, m_pid, m_addr, nullptr);
    if (data == -1 && errno != 0) {
        perror("ptrace PEEKDATA failed");
        throw std::runtime_error("ptrace PEEKDATA failed");
    }

    long restored_data = (data & ~0xFFFFFFFF) | m_saved_data;
    if (ptrace(PTRACE_POKEDATA, m_pid, m_addr, restored_data) == -1) {
        perror("ptrace POKEDATA failed");
        throw std::runtime_error("ptrace POKEDATA failed");
    }

    m_enabled = false;
}

std::intptr_t Breakpoint::get_address() const {
    return m_addr;
}

bool Breakpoint::is_enabled() const {
    return m_enabled;
}

