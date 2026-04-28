#include "HeapState.h"
#include "State.h"
#include <iostream>
#include <cmath>
#include "raymath.h"

// ---------------------------------------------------------
// 1. CONSTRUCTOR & DESTRUCTOR
// ---------------------------------------------------------
HeapState::HeapState() : DataStructureState()
{
    NextState = (int)STATE_HEAP;
    
    // Khởi tạo các biến Animation
    currentTask = HEAP_TASK_NONE;
    animCurrentIdx = -1;
    animParentIdx = -1;
    animTargetValue = -1;
    animUpdateValue = -1;
    updateHeapifyUp = false;
    insertAnimPhase = 0;
    extractAnimPhase = 0;
    activeCodeLine = -1;
    previousZoomMultiplier = 1.0f;

    // Khởi tạo các biến quản lý Input
    activeInputFocus = -1;
    previousInputFocus = -1;
    isCreateUserDefOpen = false;

    // Gán mã giả mặc định khi chưa có thao tác nào
    pseudoCode = {
        "// Select an operation from",
        "// the control panel to see",
        "// the algorithm execution."
    };
}

HeapState::~HeapState()
{
    heap.clear();
    visualNodes.clear();
}

// ---------------------------------------------------------
// 2. LOAD ASSETS
// ---------------------------------------------------------
void HeapState::loadAssets()
{
    // Gọi hàm loadAssets của lớp cha để nạp Background, Font, Button dùng chung
    DataStructureState::loadAssets(); 
}

// ---------------------------------------------------------
// 3. UPDATE (XỬ LÝ LOGIC THEO THỜI GIAN THỰC)
// ---------------------------------------------------------
void HeapState::update(float deltaTime)
{
    Vector2 mousePos = GetMousePosition();
    
    // 3.1. Cập nhật các UI dùng chung (Slider tốc độ, Slider zoom, Bắt sự kiện Animation)
    DataStructureState::updateSharedUI(deltaTime, mousePos);
    DataStructureState::updateControlPanel(deltaTime, mousePos);

    // 3.2. Cập nhật lại tọa độ đích nếu người dùng kéo Slider Zoom
    if (zoomMultiplier != previousZoomMultiplier) {
        updateTargetPositions();
        previousZoomMultiplier = zoomMultiplier;
    }

    // 3.3. Xử lý Logic nhấp nháy con trỏ và Focus vào TextBox
    if (activeInputFocus != previousInputFocus) {
        cursorIndex = (activeInputFocus == -1) ? 0 : inputBuffers[activeInputFocus].length();
        textScrollX = 0.0f; 
        cursorBlinkTimer = 0.0f;
        cursorVisible = true;
        previousInputFocus = activeInputFocus;
    }

    if (activeInputFocus != -1) {
        // Gửi cờ 'true' nếu đang ở ô Create (để cho phép nhập dấu phẩy ',')
        HandleTextInput(inputBuffers[activeInputFocus], (activeInputFocus == 0));
    }

    // 3.4. Cập nhật hiệu ứng di chuyển (Lerp) cho toàn bộ Node trên màn hình
    for (size_t i = 0; i < visualNodes.size(); i++) 
    {
        visualNodes[i].position = Vector2Lerp(
            visualNodes[i].position, 
            visualNodes[i].targetPosition, 
            deltaTime * 8.0f 
        );
    }
}

