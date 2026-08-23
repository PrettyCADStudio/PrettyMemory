/**
 * @file PrettyMemory.h
 * @brief Header-only C++17 smart pointer library with non-owning observer support.
 *
 * This library provides two smart pointer types that share a common control block:
 * - OwnerPtr<T>       — owning pointer, move-only, manages object lifetime.
 * - ShadowPtr<T>      — non-owning observer, detects when the target is destroyed.
 *
 * A class can derive from EnableShadowFromThis<T> (CRTP) to allow creating
 * ShadowPtr instances directly from @c this.
 *
 * All types live in namespace @c prtm. Internal helpers are in @c prtm::detail.
 *
 * @par Quick example
 * @code
 * #include <PrettyMemory.h>
 *
 * struct Widget : prtm::EnableShadowFromThis<Widget> {
 *     int value = 42;
 * };
 *
 * auto owner = prtm::OwnerPtr<Widget>::Create();
 * auto shadow = owner->ShadowFromThis();
 *
 * assert(shadow->value == 42);
 * assert(!shadow.Expired());
 *
 * owner.Reset();
 * assert(shadow.Expired());
 * @endcode
 *
 * @par Thread safety
 * ControlBlock::ShadowCount is @em not atomic. Concurrent mutation of the same
 * OwnerPtr / ShadowPtr group from multiple threads requires external synchronisation.
 *
 * @par Requirements
 * - C++17 or later (C++20 concepts used when available).
 * - No third-party dependencies; standard library only.
 */
#pragma once

#include <cstddef>
#include <functional>
#include <type_traits>
#include <utility>
#include <version>

#if defined(_MSVC_LANG)
#define PRTM_CXX_STANDARD _MSVC_LANG
#else
#define PRTM_CXX_STANDARD __cplusplus
#endif

#if PRTM_CXX_STANDARD >= 202002L
#define PRTM_HAS_CPP20 1
#else
#define PRTM_HAS_CPP20 0
#endif

#if PRTM_HAS_CPP20 && defined(__cpp_concepts)
#define PRTM_HAS_CONCEPTS 1
#else
#define PRTM_HAS_CONCEPTS 0
#endif

#if PRTM_HAS_CPP20 && defined(__cpp_lib_three_way_comparison) && __cpp_lib_three_way_comparison >= 201907L
#define PRTM_HAS_THREE_WAY_COMPARISON 1
#else
#define PRTM_HAS_THREE_WAY_COMPARISON 0
#endif

#if PRTM_HAS_THREE_WAY_COMPARISON
#include <compare>
#endif

namespace prtm
{
    namespace detail
    {
        /**
         * @brief Shared control block linking an OwnerPtr to its ShadowPtr observers.
         *
         * Allocated on the heap when an OwnerPtr is created. Destroyed when both
         * the owning pointer and all shadow pointers have released it.
         */
        struct ControlBlock
        {
            void* Data{ nullptr };                        ///< Managed object pointer (nullptr after OwnerPtr releases or destroys).
            std::function<void(void*)> Deleter{ nullptr }; ///< Type-erased deleter invoked by OwnerPtr.
            std::size_t ShadowCount{ 0 };                 ///< Number of live ShadowPtr instances referencing this block.
        };

        /**
         * @brief Default deleter that calls @c delete on a typed pointer.
         * @tparam T Object type.
         */
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
    class EnableShadowFromThis;

    /**
     * @brief Non-owning observer pointer that detects when the target object is destroyed.
     * @tparam T Pointed-to object type.
     *
     * A ShadowPtr does @em not extend the lifetime of the observed object.  When the
     * owning OwnerPtr (or EnableShadowFromThis host) destroys or releases the object,
     * subsequent calls to Get(), Expired(), operator->(), and operator*() reflect the
     * expired state:
     * - Get() returns @c nullptr.
     * - Expired() returns @c true.
     * - operator bool() returns @c false.
     *
     * ShadowPtr is copyable and movable.  Copying increments the control block's
     * shadow count; moving transfers ownership of the reference without touching the count.
     *
     * @see OwnerPtr, EnableShadowFromThis
     */
    template<typename T>
    class ShadowPtr
    {
    public:

        using ValueType = T;
        using Pointer = ValueType*;
        using ConstPointer = const ValueType*;
        using Reference = ValueType&;
        using ConstReference = const ValueType&;

        template<typename>
        friend class ShadowPtr;

        template<typename, typename>
        friend class OwnerPtr;

        template<typename>
        friend class EnableShadowFromThis;

    public:

        /** @brief Construct an empty ShadowPtr. */
        ShadowPtr() = default;

        /** @brief Construct an empty ShadowPtr from nullptr. */
        ShadowPtr(std::nullptr_t) {}

        /**
         * @brief Copy-construct a ShadowPtr of the same type.
         * @param other Source ShadowPtr.
         */
        ShadowPtr(const ShadowPtr& other)
        {
            if (other.m_pControlBlock)
            {
                m_pTyped = other.m_pTyped;
                m_pControlBlock = other.m_pControlBlock;
                ++m_pControlBlock->ShadowCount;
            }
        }

        /**
         * @brief Copy-construct from a ShadowPtr of a convertible type.
         * @tparam T2 Source object type.
         * @param other Source ShadowPtr.
         */
#if PRTM_HAS_CONCEPTS
        template<typename T2 = T>
        requires std::convertible_to<T2*, T*>
#else
        template<typename T2 = T, std::enable_if_t<std::is_convertible_v<T2*, T*>, int> = 0>
#endif
        ShadowPtr(const ShadowPtr<T2>& other)
        {
            if (other.m_pControlBlock)
            {
                m_pTyped = other.m_pTyped;
                m_pControlBlock = other.m_pControlBlock;
                ++m_pControlBlock->ShadowCount;
            }
        }

