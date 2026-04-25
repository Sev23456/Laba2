#pragma once
#include "Bit.h"
#include "DynamicArray.h"
#include "Sequence.h"
#include <algorithm>
#include <stdexcept>

class BitSequence : public Sequence<Bit> {
private:
    DynamicArray<Bit>* bits;
    
public:
    BitSequence() : bits(new DynamicArray<Bit>()) {}
    BitSequence(int size) : bits(new DynamicArray<Bit>(size)) {}
    BitSequence(const BitSequence& other) : bits(new DynamicArray<Bit>(*other.bits)) {}

    BitSequence& operator=(const BitSequence& other) {
        if (this != &other) {
            DynamicArray<Bit>* newBits = new DynamicArray<Bit>(*other.bits);
            delete bits;
            bits = newBits;
        }
        return *this;
    }

    ~BitSequence() override {
        delete bits;
    }

    BitSequence* Clone() const override {
        return new BitSequence(*this);
    }

    Sequence<Bit>* Instance() const override {
        return const_cast<BitSequence*>(this);
    }
    
    Bit GetFirst() const override {
        if (bits->GetSize() == 0) throw std::out_of_range("Sequence is empty");
        return bits->Get(0);
    }
    
    Bit GetLast() const override {
        if (bits->GetSize() == 0) throw std::out_of_range("Sequence is empty");
        return bits->Get(bits->GetSize() - 1);
    }
    
    Bit Get(int index) const override {
        return bits->Get(index);
    }
    
    int GetLength() const override { return bits->GetSize(); }
    
    Sequence<Bit>* Append(Bit item) override {
        bits->Resize(bits->GetSize() + 1);
        bits->Set(bits->GetSize() - 1, item);
        return this;
    }
    
    Sequence<Bit>* Prepend(Bit item) override {
        DynamicArray<Bit>* newBits = new DynamicArray<Bit>(bits->GetSize() + 1);
        newBits->Set(0, item);
        for (int i = 0; i < bits->GetSize(); i++) {
            newBits->Set(i + 1, bits->Get(i));
        }
        delete bits;
        bits = newBits;
        return this;
    }
    
    Sequence<Bit>* InsertAt(Bit item, int index) override {
        if (index < 0 || index > bits->GetSize()) throw std::out_of_range("Index out of range");
        DynamicArray<Bit>* newBits = new DynamicArray<Bit>(bits->GetSize() + 1);
        for (int i = 0; i < index; i++) {
            newBits->Set(i, bits->Get(i));
        }
        newBits->Set(index, item);
        for (int i = index; i < bits->GetSize(); i++) {
            newBits->Set(i + 1, bits->Get(i));
        }
        delete bits;
        bits = newBits;
        return this;
    }
    
    Sequence<Bit>* Concat(const Sequence<Bit>* other) override {
        if (other == nullptr) throw std::invalid_argument("Other sequence must not be null");
        for (int i = 0; i < other->GetLength(); i++) {
            Append(other->Get(i));
        }
        return this;
    }
    
    Sequence<Bit>* GetSubsequence(int startIndex, int endIndex) override {
        if (startIndex < 0 || endIndex >= bits->GetSize() || startIndex > endIndex) {
            throw std::out_of_range("Index out of range");
        }
        BitSequence* result = new BitSequence();
        for (int i = startIndex; i <= endIndex; i++) {
            result->Append(bits->Get(i));
        }
        return result;
    }
    
    Sequence<Bit>* Map(std::function<Bit(Bit)> func) override {
        for (int i = 0; i < bits->GetSize(); i++) {
            bits->Set(i, func(bits->Get(i)));
        }
        return this;
    }
    
    Sequence<Bit>* Where(std::function<bool(Bit)> predicate) override {
        BitSequence* result = new BitSequence();
        for (int i = 0; i < bits->GetSize(); i++) {
            Bit value = bits->Get(i);
            if (predicate(value)) {
                result->Append(value);
            }
        }
        delete bits;
        bits = result->bits;
        result->bits = nullptr;
        delete result;
        return this;
    }
    
    Bit Reduce(std::function<Bit(Bit, Bit)> func, Bit initial) override {
        Bit result = initial;
        for (int i = 0; i < bits->GetSize(); i++) {
            result = func(result, bits->Get(i));
        }
        return result;
    }
    
    Option<Bit> TryGetFirst(std::function<bool(Bit)> predicate = nullptr) override {
        if (bits->GetSize() == 0) return Option<Bit>::None();
        if (predicate == nullptr) return Option<Bit>::Some(bits->Get(0));
        for (int i = 0; i < bits->GetSize(); i++) {
            Bit value = bits->Get(i);
            if (predicate(value)) return Option<Bit>::Some(value);
        }
        return Option<Bit>::None();
    }
    
    Option<Bit> TryGetLast(std::function<bool(Bit)> predicate = nullptr) override {
        if (bits->GetSize() == 0) return Option<Bit>::None();
        if (predicate == nullptr) return Option<Bit>::Some(bits->Get(bits->GetSize() - 1));
        for (int i = bits->GetSize() - 1; i >= 0; i--) {
            Bit value = bits->Get(i);
            if (predicate(value)) return Option<Bit>::Some(value);
        }
        return Option<Bit>::None();
    }
    
    // Bit operations
    BitSequence* BitAnd(const BitSequence& other) const {
        return new BitSequence(*this & other);
    }
    
    BitSequence* BitOr(const BitSequence& other) const {
        return new BitSequence(*this | other);
    }
    
    BitSequence* BitXor(const BitSequence& other) const {
        return new BitSequence(*this ^ other);
    }
    
    BitSequence* BitNot() const {
        return new BitSequence(~(*this));
    }

    BitSequence operator&(const BitSequence& other) const {
        BitSequence result;
        int minLen = std::min(bits->GetSize(), other.bits->GetSize());
        for (int i = 0; i < minLen; i++) {
            result.Append(bits->Get(i) & other.bits->Get(i));
        }
        return result;
    }

    BitSequence operator|(const BitSequence& other) const {
        BitSequence result;
        int minLen = std::min(bits->GetSize(), other.bits->GetSize());
        for (int i = 0; i < minLen; i++) {
            result.Append(bits->Get(i) | other.bits->Get(i));
        }
        return result;
    }

    BitSequence operator^(const BitSequence& other) const {
        BitSequence result;
        int minLen = std::min(bits->GetSize(), other.bits->GetSize());
        for (int i = 0; i < minLen; i++) {
            result.Append(bits->Get(i) ^ other.bits->Get(i));
        }
        return result;
    }

    BitSequence operator~() const {
        BitSequence result;
        for (int i = 0; i < bits->GetSize(); i++) {
            result.Append(~bits->Get(i));
        }
        return result;
    }

    BitSequence& operator&=(const BitSequence& other) {
        *this = *this & other;
        return *this;
    }

    BitSequence& operator|=(const BitSequence& other) {
        *this = *this | other;
        return *this;
    }

    BitSequence& operator^=(const BitSequence& other) {
        *this = *this ^ other;
        return *this;
    }
    
    void Print() const override {
        std::cout << "[";
        for (int i = 0; i < bits->GetSize(); i++) {
            std::cout << (bits->Get(i).GetValue() ? 1 : 0);
            if (i < bits->GetSize() - 1) std::cout << ", ";
        }
        std::cout << "]" << std::endl;
    }
};
