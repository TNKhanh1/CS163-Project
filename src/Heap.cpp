#include "Heap.h"
#include <algorithm>
#include <stdexcept>

Heap::Heap() {}

Heap::~Heap() {
    clear();
}

int Heap::parent(int index) const { return (index - 1) / 2; }
int Heap::left(int index) const { return 2 * index + 1; }
int Heap::right(int index) const { return 2 * index + 2; }

void Heap::heapifyUp(int index) {
    while (index > 0 && data[parent(index)] > data[index]) {
        std::swap(data[parent(index)], data[index]);
        index = parent(index);
    }
}

void Heap::heapifyDown(int index) {
    int size = data.size();
    int minIndex = index;

    while (true) {
        int l = left(index);
        int r = right(index);

        if (l < size && data[l] < data[minIndex]) {
            minIndex = l;
        }
        if (r < size && data[r] < data[minIndex]) {
            minIndex = r;
        }

        if (minIndex != index) {
            std::swap(data[index], data[minIndex]);
            index = minIndex;
        } else {
            break;
        }
    }
}

void Heap::buildHeap(const std::vector<int>& initialData) {
    data = initialData;
    for (int i = (data.size() / 2) - 1; i >= 0; --i) {
        heapifyDown(i);
    }
}

void Heap::clear() {
    data.clear();
}

void Heap::insert(int value) {
    data.push_back(value);
    heapifyUp(data.size() - 1);
}

int Heap::extractTop() {
    if (isEmpty()) return -1;
    
    int topValue = data[0];
    data[0] = data.back();
    data.pop_back();

    if (!isEmpty()) {
        heapifyDown(0);
    }
    return topValue;
}

const std::vector<int>& Heap::getData() const {
    return data;
}

int Heap::getSize() const {
    return data.size();
}

bool Heap::isEmpty() const {
    return data.empty();
}