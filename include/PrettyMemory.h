#pragma once

#include <cstddef>
#include <functional>

namespace prtm
{
    namespace detail
    {
        struct ControlBlock
        {
            void* Data{ nullptr };
            std::function<void(void*)> Deleter{ nullptr };
            std::size_t ShadowCount{ 0 };
        };

        template <typename T>
        struct DefaultDeleter
        {
            using Pointer = T*;

            void operator()(void* ptr) const
            {
                Pointer typed = static_cast<Pointer>(ptr);
                delete typed;
            }
        };
    }

    template <typename VT, typename DT = detail::DefaultDeleter<VT>>
    class OwnerPtr
    {
    public:

        OwnerPtr() = default;

        OwnerPtr(std::nullptr_t)
        {
        }



    private:

        detail::ControlBlock* m_pControlBlock{ nullptr };
    };
}