        /**
         * @brief Move-construct from a ShadowPtr of a convertible type.
         * @tparam T2 Source object type.
         * @param other Source ShadowPtr.
         */
#if PRTM_HAS_CONCEPTS
        template<typename T2 = T>
        requires std::convertible_to<T2*, T*>
#else
        template<typename T2 = T, std::enable_if_t<std::is_convertible_v<T2*, T*>, int> = 0>
#endif
        ShadowPtr(ShadowPtr<T2>&& other) noexcept
        {
            m_pTyped = other.m_pTyped;
            m_pControlBlock = other.m_pControlBlock;
            other.m_pTyped = nullptr;
            other.m_pControlBlock = nullptr;
        }

        /**
         * @brief Copy-assign from a ShadowPtr of the same type.
         * @param other Source ShadowPtr.
         * @return Reference to this object.
         */
        ShadowPtr& operator=(const ShadowPtr& other)
        {
            if (this != &other)
            {
                Destroy();
                m_pTyped = other.m_pTyped;
                m_pControlBlock = other.m_pControlBlock;
                if (m_pControlBlock)
                {
                    ++m_pControlBlock->ShadowCount;
                }
            }
            return *this;
        }

        /**
         * @brief Copy-assign from a ShadowPtr of a convertible type.
         * @tparam T2 Source object type.
         * @param other Source ShadowPtr.
         * @return Reference to this object.
         */
#if PRTM_HAS_CONCEPTS
        template<typename T2 = T>
#else
        template<typename T2 = T, std::enable_if_t<std::is_convertible_v<T2*, T*>, int> = 0>
#endif
        ShadowPtr& operator=(const ShadowPtr<T2>& other)
        {
            if (static_cast<const void*>(this) != static_cast<const void*>(&other))
            {
                Destroy();
                m_pTyped = other.m_pTyped;
                m_pControlBlock = other.m_pControlBlock;
                if (m_pControlBlock)
                {
                    ++m_pControlBlock->ShadowCount;
                }
            }
            return *this;
        }

        /**
         * @brief Move-assign from a ShadowPtr of the same type.
         * @param other Source ShadowPtr.
         * @return Reference to this object.
         */
        ShadowPtr& operator=(ShadowPtr&& other) noexcept
        {
            if (this != &other)
            {
                Destroy();
                m_pTyped = other.m_pTyped;
                m_pControlBlock = other.m_pControlBlock;
                other.m_pTyped = nullptr;
                other.m_pControlBlock = nullptr;
            }
            return *this;
        }

        /**
         * @brief Move-assign from a ShadowPtr of a convertible type.
         * @tparam T2 Source object type.
         * @param other Source ShadowPtr.
         * @return Reference to this object.
         */
#if PRTM_HAS_CONCEPTS
        template<typename T2 = T>
        requires std::convertible_to<T2*, T*>
#else
        template<typename T2 = T, std::enable_if_t<std::is_convertible_v<T2*, T*>, int> = 0>
#endif
        ShadowPtr& operator=(ShadowPtr<T2>&& other) noexcept
        {
            if (static_cast<const void*>(this) != static_cast<const void*>(&other))
            {
                Destroy();
                m_pTyped = other.m_pTyped;
                m_pControlBlock = other.m_pControlBlock;
                other.m_pTyped = nullptr;
                other.m_pControlBlock = nullptr;
            }
            return *this;
        }

        /** @brief Destroy this ShadowPtr. */
        ~ShadowPtr() { Destroy(); }

        /**
         * @brief Get the number of ShadowPtr instances sharing this control block.
         * @return The current shadow count, or 0 if this ShadowPtr is empty.
         */
        std::size_t ShadowCount() const { return m_pControlBlock ? m_pControlBlock->ShadowCount : 0; }

        /** @brief Get the writable raw pointer, or nullptr if the object has expired. */
        Pointer Get() { return (m_pControlBlock && m_pControlBlock->Data) ? m_pTyped : nullptr; }

        /** @brief Get the const raw pointer, or nullptr if the object has expired. */
        ConstPointer Get() const { return (m_pControlBlock && m_pControlBlock->Data) ? m_pTyped : nullptr; }

        /** @brief Get the writable data pointer. */
        Pointer Data() { return Get(); }

        /** @brief Get the const data pointer. */
        ConstPointer Data() const { return Get(); }

        /** @brief Check whether this ShadowPtr points to a live object. */
        explicit operator bool() const { return nullptr != Get(); }

        /** @brief Check whether the referenced object has expired. */
        bool Expired() const { return nullptr == Get(); }

        /** @brief Check whether this ShadowPtr is null. */
        bool IsNull() const { return nullptr == Get(); }

        /** @brief Access the referenced object through operator->. */
        Pointer operator->() { return Get(); }

        /** @brief Const access to the referenced object through operator->. */
        ConstPointer operator->() const { return Get(); }

        /** @brief Dereference to obtain the referenced object. */
        Reference operator*() { return *Get(); }

        /** @brief Dereference to obtain the const referenced object. */
        ConstReference operator*() const { return *Get(); }

        /**
         * @brief Swap state with another ShadowPtr.
         * @param other ShadowPtr to swap with.
         */
        void Swap(ShadowPtr& other) noexcept
        {
            std::swap(m_pTyped, other.m_pTyped);
            std::swap(m_pControlBlock, other.m_pControlBlock);
        }

        /**
         * @brief Cast the observed object to another type using dynamic_cast.
         * @tparam VT2 Target object type.
         * @return A new ShadowPtr<VT2> sharing the same control block.
         *
         * If the dynamic_cast fails or the object has expired, an empty ShadowPtr
         * is returned.  The original ShadowPtr is not modified.
         */
        template<typename VT2>
        [[nodiscard]] ShadowPtr<VT2> Cast() const
        {
            ShadowPtr<VT2> casted;
            if (m_pControlBlock && m_pControlBlock->Data)
            {
                typename ShadowPtr<VT2>::Pointer pCasted = dynamic_cast<typename ShadowPtr<VT2>::Pointer>(m_pTyped);
                if (pCasted)
                {
                    casted.m_pTyped = pCasted;
                    casted.m_pControlBlock = m_pControlBlock;
                    ++casted.m_pControlBlock->ShadowCount;
                }
            }
            return casted;
        }

    private:

