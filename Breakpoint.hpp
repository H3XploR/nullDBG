// Breakpoint.hpp
#pragma once
#include <sys/types.h>
#include <cstdint>
#include <vector>
#include <iostream>

class Breakpoint {
private:
    pid_t m_pid;
    std::intptr_t m_addr;
    bool m_enabled = false;
    uint8_t m_saved_data;
public:

    Breakpoint(pid_t pid, std::intptr_t addr);
    void enable();
    void disable();

    std::intptr_t get_address() const;

    bool is_enabled() const;

    friend std::ostream& operator<<(std::ostream& os, const Breakpoint& bp);

    
};