// ---------------------------------------------------------
// 4. MENU GIAO DIỆN (SUB-MENU)
// ---------------------------------------------------------
void HeapState::DrawSubMenuContent()
{
    float mainHeight = 45.0f;
    float gap = 8.0f;
    float subX = controlBtnPos.x + (float)controlTex.width + 15.0f + 125.0f + gap;
    float startY = controlBtnPos.y;

    switch (activeMainOp) 
    {
        case OP_SLOT1: // CREATE
            if (DrawButtonText({subX, startY}, "Empty", 90, mainHeight, false)) {
                heap.clear();
                syncVisualNodes();
            }
            if (DrawButtonText({subX + 98, startY}, "User Defined", 160, mainHeight, isCreateUserDefOpen)) {
                isCreateUserDefOpen = !isCreateUserDefOpen;
            }
            if (DrawButtonText({subX + 266, startY}, "Random", 110, mainHeight, false)) {
                heap.clear();
                std::vector<int> randData;
                int numNodes = GetRandomValue(5, 15);
                for(int i = 0; i < numNodes; i++) randData.push_back(GetRandomValue(1, 99));
                heap.buildHeap(randData);
                syncVisualNodes();
            }
            if (isCreateUserDefOpen) {
                if (DrawTextBox({subX + 98, startY + mainHeight + gap}, inputBuffers[0], activeInputFocus == 0, 230, mainHeight, cursorIndex, textScrollX, cursorVisible)) {
                    activeInputFocus = 0;
                }
                if (DrawButtonText({subX + 336, startY + mainHeight + gap}, "GO", 50, mainHeight, false)) {
                    onExecuteOp(OP_SLOT1);
                }
            }
            break;

        case OP_SLOT2: // INSERT
            DrawLabel({subX, startY + mainHeight + gap}, "Value=");
            if (DrawTextBox({subX + 80, startY + mainHeight + gap}, inputBuffers[1], activeInputFocus == 1, 100, mainHeight, cursorIndex, textScrollX, cursorVisible)) {
                activeInputFocus = 1;
            }
            if (DrawButtonText({subX + 190, startY + mainHeight + gap}, "GO", 50, mainHeight, false)) {
                onExecuteOp(OP_SLOT2);
            }
            break;

        case OP_SLOT3: // SEARCH
            DrawLabel({subX, startY + 2 * (mainHeight + gap)}, "Value=");
            if (DrawTextBox({subX + 80, startY + 2 * (mainHeight + gap)}, inputBuffers[2], activeInputFocus == 2, 100, mainHeight, cursorIndex, textScrollX, cursorVisible)) {
                activeInputFocus = 2;
            }
            if (DrawButtonText({subX + 190, startY + 2 * (mainHeight + gap)}, "GO", 50, mainHeight, false)) {
                onExecuteOp(OP_SLOT3);
            }
            break;

        case OP_SLOT4: // UPDATE
            DrawLabel({subX, startY + 3 * (mainHeight + gap)}, "Idx=");
            if (DrawTextBox({subX + 50, startY + 3 * (mainHeight + gap)}, inputBuffers[3], activeInputFocus == 3, 60, mainHeight, cursorIndex, textScrollX, cursorVisible)) {
                activeInputFocus = 3;
            }
            DrawLabel({subX + 120, startY + 3 * (mainHeight + gap)}, "Val=");
            if (DrawTextBox({subX + 170, startY + 3 * (mainHeight + gap)}, inputBuffers[4], activeInputFocus == 4, 60, mainHeight, cursorIndex, textScrollX, cursorVisible)) {
                activeInputFocus = 4;
            }
            if (DrawButtonText({subX + 240, startY + 3 * (mainHeight + gap)}, "GO", 50, mainHeight, false)) {
                onExecuteOp(OP_SLOT4);
            }
            break;

        case OP_SLOT5: // DELETE (EXTRACT MIN/MAX)
            if (DrawButtonText({subX, startY + 4 * (mainHeight + gap)}, "Extract Root", 150, mainHeight, false)) {
                onExecuteOp(OP_SLOT5);
            }
            break;

        default: 
            break;
    }
}