        void Destroy()
        {
            if (m_pControlBlock)
            {
                if (m_pControlBlock->ShadowCount > 0)
                {
                    --m_pControlBlock->ShadowCount;
                }
                if (nullptr == m_pControlBlock->Data && 0 == m_pControlBlock->ShadowCount)
                {
                    delete m_pControlBlock;
                }
                m_pControlBlock = nullptr;
            }
            m_pTyped = nullptr;
        }

    private:

        Pointer m_pTyped{ nullptr };
        detail::ControlBlock* m_pControlBlock{ nullptr };
    };

    /** @brief Compare whether two ShadowPtr objects point to the same object. */
    template<typename T>
    bool operator==(const ShadowPtr<T>& lhs, const ShadowPtr<T>& rhs)
    {
        return lhs.Get() == rhs.Get();
    }

#if PRTM_HAS_THREE_WAY_COMPARISON

    template<typename T>
    auto operator<=>(const ShadowPtr<T>& lhs, const ShadowPtr<T>& rhs)
    {
        return lhs.Get() <=> rhs.Get();
    }

#else

    /** @brief Compare whether two ShadowPtr objects point to different objects. */
    template<typename T>
    bool operator!=(const ShadowPtr<T>& lhs, const ShadowPtr<T>& rhs)
    {
        return lhs.Get() != rhs.Get();
    }

    /** @brief Compare two ShadowPtr objects by raw pointer address. */
    template<typename T>
    bool operator<(const ShadowPtr<T>& lhs, const ShadowPtr<T>& rhs)
    {
        return lhs.Get() < rhs.Get();
    }

    /** @brief Compare two ShadowPtr objects by raw pointer address. */
    template<typename T>
    bool operator<=(const ShadowPtr<T>& lhs, const ShadowPtr<T>& rhs)
    {
        return lhs.Get() <= rhs.Get();
    }

    /** @brief Compare two ShadowPtr objects by raw pointer address. */
    template<typename T>
    bool operator>(const ShadowPtr<T>& lhs, const ShadowPtr<T>& rhs)
    {
        return lhs.Get() > rhs.Get();
    }

    /** @brief Compare two ShadowPtr objects by raw pointer address. */
    template<typename T>
    bool operator>=(const ShadowPtr<T>& lhs, const ShadowPtr<T>& rhs)
    {
        return lhs.Get() >= rhs.Get();
    }

#endif

    /** @brief Compare a ShadowPtr with nullptr. */
    template<typename T>
    bool operator==(const ShadowPtr<T>& lhs, std::nullptr_t)
    {
        return lhs.Get() == nullptr;
    }

#if PRTM_HAS_THREE_WAY_COMPARISON

    template<typename T>
    auto operator<=>(const ShadowPtr<T>& lhs, std::nullptr_t)
    {
        T* right{ nullptr };
        return lhs.Get() <=> right;
    }

#else

    /** @brief Compare a ShadowPtr with nullptr. */
    template<typename T>
    bool operator!=(const ShadowPtr<T>& lhs, std::nullptr_t)
    {
        return lhs.Get() != nullptr;
    }

    /** @brief Compare a ShadowPtr with nullptr by raw pointer address. */
    template<typename T>
    bool operator<(const ShadowPtr<T>& lhs, std::nullptr_t)
    {
        return lhs.Get() < static_cast<typename ShadowPtr<T>::ConstPointer>(nullptr);
    }

    /** @brief Compare a ShadowPtr with nullptr by raw pointer address. */
    template<typename T>
    bool operator<=(const ShadowPtr<T>& lhs, std::nullptr_t)
    {
        return lhs.Get() <= static_cast<typename ShadowPtr<T>::ConstPointer>(nullptr);
    }

    /** @brief Compare a ShadowPtr with nullptr by raw pointer address. */
    template<typename T>
    bool operator>(const ShadowPtr<T>& lhs, std::nullptr_t)
    {
        return lhs.Get() > static_cast<typename ShadowPtr<T>::ConstPointer>(nullptr);
    }

    /** @brief Compare a ShadowPtr with nullptr by raw pointer address. */
    template<typename T>
    bool operator>=(const ShadowPtr<T>& lhs, std::nullptr_t)
    {
        return lhs.Get() >= static_cast<typename ShadowPtr<T>::ConstPointer>(nullptr);
    }

#endif

    /** @brief Compare nullptr with a ShadowPtr. */
    template<typename T>
    bool operator==(std::nullptr_t, const ShadowPtr<T>& rhs)
    {
        return nullptr == rhs.Get();
    }

#if PRTM_HAS_THREE_WAY_COMPARISON

    /** @brief Compare nullptr with a ShadowPtr. */
    template<typename T>
    auto operator<=>(std::nullptr_t, const ShadowPtr<T>& rhs)
    {
        T* left{ nullptr };
        return left <=> rhs.Get();
    }

#else

    /** @brief Compare nullptr with a ShadowPtr. */
    template<typename T>
    bool operator!=(std::nullptr_t, const ShadowPtr<T>& rhs)
    {
        return nullptr != rhs.Get();
    }

    /** @brief Compare nullptr with a ShadowPtr by raw pointer address. */
    template<typename T>
    bool operator<(std::nullptr_t, const ShadowPtr<T>& rhs)
    {
        return static_cast<typename ShadowPtr<T>::ConstPointer>(nullptr) < rhs.Get();
    }

    /** @brief Compare nullptr with a ShadowPtr by raw pointer address. */
    template<typename T>
    bool operator<=(std::nullptr_t, const ShadowPtr<T>& rhs)
    {
        return static_cast<typename ShadowPtr<T>::ConstPointer>(nullptr) <= rhs.Get();
    }

    /** @brief Compare nullptr with a ShadowPtr by raw pointer address. */
    template<typename T>
    bool operator>(std::nullptr_t, const ShadowPtr<T>& rhs)
    {
        return static_cast<typename ShadowPtr<T>::ConstPointer>(nullptr) > rhs.Get();
    }

