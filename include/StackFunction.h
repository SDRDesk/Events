#pragma once
/*/
#include <cassert>
#include <cstring>
#include <utility>
#include <type_traits>

template <typename Signature, std::size_t BufferSize = 64>
class StackFunction;

template <typename R, typename... Args, std::size_t BufferSize>
class StackFunction<R(Args...), BufferSize> {
public:
    StackFunction() noexcept : invoker(nullptr) {}

    template <typename F>
    StackFunction(F&& f) {
        static_assert(sizeof(F) <= BufferSize, "Callable object too large for StackFunction buffer");
        invoker = &invokeImpl<std::decay_t<F>>;
        new (buffer) std::decay_t<F>(std::forward<F>(f));
    }

    StackFunction(const StackFunction& other) {
        if (other.invoker) {
            invoker = other.invoker;
            other.invoker(Operation::Copy, buffer, other.buffer);
        } else {
            invoker = nullptr;
        }
    }

    StackFunction(StackFunction&& other) noexcept {
        if (other.invoker) {
            invoker = other.invoker;
            other.invoker(Operation::Move, buffer, other.buffer);
            other.invoker = nullptr;
        } else {
            invoker = nullptr;
        }
    }

    StackFunction& operator=(const StackFunction& other) {
        if (this != &other) {
            if (invoker) {
                invoker(Operation::Destroy, buffer, nullptr);
            }
            if (other.invoker) {
                invoker = other.invoker;
                other.invoker(Operation::Copy, buffer, other.buffer);
            } else {
                invoker = nullptr;
            }
        }
        return *this;
    }

    StackFunction& operator=(StackFunction&& other) noexcept {
        if (this != &other) {
            if (invoker) {
                invoker(Operation::Destroy, buffer, nullptr);
            }
            if (other.invoker) {
                invoker = other.invoker;
                other.invoker(Operation::Move, buffer, other.buffer);
                other.invoker = nullptr;
            } else {
                invoker = nullptr;
            }
        }
        return *this;
    }

    ~StackFunction() {
        if (invoker) {
            invoker(Operation::Destroy, buffer, nullptr);
        }
    }

    R operator()(Args... args) const {
        assert(invoker);
        return invoker(Operation::Invoke, const_cast<void*>(static_cast<const void*>(buffer)), std::forward<Args>(args)...);
    }

    explicit operator bool() const noexcept {
        return invoker != nullptr;
    }

    void swap(StackFunction& other) noexcept {
        if (this != &other) {
            StackFunction temp(std::move(*this));
            *this = std::move(other);
            other = std::move(temp);
        }
    }

    bool operator==(const StackFunction& other) const noexcept {
        if (invoker != other.invoker) {
            return false;
        }
        if (!invoker) {
            return true;
        }
        return invoker(Operation::Equals, const_cast<void*>(static_cast<const void*>(buffer)), const_cast<void*>(static_cast<const void*>(other.buffer)));
    }

private:
    enum class Operation { Invoke, Copy, Move, Destroy, Equals };

    using Invoker = R(*)(Operation, void*, Args&&...);

    template <typename F>
    static R invokeImpl(Operation op, void* buffer, Args&&... args) {
        switch (op) {
            case Operation::Invoke:
                return (*reinterpret_cast<F*>(buffer))(std::forward<Args>(args)...);
            case Operation::Copy:
                new (buffer) F(*reinterpret_cast<F*>(args[0]));
                break;
            case Operation::Move:
                new (buffer) F(std::move(*reinterpret_cast<F*>(args[0])));
                reinterpret_cast<F*>(args[0])->~F();
                break;
            case Operation::Destroy:
                reinterpret_cast<F*>(buffer)->~F();
                break;
            case Operation::Equals:
                return *reinterpret_cast<F*>(buffer) == *reinterpret_cast<F*>(args[0]);
        }
        return R();
    }

    alignas(std::max_align_t) char buffer[BufferSize];
    Invoker invoker;
};

// Non-member swap function
template <typename R, typename... Args, std::size_t BufferSize>
void swap(StackFunction<R(Args...), BufferSize>& lhs, StackFunction<R(Args...), BufferSize>& rhs) noexcept {
    lhs.swap(rhs);
}
    /*/