void HeapState::onExecuteOp(MainOp op)
{
    try {
        switch (op) {
            case OP_SLOT1: // Create
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
                    syncVisualNodes();
                    inputBuffers[0].clear();
                    isCreateUserDefOpen = false;
                }
                break;

            case OP_SLOT2: // Insert
                if (!inputBuffers[1].empty()) {
                    int val = std::stoi(inputBuffers[1]);
                    startAnimation(HEAP_TASK_INSERT, val);
                    inputBuffers[1].clear();
                }
                break;

            case OP_SLOT3: // Search
                if (!inputBuffers[2].empty()) {
                    int val = std::stoi(inputBuffers[2]);
                    startAnimation(HEAP_TASK_SEARCH, val);
                    inputBuffers[2].clear();
                }
                break;

            case OP_SLOT4: // Update
                if (!inputBuffers[3].empty() && !inputBuffers[4].empty()) {
                    int targetIdx = std::stoi(inputBuffers[3]);
                    int newVal = std::stoi(inputBuffers[4]);
                    
                    if (targetIdx >= 0 && targetIdx < (int)visualNodes.size()) {
                        startAnimation(HEAP_TASK_UPDATE, targetIdx, newVal);
                    } else {
                        inputErrorMsg = "Index out of bounds!";
                        inputErrorTimer = 2.5f;
                        currentErrorSlot = 4;
                    }
                    inputBuffers[3].clear();
                    inputBuffers[4].clear();
                }
                break;

            case OP_SLOT5: // Extract
                if (!heap.isEmpty()) {
                    startAnimation(HEAP_TASK_EXTRACT, 0);
                } else {
                    inputErrorMsg = "Heap is empty!";
                    inputErrorTimer = 2.5f;
                    currentErrorSlot = 5;
                }
                break;

            default: 
                break;
        }
    } catch (...) {
        inputErrorMsg = "Invalid Input!";
        inputErrorTimer = 2.0f;
    }
    activeInputFocus = -1;
}

// ---------------------------------------------------------
// 5. LOGIC ANIMATION & STEP-BY-STEP
// ---------------------------------------------------------
void HeapState::startAnimation(HeapTask task, int val1, int val2)
{
    history.clear(); // Clear history when starting a new animation
    syncVisualNodes();
    currentTask = task;
    animCurrentIdx = 0;
    animParentIdx = -1;
    animTargetValue = val1;
    animUpdateValue = val2;
    isAnimFinished = false;
    isAnimating = true;
    animTimer = 0.0f;

    // Khởi tạo mã giả (Pseudo-code) cho từng chức năng
    if (task == HEAP_TASK_SEARCH) {
        pseudoCode = {
            "int search(int val) {",
            "    for (int i = 0; i < size; i++) {",
            "        if (data[i] == val)",
            "            return i;",
            "    }",
            "    return -1;",
            "}"
        };
    } 
    else if (task == HEAP_TASK_INSERT) {
        pseudoCode = {
            "void insert(int val) {",
            "    data.push_back(val);",
            "    int i = size - 1;",
            "    while (i > 0 && data[parent(i)] > data[i]) {",
            "        swap(data[parent(i)], data[i]);",
            "        i = parent(i);",
            "    }",
            "}"
        };
        insertAnimPhase = 0;
        // Thêm node mới vào cuối mảng trực quan trước khi bắt đầu step-by-step
        heap.insert(val1); // Thêm vào dữ liệu gốc
        
        HeapNodeVisual newNode;
        newNode.value = val1;
        newNode.color = SKYBLUE;
        newNode.position = { 1800.0f / 2.0f, 150.0f }; // Rơi từ đỉnh xuống
        visualNodes.push_back(newNode);
        
        updateTargetPositions();
        animCurrentIdx = visualNodes.size() - 1; // Bắt đầu duyệt từ node cuối
    }
    else if (task == HEAP_TASK_EXTRACT) {
        pseudoCode = {
            "int extract() {",
            "    int root = data[0];",
            "    data[0] = data.back();",
            "    data.pop_back();",
            "    heapifyDown(0);",
            "    return root;",
            "}"
        };
        extractAnimPhase = 0;
        animCurrentIdx = 0; // Sẽ xử lý trong handleAnimationStep
    }
    else if (task == HEAP_TASK_UPDATE) {
        const auto& data = heap.getData();
        if (val1 < 0 || val1 >= (int)data.size()) {
            currentTask = HEAP_TASK_NONE;
            isAnimating = false;
            return;
        }
        int oldValue = data[val1];
        updateHeapifyUp = (val2 < oldValue);

        pseudoCode = {
            "void update(int idx, int val) {",
            "    data[idx] = val;",
            updateHeapifyUp ? "    while (idx > 0 && data[parent(idx)] > data[idx]) {" : "    while (hasSmallerChild(idx)) {",
            updateHeapifyUp ? "        swap(data[parent(idx)], data[idx]);" : "        swap(data[idx], data[smallestChild(idx)]);",
            updateHeapifyUp ? "        idx = parent(idx);" : "        idx = smallestChild(idx);",
            "    }",
            "}"
        };

        heap.setValue(val1, val2);
        animCurrentIdx = val1; // Lưu index cần update
        syncVisualNodes();
    }
}

