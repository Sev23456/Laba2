#include <cassert>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include "ArraySequence.h"
#include "BitSequence.h"
#include "DynamicArray.h"
#include "ListSequence.h"
#include "Performance.h"

template<typename ExceptionType, typename Func>
void ExpectThrows(Func func) {
    bool caught = false;
    try {
        func();
    } catch (const ExceptionType&) {
        caught = true;
    }
    assert(caught);
}

template<typename T>
void AssertSequenceEquals(const Sequence<T>& sequence, std::initializer_list<T> expected) {
    assert(sequence.GetLength() == static_cast<int>(expected.size()));

    int index = 0;
    for (const T& value : expected) {
        assert(sequence.Get(index) == value);
        index++;
    }
}

template<typename T>
void AssertLinkedListEquals(const LinkedList<T>& list, std::initializer_list<T> expected) {
    assert(list.GetLength() == static_cast<int>(expected.size()));

    int index = 0;
    for (const T& value : expected) {
        assert(list.Get(index) == value);
        index++;
    }

    IEnumerator<T>* enumerator = list.GetEnumerator();
    index = 0;
    try {
        for (const T& value : expected) {
            assert(enumerator->MoveNext());
            assert(enumerator->GetCurrent() == value);
            index++;
        }
        assert(!enumerator->MoveNext());
    } catch (...) {
        delete enumerator;
        throw;
    }
    delete enumerator;
}

void AssertBitSequenceEquals(const Sequence<Bit>& sequence, std::initializer_list<int> expected) {
    assert(sequence.GetLength() == static_cast<int>(expected.size()));

    int index = 0;
    for (int value : expected) {
        assert(sequence.Get(index) == Bit(value != 0));
        index++;
    }
}

template<typename T>
void AssertEnumeratorEquals(Sequence<T>& sequence, std::initializer_list<T> expected) {
    IEnumerator<T>* enumerator = sequence.GetEnumerator();
    int index = 0;

    try {
        while (enumerator->MoveNext()) {
            assert(index < static_cast<int>(expected.size()));
            auto it = expected.begin();
            std::advance(it, index);
            assert(enumerator->GetCurrent() == *it);
            index++;
        }

        assert(index == static_cast<int>(expected.size()));
        enumerator->Reset();
        if (!expected.size()) {
            assert(!enumerator->MoveNext());
        } else {
            assert(enumerator->MoveNext());
            assert(enumerator->GetCurrent() == *expected.begin());
        }
    } catch (...) {
        delete enumerator;
        throw;
    }
    delete enumerator;
}

void TestDynamicArray() {
    std::cout << "Testing DynamicArray... ";

    DynamicArray<int> empty;
    assert(empty.GetSize() == 0);

    DynamicArray<int> zeroSized(0);
    assert(zeroSized.GetSize() == 0);

    empty.Resize(3);
    for (int i = 0; i < 3; i++) {
        empty.Set(i, (i + 1) * 10);
    }
    assert(empty.Get(0) == 10);
    assert(empty.Get(2) == 30);

    empty.Resize(1);
    assert(empty.GetSize() == 1);
    assert(empty.Get(0) == 10);

    empty.Resize(0);
    assert(empty.GetSize() == 0);
    ExpectThrows<std::out_of_range>([&]() { empty.Get(0); });

    int data[] = {4, 5, 6};
    DynamicArray<int> fromItems(data, 3);
    assert(fromItems.GetSize() == 3);
    assert(fromItems.Get(1) == 5);
    fromItems[1] = 55;
    const DynamicArray<int>& constArray = fromItems;
    assert(constArray[1] == 55);

    DynamicArray<int> fromEmptyItems(nullptr, 0);
    assert(fromEmptyItems.GetSize() == 0);

    ExpectThrows<std::invalid_argument>([]() {
        DynamicArray<int> invalid(nullptr, 2);
    });

    std::cout << "OK" << std::endl;
}

