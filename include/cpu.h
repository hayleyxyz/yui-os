#pragma once

static inline void halt() {
    for(;;) asm volatile("hlt");
}