void HeapState::handleAnimationStep()
{
    // Safeguard: Nếu animation không active, reset màu và return
    if (currentTask == HEAP_TASK_NONE) {
        resetNodeColors();
        return;
    }

    // Tìm kiếm: Duyệt qua toàn bộ mảng
    if (currentTask == HEAP_TASK_SEARCH) {
        resetNodeColors();
        activeCodeLine = 1; // for loop

        if (animCurrentIdx < (int)visualNodes.size()) {
            visualNodes[animCurrentIdx].color = YELLOW;
            
            if (visualNodes[animCurrentIdx].value == animTargetValue) {
                visualNodes[animCurrentIdx].color = ORANGE;
                activeCodeLine = 3; // return i
                isAnimating = false;
                currentTask = HEAP_TASK_NONE;
            } else {
                animCurrentIdx++;
            }
        } else {
            activeCodeLine = 5; // return -1
            inputErrorMsg = "Value not found!";
            inputErrorTimer = 2.5f;
            currentErrorSlot = 2;
            isAnimating = false;
            currentTask = HEAP_TASK_NONE;
            resetNodeColors();
        }
    }
    // Thêm (Insert): Heapify Up từ dưới lên
    else if (currentTask == HEAP_TASK_INSERT) {
        resetNodeColors();
        
        if (animCurrentIdx > 0) {
            animParentIdx = (animCurrentIdx - 1) / 2;
            visualNodes[animCurrentIdx].color = YELLOW;
            visualNodes[animParentIdx].color = YELLOW;

            if (insertAnimPhase == 0) {
                activeCodeLine = 3; // while condition
                if (visualNodes[animCurrentIdx].value < visualNodes[animParentIdx].value) {
                    insertAnimPhase = 1;
                } else {
                    // Đã đúng vị trí
                    activeCodeLine = -1;
                    isAnimating = false;
                    currentTask = HEAP_TASK_NONE;
                    resetNodeColors(); // Reset màu trước khi kết thúc
                    syncVisualNodes(); // Đồng bộ lại 1 lần cuối cho chắc chắn
                }
            } else if (insertAnimPhase == 1) {
                activeCodeLine = 4; // swap
                visualNodes[animCurrentIdx].color = GREEN;
                visualNodes[animParentIdx].color = GREEN;
                std::swap(visualNodes[animCurrentIdx].value, visualNodes[animParentIdx].value);
                insertAnimPhase = 2;
            } else if (insertAnimPhase == 2) {
                activeCodeLine = 5; // i = parent
                animCurrentIdx = animParentIdx; // Tiếp tục đi lên
                insertAnimPhase = 0;
            }
        } else {
            activeCodeLine = -1;
            isAnimating = false;
            currentTask = HEAP_TASK_NONE;
            syncVisualNodes();
            resetNodeColors();
        }
    }
    // Xóa (Extract): Heapify Down từ trên xuống
    else if (currentTask == HEAP_TASK_EXTRACT) {
        resetNodeColors();
        
        if (heap.isEmpty()) {
            activeCodeLine = -1;
            isAnimating = false;
            currentTask = HEAP_TASK_NONE;
            resetNodeColors();
            return;
        }

        int heapSize = (int)visualNodes.size();
        
        if (extractAnimPhase == 0) {
            // Phase 0: Highlight root for extraction
            activeCodeLine = 1; // int root = data[0]
            visualNodes[0].color = ORANGE;
            extractAnimPhase = 1;
        } else if (extractAnimPhase == 1) {
            // Phase 1: Move last element to root
            activeCodeLine = 2; // data[0] = data.back()
            if (heapSize > 1) {
                visualNodes[heapSize - 1].color = YELLOW;
                visualNodes[0].color = YELLOW;
            }
            extractAnimPhase = 2;
        } else if (extractAnimPhase == 2) {
            // Phase 2: Remove last element (pop_back)
            activeCodeLine = 3; // data.pop_back()
            if (heapSize > 1) {
                // Move last element to root
                visualNodes[0].value = visualNodes[heapSize - 1].value;
                visualNodes.pop_back();
                updateTargetPositions(); // Update positions after removing node
            } else {
                // Only 1 element, just remove it
                visualNodes.pop_back();
                activeCodeLine = -1;
                isAnimating = false;
                currentTask = HEAP_TASK_NONE;
                extractAnimPhase = 0;
                resetNodeColors();
                // Update heap data
                std::vector<int> heapData;
                for (auto& node : visualNodes) {
                    heapData.push_back(node.value);
                }
                heap.buildHeap(heapData);
                return;
            }
            animCurrentIdx = 0; // Start heapify from root
            extractAnimPhase = 3;
        } else if (extractAnimPhase >= 3) {
            // Phase 3+: Heapify down process
            activeCodeLine = 4; // heapifyDown(0)
            
            int size = (int)visualNodes.size();
            if (animCurrentIdx >= size) {
                activeCodeLine = -1;
                isAnimating = false;
                currentTask = HEAP_TASK_NONE;
                extractAnimPhase = 0;
                resetNodeColors();
                // Update heap data
                std::vector<int> heapData;
                for (auto& node : visualNodes) {
                    heapData.push_back(node.value);
                }
                heap.buildHeap(heapData);
                return;
            }

            visualNodes[animCurrentIdx].color = YELLOW;
            
            int leftIdx = animCurrentIdx * 2 + 1;
            int rightIdx = animCurrentIdx * 2 + 2;
            int smallest = animCurrentIdx;

            if (leftIdx < size && visualNodes[leftIdx].value < visualNodes[smallest].value) {
                smallest = leftIdx;
            }
            if (rightIdx < size && visualNodes[rightIdx].value < visualNodes[smallest].value) {
                smallest = rightIdx;
            }

            if (smallest != animCurrentIdx) {
                visualNodes[smallest].color = YELLOW;
                visualNodes[animCurrentIdx].color = GREEN;
                visualNodes[smallest].color = GREEN;
                std::swap(visualNodes[animCurrentIdx].value, visualNodes[smallest].value);
                updateTargetPositions(); // Update positions after swap
                animCurrentIdx = smallest;
            } else {
                activeCodeLine = -1;
                isAnimating = false;
                currentTask = HEAP_TASK_NONE;
                extractAnimPhase = 0;
                resetNodeColors();
                // Update heap data
                std::vector<int> heapData;
                for (auto& node : visualNodes) {
                    heapData.push_back(node.value);
                }
                heap.buildHeap(heapData);
            }
        }
    }
    else if (currentTask == HEAP_TASK_UPDATE) {
        resetNodeColors();
        activeCodeLine = 3; // while condition

        const auto& data = heap.getData();
        if (animCurrentIdx < 0 || animCurrentIdx >= heap.getSize()) {
            activeCodeLine = -1;
            isAnimating = false;
            currentTask = HEAP_TASK_NONE;
            resetNodeColors();
            return;
        }

        visualNodes[animCurrentIdx].color = YELLOW;

        if (updateHeapifyUp) {
            if (animCurrentIdx > 0) {
                animParentIdx = (animCurrentIdx - 1) / 2;
                visualNodes[animParentIdx].color = YELLOW;
                activeCodeLine = 4; // swap

                if (data[animCurrentIdx] < data[animParentIdx]) {
                    visualNodes[animCurrentIdx].color = GREEN;
                    visualNodes[animParentIdx].color = GREEN;
                    int currentValue = data[animCurrentIdx];
                    int parentValue = data[animParentIdx];
                    heap.setValue(animCurrentIdx, parentValue);
                    heap.setValue(animParentIdx, currentValue);
                    syncVisualNodes();
                    animCurrentIdx = animParentIdx;
                } else {
                    activeCodeLine = -1;
                    isAnimating = false;
                    currentTask = HEAP_TASK_NONE;
                    resetNodeColors();
                }
            } else {
                activeCodeLine = -1;
                isAnimating = false;
                currentTask = HEAP_TASK_NONE;
                resetNodeColors();
            }
        } else {
            int size = heap.getSize();
            int leftIdx = animCurrentIdx * 2 + 1;
            int rightIdx = animCurrentIdx * 2 + 2;
            int smallest = animCurrentIdx;

            if (leftIdx < size && data[leftIdx] < data[smallest]) smallest = leftIdx;
            if (rightIdx < size && data[rightIdx] < data[smallest]) smallest = rightIdx;

            if (smallest != animCurrentIdx) {
                visualNodes[smallest].color = YELLOW;
                activeCodeLine = 4; // swap
                visualNodes[animCurrentIdx].color = GREEN;
                visualNodes[smallest].color = GREEN;
                int currentValue = data[animCurrentIdx];
                int smallestValue = data[smallest];
                heap.setValue(animCurrentIdx, smallestValue);
                heap.setValue(smallest, currentValue);
                syncVisualNodes();
                animCurrentIdx = smallest;
            } else {
                activeCodeLine = -1;
                isAnimating = false;
                currentTask = HEAP_TASK_NONE;
                resetNodeColors();
            }
        }
    }
}

