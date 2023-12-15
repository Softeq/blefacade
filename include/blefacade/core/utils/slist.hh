#ifndef SLIST_HH
#define SLIST_HH
#include <cassert>
#include <list>

namespace softeq
{
namespace ble
{
namespace core
{
template <typename T>
class Slist : public std::list<T>
{
public:
    Slist<T>() = default;

    const T &get(const size_t index) const
    {
        assert(index < this->size()); //out-of-bound error
        auto it = this->cbegin();
        std::advance(it, index);
        return *it;
    }

    T &get(const size_t index)
    {
        return const_cast<T &>(const_cast<const Slist<T> *>(this)->get(index));
    }
};
} // namespace core
} // namespace ble
} // namespace softeq

#endif //SLIST_HH