#include "Heap.h"
#include <algorithm>
#include <stdexcept>

BinaryHeap::BinaryHeap() {}

BinaryHeap::~BinaryHeap() {
    clear();
}

int BinaryHeap::parent(int index) const { return (index - 1) / 2; }
int BinaryHeap::left(int index) const { return 2 * index + 1; }
int BinaryHeap::right(int index) const { return 2 * index + 2; }

void BinaryHeap::heapifyUp(int index) {
    while (index > 0 && data[parent(index)] > data[index]) {
        std::swap(data[parent(index)], data[index]);
        index = parent(index);
    }
}

void BinaryHeap::heapifyDown(int index) {
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

void BinaryHeap::buildHeap(const std::vector<int>& initialData) {
    data = initialData;
    for (int i = (data.size() / 2) - 1; i >= 0; --i) {
        heapifyDown(i);
    }
}

void BinaryHeap::clear() {
    data.clear();
}

void BinaryHeap::insert(int value) {
    data.push_back(value);
    heapifyUp(data.size() - 1);
}

int BinaryHeap::extractTop() {
    if (isEmpty()) return -1;
    
    int topValue = data[0];
    data[0] = data.back();
    data.pop_back();

    if (!isEmpty()) {
        heapifyDown(0);
    }
    return topValue;
}

void BinaryHeap::updateValue(int index, int newValue) {
    if (index < 0 || index >= (int)data.size()) {
        throw std::out_of_range("Heap index out of range");
    }

    int oldValue = data[index];
    data[index] = newValue;
    if (newValue < oldValue) {
        heapifyUp(index);
    } else if (newValue > oldValue) {
        heapifyDown(index);
    }
}

void BinaryHeap::setValue(int index, int newValue) {
    if (index < 0 || index >= (int)data.size()) {
        throw std::out_of_range("Heap index out of range");
    }
    data[index] = newValue;
}

const std::vector<int>& BinaryHeap::getData() const {
    return data;
}

int BinaryHeap::getSize() const {
    return data.size();
}

bool BinaryHeap::isEmpty() const {
    return data.empty();
}