#pragma once
#include "DynamicArray.h"
#include "IEnumerable.h"
#include "LinkedList.h"
#include "Option.h"
#include <functional>
#include <stdexcept>

template<typename T>
class Sequence : public IEnumerable<T> {
private:
    class SequenceEnumerator : public IEnumerator<T> {
    private:
        const Sequence<T>* sequence;
        int index;

    public:
        explicit SequenceEnumerator(const Sequence<T>* sequence) : sequence(sequence), index(-1) {}

        T GetCurrent() const override {
            if (index < 0 || index >= sequence->GetLength()) {
                throw std::out_of_range("Index out of range");
            }
            return sequence->Get(index);
        }

        bool MoveNext() override {
            if (index + 1 < sequence->GetLength()) {
                index++;
                return true;
            }
            return false;
        }

        void Reset() override {
            index = -1;
        }
    };

public:
    virtual ~Sequence() = default;

    virtual Sequence<T>* Clone() const = 0;
    virtual Sequence<T>* Instance() const = 0;
    
    virtual T GetFirst() const = 0;
    virtual T GetLast() const = 0;
    virtual T Get(int index) const = 0;
    virtual int GetLength() const = 0;
    
    virtual Sequence<T>* Append(T item) = 0;
    virtual Sequence<T>* Prepend(T item) = 0;
    virtual Sequence<T>* InsertAt(T item, int index) = 0;
    virtual Sequence<T>* Concat(const Sequence<T>* other) = 0;
    virtual Sequence<T>* GetSubsequence(int startIndex, int endIndex) = 0;
    
    // Map-Reduce
    virtual Sequence<T>* Map(std::function<T(T)> func) = 0;
    virtual Sequence<T>* Where(std::function<bool(T)> predicate) = 0;
    virtual T Reduce(std::function<T(T, T)> func, T initial) = 0;
    
    virtual Option<T> TryGetFirst(std::function<bool(T)> predicate = nullptr) = 0;
    virtual Option<T> TryGetLast(std::function<bool(T)> predicate = nullptr) = 0;
    
    IEnumerator<T>* GetEnumerator() const override {
        return new SequenceEnumerator(this);
    }

    T operator[](int index) const {
        return Get(index);
    }

    virtual void Print() const = 0;
};
