/*
** EPITECH PROJECT, 2026
** G-ING-500-COT-5-1-rtype-2
** File description:
** CompileFail
*/

#include "engine/reflection/TypeRegistry.hpp"

#include <complex>

namespace probe {

struct NoMacro {
    int value = 0;
};

struct Ping {
    int value = 0;
};

struct Odd {
    std::complex<float> value;
};

}

ENGINE_MESSAGE(probe::Ping)
ENGINE_COMPONENT(probe::Odd)

int main() {
    engine::TypeRegistry types;
#if ENGINE_COMPILE_FAIL_CASE == 1
    types.add<probe::NoMacro>();
#elif ENGINE_COMPILE_FAIL_CASE == 2
    types.add<probe::Ping>().flags(engine::Replicated);
#elif ENGINE_COMPILE_FAIL_CASE == 3
    types.add<probe::Odd>().field("value", &probe::Odd::value);
#endif
    return 0;
}
