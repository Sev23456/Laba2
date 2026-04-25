#include <iostream>
#include <limits>
#include <string>
#include "ArraySequence.h"
#include "ListSequence.h"
#include "BitSequence.h"
#include "Performance.h"

int ReadInt(const std::string& prompt) {
    while (true) {
        std::cout << prompt;
        int value;
        if (std::cin >> value) {
            return value;
        }
        std::cout << "Invalid input. Please enter an integer." << std::endl;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

void WaitForEnter() {
    std::cout << "\nPress Enter to continue...";
    bool hasBufferedInput = std::cin.rdbuf()->in_avail() > 0;
    std::string dummy;
    std::getline(std::cin, dummy);
    if (hasBufferedInput && dummy.empty()) {
        std::getline(std::cin, dummy);
    }
}

void PrintMenu() {
    std::cout << "\n=== Lab work #2 ===" << std::endl;
    std::cout << "1. Test ArraySequence" << std::endl;
    std::cout << "2. Test ListSequence" << std::endl;
    std::cout << "3. Test BitSequence" << std::endl;
    std::cout << "4. Test Map-Reduce" << std::endl;
    std::cout << "5. Test Immutable" << std::endl;
    std::cout << "6. Test Iterators" << std::endl;
    std::cout << "7. Benchmark performance" << std::endl;
    std::cout << "0. Exit" << std::endl;
}

void TestArraySequence() {
    std::cout << "\n--- ArraySequence ---" << std::endl;
    int arr[] = {1, 2, 3, 4, 5};
    ArraySequence<int>* seq = new ArraySequence<int>(arr, 5);
    
    std::cout << "Original: ";
    seq->Print();
    
    seq->Append(6);
    std::cout << "After Append(6): ";
    seq->Print();
    
    seq->Prepend(0);
    std::cout << "After Prepend(0): ";
    seq->Print();
    
    std::cout << "Get(3) = " << seq->Get(3) << std::endl;
    std::cout << "GetFirst() = " << seq->GetFirst() << std::endl;
    std::cout << "GetLast() = " << seq->GetLast() << std::endl;
    std::cout << "Length = " << seq->GetLength() << std::endl;
    
    delete seq;
}

void TestListSequence() {
    std::cout << "\n--- ListSequence ---" << std::endl;
    int arr[] = {10, 20, 30};
    ListSequence<int>* seq = new ListSequence<int>(arr, 3);
    
    std::cout << "Original: ";
    seq->Print();
    
    seq->Append(40);
    std::cout << "After Append(40): ";
    seq->Print();
    
    delete seq;
}

void TestBitSequence() {
    std::cout << "\n--- BitSequence ---" << std::endl;
    BitSequence bs1;
    bs1.Append(Bit(1));
    bs1.Append(Bit(0));
    bs1.Append(Bit(1));
    bs1.Append(Bit(1));
    
    BitSequence bs2;
    bs2.Append(Bit(1));
    bs2.Append(Bit(1));
    bs2.Append(Bit(0));
    bs2.Append(Bit(0));
    
    std::cout << "BS1: "; bs1.Print();
    std::cout << "BS2: "; bs2.Print();
    
    BitSequence* andResult = bs1.BitAnd(bs2);
    BitSequence* orResult = bs1.BitOr(bs2);
    BitSequence* xorResult = bs1.BitXor(bs2);
    BitSequence* notResult = bs1.BitNot();

    try {
        std::cout << "BS1 AND BS2: "; andResult->Print();
        std::cout << "BS1 OR BS2: "; orResult->Print();
        std::cout << "BS1 XOR BS2: "; xorResult->Print();
        std::cout << "NOT BS1: "; notResult->Print();
    } catch (...) {
        delete andResult;
        delete orResult;
        delete xorResult;
        delete notResult;
        throw;
    }

    delete andResult;
    delete orResult;
    delete xorResult;
    delete notResult;
}

void TestMapReduce() {
    std::cout << "\n--- Map-Reduce ---" << std::endl;
    int arr[] = {1, 2, 3, 4, 5};
    ArraySequence<int>* seq = new ArraySequence<int>(arr, 5);
    
    std::cout << "Original: ";
    seq->Print();
    
    seq->Map([](int x) { return x * 2; });
    std::cout << "After Map(x*2): ";
    seq->Print();
    
    seq->Where([](int x) { return x % 2 == 0; });
    std::cout << "After Where(even): ";
    seq->Print();
    
    int sum = seq->Reduce([](int a, int b) { return a + b; }, 0);
    std::cout << "Reduce(sum) = " << sum << std::endl;
    
    Option<int> first = seq->TryGetFirst();
    if (first.IsSome()) {
        std::cout << "TryGetFirst = " << first.GetValue() << std::endl;
    }
    
    delete seq;
}

void TestImmutable() {
    std::cout << "\n--- Immutable Sequence ---" << std::endl;
    int arr[] = {1, 2, 3};
    ImmutableArraySequence<int> seq1(arr, 3);
    
    std::cout << "seq1: ";
    seq1.Print();
    
    Sequence<int>* seq2 = seq1.Append(4);
    
    std::cout << "seq1 after Append (unchanged): ";
    seq1.Print();
    std::cout << "seq2 (new): ";
    seq2->Print();

    delete seq2;
}

void TestIterators() {
    std::cout << "\n--- Iterator Demo ---" << std::endl;
    int arr[] = {3, 6, 9, 12};
    ArraySequence<int> seq(arr, 4);
    IEnumerator<int>* enumerator = seq.GetEnumerator();

    std::cout << "Sequence via IEnumerator: [";
    try {
        bool first = true;
        while (enumerator->MoveNext()) {
            if (!first) std::cout << ", ";
            std::cout << enumerator->GetCurrent();
            first = false;
        }
    } catch (...) {
        delete enumerator;
        throw;
    }
    std::cout << "]" << std::endl;
    delete enumerator;
}

void TestPerformance() {
    std::cout << "\n--- Performance Benchmark ---" << std::endl;
    int size = ReadInt("Enter benchmark size: ");
    if (size <= 0) {
        std::cout << "Benchmark size must be positive." << std::endl;
        return;
    }

    DynamicArray<PerformanceResult> results = BenchmarkAllSequences(size);
    for (int i = 0; i < results.GetSize(); i++) {
        const PerformanceResult& result = results[i];
        std::cout << result.name << ": " << result.microseconds << " us" << std::endl;
    }
}

int main() {
    setlocale(LC_ALL, "");
    
    while (true) {
        PrintMenu();
        int choice = ReadInt("Choice: ");
        
        try {
            switch (choice) {
                case 1: TestArraySequence(); break;
                case 2: TestListSequence(); break;
                case 3: TestBitSequence(); break;
                case 4: TestMapReduce(); break;
                case 5: TestImmutable(); break;
                case 6: TestIterators(); break;
                case 7: TestPerformance(); break;
                case 0: std::cout << "Exiting..." << std::endl; return 0;
                default: std::cout << "Invalid choice!" << std::endl;
            }
            WaitForEnter();
        } catch (std::exception& ex) {
            std::cout << "Error: " << ex.what() << std::endl;
            WaitForEnter();
        }
    }
    
    return 0;
}
