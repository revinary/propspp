#pragma once
#include <concepts>
#include <type_traits>
#include <functional>
#include <vector>

namespace propspp
{

template<typename R, typename Fn, typename... Args>
concept invocable = std::is_invocable_v<R, Fn, Args...>;

template<std::copyable T>
class bindable_value;

class bindable_base
{
public:
    using dirty_callback = std::function<void()>;

    template<std::copyable T>
    static void connect(bindable_value<T>& source, bindable_value<T>& target)
    {
        source._dirty_callbacks.push_back([target]() { target.set_dirty(); });
    }

protected:
    void start_binding()
    {
        _binding_target = this;
    }

    bool binding_in_progress()
    {
        return _binding_target;
    }

    void stop_binding()
    {
        _binding_target = nullptr;
    }

    bindable_base* binding_target()
    {
        return _binding_target;
    }

    void set_dirty(bool dirty = true)
    {
        _is_binding_dirty = dirty;
    }

    void notify_dirty_watchers()
    {
        for (const auto& callback : _dirty_callbacks)
        {
            callback();
        }
    }

private:
    static thread_local bindable_base* _binding_target;
    bool _is_binding_dirty = false;
    std::vector<dirty_callback> _dirty_callbacks;
};

thread_local bindable_base* bindable_base::_binding_target = nullptr;

template<std::copyable T>
class bindable_value : public bindable_base
{
public:
    using value_type = T;
    using value_changed_callback = std::function<void(const T& value)>;

    bindable_value& operator=(const T& value)
    {
        if (_value != value)
        {
            _value = value;
            notify_watchers();
        }
    }

    const T& value()
    {
        if (binding_in_progress())
        {
            binding_target();
        }
        return _value;
    }

    operator const T&() const
    {
        return value();
    }

    template<invocable<T> Callable>
    void setBinding(Callable callable)
    {
        start_binding();
        _value = callable();
        stop_binding();
        _binding_expression = callable;
    }

private:
    void notify_watchers()
    {
        notify_dirty_watchers();
        for (const auto& callback : _callbacks)
        {
            callback(_value);
        }
    }

    T _value {};

    std::function<T()> _binding_expression;
    std::vector<value_changed_callback> _callbacks;
};

} // namespace propspp