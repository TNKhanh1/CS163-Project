#pragma once
#include <vector>

class Heap {
private:
    std::vector<int> data;

    int parent(int index) const;
    int left(int index) const;
    int right(int index) const;

    void heapifyUp(int index);
    void heapifyDown(int index);

public:
    Heap();
    ~Heap();

    void buildHeap(const std::vector<int>& initialData);
    void clear();
    void insert(int value);
    int extractTop(); // Dùng hàm O(log N) thay vì search & remove
    
    const std::vector<int>& getData() const;
    int getSize() const;
    bool isEmpty() const;
};