void TestLinkedList() {
    std::cout << "Testing LinkedList... ";

    int base[] = {1, 2, 3};
    LinkedList<int> list(base, 3);
    assert(list[1] == 2);
    list[1] = 42;
    const LinkedList<int>& constList = list;
    assert(constList[1] == 42);

    int extra[] = {5, 6};
    LinkedList<int> other(extra, 2);

    LinkedList<int>* concatenated = list.Concat(&other);
    AssertLinkedListEquals(*concatenated, {1, 42, 3, 5, 6});
    AssertLinkedListEquals(list, {1, 42, 3});
    delete concatenated;

    LinkedList<int> sum = list + other;
    AssertLinkedListEquals(sum, {1, 42, 3, 5, 6});
    AssertLinkedListEquals(list, {1, 42, 3});

    LinkedList<int> assigned;
    assigned = other;
    other[0] = 99;
    AssertLinkedListEquals(assigned, {5, 6});
    AssertLinkedListEquals(other, {99, 6});

    list += other;
    AssertLinkedListEquals(list, {1, 42, 3, 99, 6});

    std::cout << "OK" << std::endl;
}

void TestMutableArraySequence() {
    std::cout << "Testing MutableArraySequence... ";

    int base[] = {1, 2, 3};
    ArraySequence<int> sequence(base, 3);
    sequence.Append(4);
    sequence.Prepend(0);
    sequence.InsertAt(99, 2);
    AssertSequenceEquals(sequence, {0, 1, 99, 2, 3, 4});
    assert(sequence[2] == 99);

    int extra[] = {7, 8};
    ArraySequence<int> other(extra, 2);
    sequence.Concat(&other);
    AssertSequenceEquals(sequence, {0, 1, 99, 2, 3, 4, 7, 8});

    Sequence<int>* subsequence = sequence.GetSubsequence(1, 3);
    AssertSequenceEquals(*subsequence, {1, 99, 2});
    delete subsequence;

    int mapData[] = {1, 2, 3, 4, 5};
    ArraySequence<int> mapSequence(mapData, 5);
    mapSequence.Map([](int value) { return value * 3; });
    AssertSequenceEquals(mapSequence, {3, 6, 9, 12, 15});

    mapSequence.Where([](int value) { return value % 2 == 0; });
    AssertSequenceEquals(mapSequence, {6, 12});
    assert(mapSequence.Reduce([](int left, int right) { return left + right; }, 0) == 18);

    Option<int> firstLarge = mapSequence.TryGetFirst([](int value) { return value > 10; });
    assert(firstLarge.IsSome());
    assert(firstLarge.GetValue() == 12);

    Option<int> lastNegative = mapSequence.TryGetLast([](int value) { return value < 0; });
    assert(lastNegative.IsNone());

    mapSequence.Where([](int value) { return value < 0; });
    assert(mapSequence.GetLength() == 0);

    LinkedList<int> linked(base, 3);
    MutableArraySequence<int> fromLinked(linked);
    AssertSequenceEquals(fromLinked, {1, 2, 3});
    AssertEnumeratorEquals(fromLinked, {1, 2, 3});

    int leftData[] = {4, 5};
    int rightData[] = {6, 7};
    ArraySequence<int> left(leftData, 2);
    ArraySequence<int> right(rightData, 2);
    ArraySequence<int> assigned = left;
    right[0] = 60;
    AssertSequenceEquals(assigned, {4, 5});

    ArraySequence<int> sum = left + right;
    AssertSequenceEquals(left, {4, 5});
    AssertSequenceEquals(sum, {4, 5, 60, 7});

    Sequence<int>* sameInstance = left.Instance();
    assert(sameInstance == static_cast<Sequence<int>*>(&left));
    ArraySequence<int>& plusEqualsResult = (left += right);
    assert(&plusEqualsResult == &left);
    AssertSequenceEquals(left, {4, 5, 60, 7});

    std::cout << "OK" << std::endl;
}

