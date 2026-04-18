#include "HeapState.h"
#include <iostream>
#include <cmath>
#include "raymath.h"

HeapState::HeapState() : DataStructureState()
{
    // Cài đặt trạng thái chuyển cảnh cho Heap (giả định STATE_HEAP = 3)
    NextState = 3; 

    // Cấu hình thông số hiển thị ban đầu
    nodeRadius = 35.0f;
    levelHeight = 100.0f;
    arrayStartX = 100.0f;
    arrayStartY = 850.0f;
    arrayNodeSize = 60.0f;

    // Quản lý trạng thái Animation
    currentStepIdx = -1;
    activeCodeLine = -1;
    isManualStepMode = false;
    isAnimating = false;

    // Quản lý Input
    activeInputFocus = -1;
    isCreateUserDefOpen = false;
    isPanelOpen = false;

    // Khởi tạo các nhãn Pseudo-code mặc định
    currentOperationName = "MIN HEAP";
    pseudoCodeLines = { "Select an operation", "to view algorithm", "steps here." };
}

HeapState::~HeapState()
{
    heap.clear();
    visualNodes.clear();
    animationQueue.clear();
}

void HeapState::loadAssets()
{
    // Load assets chung từ DataStructureState (Background, Fonts, Home Button)
    DataStructureState::loadAssets(); 
}

void HeapState::update(float deltaTime)
{
    Vector2 mousePos = GetMousePosition();
    
    // 1. Cập nhật UI chung (Sliders, Home Button, Control Panel)
    DataStructureState::updateSharedUI(deltaTime, mousePos);
    DataStructureState::updateControlPanel(deltaTime, mousePos);

    // 2. Logic xử lý nhập liệu (Sử dụng inputBuffers từ lớp cha)
    if (activeInputFocus != -1) {
        HandleTextInput(inputBuffers[activeInputFocus], (activeInputFocus == 0));
    }

    // 3. Hiệu ứng Lerp cho các Node (Di chuyển mượt mà tới vị trí đích)
    for (auto& node : visualNodes) {
        if (node.isVisible) {
            // Tốc độ di chuyển phụ thuộc vào animSpeedMultiplier từ thanh slider
            float speed = deltaTime * 8.0f * animSpeedMultiplier;
            node.currentPos = Vector2Lerp(node.currentPos, node.treeTarget, speed);
        }
    }

    // 4. Kiểm tra sự thay đổi của Zoom để tính toán lại layout
    static float lastZoom = 1.0f;
    if (zoomMultiplier != lastZoom) {
        updateTargetLayout();
        lastZoom = zoomMultiplier;
    }
}

void HeapState::onExecuteOp(MainOp op)
{
    // Dừng animation cũ nếu có
    isAnimating = false;
    resetNodeColors();

    try {
        switch (op) {
            case OP_SLOT1: // CREATE
                if (!inputBuffers[0].empty()) {
                    heap.clear();
                    std::vector<int> vals;
                    std::string temp = "";
                    for (char c : inputBuffers[0]) {
                        if (c == ',') {
                            if (!temp.empty()) { vals.push_back(std::stoi(temp)); temp = ""; }
                        } else temp += c;
                    }
                    if (!temp.empty()) vals.push_back(std::stoi(temp));
                    
                    heap.buildHeap(vals);
                    syncVisuals();
                    inputBuffers[0].clear();
                    isCreateUserDefOpen = false;
                }
                break;

            case OP_SLOT2: // INSERT
                if (!inputBuffers[1].empty()) {
                    int val = std::stoi(inputBuffers[1]);
                    heap.insert(val);
                    
                    // Thiết lập Animation Step-by-step
                    std::vector<std::string> code = {
                        "void insert(int val) {",
                        "  data.push_back(val);",
                        "  int i = size - 1;",
                        "  while (i > 0 && data[parent] > data[i]) {",
                        "    swap(data[parent], data[i]);",
                        "    i = parent;",
                        "  }",
                        "}"
                    };
                    startAlgorithmAnimation(heap.getSteps(), "INSERT");
                    currentPseudoCode = code;
                    inputBuffers[1].clear();
                }
                break;

            case OP_SLOT3: // SEARCH
                if (!inputBuffers[2].empty()) {
                    int target = std::stoi(inputBuffers[2]);
                    bool found = false;
                    syncVisuals();
                    for (size_t i = 0; i < visualNodes.size(); i++) {
                        if (visualNodes[i].value == target) {
                            visualNodes[i].color = ORANGE;
                            found = true;
                        }
                    }
                    if (!found) {
                        inputErrorMsg = "Value not found!";
                        inputErrorTimer = 2.0f;
                        currentErrorSlot = 2;
                    }
                    inputBuffers[2].clear();
                }
                break;

            case OP_SLOT4: // UPDATE (Update value at index)
                if (!inputBuffers[3].empty() && !inputBuffers[4].empty()) {
                    // Giả định slot 3 là Index, slot 4 là New Value
                    // Note: BinaryHeap cần hàm updateIndex(idx, val) để log steps
                    // Ở đây demo logic cơ bản:
                    inputBuffers[3].clear(); inputBuffers[4].clear();
                }
                break;

            case OP_SLOT5: // DELETE (Extract Top)
                if (!heap.isEmpty()) {
                    heap.extractTop();
                    std::vector<std::string> code = {
                        "int extractTop() {",
                        "  int root = data[0];",
                        "  data[0] = data.back(); data.pop_back();",
                        "  heapifyDown(0);",
                        "  return root;",
                        "}"
                    };
                    startAlgorithmAnimation(heap.getSteps(), "EXTRACT TOP");
                    currentPseudoCode = code;
                } else {
                    inputErrorMsg = "Heap is empty!";
                    inputErrorTimer = 2.0f;
                    currentErrorSlot = 5;
                }
                break;

            default: break;
        }
    } catch (...) {
        inputErrorMsg = "Invalid Input!";
        inputErrorTimer = 2.0f;
    }
    activeInputFocus = -1;
}

void HeapState::syncVisuals()
{
    const auto& data = heap.getData();
    
    // Đồng bộ số lượng node
    while (visualNodes.size() < data.size()) {
        HeapVisualNode newNode;
        newNode.value = 0;
        newNode.currentPos = { (float)GetScreenWidth() / 2, -100 }; // Bay từ trên xuống
        newNode.color = SKYBLUE;
        newNode.isVisible = true;
        visualNodes.push_back(newNode);
    }
    while (visualNodes.size() > data.size()) {
        visualNodes.pop_back();
    }

    // Cập nhật giá trị
    for (size_t i = 0; i < data.size(); i++) {
        visualNodes[i].value = data[i];
    }

    updateTargetLayout();
}

void HeapState::startAlgorithmAnimation(const std::vector<HeapStep>& steps, std::string opName)
{
    syncVisuals();
    animationQueue = steps;
    currentStepIdx = -1;
    currentOperationName = opName;
    isAnimating = true;
    animTimer = 0.0f; 
}

void HeapState::resetNodeColors()
{
    for (auto& node : visualNodes) {
        node.color = SKYBLUE;
    }
}