// ---------------------------------------------------------
// 6. ĐỒNG BỘ DỮ LIỆU & TÍNH TOÁN TỌA ĐỘ
// ---------------------------------------------------------
void HeapState::syncVisualNodes()
{
    const auto& data = heap.getData();
    
    while (visualNodes.size() < data.size()) {
        HeapNodeVisual newNode;
        newNode.value = 0;
        newNode.color = SKYBLUE;
        if (visualNodes.empty()) {
            newNode.position = { 1800.0f / 2.0f, 150.0f };
        } else {
            int pIdx = (visualNodes.size() - 1) / 2;
            newNode.position = visualNodes[pIdx].position;
        }
        visualNodes.push_back(newNode);
    }
    
    while (visualNodes.size() > data.size()) {
        visualNodes.pop_back();
    }
    
    for (size_t i = 0; i < data.size(); i++) {
        visualNodes[i].value = data[i];
    }
    
    updateTargetPositions();
}

void HeapState::updateTargetPositions()
{
    if (visualNodes.empty()) return;

    // Tọa độ Cây Nhị Phân
    float rootX = 1800.0f / 2.0f - 150.0f; // Dịch sang trái nhường chỗ cho Code Panel
    float rootY = 150.0f;
    float baseSpacingX = 250.0f * zoomMultiplier; 
    float levelHeight = 100.0f * zoomMultiplier;

    visualNodes[0].targetPosition = {rootX, rootY};

    for (size_t i = 0; i < visualNodes.size(); i++) {
        int leftChild = 2 * i + 1;
        int rightChild = 2 * i + 2;
        
        int depth = (int)log2(i + 1);
        float currentSpacingX = baseSpacingX / pow(2.0f, depth); 

        if (leftChild < (int)visualNodes.size()) {
            visualNodes[leftChild].targetPosition = {
                visualNodes[i].targetPosition.x - currentSpacingX, 
                visualNodes[i].targetPosition.y + levelHeight
            };
        }
        if (rightChild < (int)visualNodes.size()) {
            visualNodes[rightChild].targetPosition = {
                visualNodes[i].targetPosition.x + currentSpacingX, 
                visualNodes[i].targetPosition.y + levelHeight
            };
        }
    }
}

