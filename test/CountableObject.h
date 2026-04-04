#pragma once

template<typename TC>
class CountableObject
{
public:

    static inline int Balance{ 0 };

    CountableObject()
    {
        ++Balance;
    }

    CountableObject(const CountableObject&)
    {
        ++Balance;
    }

    CountableObject(CountableObject&&) noexcept
    {
        ++Balance;
    }

    virtual ~CountableObject()
    {
        --Balance;
    }
};
