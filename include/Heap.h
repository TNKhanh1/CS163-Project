#pragma once
#include <vector>

// Enum định nghĩa các loại thao tác để phục vụ cho việc visualize Step-by-step
enum HeapOpType { 
    H_COMPARE,    // Trạng thái đang so sánh 2 node (sẽ highlight màu Vàng)
    H_SWAP,       // Trạng thái hoán vị 2 node (sẽ highlight màu Xanh lá)
    H_HIGHLIGHT,  // Trạng thái chọn/đánh dấu 1 node đang xét (sẽ highlight màu Cam)
    H_DONE        // Đánh dấu hoàn thành toàn bộ thao tác
};

// Struct lưu trữ thông tin của từng bước chạy thuật toán
struct HeapStep {
    HeapOpType type;
    int idx1;      // Vị trí node thứ 1 trong mảng data (-1 nếu không dùng tới)
    int idx2;      // Vị trí node thứ 2 trong mảng data (-1 nếu không dùng tới)
    int lineCode;  // Dòng pseudo-code tương ứng để highlight trên màn hình
};

class BinaryHeap {
private:
    std::vector<int> data;
    std::vector<HeapStep> steps; // Hàng đợi lưu các bước chạy để UI pop ra và render từ từ

    // --- CÁC HÀM CỐT LÕI CỦA BẠN (GIỮ NGUYÊN) ---
    int parent(int index) const;
    int left(int index) const;
    int right(int index) const;

    void heapifyUp(int index);
    void heapifyDown(int index);

    // --- HÀM TRỢ GIÚP CHO VISUALIZATION ---
    // Dùng nội bộ trong file .cpp để ghi lại lịch sử chạy của heapifyUp/Down
    void log(HeapOpType t, int i1, int i2, int line);

public:
    BinaryHeap();
    ~BinaryHeap();

    // --- CÁC THAO TÁC MIN HEAP (GIỮ NGUYÊN) ---
    void buildHeap(const std::vector<int>& initialData);
    void clear();
    void insert(int value);
    int extractTop(); // Xóa và trả về phần tử nhỏ nhất gốc (O(log N))
    
    // --- GETTERS (GIỮ NGUYÊN) ---
    const std::vector<int>& getData() const;
    int getSize() const;
    bool isEmpty() const;

    // --- GETTERS CHO VISUALIZATION ---
    const std::vector<HeapStep>& getSteps() const;
    void clearSteps(); // Dùng để reset lịch sử sau khi animation chạy xong
};