    /** @brief Compare nullptr with a ShadowPtr by raw pointer address. */
    template<typename T>
    bool operator>=(std::nullptr_t, const ShadowPtr<T>& rhs)
    {
        return static_cast<typename ShadowPtr<T>::ConstPointer>(nullptr) >= rhs.Get();
    }

#endif

    /**
     * @brief CRTP base class that enables creating ShadowPtr instances from @c this.
     * @tparam T Derived class type (the CRTP parameter).
     *
     * Derive your class from EnableShadowFromThis<YourClass> and call
     * ShadowFromThis() to obtain a ShadowPtr that observes the current instance.
     *
     * The control block is lazily allocated on the first call to ShadowFromThis()
     * and automatically invalidated when the derived object is destroyed.
     *
     * @note The derived class must be owned by an OwnerPtr or otherwise have a
     *       well-defined lifetime.  Do not destroy the object while ShadowPtr
     *       instances created via ShadowFromThis() are still in use from another
     *       thread without external synchronisation.
     *
     * @see ShadowPtr, OwnerPtr::Shadow()
     */
    template<typename T>
    class EnableShadowFromThis
    {
    public:

        /**
         * @brief Create a ShadowPtr that observes this object.
         * @return A ShadowPtr<T> sharing a control block with this object.
         *
         * The control block is allocated on first call.  Subsequent calls reuse
         * the same block, incrementing the shadow count each time.
         */
        ShadowPtr<T> ShadowFromThis()
        {
            if (nullptr == m_pControlBlock)
            {
                m_pControlBlock = new detail::ControlBlock;
                m_pControlBlock->Data = static_cast<void*>(this);
                m_pControlBlock->Deleter = detail::DefaultDeleter<T>{};
            }

            ShadowPtr<T> shadow;
            shadow.m_pTyped = dynamic_cast<T*>(this);
            shadow.m_pControlBlock = m_pControlBlock;
            ++m_pControlBlock->ShadowCount;
            return shadow;
        }

        /**
         * @brief Create a const ShadowPtr that observes this object.
         * @return A ShadowPtr<const T> sharing a control block with this object.
         */
        ShadowPtr<const T> ShadowFromThis() const
        {
            if (nullptr == m_pControlBlock)
            {
                m_pControlBlock = new detail::ControlBlock;
                m_pControlBlock->Data = const_cast<T*>(static_cast<const T*>(this));
            }

            ShadowPtr<T> shadow;
            shadow.m_pTyped = const_cast<T*>(static_cast<const T*>(this));
            shadow.m_pControlBlock = m_pControlBlock;
            ++m_pControlBlock->ShadowCount;
            return shadow;
        }

    protected:

        EnableShadowFromThis() = default;

        EnableShadowFromThis(const EnableShadowFromThis&) {}

        EnableShadowFromThis(EnableShadowFromThis&&) noexcept {}

        EnableShadowFromThis& operator=(const EnableShadowFromThis&) { return *this; }

        EnableShadowFromThis& operator=(EnableShadowFromThis&&) noexcept { return *this; }

        virtual ~EnableShadowFromThis()
        {
            if (nullptr != m_pControlBlock)
            {
                m_pControlBlock->Data = nullptr;
                if (0 == m_pControlBlock->ShadowCount)
                {
                    delete m_pControlBlock;
                }
                m_pControlBlock = nullptr;
            }
        }

    private:

        mutable detail::ControlBlock* m_pControlBlock{ nullptr };
    };

    /**
     * @brief Owning smart pointer that manages an object's lifetime and supports ShadowPtr observers.
     * @tparam VT Object type.
     * @tparam DT Deleter type.  Must be callable with @c void*. Defaults to DefaultDeleter<VT>.
     *
     * OwnerPtr is move-only (copy is deleted).  Moving transfers both the managed
     * pointer and the control block to the new OwnerPtr.
     *
     * Use the static factory Create() to construct an OwnerPtr in-place, or
     * construct from a raw pointer / nullptr.
     *
     * When the OwnerPtr is destroyed or reset, it invokes the deleter on the
     * managed object and sets the control block's Data pointer to @c nullptr.
     * Existing ShadowPtr instances then report Expired() == @c true.
     *
     * @see ShadowPtr, EnableShadowFromThis
     */
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

        /**
         * @brief Construct an object in place and return an OwnerPtr managing it.
         * @tparam ArgTypes Constructor argument types (deduced).
         * @param args Arguments forwarded to the VT constructor.
         * @return OwnerPtr managing the newly created object.
         *
         * Equivalent to @c new VT{args...} wrapped in an OwnerPtr.
         */
#if PRTM_HAS_CONCEPTS
        template<typename... ArgTypes>
        requires std::constructible_from<VT, ArgTypes...>
#else
        template<typename... ArgTypes, std::enable_if_t<std::is_constructible_v<VT, ArgTypes...>, int> = 0>
#endif
        static OwnerPtr Create(ArgTypes&&... args)
        {
            auto pRaw = new VT{ std::forward<ArgTypes>(args)... };
            OwnerPtr<VT, DT> owner{ pRaw };
            return owner;
        }

        /** @brief Construct an empty OwnerPtr. */
        OwnerPtr() = default;

        /** @brief Construct an empty OwnerPtr from nullptr. */
        OwnerPtr(std::nullptr_t) {}

        /**
         * @brief Construct an OwnerPtr from a raw pointer.
         * @tparam VT2 Source object type.
         * @tparam DT2 Source deleter type.
         * @param pOther Raw pointer to take ownership of.
         */
#if PRTM_HAS_CONCEPTS
        template<typename VT2 = VT, typename DT2 = DT>
        requires std::convertible_to<VT2*, VT*>
#else
        template<typename VT2 = VT, typename DT2 = DT, std::enable_if_t<std::is_convertible_v<VT2*, VT*>, int> = 0>
#endif
        OwnerPtr(typename OwnerPtr<VT2, DT2>::Pointer pOther)
        {
            if (pOther)
            {
                m_pTyped = pOther;
                m_pControlBlock = new detail::ControlBlock;
                m_pControlBlock->Data = pOther;
                m_pControlBlock->Deleter = DT2{};
            }
        }

    private:

        OwnerPtr(const OwnerPtr&) = delete;