void HeapState::resetNodeColors()
{
    for (auto& node : visualNodes) {
        node.color = SKYBLUE;
    }
}

// ---------------------------------------------------------
// 7. VẼ GIAO DIỆN LÊN MÀN HÌNH (DRAWING)
// ---------------------------------------------------------
void HeapState::draw()
{
    // Vẽ nền, nút Home, và các thanh trượt
    DataStructureState::drawSharedUI();

    const char* titleText = "HEAP VISUALIZATION";
    DrawTextEx(listFont, titleText, { (1800.0f - MeasureTextEx(listFont, titleText, 55, 6.5f).x) / 2.0f, 20.0f }, 55, 6.5f, BLACK);

    BeginCanvasTransform();
    drawBinaryTree();
    EndCanvasTransform();
    drawHorizontalArray();

    // Vẽ Khung hiển thị Mã giả (Pseudo-code)
    drawPseudoCode();
    drawDropZone();
    // Vẽ Menu tùy chỉnh
    DrawTextureV(controlTex, controlBtnPos, WHITE);
    DrawSideMenuFrame({"Create", "Insert", "Search", "Update", "Delete"});
}

void HeapState::drawBinaryTree()
{
    float currentRadius = 35.0f * zoomMultiplier;

    // Vẽ các đường nối giữa Cha và Con
    for (size_t i = 1; i < visualNodes.size(); i++) {
        int parentIdx = (i - 1) / 2;
        DrawLineEx(visualNodes[parentIdx].position, visualNodes[i].position, 3.0f * zoomMultiplier, DARKGRAY);
    }

    // Vẽ Vòng tròn và Số liệu bên trong
    for (size_t i = 0; i < visualNodes.size(); i++) {
        DrawCircleV(visualNodes[i].position, currentRadius, visualNodes[i].color);
        DrawCircleLines(visualNodes[i].position.x, visualNodes[i].position.y, currentRadius, DARKBLUE);
        
        float scaledFontSize = 25.0f * zoomMultiplier;
        const char* valText = TextFormat("%d", visualNodes[i].value);
        Vector2 tSize = MeasureTextEx(numberFont, valText, scaledFontSize, 1.0f);
        DrawTextEx(numberFont, valText, { visualNodes[i].position.x - tSize.x / 2.0f, visualNodes[i].position.y - tSize.y / 2.0f }, scaledFontSize, 1.0f, WHITE);
    }
}

