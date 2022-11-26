#pragma once

double get_cpu_temp() {
    extern float tempmonGetTemp(void);
    return tempmonGetTemp();
}