        OwnerPtr& operator=(const OwnerPtr&) = delete;

    public:

        /**
         * @brief Move-construct from an OwnerPtr of a convertible type.
         * @tparam VT2 Source object type.
         * @tparam DT2 Source deleter type.
         * @param other Source OwnerPtr.
         */
#if PRTM_HAS_CONCEPTS
        template<typename VT2 = VT, typename DT2 = detail::DefaultDeleter<VT2>>
        requires std::convertible_to<VT2*, VT*>
#else
        template<typename VT2 = VT, typename DT2 = detail::DefaultDeleter<VT2>, std::enable_if_t<std::is_convertible_v<VT2*, VT*>, int> = 0>
#endif
        OwnerPtr(OwnerPtr<VT2, DT2>&& other) noexcept
        {
            m_pTyped = other.m_pTyped;
            m_pControlBlock = other.m_pControlBlock;
            other.m_pTyped = nullptr;
            other.m_pControlBlock = nullptr;
        }

        /**
         * @brief Move-assign from an OwnerPtr of a convertible type.
         * @tparam VT2 Source object type.
         * @tparam DT2 Source deleter type.
         * @param other Source OwnerPtr.
         * @return Reference to this object.
         */
#if PRTM_HAS_CONCEPTS
        template<typename VT2 = VT, typename DT2 = detail::DefaultDeleter<VT2>>
        requires std::convertible_to<VT2*, VT*>
#else
        template<typename VT2 = VT, typename DT2 = detail::DefaultDeleter<VT2>, std::enable_if_t<std::is_convertible_v<VT2*, VT*>, int> = 0>
#endif
        OwnerPtr& operator=(OwnerPtr<VT2, DT2>&& other) noexcept
        {
            if (static_cast<const void*>(this) != static_cast<const void*>(&other))
            {
                Destroy();
                m_pTyped = other.m_pTyped;
                m_pControlBlock = other.m_pControlBlock;
                other.m_pTyped = nullptr;
                other.m_pControlBlock = nullptr;
            }
            return *this;
        }

        /** @brief Destroy this OwnerPtr and delete the managed object when needed. */
        ~OwnerPtr() { Destroy(); }

        /**
         * @brief Get the number of ShadowPtr instances observing the managed object.
         * @return The current shadow count, or 0 if this OwnerPtr is empty.
         */
        std::size_t ShadowCount() const { return m_pControlBlock ? m_pControlBlock->ShadowCount : 0; }

        /** @brief Get the writable raw pointer. */
        Pointer Get() { return m_pTyped; }

        /** @brief Get the const raw pointer. */
        ConstPointer Get() const { return m_pTyped; }

        /** @brief Get the writable data pointer. */
        Pointer Data() { return Get(); }

        /** @brief Get the const data pointer. */
        ConstPointer Data() const { return Get(); }

        /** @brief Dereference to obtain the managed object. */
        Reference operator*() { return *Get(); }

        /** @brief Dereference to obtain the const managed object. */
        ConstReference operator*() const { return *Get(); }

        /** @brief Access the managed object through operator->. */
        Pointer operator->() { return Get(); }

        /** @brief Const access to the managed object through operator->. */
        ConstPointer operator->() const { return Get(); }

        /** @brief Check whether this OwnerPtr currently owns an object. */
        explicit operator bool() const { return nullptr != Get(); }

        /** @brief Check whether this OwnerPtr is null. */
        bool IsNull() const { return nullptr == Get(); }

        /** @brief Destroy the managed object and reset this pointer to empty. */
        void Nullify() { Destroy(); }

        /** @brief Destroy the managed object and reset this pointer to empty. */
        void Reset() { Destroy(); }

        /** @brief Destroy the managed object and reset this pointer to empty. */
        void Reset(std::nullptr_t) { Destroy(); }

        /**
         * @brief Replace the managed object with a new raw pointer.
         * @tparam VT2 New object type (must be convertible to VT).
         * @tparam DT2 Deleter type for the new object.
         * @param pNew New raw pointer to take ownership of.  May be nullptr.
         *
         * The previously managed object (if any) is destroyed before the new
         * pointer is adopted.
         */
#if PRTM_HAS_CONCEPTS
        template<typename VT2 = VT, typename DT2 = DT>
        requires std::convertible_to<VT2*, VT*>
#else
        template<typename VT2 = VT, typename DT2 = DT, std::enable_if_t<std::is_convertible_v<VT2*, VT*>, int> = 0>
#endif
        void Reset(typename OwnerPtr<VT2, DT2>::Pointer pNew)
        {
            Destroy();
            if (nullptr != pNew)
            {
                m_pTyped = pNew;
                m_pControlBlock = new detail::ControlBlock;
                m_pControlBlock->Data = pNew;
                m_pControlBlock->Deleter = DT2{};
            }
        }

        /**
         * @brief Release ownership of the managed object and return the raw pointer.
         * @return The raw pointer to the formerly managed object.
         *
         * The caller is responsible for eventually deleting the returned pointer.
         * Existing ShadowPtr instances will report Expired() == @c true after this call.
         */
        [[nodiscard]] Pointer Release()
        {
            Pointer pReleased = m_pTyped;
            if (m_pControlBlock)
            {
                m_pControlBlock->Data = nullptr;
                if (0 == m_pControlBlock->ShadowCount)
                {
                    delete m_pControlBlock;
                }
                m_pControlBlock = nullptr;
            }
            m_pTyped = nullptr;
            return pReleased;
        }

        /**
         * @brief Swap state with another OwnerPtr.
         * @tparam DT2 Other deleter type.
         * @param other OwnerPtr to swap with.
         */
        template<typename DT2 = DT>
        void Swap(OwnerPtr<VT, DT2>& other) noexcept
        {
            std::swap(m_pTyped, other.m_pTyped);
            std::swap(m_pControlBlock, other.m_pControlBlock);
        }