void HeapState::drawHorizontalArray()
{
    if (visualNodes.empty()) return;

    float arrayStartX = 350.0f;
    float arrayStartY = 880.0f;
    float nodeSize = 60.0f * zoomMultiplier;
    float gap = 10.0f * zoomMultiplier;

    DrawTextEx(listFont, "Array Representation:", { arrayStartX, arrayStartY - 40.0f }, 25.0f * zoomMultiplier, 1.0f, DARKGRAY);

    for (size_t i = 0; i < visualNodes.size(); i++) {
        float x = arrayStartX + i * (nodeSize + gap);
        float y = arrayStartY;
        Rectangle rect = { x, y, nodeSize, nodeSize };

        DrawRectangleRec(rect, visualNodes[i].color);
        DrawRectangleLinesEx(rect, 2.0f, BLACK);

        // Hiển thị giá trị (Value)
        float fontSize = 22.0f * zoomMultiplier;
        const char* valText = TextFormat("%d", visualNodes[i].value);
        Vector2 tSize = MeasureTextEx(numberFont, valText, fontSize, 1.0f);
        DrawTextEx(numberFont, valText, { rect.x + (nodeSize - tSize.x) / 2.0f, rect.y + (nodeSize - tSize.y) / 2.0f }, fontSize, 1.0f, BLACK);

        // Hiển thị chỉ mục (Index)
        const char* idxText = TextFormat("[%d]", i);
        Vector2 idxSize = MeasureTextEx(numberFont, idxText, 18.0f * zoomMultiplier, 1.0f);
        DrawTextEx(numberFont, idxText, { rect.x + (nodeSize - idxSize.x) / 2.0f, rect.y + nodeSize + 5.0f }, 18.0f * zoomMultiplier, 1.0f, DARKGRAY);
    }
}

