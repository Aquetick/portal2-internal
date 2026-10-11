#pragma once
#include "util/mem/mem.hpp"

class c_engine_client {
public:
    int get_local_player() {
        return util::mem::virtual_function<int(__thiscall*)(void*)>(this, 12)(this);
    }

    // VEngineClient::ServerCmd. Fully guarded: SEH catches faults, and ESP is
    // restored after the call so any arg-count mismatch can't corrupt the stack.
    void server_cmd(const char* cmd) {
        void* fn = util::mem::virtual_function<void*>(this, 6);
        if (!fn || !cmd)
            return;
        __try {
            DWORD esp0 = 0;
            void* self = this;
            const char* arg = cmd;
            __asm {
                mov esp0, esp
                push 1
                push arg
                mov ecx, self
                call DWORD PTR [fn]
                mov esp, esp0
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {
        }
    }
};
