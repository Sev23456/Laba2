#pragma once
#include <stdexcept>

template<typename T>
class DynamicArray {
private:
    T* data;
    int size;
    
public:
    DynamicArray() : data(nullptr), size(0) {}
    
    DynamicArray(int size) : size(size) {
        if (size < 0) throw std::invalid_argument("Size must be non-negative");
        data = (size == 0) ? nullptr : new T[size]();
    }
    
    DynamicArray(T* items, int count) : size(count) {
        if (count < 0) throw std::invalid_argument("Count must be non-negative");
        if (count > 0 && items == nullptr) throw std::invalid_argument("Items pointer must not be null");
        data = (count == 0) ? nullptr : new T[count];
        for (int i = 0; i < count; i++) {
            data[i] = items[i];
        }
    }
    
    DynamicArray(const DynamicArray<T>& other) : size(other.size) {
        data = (size == 0) ? nullptr : new T[size];
        for (int i = 0; i < size; i++) {
            data[i] = other.data[i];
        }
    }

    DynamicArray<T>& operator=(const DynamicArray<T>& other) {
        if (this == &other) {
            return *this;
        }

        T* newData = (other.size == 0) ? nullptr : new T[other.size];
        for (int i = 0; i < other.size; i++) {
            newData[i] = other.data[i];
        }

        delete[] data;
        data = newData;
        size = other.size;
        return *this;
    }
    
    ~DynamicArray() {
        delete[] data;
    }
    
    T Get(int index) const {
        if (index < 0 || index >= size) throw std::out_of_range("Index out of range");
        return data[index];
    }
    
    void Set(int index, T value) {
        if (index < 0 || index >= size) throw std::out_of_range("Index out of range");
        data[index] = value;
    }
    
    int GetSize() const { return size; }

    T* GetData() { return data; }
    const T* GetData() const { return data; }
    
    void Resize(int newSize) {
        if (newSize < 0) throw std::invalid_argument("New size must be non-negative");
        T* newData = (newSize == 0) ? nullptr : new T[newSize]();
        int copySize = (newSize < size) ? newSize : size;
        for (int i = 0; i < copySize; i++) {
            newData[i] = data[i];
        }
        delete[] data;
        data = newData;
        size = newSize;
    }
    
    T& operator[](int index) {
        if (index < 0 || index >= size) throw std::out_of_range("Index out of range");
        return data[index];
    }
    
    const T& operator[](int index) const {
        if (index < 0 || index >= size) throw std::out_of_range("Index out of range");
        return data[index];
    }
};
