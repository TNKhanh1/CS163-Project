#pragma once
#include "DataStructureState.h"
#include "Heap.h"
#include <raylib.h>
#include <raymath.h>
#include <vector>
#include <string>

// Enum quản lý các tác vụ đang chờ chạy animation (Step-by-step)
enum HeapTask { HEAP_TASK_NONE, HEAP_TASK_INSERT, HEAP_TASK_SEARCH, HEAP_TASK_UPDATE, HEAP_TASK_EXTRACT };

// Cấu trúc lưu trữ trạng thái hiển thị của từng Node trên màn hình
struct HeapNodeVisual {
    int value;
    Vector2 position;
    Vector2 targetPosition;
    Color color;
};

class HeapState : public DataStructureState 
{
public:
    HeapState();
    ~HeapState();

    // Các hàm vòng đời của State
    void loadAssets() override;
    void update(float deltaTime) override;
    void draw() override;

protected:
    // Cài đặt các hàm ảo bắt buộc từ DataStructureState
    void handleAnimationStep() override;
    void DrawSubMenuContent() override;
    void onExecuteOp(MainOp op) override;

private:
    BinaryHeap heap; 
    std::vector<HeapNodeVisual> visualNodes;

    // Quản lý trạng thái Animation (Step-by-step)
    HeapTask currentTask;
    int animCurrentIdx;
    int animParentIdx;
    int animTargetValue;
    int animUpdateValue;
    bool updateHeapifyUp;
    int insertAnimPhase;
    int extractAnimPhase;
    float previousZoomMultiplier;

    // Quản lý trạng thái UI phụ (Kế thừa pattern từ LinkedListState)
    int activeInputFocus; // -1: None, 0: Create, 1: Insert, 2: Search, 3: UpdIdx, 4: UpdVal
    int previousInputFocus;
    bool isCreateUserDefOpen;

    // Các hàm bổ trợ xử lý dữ liệu và giao diện
    void syncVisualNodes();
    void updateTargetPositions();
    void resetNodeColors();
    void startAnimation(HeapTask task, int val1, int val2 = 0);
    
    // Tách riêng hàm vẽ để code clear hơn
    void drawBinaryTree();
    void drawHorizontalArray();

    // Undo/redo state management
    struct HeapStateSnapshot {
        int activeCodeLine;
        HeapTask currentTask;
        int animCurrentIdx;
        int animParentIdx;
        int insertAnimPhase;
        int extractAnimPhase;
        std::vector<int> nodeValues;
        std::vector<Color> nodeColors;
    };
    std::vector<HeapStateSnapshot> history;

    void saveState() override;
    void undoState() override;
    void onModeSwitch(bool toAutoMode) override;
};