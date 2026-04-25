#pragma once
#include <stdexcept>

template<typename T>
class Option {
private:
    T value;
    bool hasValue;
public:
    Option() : hasValue(false) {}
    Option(T val) : value(val), hasValue(true) {}
    
    bool IsSome() const { return hasValue; }
    bool IsNone() const { return !hasValue; }
    
    T GetValue() const {
        if (!hasValue) throw std::runtime_error("Option is None");
        return value;
    }
    
    static Option<T> None() { return Option<T>(); }
    static Option<T> Some(T val) { return Option<T>(val); }
};