// ---------------------------------------------------------
// 8. UNDO/REDO STATE MANAGEMENT
// ---------------------------------------------------------
void HeapState::saveState()
{
    HeapStateSnapshot snap;
    snap.activeCodeLine = activeCodeLine;
    snap.currentTask = currentTask;
    snap.animCurrentIdx = animCurrentIdx;
    snap.animParentIdx = animParentIdx;
    snap.insertAnimPhase = insertAnimPhase;
    snap.extractAnimPhase = extractAnimPhase;

    // Copy the exact state of every node into the snapshot
    for (auto& node : visualNodes) {
        snap.nodeValues.push_back(node.value);
        snap.nodeColors.push_back(node.color);
    }

    history.push_back(snap);
}

void HeapState::undoState()
{
    if (history.empty()) return;

    // Get the last snapshot and remove it from history
    HeapStateSnapshot snap = history.back();
    history.pop_back();

    // Restore animation state
    activeCodeLine = snap.activeCodeLine;
    currentTask = snap.currentTask;
    animCurrentIdx = snap.animCurrentIdx;
    animParentIdx = snap.animParentIdx;
    insertAnimPhase = snap.insertAnimPhase;
    extractAnimPhase = snap.extractAnimPhase;

    // Update existing nodes in-place to preserve current Vector2 physical screen coordinates
    size_t snapIndex = 0;

    // Phase A: Update existing nodes
    while (snapIndex < snap.nodeValues.size() && snapIndex < visualNodes.size()) {
        visualNodes[snapIndex].value = snap.nodeValues[snapIndex];
        visualNodes[snapIndex].color = snap.nodeColors[snapIndex];
        snapIndex++;
    }

    // Phase B: Snapshot has more nodes (undid a delete operation)
    while (snapIndex < snap.nodeValues.size()) {
        HeapNodeVisual newNode;
        newNode.value = snap.nodeValues[snapIndex];
        newNode.color = snap.nodeColors[snapIndex];
        newNode.position = { 1800.0f / 2.0f, 150.0f }; // Will be corrected by updateTargetPositions
        newNode.targetPosition = { 0, 0 };
        visualNodes.push_back(newNode);
        snapIndex++;
    }

    // Phase C: Snapshot has fewer nodes (undid an insert operation)
    while (visualNodes.size() > snap.nodeValues.size()) {
        visualNodes.pop_back();
    }

    // Sync with actual heap data
    std::vector<int> heapData;
    for (auto& node : visualNodes) {
        heapData.push_back(node.value);
    }
    heap.buildHeap(heapData);

    // Recalculate target positions
    updateTargetPositions();
}

void HeapState::onModeSwitch(bool toAutoMode)
{
    if (toAutoMode && isAnimating && !isAnimFinished)
    {
        // Complete the remaining animation steps automatically
        while (isAnimating && !isAnimFinished && CheckStepReady(0.0f, 0.0f))
        {
            saveState();
            handleAnimationStep();
        }
        
        // Reset animation state to idle - don't set isAnimFinished to true
        // as it may block new animations from starting
        isAnimating = false;
        currentTask = HEAP_TASK_NONE;
        activeCodeLine = -1;
        resetNodeColors();
        
        // Final sync to ensure tree is in correct state
        syncVisualNodes();
    }
}

bool HeapState::processDroppedFile(const std::string& filePath) 
{
    std::ifstream file(filePath);
    if (!file) return false;

    std::vector<int> temp;
    int val;
    while (file >> val) {
        temp.push_back(val);
        if (temp.size() > 20) { inputErrorMsg = "ERROR: MAX 20 NODES EXCEEDED"; return false; }
    }
    
    if (temp.empty()) return false;

    heap.clear(); 
    history.clear();

    for (int x : temp) heap.insert(x);
    
    isAnimating = false;
    isAnimFinished = true;
    currentTask = HEAP_TASK_NONE;
    activeCodeLine = -1;

    syncVisualNodes(); 
    updateTargetPositions();
    return true;
}