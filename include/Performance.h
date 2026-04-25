#pragma once
#include <chrono>
#include <cstddef>
#include <string>
#include "ArraySequence.h"
#include "DynamicArray.h"
#include "ListSequence.h"

struct PerformanceResult {
    std::string name;
    long long microseconds;
};

inline DynamicArray<int> BuildBenchmarkData(int size) {
    DynamicArray<int> data(size);
    for (int i = 0; i < size; i++) {
        data.Set(i, i % 97);
    }
    return data;
}

template<typename Func>
long long MeasureMicroseconds(Func func) {
    auto start = std::chrono::steady_clock::now();
    func();
    auto finish = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(finish - start).count();
}

template<typename SequenceType, bool ReplacesInstance>
SequenceType* ApplyBenchmarkStep(SequenceType* current, Sequence<int>* next) {
    SequenceType* updated = static_cast<SequenceType*>(next);
    if constexpr (ReplacesInstance) {
        delete current;
    }
    return updated;
}

template<typename SequenceType, bool ReplacesInstance>
long long BenchmarkSequence(int size) {
    const DynamicArray<int> data = BuildBenchmarkData(size);
    return MeasureMicroseconds([&]() {
        SequenceType* sequence = new SequenceType();
        try {
            for (int i = 0; i < data.GetSize(); i++) {
                sequence = ApplyBenchmarkStep<SequenceType, ReplacesInstance>(sequence, sequence->Append(data.Get(i)));
            }

            sequence = ApplyBenchmarkStep<SequenceType, ReplacesInstance>(
                sequence,
                sequence->Map([](int value) { return value * 2; })
            );

            sequence = ApplyBenchmarkStep<SequenceType, ReplacesInstance>(
                sequence,
                sequence->Where([](int value) { return value % 3 == 0; })
            );

            volatile int sum = sequence->Reduce([](int left, int right) { return left + right; }, 0);
            (void)sum;
        } catch (...) {
            delete sequence;
            throw;
        }
        delete sequence;
    });
}

inline DynamicArray<PerformanceResult> BenchmarkAllSequences(int size) {
    DynamicArray<PerformanceResult> results(4);
    results[0] = {"MutableArraySequence", BenchmarkSequence<MutableArraySequence<int>, false>(size)};
    results[1] = {"ImmutableArraySequence", BenchmarkSequence<ImmutableArraySequence<int>, true>(size)};
    results[2] = {"MutableListSequence", BenchmarkSequence<MutableListSequence<int>, false>(size)};
    results[3] = {"ImmutableListSequence", BenchmarkSequence<ImmutableListSequence<int>, true>(size)};
    return results;
}
