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

    template<typename VT, typename DT>
    class OwnerPtr;

    template<typename T>
    class ShadowPtr
    {
    public:

        using ValueType = T;
        using Pointer = ValueType*;
        using ConstPointer = const ValueType*;
        using Reference = ValueType&;
        using ConstReference = const ValueType&;

    public:

        ShadowPtr() = default;

        ShadowPtr(std::nullptr_t)
        {
        }

        ShadowPtr(const ShadowPtr& other)
        {
            if (other.m_pControlBlock)
            {
                m_pControlBlock = other.m_pControlBlock;
                ++m_pControlBlock->ShadowCount;
            }
        }

        Pointer Get()
        {
            return m_pControlBlock ? static_cast<Pointer>(m_pControlBlock->Data) : nullptr;
        }

        ConstPointer Get() const
        {
            return m_pControlBlock ? static_cast<ConstPointer>(m_pControlBlock->Data) : nullptr;
        }

        Pointer Data() { return Get(); }

        ConstPointer Data() const { return Get(); }

        operator bool() const { return nullptr != Get(); }

        bool Expired() const { return nullptr == Get(); }

        Pointer operator->() { return Get(); }

        ConstPointer operator->() const { return Get(); }

        Reference operator*() { return *Get(); }

        ConstReference operator*() const { return *Get(); }

    private:

        detail::ControlBlock* m_pControlBlock{ nullptr };
    };

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

        template<typename>
        friend class ShadowPtr;

        template<typename, typename>
        friend class OwnerPtr;

    public:

        OwnerPtr() = default;

        OwnerPtr(std::nullptr_t) {}

        template<typename VT2 = VT, typename DT2 = detail::DefaultDeleter<VT2>, std::enable_if_t<std::is_convertible_v<VT2*, VT*>, int> = 0>
        OwnerPtr(typename OwnerPtr<VT2, DT2>::Pointer pOther)
        {
            if (pOther)
            {
                m_pControlBlock = new detail::ControlBlock;
                m_pControlBlock->Data = pOther;
                m_pControlBlock->Deleter = DT2{};
            }
        }

    private:

        OwnerPtr(const OwnerPtr&) = delete;

        OwnerPtr& operator=(const OwnerPtr&) = delete;

    public:

        template<typename VT2 = VT, typename DT2 = detail::DefaultDeleter<VT2>, std::enable_if_t<std::is_convertible_v<VT2*, VT*>, int> = 0>
        OwnerPtr(OwnerPtr<VT2, DT2>&& other) noexcept
        {
            m_pControlBlock = other.m_pControlBlock;
            other.m_pControlBlock = nullptr;
        }

        template<typename VT2 = VT, typename DT2 = detail::DefaultDeleter<VT2>, std::enable_if_t<std::is_convertible_v<VT2*, VT*>, int> = 0>
        OwnerPtr& operator=(OwnerPtr<VT2, DT2>&& other) noexcept
        {
            if (this != &other)
            {
                Destroy();
                m_pControlBlock = other.m_pControlBlock;
                other.m_pControlBlock = nullptr;
            }
            return *this;
        }

        ~OwnerPtr() { Destroy(); }

        std::size_t ShadowCount() const { return m_pControlBlock ? m_pControlBlock->ShadowCount : 0; }

        Pointer Get() { return m_pControlBlock ? static_cast<Pointer>(m_pControlBlock->Data) : nullptr; }

        ConstPointer Get() const { return m_pControlBlock ? static_cast<ConstPointer>(m_pControlBlock->Data) : nullptr; }

        Pointer Data() { return Get(); }

        ConstPointer Data() const { return Get(); }

        Reference operator*() { return *Get(); }

        ConstReference operator*() const { return *Get(); }

        operator bool() const { return nullptr != Get(); }

        bool IsNull() const { return nullptr == Get(); }

        void Nullify() { Destroy(); }

        void Reset() { Destroy(); }

        void Reset(std::nullptr_t) { Destroy(); }

        template<typename VT2 = VT, typename DT2 = detail::DefaultDeleter<VT2>, std::enable_if_t<std::is_convertible_v<VT2*, VT*>, int> = 0>
        void Reset(typename OwnerPtr<VT2, DT2>::Pointer pNew)
        {
            Destroy();
            if (nullptr != pNew)
            {
                m_pControlBlock = new detail::ControlBlock;
                m_pControlBlock->Data = pNew;
                m_pControlBlock->Deleter = DT2{};
            }
        }

        [[nodiscard]] Pointer Release()
        {
            Pointer pReleased = Get();
            if (m_pControlBlock)
            {
                m_pControlBlock->Data = nullptr;
                if (0 == m_pControlBlock->ShadowCount)
                {
                    delete m_pControlBlock;
                    m_pControlBlock = nullptr;
                }
            }
            return pReleased;
        }

        template<typename DT2 = DT>
        void Swap(OwnerPtr<VT, DT2>& other) noexcept
        {
            std::swap(m_pControlBlock, other.m_pControlBlock);
        }

        template<typename VT2 = VT, typename DT2 = detail::DefaultDeleter<VT2>, std::enable_if_t<std::is_convertible_v<VT2*, VT*>, int> = 0>
        [[nodiscard]] OwnerPtr<VT2, DT2> Transfer()
        {
            OwnerPtr<VT2, DT2> transferred;
            if (m_pControlBlock)
            {
                transferred.m_pControlBlock = m_pControlBlock;
                m_pControlBlock = nullptr;
            }
            return transferred;
        }

        template<typename VT2 = VT, typename DT2 = detail::DefaultDeleter<VT2>>
        [[nodiscard]] OwnerPtr<VT2, DT2> Cast()
        {
            OwnerPtr<VT2, DT2> casted;
            Pointer pCurrent = Get();
            typename OwnerPtr<VT2, DT2>::Pointer pCasted = dynamic_cast<typename OwnerPtr<VT2, DT2>::Pointer>(pCurrent);
            if (pCasted)
            {
                casted.m_pControlBlock = m_pControlBlock;
                m_pControlBlock = nullptr;
            }
            else
            {
                Destroy();
            }
            return casted;
        }

        template<typename VT2 = VT, std::enable_if_t<std::is_convertible_v<VT*, VT2*>, int> = 0>
        ShadowPtr<VT2> Shadow() const
        {
            ShadowPtr<VT2> shadow;
            if (m_pControlBlock)
            {
                shadow.m_pControlBlock = m_pControlBlock;
                ++m_pControlBlock->ShadowCount;
            }
            return shadow;
        }

        template<typename VT2 = VT, std::enable_if_t<std::is_convertible_v<VT*, VT2*>, int> = 0>
        ShadowPtr<VT2> Shadow()
        {
            ShadowPtr<VT2> shadow;
            if (m_pControlBlock)
            {
                shadow.m_pControlBlock = m_pControlBlock;
                ++m_pControlBlock->ShadowCount;
            }
            return shadow;
        }

    private:

        void Destroy()
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