void TestImmutableArraySequence() {
    std::cout << "Testing ImmutableArraySequence... ";

    int base[] = {1, 2, 3};
    ImmutableArraySequence<int> original(base, 3);

    Sequence<int>* appended = original.Append(4);
    AssertSequenceEquals(original, {1, 2, 3});
    AssertSequenceEquals(*appended, {1, 2, 3, 4});
    delete appended;

    Sequence<int>* prepended = original.Prepend(0);
    AssertSequenceEquals(*prepended, {0, 1, 2, 3});
    AssertSequenceEquals(original, {1, 2, 3});
    delete prepended;

    Sequence<int>* inserted = original.InsertAt(99, 1);
    AssertSequenceEquals(*inserted, {1, 99, 2, 3});
    delete inserted;

    int extra[] = {7, 8};
    ArraySequence<int> other(extra, 2);
    Sequence<int>* concatenated = original.Concat(&other);
    AssertSequenceEquals(*concatenated, {1, 2, 3, 7, 8});
    delete concatenated;

    Sequence<int>* subsequence = original.GetSubsequence(1, 2);
    AssertSequenceEquals(*subsequence, {2, 3});
    delete subsequence;

    Sequence<int>* mapped = original.Map([](int value) { return value * 2; });
    AssertSequenceEquals(*mapped, {2, 4, 6});
    AssertSequenceEquals(original, {1, 2, 3});
    delete mapped;

    Sequence<int>* filtered = original.Where([](int value) { return value > 10; });
    assert(filtered->GetLength() == 0);
    delete filtered;

    Sequence<int>* emptyFiltered = ImmutableArraySequence<int>().Where([](int value) { return value > 0; });
    assert(emptyFiltered->GetLength() == 0);
    delete emptyFiltered;

    assert(original.Reduce([](int left, int right) { return left + right; }, 0) == 6);

    Option<int> firstEven = original.TryGetFirst([](int value) { return value % 2 == 0; });
    assert(firstEven.IsSome());
    assert(firstEven.GetValue() == 2);

    Option<int> missing = original.TryGetLast([](int value) { return value > 10; });
    assert(missing.IsNone());

    LinkedList<int> linked(base, 3);
    ImmutableArraySequence<int> fromLinked(linked);
    AssertSequenceEquals(fromLinked, {1, 2, 3});
    AssertEnumeratorEquals(fromLinked, {1, 2, 3});

    int leftData[] = {4, 5};
    int rightData[] = {6, 7};
    ImmutableArraySequence<int> left(leftData, 2);
    ArraySequence<int> right(rightData, 2);
    ImmutableArraySequence<int> assigned;
    assigned = left;
    AssertSequenceEquals(assigned, {4, 5});

    ImmutableArraySequence<int> sum = left + right;
    AssertSequenceEquals(left, {4, 5});
    AssertSequenceEquals(sum, {4, 5, 6, 7});

    Sequence<int>* instance = left.Instance();
    assert(instance != static_cast<Sequence<int>*>(&left));
    AssertSequenceEquals(*instance, {4, 5});
    delete instance;

    std::cout << "OK" << std::endl;
}

void TestMutableListSequence() {
    std::cout << "Testing MutableListSequence... ";

    int base[] = {10, 20, 30};
    ListSequence<int> sequence(base, 3);
    sequence.Append(40);
    sequence.Prepend(5);
    sequence.InsertAt(15, 2);
    AssertSequenceEquals(sequence, {5, 10, 15, 20, 30, 40});

    int extra[] = {50, 60};
    ListSequence<int> other(extra, 2);
    sequence.Concat(&other);
    AssertSequenceEquals(sequence, {5, 10, 15, 20, 30, 40, 50, 60});

    Sequence<int>* subsequence = sequence.GetSubsequence(2, 4);
    AssertSequenceEquals(*subsequence, {15, 20, 30});
    delete subsequence;

    int mapData[] = {1, 2, 3, 4};
    ListSequence<int> mapSequence(mapData, 4);
    mapSequence.Map([](int value) { return value + 1; });
    AssertSequenceEquals(mapSequence, {2, 3, 4, 5});

    mapSequence.Where([](int value) { return value % 2 != 0; });
    AssertSequenceEquals(mapSequence, {3, 5});
    assert(mapSequence.Reduce([](int left, int right) { return left * right; }, 1) == 15);

    Option<int> lastLarge = mapSequence.TryGetLast([](int value) { return value > 3; });
    assert(lastLarge.IsSome());
    assert(lastLarge.GetValue() == 5);

    Option<int> noMatch = mapSequence.TryGetFirst([](int value) { return value < 0; });
    assert(noMatch.IsNone());

    LinkedList<int> linked(base, 3);
    MutableListSequence<int> fromLinked(linked);
    AssertSequenceEquals(fromLinked, {10, 20, 30});
    AssertEnumeratorEquals(fromLinked, {10, 20, 30});

    int leftData[] = {4, 5};
    int rightData[] = {6, 7};
    ListSequence<int> left(leftData, 2);
    ListSequence<int> right(rightData, 2);
    ListSequence<int> assigned;
    assigned = left;
    right.InsertAt(99, 1);
    AssertSequenceEquals(assigned, {4, 5});

    ListSequence<int> sum = left + right;
    AssertSequenceEquals(sum, {4, 5, 6, 99, 7});
    left += right;
    AssertSequenceEquals(left, {4, 5, 6, 99, 7});

    std::cout << "OK" << std::endl;
}

