#pragma once
#include "IEnumerator.h"

template<typename T>
class IEnumerable {
public:
    virtual ~IEnumerable() = default;
    virtual IEnumerator<T>* GetEnumerator() const = 0;
};
