#pragma once
#include "DataStructureState.h"
#include "Heap.h"
#include <raylib.h>
#include <raymath.h>
#include <vector>
#include <string>

// Enum for managing animation tasks (step-by-step)
enum HeapTask { HEAP_TASK_NONE, HEAP_TASK_INSERT, HEAP_TASK_SEARCH, HEAP_TASK_UPDATE, HEAP_TASK_EXTRACT };

// Structure for storing the visual state of each node
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

    // State lifecycle functions
    void loadAssets() override;
    void update(float deltaTime) override;
    void draw() override;
    bool hasPrevStep() override { return !history.empty(); }

protected:
    // Implement required virtual functions from DataStructureState
    void handleAnimationStep() override;
    void DrawSubMenuContent() override;
    void onExecuteOp(MainOp op) override;

private:
    BinaryHeap heap; 
    std::vector<HeapNodeVisual> visualNodes;

    // Animation state management (step-by-step)
    HeapTask currentTask;
    int animCurrentIdx;
    int animParentIdx;
    int animTargetValue;
    int animUpdateValue;
    bool updateHeapifyUp;
    int insertAnimPhase;
    int extractAnimPhase;
    float previousZoomMultiplier;

    // UI state management (inherited pattern from LinkedListState)
    int activeInputFocus; // -1: None, 0: Create, 1: Insert, 2: Search, 3: UpdIdx, 4: UpdVal
    int previousInputFocus;
    bool isCreateUserDefOpen;

    // Helper functions for data and UI
    void syncVisualNodes();
    void updateTargetPositions();
    void resetNodeColors();
    void startAnimation(HeapTask task, int val1, int val2 = 0);

    // Drawing functions
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

    bool processDroppedFile(const std::string& filePath) override;
};