        /**
         * @brief Transfer ownership to an OwnerPtr of a different type.
         * @tparam VT2 Target object type (must be convertible from VT).
         * @tparam DT2 Target deleter type (defaults to DefaultDeleter<VT2>).
         * @return A new OwnerPtr<VT2, DT2> owning the managed object.
         *
         * After the call, this OwnerPtr is emptied.  Unlike Cast(), no dynamic_cast
         * is performed — the types must be statically compatible.
         */
#if PRTM_HAS_CONCEPTS
        template<typename VT2 = VT, typename DT2 = detail::DefaultDeleter<VT2>>
        requires std::convertible_to<VT2*, VT*>
#else
        template<typename VT2 = VT, typename DT2 = detail::DefaultDeleter<VT2>, std::enable_if_t<std::is_convertible_v<VT2*, VT*>, int> = 0>
#endif
        [[nodiscard]] OwnerPtr<VT2, DT2> Transfer()
        {
            OwnerPtr<VT2, DT2> transferred;
            if (m_pControlBlock)
            {
                transferred.m_pTyped = m_pTyped;
                transferred.m_pControlBlock = m_pControlBlock;
                m_pTyped = nullptr;
                m_pControlBlock = nullptr;
            }
            return transferred;
        }

        /**
         * @brief Cast the owned object to another type using dynamic_cast, transferring ownership.
         * @tparam VT2 Target object type.
         * @tparam DT2 Target deleter type (defaults to DefaultDeleter<VT2>).
         * @return A new OwnerPtr<VT2, DT2> owning the casted pointer.
         *
         * If the dynamic_cast fails, the current object is destroyed and an empty
         * OwnerPtr is returned.  On success, this OwnerPtr is emptied (ownership moved).
         */
        template<typename VT2 = VT, typename DT2 = detail::DefaultDeleter<VT2>>
        [[nodiscard]] OwnerPtr<VT2, DT2> Cast()
        {
            OwnerPtr<VT2, DT2> casted;
            typename OwnerPtr<VT2, DT2>::Pointer pCasted = dynamic_cast<typename OwnerPtr<VT2, DT2>::Pointer>(m_pTyped);
            if (pCasted)
            {
                casted.m_pTyped = pCasted;
                casted.m_pControlBlock = m_pControlBlock;
                m_pTyped = nullptr;
                m_pControlBlock = nullptr;
            }
            else
            {
                Destroy();
            }
            return casted;
        }

        /**
         * @brief Create a ShadowPtr that observes the managed object (const overload).
         * @tparam VT2 Target ShadowPtr element type (must be convertible from VT).
         * @return A new ShadowPtr sharing this OwnerPtr's control block.
         *
         * The returned ShadowPtr remains valid until the managed object is destroyed.
         * @see EnableShadowFromThis::ShadowFromThis()
         */
#if PRTM_HAS_CONCEPTS
        template<typename VT2 = VT>
        requires std::convertible_to<VT*, VT2*>
#else
        template<typename VT2 = VT, std::enable_if_t<std::is_convertible_v<VT*, VT2*>, int> = 0>
#endif
        ShadowPtr<VT2> Shadow() const
        {
            ShadowPtr<VT2> shadow;
            if (m_pControlBlock)
            {
                shadow.m_pTyped = m_pTyped;
                shadow.m_pControlBlock = m_pControlBlock;
                ++m_pControlBlock->ShadowCount;
            }
            return shadow;
        }

        /**
         * @brief Create a ShadowPtr that observes the managed object (non-const overload).
         * @tparam VT2 Target ShadowPtr element type (must be convertible from VT).
         * @return A new ShadowPtr sharing this OwnerPtr's control block.
         *
         * The returned ShadowPtr remains valid until the managed object is destroyed.
         * @see EnableShadowFromThis::ShadowFromThis()
         */
#if PRTM_HAS_CONCEPTS
        template<typename VT2 = VT>
        requires std::convertible_to<VT*, VT2*>
#else
        template<typename VT2 = VT, std::enable_if_t<std::is_convertible_v<VT*, VT2*>, int> = 0>
#endif
        ShadowPtr<VT2> Shadow()
        {
            ShadowPtr<VT2> shadow;
            if (m_pControlBlock)
            {
                shadow.m_pTyped = m_pTyped;
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
                }
                m_pControlBlock = nullptr;
            }
            m_pTyped = nullptr;
        }

    private:

        Pointer m_pTyped{ nullptr };
        detail::ControlBlock* m_pControlBlock{ nullptr };
    };

    /** @brief Compare whether two OwnerPtr objects point to the same object. */
    template<typename VT, typename DT1 = detail::DefaultDeleter<VT>, typename DT2 = detail::DefaultDeleter<VT>>
    bool operator==(const OwnerPtr<VT, DT1>& lhs, const OwnerPtr<VT, DT2>& rhs)
    {
        return lhs.Get() == rhs.Get();
    }

#if PRTM_HAS_THREE_WAY_COMPARISON

    template<typename VT, typename DT1 = detail::DefaultDeleter<VT>, typename DT2 = detail::DefaultDeleter<VT>>
    auto operator<=>(const OwnerPtr<VT, DT1>& lhs, const OwnerPtr<VT, DT2>& rhs)
    {
        return lhs.Get() <=> rhs.Get();
    }

