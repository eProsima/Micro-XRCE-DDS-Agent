#ifndef _OPTIONAL_H_
#define _OPTIONAL_H_

namespace eprosima {

template <typename T>
class Optional
{
  public:
    Optional() = default;

    explicit Optional(const T& value)
    {
        present_ = true;
        data_    = value;
    }

    explicit Optional(T&& value)
    {
        present_ = true;
        data_    = std::move(value);
    }

    Optional& operator=(const T& value)
    {
        present_ = true;
        data_    = value;
        return *this;
    }

    Optional& operator=(T&& value)
    {
        present_ = true;
        data_    = std::move(value);
        return *this;
    }

    const T* operator->() const
    {
        return present_ ? &data_ : nullptr;
    }

    T* operator->()
    {
        return present_ ? &data_ : nullptr;
    }

    const T& operator*() const &
    {
        return data_;
    }

    T& operator*() &
    {
        return data_;
    }

    const T&& operator*() const &&
    {
        return std::move(data_);
    }

    T&& operator*() &&
    {
        return std::move(data_);
    }

    explicit operator bool() const noexcept
    {
        return present_;
    }

  private:
    bool present_ = false;
    T data_;
};
} // namespace eprosima
#endif