void TestImmutableListSequence() {
    std::cout << "Testing ImmutableListSequence... ";

    int base[] = {2, 4, 6};
    ImmutableListSequence<int> original(base, 3);

    Sequence<int>* appended = original.Append(8);
    AssertSequenceEquals(original, {2, 4, 6});
    AssertSequenceEquals(*appended, {2, 4, 6, 8});
    delete appended;

    Sequence<int>* prepended = original.Prepend(0);
    AssertSequenceEquals(*prepended, {0, 2, 4, 6});
    delete prepended;

    Sequence<int>* inserted = original.InsertAt(99, 1);
    AssertSequenceEquals(*inserted, {2, 99, 4, 6});
    delete inserted;

    int extra[] = {10, 12};
    ListSequence<int> other(extra, 2);
    Sequence<int>* concatenated = original.Concat(&other);
    AssertSequenceEquals(*concatenated, {2, 4, 6, 10, 12});
    delete concatenated;

    Sequence<int>* subsequence = original.GetSubsequence(1, 2);
    AssertSequenceEquals(*subsequence, {4, 6});
    delete subsequence;

    Sequence<int>* mapped = original.Map([](int value) { return value / 2; });
    AssertSequenceEquals(*mapped, {1, 2, 3});
    AssertSequenceEquals(original, {2, 4, 6});
    delete mapped;

    Sequence<int>* filtered = original.Where([](int value) { return value > 10; });
    assert(filtered->GetLength() == 0);
    delete filtered;

    assert(original.Reduce([](int left, int right) { return left + right; }, 0) == 12);

    Option<int> firstLarge = original.TryGetFirst([](int value) { return value > 3; });
    assert(firstLarge.IsSome());
    assert(firstLarge.GetValue() == 4);

    Option<int> missing = original.TryGetLast([](int value) { return value < 0; });
    assert(missing.IsNone());

    LinkedList<int> linked(base, 3);
    ImmutableListSequence<int> fromLinked(linked);
    AssertSequenceEquals(fromLinked, {2, 4, 6});
    AssertEnumeratorEquals(fromLinked, {2, 4, 6});

    int leftData[] = {8, 10};
    int rightData[] = {12, 14};
    ImmutableListSequence<int> left(leftData, 2);
    ListSequence<int> right(rightData, 2);
    ImmutableListSequence<int> assigned;
    assigned = left;
    AssertSequenceEquals(assigned, {8, 10});

    ImmutableListSequence<int> sum = left + right;
    AssertSequenceEquals(left, {8, 10});
    AssertSequenceEquals(sum, {8, 10, 12, 14});

    std::cout << "OK" << std::endl;
}

