// Breakpoint.hpp
#pragma once
#include <sys/types.h>
#include <cstdint>

class Breakpoint {
public:
    Breakpoint(pid_t pid, std::intptr_t addr);

    void enable();
    void disable();

    std::intptr_t get_address() const;

    bool is_enabled() const;

private:
    pid_t m_pid;
    std::intptr_t m_addr;
    bool m_enabled = false;
    uint8_t m_saved_data;
};

