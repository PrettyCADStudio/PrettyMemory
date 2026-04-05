/**
 * @file PrettyMemory.h
 * @brief @~chinese{PrettyMemory 的公开接口与头文件实现。} @~english{Public API and header-only implementation for PrettyMemory.}
 */
#pragma once

#include <cstddef>
#include <functional>
#include <type_traits>
#include <utility>

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
    class EnableShadowFromThis;

    /**
     * @brief @~chinese{非拥有型阴影指针，用于观察对象是否仍然有效。} @~english{Non-owning shadow pointer used to observe whether an object is still alive.}
     * @tparam T @~chinese{指向的对象类型。} @~english{Pointed-to object type.}
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

        /** @brief @~chinese{构造一个空的 ShadowPtr。} @~english{Construct an empty ShadowPtr.} */
        ShadowPtr() = default;

        /** @brief @~chinese{由 nullptr 构造空的 ShadowPtr。} @~english{Construct an empty ShadowPtr from nullptr.} */
        ShadowPtr(std::nullptr_t) {}

        /**
         * @brief @~chinese{拷贝构造同类型 ShadowPtr。} @~english{Copy-construct a ShadowPtr of the same type.}
         * @param other @~chinese{源 ShadowPtr。} @~english{Source ShadowPtr.}
         */
        ShadowPtr(const ShadowPtr& other)
        {
            if (other.m_pControlBlock)
            {
                m_pControlBlock = other.m_pControlBlock;
                ++m_pControlBlock->ShadowCount;
            }
        }

        /**
         * @brief @~chinese{从可转换类型的 ShadowPtr 拷贝构造。} @~english{Copy-construct from a ShadowPtr of a convertible type.}
         * @tparam T2 @~chinese{源对象类型。} @~english{Source object type.}
         * @param other @~chinese{源 ShadowPtr。} @~english{Source ShadowPtr.}
         */
        template<typename T2 = T, std::enable_if_t<std::is_convertible_v<T2*, T*>, int> = 0>
        ShadowPtr(const ShadowPtr<T2>& other)
        {
            if (other.m_pControlBlock)
            {
                m_pControlBlock = other.m_pControlBlock;
                ++m_pControlBlock->ShadowCount;
            }
        }

        /**
         * @brief @~chinese{从可转换类型的 ShadowPtr 移动构造。} @~english{Move-construct from a ShadowPtr of a convertible type.}
         * @tparam T2 @~chinese{源对象类型。} @~english{Source object type.}
         * @param other @~chinese{源 ShadowPtr。} @~english{Source ShadowPtr.}
         */
        template<typename T2 = T, std::enable_if_t<std::is_convertible_v<T2*, T*>, int> = 0>
        ShadowPtr(ShadowPtr<T2>&& other) noexcept
        {
            m_pControlBlock = other.m_pControlBlock;
            other.m_pControlBlock = nullptr;
        }

        /**
         * @brief @~chinese{拷贝赋值同类型 ShadowPtr。} @~english{Copy-assign from a ShadowPtr of the same type.}
         * @param other @~chinese{源 ShadowPtr。} @~english{Source ShadowPtr.}
         * @return @~chinese{当前对象引用。} @~english{Reference to this object.}
         */
        ShadowPtr& operator=(const ShadowPtr& other)
        {
            if (this != &other)
            {
                Destroy();
                m_pControlBlock = other.m_pControlBlock;
                if (m_pControlBlock)
                {
                    ++m_pControlBlock->ShadowCount;
                }
            }
            return *this;
        }

        /**
         * @brief @~chinese{从可转换类型的 ShadowPtr 拷贝赋值。} @~english{Copy-assign from a ShadowPtr of a convertible type.}
         * @tparam T2 @~chinese{源对象类型。} @~english{Source object type.}
         * @param other @~chinese{源 ShadowPtr。} @~english{Source ShadowPtr.}
         * @return @~chinese{当前对象引用。} @~english{Reference to this object.}
         */
        template<typename T2 = T, std::enable_if_t<std::is_convertible_v<T2*, T*>, int> = 0>
        ShadowPtr& operator=(const ShadowPtr<T2>& other)
        {
            Destroy();
            m_pControlBlock = other.m_pControlBlock;
            if (m_pControlBlock)
            {
                ++m_pControlBlock->ShadowCount;
            }
            return *this;
        }

        /**
         * @brief @~chinese{从可转换类型的 ShadowPtr 移动赋值。} @~english{Move-assign from a ShadowPtr of a convertible type.}
         * @tparam T2 @~chinese{源对象类型。} @~english{Source object type.}
         * @param other @~chinese{源 ShadowPtr。} @~english{Source ShadowPtr.}
         * @return @~chinese{当前对象引用。} @~english{Reference to this object.}
         */
        template<typename T2 = T, std::enable_if_t<std::is_convertible_v<T2*, T*>, int> = 0>
        ShadowPtr& operator=(ShadowPtr<T2>&& other) noexcept
        {
            Destroy();
            m_pControlBlock = other.m_pControlBlock;
            other.m_pControlBlock = nullptr;
            return *this;
        }

        /** @brief @~chinese{析构当前 ShadowPtr。} @~english{Destroy this ShadowPtr.} */
        ~ShadowPtr() { Destroy(); }

        /** @brief @~chinese{返回共享该控制块的 ShadowPtr 数量。} @~english{Get the number of ShadowPtr instances sharing the control block.} */
        std::size_t ShadowCount() const { return m_pControlBlock ? m_pControlBlock->ShadowCount : 0; }

        /** @brief @~chinese{获取可写原始指针；若对象已失效则返回 nullptr。} @~english{Get the writable raw pointer, or nullptr if the object has expired.} */
        Pointer Get() { return m_pControlBlock ? static_cast<Pointer>(m_pControlBlock->Data) : nullptr; }

        /** @brief @~chinese{获取只读原始指针；若对象已失效则返回 nullptr。} @~english{Get the const raw pointer, or nullptr if the object has expired.} */
        ConstPointer Get() const { return m_pControlBlock ? static_cast<ConstPointer>(m_pControlBlock->Data) : nullptr; }

        /** @brief @~chinese{获取可写数据指针。} @~english{Get the writable data pointer.} */
        Pointer Data() { return Get(); }

        /** @brief @~chinese{获取只读数据指针。} @~english{Get the const data pointer.} */
        ConstPointer Data() const { return Get(); }

        /** @brief @~chinese{判断当前 ShadowPtr 是否指向有效对象。} @~english{Check whether this ShadowPtr points to a live object.} */
        operator bool() const { return nullptr != Get(); }

        /** @brief @~chinese{判断关联对象是否已经失效。} @~english{Check whether the referenced object has expired.} */
        bool Expired() const { return nullptr == Get(); }

        /** @brief @~chinese{判断当前 ShadowPtr 是否为空。} @~english{Check whether this ShadowPtr is null.} */
        bool IsNull() const { return nullptr == Get(); }

        /** @brief @~chinese{通过箭头运算符访问对象。} @~english{Access the referenced object through operator->.} */
        Pointer operator->() { return Get(); }

        /** @brief @~chinese{通过箭头运算符只读访问对象。} @~english{Const access to the referenced object through operator->.} */
        ConstPointer operator->() const { return Get(); }

        /** @brief @~chinese{解引用获得对象引用。} @~english{Dereference to obtain the referenced object.} */
        Reference operator*() { return *Get(); }

        /** @brief @~chinese{解引用获得对象常量引用。} @~english{Dereference to obtain the const referenced object.} */
        ConstReference operator*() const { return *Get(); }

        /**
         * @brief @~chinese{与另一个 ShadowPtr 交换状态。} @~english{Swap state with another ShadowPtr.}
         * @param other @~chinese{要交换的 ShadowPtr。} @~english{ShadowPtr to swap with.}
         */
        void Swap(ShadowPtr& other) noexcept
        {
            std::swap(m_pControlBlock, other.m_pControlBlock);
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
        }

    private:

        detail::ControlBlock* m_pControlBlock{ nullptr };
    };

    /** @brief @~chinese{比较两个 ShadowPtr 是否指向同一对象。} @~english{Compare whether two ShadowPtr objects point to the same object.} */
    template<typename T>
    bool operator==(const ShadowPtr<T>& lhs, const ShadowPtr<T>& rhs)
    {
        return lhs.Get() == rhs.Get();
    }

    /** @brief @~chinese{比较两个 ShadowPtr 是否指向不同对象。} @~english{Compare whether two ShadowPtr objects point to different objects.} */
    template<typename T>
    bool operator!=(const ShadowPtr<T>& lhs, const ShadowPtr<T>& rhs)
    {
        return lhs.Get() != rhs.Get();
    }

    /** @brief @~chinese{按原始指针地址比较两个 ShadowPtr。} @~english{Compare two ShadowPtr objects by raw pointer address.} */
    template<typename T>
    bool operator<(const ShadowPtr<T>& lhs, const ShadowPtr<T>& rhs)
    {
        return lhs.Get() < rhs.Get();
    }

    /** @brief @~chinese{按原始指针地址比较两个 ShadowPtr。} @~english{Compare two ShadowPtr objects by raw pointer address.} */
    template<typename T>
    bool operator<=(const ShadowPtr<T>& lhs, const ShadowPtr<T>& rhs)
    {
        return lhs.Get() <= rhs.Get();
    }

    /** @brief @~chinese{按原始指针地址比较两个 ShadowPtr。} @~english{Compare two ShadowPtr objects by raw pointer address.} */
    template<typename T>
    bool operator>(const ShadowPtr<T>& lhs, const ShadowPtr<T>& rhs)
    {
        return lhs.Get() > rhs.Get();
    }

    /** @brief @~chinese{按原始指针地址比较两个 ShadowPtr。} @~english{Compare two ShadowPtr objects by raw pointer address.} */
    template<typename T>
    bool operator>=(const ShadowPtr<T>& lhs, const ShadowPtr<T>& rhs)
    {
        return lhs.Get() >= rhs.Get();
    }

    /** @brief @~chinese{判断 ShadowPtr 是否非空。} @~english{Compare a ShadowPtr with nullptr.} */
    template<typename T>
    bool operator==(const ShadowPtr<T>& lhs, std::nullptr_t)
    {
        return lhs.Get() == nullptr;
    }

    /** @brief @~chinese{判断 ShadowPtr 是否非空。} @~english{Compare a ShadowPtr with nullptr.} */
    template<typename T>
    bool operator!=(const ShadowPtr<T>& lhs, std::nullptr_t)
    {
        return lhs.Get() != nullptr;
    }

    /** @brief @~chinese{按原始指针地址将 ShadowPtr 与 nullptr 比较。} @~english{Compare a ShadowPtr with nullptr by raw pointer address.} */
    template<typename T>
    bool operator<(const ShadowPtr<T>& lhs, std::nullptr_t)
    {
        return lhs.Get() < nullptr;
    }

    /** @brief @~chinese{按原始指针地址将 ShadowPtr 与 nullptr 比较。} @~english{Compare a ShadowPtr with nullptr by raw pointer address.} */
    template<typename T>
    bool operator<=(const ShadowPtr<T>& lhs, std::nullptr_t)
    {
        return lhs.Get() <= nullptr;
    }

    /** @brief @~chinese{按原始指针地址将 ShadowPtr 与 nullptr 比较。} @~english{Compare a ShadowPtr with nullptr by raw pointer address.} */
    template<typename T>
    bool operator>(const ShadowPtr<T>& lhs, std::nullptr_t)
    {
        return lhs.Get() > nullptr;
    }

    /** @brief @~chinese{按原始指针地址将 ShadowPtr 与 nullptr 比较。} @~english{Compare a ShadowPtr with nullptr by raw pointer address.} */
    template<typename T>
    bool operator>=(const ShadowPtr<T>& lhs, std::nullptr_t)
    {
        return lhs.Get() >= nullptr;
    }

    /** @brief @~chinese{判断 ShadowPtr 是否非空。} @~english{Compare nullptr with a ShadowPtr.} */
    template<typename T>
    bool operator==(std::nullptr_t, const ShadowPtr<T>& rhs)
    {
        return nullptr == rhs.Get();
    }

    /** @brief @~chinese{判断 ShadowPtr 是否非空。} @~english{Compare nullptr with a ShadowPtr.} */
    template<typename T>
    bool operator!=(std::nullptr_t, const ShadowPtr<T>& rhs)
    {
        return nullptr != rhs.Get();
    }

    /** @brief @~chinese{按原始指针地址将 nullptr 与 ShadowPtr 比较。} @~english{Compare nullptr with a ShadowPtr by raw pointer address.} */
    template<typename T>
    bool operator<(std::nullptr_t, const ShadowPtr<T>& rhs)
    {
        return nullptr < rhs.Get();
    }

    /** @brief @~chinese{按原始指针地址将 nullptr 与 ShadowPtr 比较。} @~english{Compare nullptr with a ShadowPtr by raw pointer address.} */
    template<typename T>
    bool operator<=(std::nullptr_t, const ShadowPtr<T>& rhs)
    {
        return nullptr <= rhs.Get();
    }

    /** @brief @~chinese{按原始指针地址将 nullptr 与 ShadowPtr 比较。} @~english{Compare nullptr with a ShadowPtr by raw pointer address.} */
    template<typename T>
    bool operator>(std::nullptr_t, const ShadowPtr<T>& rhs)
    {
        return nullptr > rhs.Get();
    }

    /** @brief @~chinese{按原始指针地址将 nullptr 与 ShadowPtr 比较。} @~english{Compare nullptr with a ShadowPtr by raw pointer address.} */
    template<typename T>
    bool operator>=(std::nullptr_t, const ShadowPtr<T>& rhs)
    {
        return nullptr >= rhs.Get();
    }

    /**
     * @brief @~chinese{为类提供从 this 生成 ShadowPtr 的能力。} @~english{Enables a class to create ShadowPtr instances from this.}
     * @tparam T @~chinese{派生类类型。} @~english{Derived class type.}
     */
    template<typename T>
    class EnableShadowFromThis
    {
    public:

        /** @brief @~chinese{生成指向当前对象的 ShadowPtr。} @~english{Create a ShadowPtr that references the current object.} */
        ShadowPtr<T> ShadowFromThis()
        {
            if (nullptr == m_pControlBlock)
            {
                m_pControlBlock = new detail::ControlBlock;
                m_pControlBlock->Data = static_cast<T*>(this);
            }

            ShadowPtr<T> shadow;
            shadow.m_pControlBlock = m_pControlBlock;
            ++m_pControlBlock->ShadowCount;
            return shadow;
        }

        /** @brief @~chinese{生成指向当前对象的只读 ShadowPtr。} @~english{Create a const ShadowPtr that references the current object.} */
        ShadowPtr<const T> ShadowFromThis() const
        {
            if (nullptr == m_pControlBlock)
            {
                m_pControlBlock = new detail::ControlBlock;
                m_pControlBlock->Data = const_cast<T*>(static_cast<const T*>(this));
            }

            ShadowPtr<T> shadow;
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

        ~EnableShadowFromThis()
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
     * @brief @~chinese{拥有对象生命周期的智能指针。} @~english{Owning smart pointer that manages an object's lifetime.}
     * @tparam VT @~chinese{对象类型。} @~english{Object type.}
     * @tparam DT @~chinese{删除器类型。} @~english{Deleter type.}
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
        using DeleterType = DT;

        template<typename>
        friend class ShadowPtr;

        template<typename, typename>
        friend class OwnerPtr;

    public:

        /**
         * @brief @~chinese{原地构造对象并返回 OwnerPtr。} @~english{Construct an object in place and return an OwnerPtr.}
         * @tparam ArgTypes @~chinese{构造参数类型。} @~english{Constructor argument types.}
         * @param args @~chinese{传递给对象构造函数的参数。} @~english{Arguments forwarded to the object constructor.}
         * @return @~chinese{管理新对象的 OwnerPtr。} @~english{OwnerPtr managing the newly created object.}
         */
        template<typename... ArgTypes, std::enable_if_t<std::is_constructible_v<VT, ArgTypes...>, int> = 0>
        static OwnerPtr Create(ArgTypes&&... args)
        {
            auto pRaw = new VT{ std::forward<ArgTypes>(args)... };
            OwnerPtr<VT, DT> owner{ pRaw };
            return owner;
        }

        /** @brief @~chinese{构造空的 OwnerPtr。} @~english{Construct an empty OwnerPtr.} */
        OwnerPtr() = default;

        /** @brief @~chinese{由 nullptr 构造空的 OwnerPtr。} @~english{Construct an empty OwnerPtr from nullptr.} */
        OwnerPtr(std::nullptr_t) {}

        /**
         * @brief @~chinese{从原始指针构造 OwnerPtr。} @~english{Construct an OwnerPtr from a raw pointer.}
         * @tparam VT2 @~chinese{源对象类型。} @~english{Source object type.}
         * @tparam DT2 @~chinese{源删除器类型。} @~english{Source deleter type.}
         * @param pOther @~chinese{要接管的原始指针。} @~english{Raw pointer to take ownership of.}
         */
        template<typename VT2 = VT, typename DT2 = DT, std::enable_if_t<std::is_convertible_v<VT2*, VT*>, int> = 0>
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

        /**
         * @brief @~chinese{从可转换类型的 OwnerPtr 移动构造。} @~english{Move-construct from an OwnerPtr of a convertible type.}
         * @tparam VT2 @~chinese{源对象类型。} @~english{Source object type.}
         * @tparam DT2 @~chinese{源删除器类型。} @~english{Source deleter type.}
         * @param other @~chinese{源 OwnerPtr。} @~english{Source OwnerPtr.}
         */
        template<typename VT2 = VT, typename DT2 = detail::DefaultDeleter<VT2>, std::enable_if_t<std::is_convertible_v<VT2*, VT*>, int> = 0>
        OwnerPtr(OwnerPtr<VT2, DT2>&& other) noexcept
        {
            m_pControlBlock = other.m_pControlBlock;
            other.m_pControlBlock = nullptr;
        }

        /**
         * @brief @~chinese{从可转换类型的 OwnerPtr 移动赋值。} @~english{Move-assign from an OwnerPtr of a convertible type.}
         * @tparam VT2 @~chinese{源对象类型。} @~english{Source object type.}
         * @tparam DT2 @~chinese{源删除器类型。} @~english{Source deleter type.}
         * @param other @~chinese{源 OwnerPtr。} @~english{Source OwnerPtr.}
         * @return @~chinese{当前对象引用。} @~english{Reference to this object.}
         */
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

        /** @brief @~chinese{析构 OwnerPtr 并在需要时销毁所管理对象。} @~english{Destroy this OwnerPtr and delete the managed object when needed.} */
        ~OwnerPtr() { Destroy(); }

        /** @brief @~chinese{返回与当前对象关联的 ShadowPtr 数量。} @~english{Get the number of ShadowPtr instances associated with this object.} */
        std::size_t ShadowCount() const { return m_pControlBlock ? m_pControlBlock->ShadowCount : 0; }

        /** @brief @~chinese{获取可写原始指针。} @~english{Get the writable raw pointer.} */
        Pointer Get() { return m_pControlBlock ? static_cast<Pointer>(m_pControlBlock->Data) : nullptr; }

        /** @brief @~chinese{获取只读原始指针。} @~english{Get the const raw pointer.} */
        ConstPointer Get() const { return m_pControlBlock ? static_cast<ConstPointer>(m_pControlBlock->Data) : nullptr; }

        /** @brief @~chinese{获取可写数据指针。} @~english{Get the writable data pointer.} */
        Pointer Data() { return Get(); }

        /** @brief @~chinese{获取只读数据指针。} @~english{Get the const data pointer.} */
        ConstPointer Data() const { return Get(); }

        /** @brief @~chinese{解引用获得对象引用。} @~english{Dereference to obtain the managed object.} */
        Reference operator*() { return *Get(); }

        /** @brief @~chinese{解引用获得对象常量引用。} @~english{Dereference to obtain the const managed object.} */
        ConstReference operator*() const { return *Get(); }

        /** @brief @~chinese{判断当前 OwnerPtr 是否持有对象。} @~english{Check whether this OwnerPtr currently owns an object.} */
        operator bool() const { return nullptr != Get(); }

        /** @brief @~chinese{判断当前 OwnerPtr 是否为空。} @~english{Check whether this OwnerPtr is null.} */
        bool IsNull() const { return nullptr == Get(); }

        /** @brief @~chinese{释放当前对象所有权并将指针设为空。} @~english{Release ownership of the current object and reset this pointer to null.} */
        void Nullify() { Destroy(); }

        /** @brief @~chinese{重置为空。} @~english{Reset this OwnerPtr to empty.} */
        void Reset() { Destroy(); }

        /** @brief @~chinese{使用 nullptr 重置为空。} @~english{Reset this OwnerPtr to empty using nullptr.} */
        void Reset(std::nullptr_t) { Destroy(); }

        /**
         * @brief @~chinese{接管新的原始指针。} @~english{Take ownership of a new raw pointer.}
         * @tparam VT2 @~chinese{源对象类型。} @~english{Source object type.}
         * @tparam DT2 @~chinese{源删除器类型。} @~english{Source deleter type.}
         * @param pNew @~chinese{新的原始指针。} @~english{New raw pointer.}
         */
        template<typename VT2 = VT, typename DT2 = DT, std::enable_if_t<std::is_convertible_v<VT2*, VT*>, int> = 0>
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

        /** @brief @~chinese{释放对象所有权并返回原始指针。} @~english{Release ownership and return the raw pointer.} */
        [[nodiscard]] Pointer Release()
        {
            Pointer pReleased = Get();
            if (m_pControlBlock)
            {
                m_pControlBlock->Data = nullptr;
                if (0 == m_pControlBlock->ShadowCount)
                {
                    delete m_pControlBlock;
                }
                m_pControlBlock = nullptr;
            }
            return pReleased;
        }

        /**
         * @brief @~chinese{与另一个 OwnerPtr 交换状态。} @~english{Swap state with another OwnerPtr.}
         * @tparam DT2 @~chinese{另一个删除器类型。} @~english{Other deleter type.}
         * @param other @~chinese{要交换的 OwnerPtr。} @~english{OwnerPtr to swap with.}
         */
        template<typename DT2 = DT>
        void Swap(OwnerPtr<VT, DT2>& other) noexcept
        {
            std::swap(m_pControlBlock, other.m_pControlBlock);
        }

        /**
         * @brief @~chinese{转移所有权到另一个 OwnerPtr 类型。} @~english{Transfer ownership to another OwnerPtr type.}
         * @tparam VT2 @~chinese{目标对象类型。} @~english{Target object type.}
         * @tparam DT2 @~chinese{目标删除器类型。} @~english{Target deleter type.}
         * @return @~chinese{新的 OwnerPtr。} @~english{New OwnerPtr receiving ownership.}
         */
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

        /**
         * @brief @~chinese{通过 dynamic_cast 转换所有权类型。} @~english{Cast the owned object type with dynamic_cast while transferring ownership.}
         * @tparam VT2 @~chinese{目标对象类型。} @~english{Target object type.}
         * @tparam DT2 @~chinese{目标删除器类型。} @~english{Target deleter type.}
         * @return @~chinese{转换后的 OwnerPtr；若转换失败则返回空指针。} @~english{Casted OwnerPtr, or an empty pointer if the cast fails.}
         */
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

        /**
         * @brief @~chinese{生成只观察当前对象的 ShadowPtr。} @~english{Create a ShadowPtr that only observes the current object.}
         * @tparam VT2 @~chinese{ShadowPtr 目标类型。} @~english{Target ShadowPtr type.}
         * @return @~chinese{新的 ShadowPtr。} @~english{Newly created ShadowPtr.}
         */
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

        /**
         * @brief @~chinese{生成只观察当前对象的 ShadowPtr。} @~english{Create a ShadowPtr that only observes the current object.}
         * @tparam VT2 @~chinese{ShadowPtr 目标类型。} @~english{Target ShadowPtr type.}
         * @return @~chinese{新的 ShadowPtr。} @~english{Newly created ShadowPtr.}
         */
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
                }
                m_pControlBlock = nullptr;
            }
        }

    private:

        detail::ControlBlock* m_pControlBlock{ nullptr };
    };

    /** @brief @~chinese{比较两个 OwnerPtr 是否指向同一对象。} @~english{Compare whether two OwnerPtr objects point to the same object.} */
    template<typename VT, typename DT1 = detail::DefaultDeleter<VT>, typename DT2 = detail::DefaultDeleter<VT>>
    bool operator==(const OwnerPtr<VT, DT1>& lhs, const OwnerPtr<VT, DT2>& rhs)
    {
        return lhs.Get() == rhs.Get();
    }

    /** @brief @~chinese{比较两个 OwnerPtr 是否指向不同对象。} @~english{Compare whether two OwnerPtr objects point to different objects.} */
    template<typename VT, typename DT1 = detail::DefaultDeleter<VT>, typename DT2 = detail::DefaultDeleter<VT>>
    bool operator!=(const OwnerPtr<VT, DT1>& lhs, const OwnerPtr<VT, DT2>& rhs)
    {
        return lhs.Get() != rhs.Get();
    }

    /** @brief @~chinese{按原始指针地址比较两个 OwnerPtr。} @~english{Compare two OwnerPtr objects by raw pointer address.} */
    template<typename VT, typename DT1 = detail::DefaultDeleter<VT>, typename DT2 = detail::DefaultDeleter<VT>>
    bool operator<(const OwnerPtr<VT, DT1>& lhs, const OwnerPtr<VT, DT2>& rhs)
    {
        return lhs.Get() < rhs.Get();
    }

    /** @brief @~chinese{按原始指针地址比较两个 OwnerPtr。} @~english{Compare two OwnerPtr objects by raw pointer address.} */
    template<typename VT, typename DT1 = detail::DefaultDeleter<VT>, typename DT2 = detail::DefaultDeleter<VT>>
    bool operator>(const OwnerPtr<VT, DT1>& lhs, const OwnerPtr<VT, DT2>& rhs)
    {
        return lhs.Get() > rhs.Get();
    }

    /** @brief @~chinese{按原始指针地址比较两个 OwnerPtr。} @~english{Compare two OwnerPtr objects by raw pointer address.} */
    template<typename VT, typename DT1 = detail::DefaultDeleter<VT>, typename DT2 = detail::DefaultDeleter<VT>>
    bool operator<=(const OwnerPtr<VT, DT1>& lhs, const OwnerPtr<VT, DT2>& rhs)
    {
        return lhs.Get() <= rhs.Get();
    }

    /** @brief @~chinese{按原始指针地址比较两个 OwnerPtr。} @~english{Compare two OwnerPtr objects by raw pointer address.} */
    template<typename VT, typename DT1 = detail::DefaultDeleter<VT>, typename DT2 = detail::DefaultDeleter<VT>>
    bool operator>=(const OwnerPtr<VT, DT1>& lhs, const OwnerPtr<VT, DT2>& rhs)
    {
        return lhs.Get() >= rhs.Get();
    }

    /** @brief @~chinese{判断 OwnerPtr 是否非空。} @~english{Compare an OwnerPtr with nullptr.} */
    template<typename VT, typename DT = detail::DefaultDeleter<VT>>
    bool operator==(const OwnerPtr<VT, DT>& lhs, std::nullptr_t)
    {
        return lhs.Get() == nullptr;
    }

    /** @brief @~chinese{判断 OwnerPtr 是否非空。} @~english{Compare an OwnerPtr with nullptr.} */
    template<typename VT, typename DT = detail::DefaultDeleter<VT>>
    bool operator!=(const OwnerPtr<VT, DT>& lhs, std::nullptr_t)
    {
        return lhs.Get() != nullptr;
    }

    /** @brief @~chinese{按原始指针地址将 OwnerPtr 与 nullptr 比较。} @~english{Compare an OwnerPtr with nullptr by raw pointer address.} */
    template<typename VT, typename DT = detail::DefaultDeleter<VT>>
    bool operator<(const OwnerPtr<VT, DT>& lhs, std::nullptr_t)
    {
        return lhs.Get() < nullptr;
    }

    /** @brief @~chinese{按原始指针地址将 OwnerPtr 与 nullptr 比较。} @~english{Compare an OwnerPtr with nullptr by raw pointer address.} */
    template<typename VT, typename DT = detail::DefaultDeleter<VT>>
    bool operator>(const OwnerPtr<VT, DT>& lhs, std::nullptr_t)
    {
        return lhs.Get() > nullptr;
    }

    /** @brief @~chinese{按原始指针地址将 OwnerPtr 与 nullptr 比较。} @~english{Compare an OwnerPtr with nullptr by raw pointer address.} */
    template<typename VT, typename DT = detail::DefaultDeleter<VT>>
    bool operator<=(const OwnerPtr<VT, DT>& lhs, std::nullptr_t)
    {
        return lhs.Get() <= nullptr;
    }

    /** @brief @~chinese{按原始指针地址将 OwnerPtr 与 nullptr 比较。} @~english{Compare an OwnerPtr with nullptr by raw pointer address.} */
    template<typename VT, typename DT = detail::DefaultDeleter<VT>>
    bool operator>=(const OwnerPtr<VT, DT>& lhs, std::nullptr_t)
    {
        return lhs.Get() >= nullptr;
    }

    /** @brief @~chinese{判断 OwnerPtr 是否非空。} @~english{Compare nullptr with an OwnerPtr.} */
    template<typename VT, typename DT = detail::DefaultDeleter<VT>>
    bool operator==(std::nullptr_t, const OwnerPtr<VT, DT>& rhs)
    {
        return nullptr == rhs.Get();
    }

    /** @brief @~chinese{判断 OwnerPtr 是否非空。} @~english{Compare nullptr with an OwnerPtr.} */
    template<typename VT, typename DT = detail::DefaultDeleter<VT>>
    bool operator!=(std::nullptr_t, const OwnerPtr<VT, DT>& rhs)
    {
        return nullptr != rhs.Get();
    }

    /** @brief @~chinese{按原始指针地址将 nullptr 与 OwnerPtr 比较。} @~english{Compare nullptr with an OwnerPtr by raw pointer address.} */
    template<typename VT, typename DT = detail::DefaultDeleter<VT>>
    bool operator<(std::nullptr_t, const OwnerPtr<VT, DT>& rhs)
    {
        return nullptr < rhs.Get();
    }

    /** @brief @~chinese{按原始指针地址将 nullptr 与 OwnerPtr 比较。} @~english{Compare nullptr with an OwnerPtr by raw pointer address.} */
    template<typename VT, typename DT = detail::DefaultDeleter<VT>>
    bool operator>(std::nullptr_t, const OwnerPtr<VT, DT>& rhs)
    {
        return nullptr > rhs.Get();
    }

    /** @brief @~chinese{按原始指针地址将 nullptr 与 OwnerPtr 比较。} @~english{Compare nullptr with an OwnerPtr by raw pointer address.} */
    template<typename VT, typename DT = detail::DefaultDeleter<VT>>
    bool operator<=(std::nullptr_t, const OwnerPtr<VT, DT>& rhs)
    {
        return nullptr <= rhs.Get();
    }

    /** @brief @~chinese{按原始指针地址将 nullptr 与 OwnerPtr 比较。} @~english{Compare nullptr with an OwnerPtr by raw pointer address.} */
    template<typename VT, typename DT = detail::DefaultDeleter<VT>>
    bool operator>=(std::nullptr_t, const OwnerPtr<VT, DT>& rhs)
    {
        return nullptr >= rhs.Get();
    }
}