#else

    /** @brief Compare whether two OwnerPtr objects point to different objects. */
    template<typename VT, typename DT1 = detail::DefaultDeleter<VT>, typename DT2 = detail::DefaultDeleter<VT>>
    bool operator!=(const OwnerPtr<VT, DT1>& lhs, const OwnerPtr<VT, DT2>& rhs)
    {
        return lhs.Get() != rhs.Get();
    }

    /** @brief Compare two OwnerPtr objects by raw pointer address. */
    template<typename VT, typename DT1 = detail::DefaultDeleter<VT>, typename DT2 = detail::DefaultDeleter<VT>>
    bool operator<(const OwnerPtr<VT, DT1>& lhs, const OwnerPtr<VT, DT2>& rhs)
    {
        return lhs.Get() < rhs.Get();
    }

    /** @brief Compare two OwnerPtr objects by raw pointer address. */
    template<typename VT, typename DT1 = detail::DefaultDeleter<VT>, typename DT2 = detail::DefaultDeleter<VT>>
    bool operator>(const OwnerPtr<VT, DT1>& lhs, const OwnerPtr<VT, DT2>& rhs)
    {
        return lhs.Get() > rhs.Get();
    }

    /** @brief Compare two OwnerPtr objects by raw pointer address. */
    template<typename VT, typename DT1 = detail::DefaultDeleter<VT>, typename DT2 = detail::DefaultDeleter<VT>>
    bool operator<=(const OwnerPtr<VT, DT1>& lhs, const OwnerPtr<VT, DT2>& rhs)
    {
        return lhs.Get() <= rhs.Get();
    }

    /** @brief Compare two OwnerPtr objects by raw pointer address. */
    template<typename VT, typename DT1 = detail::DefaultDeleter<VT>, typename DT2 = detail::DefaultDeleter<VT>>
    bool operator>=(const OwnerPtr<VT, DT1>& lhs, const OwnerPtr<VT, DT2>& rhs)
    {
        return lhs.Get() >= rhs.Get();
    }

#endif

    /** @brief Compare an OwnerPtr with nullptr. */
    template<typename VT, typename DT = detail::DefaultDeleter<VT>>
    bool operator==(const OwnerPtr<VT, DT>& lhs, std::nullptr_t)
    {
        return lhs.Get() == nullptr;
    }

#if PRTM_HAS_THREE_WAY_COMPARISON

    template<typename VT, typename DT = detail::DefaultDeleter<VT>>
    auto operator<=>(const OwnerPtr<VT, DT>& lhs, std::nullptr_t)
    {
        VT* right{ nullptr };
        return lhs.Get() <=> right;
    }

#else

    /** @brief Compare an OwnerPtr with nullptr. */
    template<typename VT, typename DT = detail::DefaultDeleter<VT>>
    bool operator!=(const OwnerPtr<VT, DT>& lhs, std::nullptr_t)
    {
        return lhs.Get() != nullptr;
    }

    /** @brief Compare an OwnerPtr with nullptr by raw pointer address. */
    template<typename VT, typename DT = detail::DefaultDeleter<VT>>
    bool operator<(const OwnerPtr<VT, DT>& lhs, std::nullptr_t)
    {
        return lhs.Get() < static_cast<typename OwnerPtr<VT, DT>::ConstPointer>(nullptr);
    }

    /** @brief Compare an OwnerPtr with nullptr by raw pointer address. */
    template<typename VT, typename DT = detail::DefaultDeleter<VT>>
    bool operator>(const OwnerPtr<VT, DT>& lhs, std::nullptr_t)
    {
        return lhs.Get() > static_cast<typename OwnerPtr<VT, DT>::ConstPointer>(nullptr);
    }

    /** @brief Compare an OwnerPtr with nullptr by raw pointer address. */
    template<typename VT, typename DT = detail::DefaultDeleter<VT>>
    bool operator<=(const OwnerPtr<VT, DT>& lhs, std::nullptr_t)
    {
        return lhs.Get() <= static_cast<typename OwnerPtr<VT, DT>::ConstPointer>(nullptr);
    }

    /** @brief Compare an OwnerPtr with nullptr by raw pointer address. */
    template<typename VT, typename DT = detail::DefaultDeleter<VT>>
    bool operator>=(const OwnerPtr<VT, DT>& lhs, std::nullptr_t)
    {
        return lhs.Get() >= static_cast<typename OwnerPtr<VT, DT>::ConstPointer>(nullptr);
    }

#endif

    /** @brief Compare nullptr with an OwnerPtr. */
    template<typename VT, typename DT = detail::DefaultDeleter<VT>>
    bool operator==(std::nullptr_t, const OwnerPtr<VT, DT>& rhs)
    {
        return nullptr == rhs.Get();
    }

#if PRTM_HAS_THREE_WAY_COMPARISON

    template<typename VT, typename DT = detail::DefaultDeleter<VT>>
    auto operator<=>(std::nullptr_t, const OwnerPtr<VT, DT>& rhs)
    {
        VT* left{ nullptr };
        return left <=> rhs.Get();
    }

#else

    /** @brief Compare nullptr with an OwnerPtr. */
    template<typename VT, typename DT = detail::DefaultDeleter<VT>>
    bool operator!=(std::nullptr_t, const OwnerPtr<VT, DT>& rhs)
    {
        return nullptr != rhs.Get();
    }

    /** @brief Compare nullptr with an OwnerPtr by raw pointer address. */
    template<typename VT, typename DT = detail::DefaultDeleter<VT>>
    bool operator<(std::nullptr_t, const OwnerPtr<VT, DT>& rhs)
    {
        return static_cast<typename OwnerPtr<VT, DT>::ConstPointer>(nullptr) < rhs.Get();
    }

    /** @brief Compare nullptr with an OwnerPtr by raw pointer address. */
    template<typename VT, typename DT = detail::DefaultDeleter<VT>>
    bool operator>(std::nullptr_t, const OwnerPtr<VT, DT>& rhs)
    {
        return static_cast<typename OwnerPtr<VT, DT>::ConstPointer>(nullptr) > rhs.Get();
    }

    /** @brief Compare nullptr with an OwnerPtr by raw pointer address. */
    template<typename VT, typename DT = detail::DefaultDeleter<VT>>
    bool operator<=(std::nullptr_t, const OwnerPtr<VT, DT>& rhs)
    {
        return static_cast<typename OwnerPtr<VT, DT>::ConstPointer>(nullptr) <= rhs.Get();
    }

    /** @brief Compare nullptr with an OwnerPtr by raw pointer address. */
    template<typename VT, typename DT = detail::DefaultDeleter<VT>>
    bool operator>=(std::nullptr_t, const OwnerPtr<VT, DT>& rhs)
    {
        return static_cast<typename OwnerPtr<VT, DT>::ConstPointer>(nullptr) >= rhs.Get();
    }

#endif
}

