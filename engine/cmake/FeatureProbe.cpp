// One probe per C++23 feature on the allow-list (ADR 0017). EngineFeatures.cmake compiles this file
// once per feature with -DPROBE_<NAME>; it is never linked into the engine. ENG-03

#if defined(PROBE_EXPECTED)
#include <expected>
std::expected<int, int> probeExpected() { return std::unexpected(1); }
#endif

#if defined(PROBE_MOVE_ONLY_FUNCTION)
#include <functional>
#include <memory>
std::move_only_function<int()> probeMoveOnlyFunction() {
    return [p = std::make_unique<int>(1)] { return *p; };
}
#endif

#if defined(PROBE_PRINT)
#include <print>
void probePrint() { std::print("{}", 1); }
#endif

#if defined(PROBE_DEDUCING_THIS)
struct ProbeDeducingThis {
    int value = 0;
    template <class Self>
    auto& get(this Self& self) {
        return self.value;
    }
};
int probeDeducingThis() {
    ProbeDeducingThis p;
    return p.get();
}
#endif

#if defined(PROBE_TO_UNDERLYING)
#include <utility>
enum class ProbeEnum : unsigned char { A = 1 };
int probeToUnderlying() { return std::to_underlying(ProbeEnum::A); }
#endif

#if defined(PROBE_UNREACHABLE)
#include <utility>
int probeUnreachable(int x) {
    if (x == 0)
        return 1;
    std::unreachable();
}
#endif

int main() { return 0; }