namespace std
{
    template<typename T>
    void swap(prtm::ShadowPtr<T>& lhs, prtm::ShadowPtr<T>& rhs) noexcept
    {
        lhs.Swap(rhs);
    }

    template<typename T>
    struct hash<prtm::ShadowPtr<T>>
    {
        std::size_t operator()(const prtm::ShadowPtr<T>& obj) const
        {
            return std::hash<typename prtm::ShadowPtr<T>::ConstPointer>{}(obj.Get());
        }
    };

    template<typename T>
    struct equal_to<prtm::ShadowPtr<T>>
    {
        bool operator()(const prtm::ShadowPtr<T>& lhs, const prtm::ShadowPtr<T>& rhs) const
        {
            return lhs == rhs;
        }
    };

    template<typename T>
    struct not_equal_to<prtm::ShadowPtr<T>>
    {
        bool operator()(const prtm::ShadowPtr<T>& lhs, const prtm::ShadowPtr<T>& rhs) const
        {
            return lhs != rhs;
        }
    };

    template<typename T>
    struct less<prtm::ShadowPtr<T>>
    {
        bool operator()(const prtm::ShadowPtr<T>& lhs, const prtm::ShadowPtr<T>& rhs) const
        {
            return lhs < rhs;
        }
    };

    template<typename T>
    struct less_equal<prtm::ShadowPtr<T>>
    {
        bool operator()(const prtm::ShadowPtr<T>& lhs, const prtm::ShadowPtr<T>& rhs) const
        {
            return lhs <= rhs;
        }
    };