/** @brief std specializations for prtm smart pointers. */
namespace std
{
    /** @brief Swap two ShadowPtr instances. @relates prtm::ShadowPtr */
    template<typename T>
    void swap(prtm::ShadowPtr<T>& lhs, prtm::ShadowPtr<T>& rhs) noexcept
    {
        lhs.Swap(rhs);
    }

    /** @brief Hash support for ShadowPtr, enabling use in unordered containers. @relates prtm::ShadowPtr */
    template<typename T>
    struct hash<prtm::ShadowPtr<T>>
    {
        std::size_t operator()(const prtm::ShadowPtr<T>& obj) const
        {
            return std::hash<typename prtm::ShadowPtr<T>::ConstPointer>{}(obj.Get());
        }
    };

    /** @brief Equality comparison for ShadowPtr in standard algorithms. @relates prtm::ShadowPtr */
    template<typename T>
    struct equal_to<prtm::ShadowPtr<T>>
    {
        bool operator()(const prtm::ShadowPtr<T>& lhs, const prtm::ShadowPtr<T>& rhs) const
        {
            return lhs == rhs;
        }
    };

    /** @brief Inequality comparison for ShadowPtr. @relates prtm::ShadowPtr */
    template<typename T>
    struct not_equal_to<prtm::ShadowPtr<T>>
    {
        bool operator()(const prtm::ShadowPtr<T>& lhs, const prtm::ShadowPtr<T>& rhs) const
        {
            return lhs != rhs;
        }
    };

    /** @brief Less-than comparison for ShadowPtr. @relates prtm::ShadowPtr */
    template<typename T>
    struct less<prtm::ShadowPtr<T>>
    {
        bool operator()(const prtm::ShadowPtr<T>& lhs, const prtm::ShadowPtr<T>& rhs) const
        {
            return lhs < rhs;
        }
    };

    /** @brief Less-equal comparison for ShadowPtr. @relates prtm::ShadowPtr */
    template<typename T>
    struct less_equal<prtm::ShadowPtr<T>>
    {
        bool operator()(const prtm::ShadowPtr<T>& lhs, const prtm::ShadowPtr<T>& rhs) const
        {
            return lhs <= rhs;
        }
    };

    /** @brief Greater-than comparison for ShadowPtr. @relates prtm::ShadowPtr */
    template<typename T>
    struct greater<prtm::ShadowPtr<T>>
    {
        bool operator()(const prtm::ShadowPtr<T>& lhs, const prtm::ShadowPtr<T>& rhs) const
        {
            return lhs > rhs;
        }
    };

    /** @brief Greater-equal comparison for ShadowPtr. @relates prtm::ShadowPtr */
    template<typename T>
    struct greater_equal<prtm::ShadowPtr<T>>
    {
        bool operator()(const prtm::ShadowPtr<T>& lhs, const prtm::ShadowPtr<T>& rhs) const
        {
            return lhs >= rhs;
        }
    };

    /** @brief Swap two OwnerPtr instances. @relates prtm::OwnerPtr */
    template<typename VT, typename DT1 = prtm::detail::DefaultDeleter<VT>, typename DT2 = prtm::detail::DefaultDeleter<VT>>
    void swap(prtm::OwnerPtr<VT, DT1>& lhs, prtm::OwnerPtr<VT, DT2>& rhs) noexcept
    {
        lhs.Swap(rhs);
    }

    /** @brief Hash support for OwnerPtr, enabling use in unordered containers. @relates prtm::OwnerPtr */
    template<typename VT, typename DT>
    struct hash<prtm::OwnerPtr<VT, DT>>
    {
        std::size_t operator()(const prtm::OwnerPtr<VT, DT>& obj) const
        {
            return std::hash<typename prtm::OwnerPtr<VT, DT>::ConstPointer>{}(obj.Get());
        }
    };

    /** @brief Equality comparison for OwnerPtr in standard algorithms. @relates prtm::OwnerPtr */
    template<typename VT, typename DT>
    struct equal_to<prtm::OwnerPtr<VT, DT>>
    {
        bool operator()(const prtm::OwnerPtr<VT, DT>& lhs, const prtm::OwnerPtr<VT, DT>& rhs) const
        {
            return lhs == rhs;
        }
    };

    /** @brief Inequality comparison for OwnerPtr. @relates prtm::OwnerPtr */
    template<typename VT, typename DT>
    struct not_equal_to<prtm::OwnerPtr<VT, DT>>
    {
        bool operator()(const prtm::OwnerPtr<VT, DT>& lhs, const prtm::OwnerPtr<VT, DT>& rhs) const
        {
            return !(lhs == rhs);
        }
    };

    /** @brief Less-than comparison for OwnerPtr. @relates prtm::OwnerPtr */
    template<typename VT, typename DT>
    struct less<prtm::OwnerPtr<VT, DT>>
    {
        bool operator()(const prtm::OwnerPtr<VT, DT>& lhs, const prtm::OwnerPtr<VT, DT>& rhs) const
        {
            return lhs < rhs;
        }
    };

    /** @brief Greater-than comparison for OwnerPtr. @relates prtm::OwnerPtr */
    template<typename VT, typename DT>
    struct greater<prtm::OwnerPtr<VT, DT>>
    {
        bool operator()(const prtm::OwnerPtr<VT, DT>& lhs, const prtm::OwnerPtr<VT, DT>& rhs) const
        {
            return lhs > rhs;
        }
    };

    /** @brief Less-equal comparison for OwnerPtr. @relates prtm::OwnerPtr */
    template<typename VT, typename DT>
    struct less_equal<prtm::OwnerPtr<VT, DT>>
    {
        bool operator()(const prtm::OwnerPtr<VT, DT>& lhs, const prtm::OwnerPtr<VT, DT>& rhs) const
        {
            return lhs <= rhs;
        }
    };

    /** @brief Greater-equal comparison for OwnerPtr. @relates prtm::OwnerPtr */
    template<typename VT, typename DT>
    struct greater_equal<prtm::OwnerPtr<VT, DT>>
    {
        bool operator()(const prtm::OwnerPtr<VT, DT>& lhs, const prtm::OwnerPtr<VT, DT>& rhs) const
        {
            return lhs >= rhs;
        }
    };
}
