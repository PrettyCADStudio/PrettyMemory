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

        using ValueType = VT;
        using DeleterType = DT;
        using Pointer = ValueType*;
        using ConstPointer = const ValueType*;
        using Reference = ValueType&;
        using ConstReference = const ValueType&;

        OwnerPtr() = default;

        OwnerPtr(std::nullptr_t)
        {
        }

        template<typename VT2, typename DT2 = detail::DefaultDeleter<VT2>, std::enable_if_t<std::is_convertible_v<VT2*, VT*>, int> = 0>
        OwnerPtr(typename OwnerPtr<VT2, DT2>::Pointer pOther)
        {
            if (pOther)
            {
                m_pControlBlock = new detail::ControlBlock;
                m_pControlBlock->Data = pOther;
                m_pControlBlock->Deleter = detail::DefaultDeleter<VT2>{};
            }
        }

        ~OwnerPtr()
        {
            if (nullptr != m_pControlBlock)
            {
                if (nullptr != m_pControlBlock->Data)
                {
                    m_pControlBlock->Deleter(m_pControlBlock->Data);
                    m_pControlBlock->Data = nullptr;
                }
                if (0 == m_pControlBlock->ShadowCount)
                {
                    delete m_pControlBlock;
                    m_pControlBlock = nullptr;
                }
            }
        }

    private:

        detail::ControlBlock* m_pControlBlock{ nullptr };
    };
}