    template<typename T>
    struct greater<prtm::ShadowPtr<T>>
    {
        bool operator()(const prtm::ShadowPtr<T>& lhs, const prtm::ShadowPtr<T>& rhs) const
        {
            return lhs > rhs;
        }
    };

    template<typename T>
    struct greater_equal<prtm::ShadowPtr<T>>
    {
        bool operator()(const prtm::ShadowPtr<T>& lhs, const prtm::ShadowPtr<T>& rhs) const
        {
            return lhs >= rhs;
        }
    };

    template<typename VT, typename DT1 = prtm::detail::DefaultDeleter<VT>, typename DT2 = prtm::detail::DefaultDeleter<VT>>
    void swap(prtm::OwnerPtr<VT, DT1>& lhs, prtm::OwnerPtr<VT, DT2>& rhs) noexcept
    {
        lhs.Swap(rhs);
    }

    template<typename VT, typename DT>
    struct hash<prtm::OwnerPtr<VT, DT>>
    {
        std::size_t operator()(const prtm::OwnerPtr<VT, DT>& obj) const
        {
            return std::hash<typename prtm::OwnerPtr<VT, DT>::ConstPointer>{}(obj.Get());
        }
    };

    template<typename VT, typename DT>
    struct equal_to<prtm::OwnerPtr<VT, DT>>
    {
        bool operator()(const prtm::OwnerPtr<VT, DT>& lhs, const prtm::OwnerPtr<VT, DT>& rhs) const
        {
            return lhs == rhs;
        }
    };

