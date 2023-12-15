#ifndef BOUND_DATA_HH
#define BOUND_DATA_HH
#include <stdint.h>
#include <utility>

namespace softeq
{
namespace ble
{
namespace core
{
template <typename D>
struct BData
{
    D *data;
    size_t len;
    size_t maxLen;
};

using CBData = BData<char>;

template <typename T, typename D = char>
CBData bindData(T &data)
{
    return CBData{.data = reinterpret_cast<D *>(&data), .len = sizeof(T), .maxLen = sizeof(T)};
}
} // namespace core
} // namespace ble
} // namespace softeq

#endif // BOUND_DATA_HH