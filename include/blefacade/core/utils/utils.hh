#ifndef UTILS_HH
#define UTILS_HH
#include <functional>

namespace softeq
{
namespace ble
{
namespace core
{
template <typename T>
struct Callback;

template <typename Ret, typename... Params>
struct Callback<Ret(Params...)>
{
    template <typename... Args>
    static Ret callback(Args... args)
    {
        return func(args...);
    }
    static std::function<Ret(Params...)> func;
};

template <typename Ret, typename... Params>
std::function<Ret(Params...)> Callback<Ret(Params...)>::func;
} // namespace core
} // namespace ble
} // namespace softeq
#endif //UTILS_HH