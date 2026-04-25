#pragma once
#include "IEnumerable.h"
#include <stdexcept>

template<typename T>
class LinkedList;

template<typename T>
void AppendEnumerable(LinkedList<T>& target, const IEnumerable<T>& source);

template<typename T>
class LinkedList : public IEnumerable<T> {
private:
    struct Node {
        T data;
        Node* next;
        Node(T val) : data(val), next(nullptr) {}
    };
    
    Node* head;
    Node* tail;
    int length;

    void Swap(LinkedList<T>& other) {
        Node* tempHead = head;
        head = other.head;
        other.head = tempHead;

        Node* tempTail = tail;
        tail = other.tail;
        other.tail = tempTail;

        int tempLength = length;
        length = other.length;
        other.length = tempLength;
    }

    Node* GetNode(int index) {
        if (index < 0 || index >= length) throw std::out_of_range("Index out of range");
        Node* current = head;
        for (int i = 0; i < index; i++) {
            current = current->next;
        }
        return current;
    }

    const Node* GetNode(int index) const {
        if (index < 0 || index >= length) throw std::out_of_range("Index out of range");
        const Node* current = head;
        for (int i = 0; i < index; i++) {
            current = current->next;
        }
        return current;
    }

    class Enumerator : public IEnumerator<T> {
    private:
        const LinkedList<T>* list;
        const Node* current;
        bool started;

    public:
        explicit Enumerator(const LinkedList<T>* list) : list(list), current(nullptr), started(false) {}

        T GetCurrent() const override {
            if (current == nullptr) {
                throw std::out_of_range("Enumerator is not positioned on an element");
            }
            return current->data;
        }

        bool MoveNext() override {
            if (!started) {
                current = list->head;
                started = true;
            } else if (current != nullptr) {
                current = current->next;
            }
            return current != nullptr;
        }

        void Reset() override {
            current = nullptr;
            started = false;
        }
    };
    
public:
    LinkedList() : head(nullptr), tail(nullptr), length(0) {}
    
    LinkedList(T* items, int count) : LinkedList() {
        if (count < 0) throw std::invalid_argument("Count must be non-negative");
        if (count > 0 && items == nullptr) throw std::invalid_argument("Items pointer must not be null");
        for (int i = 0; i < count; i++) {
            Append(items[i]);
        }
    }
    
    LinkedList(const LinkedList<T>& other) : LinkedList() {
        Node* current = other.head;
        while (current != nullptr) {
            Append(current->data);
            current = current->next;
        }
    }

    LinkedList<T>& operator=(const LinkedList<T>& other) {
        if (this != &other) {
            LinkedList<T> copy(other);
            Swap(copy);
        }
        return *this;
    }
    
    ~LinkedList() {
        Clear();
    }
    
    void Clear() {
        while (head != nullptr) {
            Node* temp = head;
            head = head->next;
            delete temp;
        }
        tail = nullptr;
        length = 0;
    }
    
    T GetFirst() const {
        if (head == nullptr) throw std::out_of_range("Sequence is empty");
        return head->data;
    }
    
    T GetLast() const {
        if (tail == nullptr) throw std::out_of_range("Sequence is empty");
        return tail->data;
    }
    
    T Get(int index) const {
        return GetNode(index)->data;
    }

    void Set(int index, T value) {
        GetNode(index)->data = value;
    }

    T& operator[](int index) {
        return GetNode(index)->data;
    }

    const T& operator[](int index) const {
        return GetNode(index)->data;
    }
    
    int GetLength() const { return length; }
    
    void Append(T item) {
        Node* newNode = new Node(item);
        if (tail == nullptr) {
            head = tail = newNode;
        } else {
            tail->next = newNode;
            tail = newNode;
        }
        length++;
    }
    
    void Prepend(T item) {
        Node* newNode = new Node(item);
        newNode->next = head;
        head = newNode;
        if (tail == nullptr) tail = newNode;
        length++;
    }
    
    void InsertAt(T item, int index) {
        if (index < 0 || index > length) throw std::out_of_range("Index out of range");
        if (index == 0) {
            Prepend(item);
            return;
        }
        if (index == length) {
            Append(item);
            return;
        }
        Node* current = head;
        for (int i = 0; i < index - 1; i++) {
            current = current->next;
        }
        Node* newNode = new Node(item);
        newNode->next = current->next;
        current->next = newNode;
        length++;
    }
    
    LinkedList<T>* Concat(const LinkedList<T>* other) const {
        if (other == nullptr) throw std::invalid_argument("Other list must not be null");
        LinkedList<T>* result = new LinkedList<T>(*this);
        AppendEnumerable(*result, *other);
        return result;
    }

    LinkedList<T> operator+(const LinkedList<T>& other) const {
        LinkedList<T> result(*this);
        result += other;
        return result;
    }

    LinkedList<T>& operator+=(const LinkedList<T>& other) {
        AppendEnumerable(*this, other);
        return *this;
    }
    
    LinkedList<T>* GetSubList(int startIndex, int endIndex) const {
        if (startIndex < 0 || endIndex >= length || startIndex > endIndex) {
            throw std::out_of_range("Index out of range");
        }
        LinkedList<T>* result = new LinkedList<T>();
        const Node* current = GetNode(startIndex);
        for (int i = startIndex; i <= endIndex; i++) {
            result->Append(current->data);
            current = current->next;
        }
        return result;
    }

    IEnumerator<T>* GetEnumerator() const override {
        return new Enumerator(this);
    }
};

template<typename T>
void AppendEnumerable(LinkedList<T>& target, const IEnumerable<T>& source) {
    IEnumerator<T>* enumerator = source.GetEnumerator();
    try {
        while (enumerator->MoveNext()) {
            target.Append(enumerator->GetCurrent());
        }
    } catch (...) {
        delete enumerator;
        throw;
    }
    delete enumerator;
}
