#pragma once
#include <iostream>

class Bit {
private:
    bool value;

public:
    Bit(bool v = false) : value(v) {}
    
    bool GetValue() const { return value; }
    
    void SetValue(bool v) { value = v; }
    
    Bit operator&(const Bit& other) const {
        return Bit(value && other.value);
    }
    
    Bit operator|(const Bit& other) const {
        return Bit(value || other.value);
    }
    
    Bit operator^(const Bit& other) const {
        return Bit(value != other.value);
    }
    
    Bit operator~() const {
        return Bit(!value);
    }
    
    Bit& operator=(bool v) {
        value = v;
        return *this;
    }
    
    operator bool() const { return value; }
    
    bool operator==(const Bit& other) const {
        return value == other.value;
    }
    
    bool operator!=(const Bit& other) const {
        return value != other.value;
    }
    
    friend std::ostream& operator<<(std::ostream& os, const Bit& bit) {
        os << (bit.value ? 1 : 0);
        return os;
    }
};