    template<typename VT, typename DT>
    struct not_equal_to<prtm::OwnerPtr<VT, DT>>
    {
        bool operator()(const prtm::OwnerPtr<VT, DT>& lhs, const prtm::OwnerPtr<VT, DT>& rhs) const
        {
            return !(lhs == rhs);
        }
    };

    template<typename VT, typename DT>
    struct less<prtm::OwnerPtr<VT, DT>>
    {
        bool operator()(const prtm::OwnerPtr<VT, DT>& lhs, const prtm::OwnerPtr<VT, DT>& rhs) const
        {
            return lhs < rhs;
        }
    };

    template<typename VT, typename DT>
    struct greater<prtm::OwnerPtr<VT, DT>>
    {
        bool operator()(const prtm::OwnerPtr<VT, DT>& lhs, const prtm::OwnerPtr<VT, DT>& rhs) const
        {
            return lhs > rhs;
        }
    };

    template<typename VT, typename DT>
    struct less_equal<prtm::OwnerPtr<VT, DT>>
    {
        bool operator()(const prtm::OwnerPtr<VT, DT>& lhs, const prtm::OwnerPtr<VT, DT>& rhs) const
        {
            return lhs <= rhs;
        }
    };

    template<typename VT, typename DT>
    struct greater_equal<prtm::OwnerPtr<VT, DT>>
    {
        bool operator()(const prtm::OwnerPtr<VT, DT>& lhs, const prtm::OwnerPtr<VT, DT>& rhs) const
        {
            return lhs >= rhs;
        }
    };
}
