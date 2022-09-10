#pragma once

#include <algorithm>
#include <any>
#include <concepts>
#include <functional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

class has_properties;

class property_base
{
public:
    property_base(const property_base&) = delete;
    property_base(property_base&&) = delete;
    property_base& operator=(const property_base&) = delete;
    property_base& operator=(property_base&&) = delete;
    virtual ~property_base() = default;

    void from_any(const std::any& value)
    {
        _setter(value);
    }

    [[nodiscard]] std::any as_any() const
    {
        return _getter();
    }

protected:
    property_base(std::string name,
        has_properties* owner,
        std::function<std::any()> getter,
        std::function<void(has_properties* owner, const std::any&)> setter);

private:
    std::function<std::any()> _getter;
    std::function<void(const std::any&)> _setter;
};

template<std::copyable T>
class property : public property_base
{
public:
    using value_type = T;
    property(std::string name, has_properties* owner);

    using set_function = std::function<void(has_properties* owner, const T&)>;
    property(std::string name, has_properties* owner, set_function setter);

    property& operator=(const T& value)
    {
        _value = value;
    }

    property& operator=(T&& value)
    {
        thread_local bool coming_from_setter = false;
        if (_setter && !coming_from_setter)
        {
            coming_from_setter = true;
            _setter(this, std::move(value));
            coming_from_setter = false;
        }
        else
        {
            _value = std::move(value);
        }
    }

    property& operator=(const std::any& value)
    {
        _value = std::any_cast<T>(value);
    }

    property& operator=(std::any&& value)
    {
        _value = std::any_cast<T>(std::move(value));
    }

    operator const T&() const
    {
        return _value;
    }

private:
    T _value {};
    set_function _setter;
};

template<std::copyable T>
inline property<T>::property(std::string name, has_properties* owner) :
    property_base(
        std::move(name),
        owner,
        [this]() { return _value; },
        [this](has_properties* owner, const std::any& value) { *this = std::any_cast<T>(value); })
{
}

template<std::copyable T>
inline property<T>::property(std::string name, has_properties* owner, property<T>::set_function setter) : property<T>::property(name, owner)
{
    _setter(std::move(setter))
}

class has_properties
{
    friend class property_base;

public:
    [[nodiscard]] const property_base& get_property(const std::string& name) const
    {
        return _property_by_name(name);
    }

    property_base& get_property(const std::string& name)
    {
        return const_cast<property_base&>(std::as_const(*this).get_property(name));
    }

    void set_property(const std::string& name, const std::any& value)
    {
        auto& prop = _property_by_name(name);
        prop.from_any(value);
    }

private:
    [[nodiscard]] property_base& _property_by_name(const std::string& name) const
    {
        auto found = std::ranges::find_if(_meta_props, [&name](const meta_property& prop) { return prop.name == name; });
        if (found == _meta_props.end())
        {
            throw std::runtime_error("No such property.");
        }
        return *found->instances.at(this);
    }

    struct meta_property
    {
        std::unordered_map<const has_properties*, property_base*> instances;
        std::string name;
    };

    std::vector<meta_property> _meta_props;
};

inline property_base::property_base(
    std::string name, has_properties* owner, std::function<std::any()> getter, std::function<void(const std::any&)> setter) :
    _getter { std::move(getter) },
    _setter { std::move(setter) } {
        // owner->_meta_props.push_back();
    };

// NOLINTBEGIN(bugprone-macro-parentheses)
#define GET_MACRO(_1, _2, _3, _4, NAME, ...) NAME

// PROPERTY(name, type)
// PROPERTY(name, type, setter)
// PROPERTY(name, type, getter, setter)
#define PROPERTY(...)                                                                                                                      \
    GET_MACRO(__VA_ARGS__, PROPERTY_GET_SET, PROPERTY_SET, PROPERTY_PLAIN, UNUSED_1_ARG)                                                   \
    (__VA_ARGS__)
#define PROPERTY_PLAIN(name, type) property<type> name { #name, this };
#define PROPERTY_SET(name, type, setter) property<type> name { #name, this, [this](type value) { setter(value); } };
#define PROPERTY_GET_SET(name, type, getter, setter) property<type> name { #name, this };

// NOLINTEND(bugprone-macro-parentheses)