void TestBitSequence() {
    std::cout << "Testing BitSequence... ";

    BitSequence sequence;
    sequence.Append(Bit(1));
    sequence.Append(Bit(0));
    sequence.Append(Bit(1));
    sequence.Prepend(Bit(0));
    sequence.InsertAt(Bit(1), 2);
    AssertBitSequenceEquals(sequence, {0, 1, 1, 0, 1});

    BitSequence extra;
    extra.Append(Bit(1));
    extra.Append(Bit(1));
    sequence.Concat(&extra);
    AssertBitSequenceEquals(sequence, {0, 1, 1, 0, 1, 1, 1});

    Sequence<Bit>* subsequence = sequence.GetSubsequence(1, 4);
    AssertBitSequenceEquals(*subsequence, {1, 1, 0, 1});
    delete subsequence;

    BitSequence filterSequence;
    filterSequence.Append(Bit(1));
    filterSequence.Append(Bit(0));
    filterSequence.Append(Bit(1));
    filterSequence.Append(Bit(0));

    filterSequence.Map([](Bit bit) { return ~bit; });
    AssertBitSequenceEquals(filterSequence, {0, 1, 0, 1});

    filterSequence.Where([](Bit bit) { return bit.GetValue(); });
    AssertBitSequenceEquals(filterSequence, {1, 1});

    Bit reduced = filterSequence.Reduce([](Bit left, Bit right) { return left | right; }, Bit(0));
    assert(reduced.GetValue());

    Option<Bit> firstOne = filterSequence.TryGetFirst([](Bit bit) { return bit.GetValue(); });
    assert(firstOne.IsSome());
    assert(firstOne.GetValue() == Bit(1));

    Option<Bit> noZero = filterSequence.TryGetLast([](Bit bit) { return !bit.GetValue(); });
    assert(noZero.IsNone());

    BitSequence left;
    left.Append(Bit(1));
    left.Append(Bit(0));
    left.Append(Bit(1));
    left.Append(Bit(1));

    BitSequence right;
    right.Append(Bit(1));
    right.Append(Bit(1));
    right.Append(Bit(0));
    right.Append(Bit(1));

    BitSequence* andResult = left.BitAnd(right);
    BitSequence* orResult = left.BitOr(right);
    BitSequence* xorResult = left.BitXor(right);
    BitSequence* notResult = left.BitNot();
    BitSequence andOperator = left & right;
    BitSequence orOperator = left | right;
    BitSequence xorOperator = left ^ right;
    BitSequence notOperator = ~left;

    AssertBitSequenceEquals(*andResult, {1, 0, 0, 1});
    AssertBitSequenceEquals(*orResult, {1, 1, 1, 1});
    AssertBitSequenceEquals(*xorResult, {0, 1, 1, 0});
    AssertBitSequenceEquals(*notResult, {0, 1, 0, 0});
    AssertBitSequenceEquals(andOperator, {1, 0, 0, 1});
    AssertBitSequenceEquals(orOperator, {1, 1, 1, 1});
    AssertBitSequenceEquals(xorOperator, {0, 1, 1, 0});
    AssertBitSequenceEquals(notOperator, {0, 1, 0, 0});

    BitSequence compound = left;
    compound &= right;
    AssertBitSequenceEquals(compound, {1, 0, 0, 1});
    compound |= right;
    AssertBitSequenceEquals(compound, {1, 1, 0, 1});
    compound ^= right;
    AssertBitSequenceEquals(compound, {0, 0, 0, 0});

    delete andResult;
    delete orResult;
    delete xorResult;
    delete notResult;

    AssertEnumeratorEquals(left, {Bit(1), Bit(0), Bit(1), Bit(1)});

    std::cout << "OK" << std::endl;
}

void TestPerformance() {
    std::cout << "Testing Performance helpers... ";

    DynamicArray<PerformanceResult> results = BenchmarkAllSequences(200);
    assert(results.GetSize() == 4);
    for (int i = 0; i < results.GetSize(); i++) {
        const PerformanceResult& result = results[i];
        assert(!result.name.empty());
        assert(result.microseconds >= 0);
    }

    std::cout << "OK" << std::endl;
}

void TestExceptions() {
    std::cout << "Testing Exceptions... ";

    DynamicArray<int> dynamicArray;
    ExpectThrows<std::out_of_range>([&]() { dynamicArray.Set(0, 1); });

    ArraySequence<int> emptyArray;
    ExpectThrows<std::out_of_range>([&]() { emptyArray.Get(0); });
    ExpectThrows<std::out_of_range>([&]() { emptyArray.GetFirst(); });

    ListSequence<int> emptyList;
    ExpectThrows<std::out_of_range>([&]() { emptyList.GetLast(); });

    BitSequence emptyBits;
    ExpectThrows<std::out_of_range>([&]() { emptyBits.Get(0); });

    int values[] = {1, 2, 3};
    ArraySequence<int> arraySequence(values, 3);
    ExpectThrows<std::out_of_range>([&]() { arraySequence.InsertAt(4, 10); });
    ExpectThrows<std::invalid_argument>([&]() { arraySequence.Concat(nullptr); });

    ListSequence<int> listSequence(values, 3);
    ExpectThrows<std::out_of_range>([&]() { listSequence.GetSubsequence(2, 5); });
    ExpectThrows<std::invalid_argument>([&]() { listSequence.Concat(nullptr); });

    Option<int> none = Option<int>::None();
    ExpectThrows<std::runtime_error>([&]() { none.GetValue(); });

    BitSequence bits;
    ExpectThrows<std::invalid_argument>([&]() { bits.Concat(nullptr); });

    ExpectThrows<std::invalid_argument>([]() {
        LinkedList<int> invalid(nullptr, 1);
    });

    std::cout << "OK" << std::endl;
}

int main() {
    setlocale(LC_ALL, "");
    std::cout << "=== Running tests ===" << std::endl;

    TestDynamicArray();
    TestLinkedList();
    TestMutableArraySequence();
    TestImmutableArraySequence();
    TestMutableListSequence();
    TestImmutableListSequence();
    TestBitSequence();
    TestPerformance();
    TestExceptions();

    std::cout << "\nAll tests passed!" << std::endl;
    return 0;
}
