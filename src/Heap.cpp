#include "Heap.h"
#include <algorithm>
#include <stdexcept>

BinaryHeap::BinaryHeap() {}

BinaryHeap::~BinaryHeap() {
    clear();
}

// =========================================================
// PHẦN 1: HÀM TRỢ GIÚP CHO VISUALIZATION (MỚI THÊM)
// =========================================================

void BinaryHeap::log(HeapOpType t, int i1, int i2, int line) {
    steps.push_back({t, i1, i2, line});
}

const std::vector<HeapStep>& BinaryHeap::getSteps() const {
    return steps;
}

void BinaryHeap::clearSteps() {
    steps.clear();
}

// =========================================================
// PHẦN 2: CÁC HÀM CỐT LÕI (GIỮ NGUYÊN BẢN GỐC CỦA BẠN)
// =========================================================

int BinaryHeap::parent(int index) const { return (index - 1) / 2; }
int BinaryHeap::left(int index) const { return 2 * index + 1; }
int BinaryHeap::right(int index) const { return 2 * index + 2; }

void BinaryHeap::heapifyUp(int index) {
    while (index > 0) {
        int p = parent(index);
        
        // Log: Bắt đầu so sánh Node hiện tại và Node Cha
        log(H_COMPARE, index, p, 2); 

        // Thuật toán Min Heap nguyên bản
        if (data[p] > data[index]) {
            // Log: Báo hiệu đổi chỗ
            log(H_SWAP, index, p, 3); 
            
            std::swap(data[p], data[index]);
            index = p;
        } else {
            break; // Đã đúng vị trí Min Heap
        }
    }
}

void BinaryHeap::heapifyDown(int index) {
    int size = data.size();
    int minIndex = index;

    while (true) {
        int l = left(index);
        int r = right(index);

        // Kiểm tra và so sánh với con trái
        if (l < size) {
            log(H_COMPARE, minIndex, l, 4); // Log so sánh
            if (data[l] < data[minIndex]) {
                minIndex = l;
            }
        }
        
        // Kiểm tra và so sánh với con phải
        if (r < size) {
            log(H_COMPARE, minIndex, r, 5); // Log so sánh
            if (data[r] < data[minIndex]) {
                minIndex = r;
            }
        }

        // Nếu minIndex thay đổi, tiến hành hoán đổi
        if (minIndex != index) {
            log(H_SWAP, index, minIndex, 6); // Log hoán đổi
            
            std::swap(data[index], data[minIndex]);
            index = minIndex;
        } else {
            break; // Đã đúng vị trí Min Heap
        }
    }
}

void BinaryHeap::buildHeap(const std::vector<int>& initialData) {
    steps.clear();
    data = initialData;
    
    // Thuật toán O(N) build heap nguyên bản của bạn
    for (int i = (data.size() / 2) - 1; i >= 0; --i) {
        heapifyDown(i);
    }
    
    // Log báo hiệu hoàn thành
    log(H_DONE, -1, -1, 0);
}

void BinaryHeap::clear() {
    data.clear();
    steps.clear();
}

void BinaryHeap::insert(int value) {
    steps.clear(); // Xóa lịch sử cũ trước khi bắt đầu thao tác mới
    
    data.push_back(value);
    int i = data.size() - 1;
    
    // Log: Báo UI highlight Node vừa được thêm vào cuối mảng/cây
    log(H_HIGHLIGHT, i, -1, 1); 
    
    heapifyUp(i);
    
    // Log báo hiệu hoàn thành Insert
    log(H_DONE, -1, -1, 0);
}

int BinaryHeap::extractTop() {
    if (isEmpty()) return -1;
    
    steps.clear(); // Xóa lịch sử cũ
    
    int topValue = data[0];
    
    // Log: Báo UI highlight Root (chuẩn bị xóa)
    log(H_HIGHLIGHT, 0, -1, 1); 
    
    data[0] = data.back();
    data.pop_back();

    if (!isEmpty()) {
        // Log: Báo UI highlight Node cuối vừa được bốc lên Root
        log(H_HIGHLIGHT, 0, -1, 2); 
        heapifyDown(0);
    }
    
    // Log báo hiệu hoàn thành Extract
    log(H_DONE, -1, -1, 0);
    
    return topValue;
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

// =========================================================
// HÀM TÍNH TOÁN LAYOUT (CÂY & MẢNG)
// =========================================================

void HeapState::updateTargetLayouts()
{
    if (visualNodes.empty()) return;

    // 1. Tính toán tọa độ Cây Nhị Phân
    float rootX = (float)GetScreenWidth() / 2.0f - 150.0f; // Dịch sang trái một chút để nhường chỗ cho Pseudo-code
    float rootY = 150.0f;
    float baseSpacingX = 350.0f * zoomMultiplier; 
    float scaledLevelHeight = levelHeight * zoomMultiplier;

    visualNodes[0].treeTarget = {rootX, rootY};

    for (size_t i = 0; i < visualNodes.size(); i++) {
        int leftChild = 2 * i + 1;
        int rightChild = 2 * i + 2;
        
        int depth = (int)log2(i + 1);
        float currentSpacingX = baseSpacingX / pow(2.0f, depth); 

        if (leftChild < visualNodes.size()) {
            visualNodes[leftChild].treeTarget = {
                visualNodes[i].treeTarget.x - currentSpacingX, 
                visualNodes[i].treeTarget.y + scaledLevelHeight
            };
        }
        if (rightChild < visualNodes.size()) {
            visualNodes[rightChild].treeTarget = {
                visualNodes[i].treeTarget.x + currentSpacingX, 
                visualNodes[i].treeTarget.y + scaledLevelHeight
            };
        }
    }

    // 2. Tính toán tọa độ Mảng (Array)
    float currentArrayX = arrayStartX;
    float scaledArrayNodeSize = arrayNodeSize * zoomMultiplier;
    float arraySpacing = 10.0f * zoomMultiplier;

    for (size_t i = 0; i < visualNodes.size(); i++) {
        visualNodes[i].arrayTarget = { currentArrayX, arrayStartY };
        currentArrayX += (scaledArrayNodeSize + arraySpacing);
    }

    // 3. Nếu không có animation chạy, ép currentPos về treeTarget
    if (!isAnimating) {
        for (auto& node : visualNodes) {
            node.currentPos = node.treeTarget; // Ưu tiên hiển thị mượt trên cây
        }
    }
}

// =========================================================
// XỬ LÝ ANIMATION & STEP-BY-STEP
// =========================================================

void HeapState::handleAnimationStep()
{
    // Hàm này được gọi bởi DataStructureState khi animTimer >= stepDuration
    if (isManualStepMode) return; // Nếu đang bật chế độ thủ công, bỏ qua auto step

    if (currentStepIdx < (int)animationQueue.size() - 1) {
        currentStepIdx++;
        HeapStep step = animationQueue[currentStepIdx];
        
        activeCodeLine = step.lineCode;
        resetNodeColors(); // Xóa màu cũ

        if (step.type == H_COMPARE) {
            if (step.idx1 >= 0) visualNodes[step.idx1].color = YELLOW;
            if (step.idx2 >= 0) visualNodes[step.idx2].color = YELLOW;
        } 
        else if (step.type == H_SWAP) {
            if (step.idx1 >= 0 && step.idx2 >= 0) {
                visualNodes[step.idx1].color = GREEN;
                visualNodes[step.idx2].color = GREEN;
                
                // Đổi giá trị nội bộ để UI hiển thị đúng
                std::swap(visualNodes[step.idx1].value, visualNodes[step.idx2].value);
            }
        } 
        else if (step.type == H_HIGHLIGHT) {
            if (step.idx1 >= 0) visualNodes[step.idx1].color = ORANGE;
        }
    } else {
        // Hoàn thành chuỗi animation
        isAnimating = false;
        activeCodeLine = -1;
        resetNodeColors();
        syncVisuals();
    }
}

// =========================================================
// CÁC HÀM VẼ GIAO DIỆN CHÍNH
// =========================================================

void HeapState::draw()
{
    // 1. Vẽ các thành phần dùng chung (Background, Sliders)
    DataStructureState::drawSharedUI();

    // 2. Vẽ Tiêu đề
    const char* titleText = "MIN HEAP VISUALIZATION";
    DrawTextEx(listFont, titleText, { (1800.0f - MeasureTextEx(listFont, titleText, 55, 6.5f).x) / 2.0f, 20.0f }, 55, 6.5f, BLACK);

    // 3. Vẽ cấu trúc Cây và Mảng
    drawBinaryTree();
    drawHorizontalArray();

    // 4. Vẽ Pseudo-code và bộ điều khiển
    drawSourceCodeBox();
    if (isAnimating) drawStepControls();

    // 5. Vẽ Control Panel (Bánh răng và Menu Trượt)
    DrawTextureV(controlTex, controlBtnPos, WHITE);
    DrawSideMenuFrame({"Create", "Insert", "Search", "Update", "Extract Top"});
}

void HeapState::drawBinaryTree()
{
    float currentRadius = nodeRadius * zoomMultiplier;

    // Vẽ các đường nối (vẽ trước để nằm dưới hình tròn)
    for (size_t i = 1; i < visualNodes.size(); i++) {
        if (!visualNodes[i].isVisible) continue;
        int parentIdx = (i - 1) / 2;
        DrawLineEx(visualNodes[parentIdx].currentPos, visualNodes[i].currentPos, 3.0f * zoomMultiplier, DARKGRAY);
    }

    // Vẽ các Node
    for (const auto& node : visualNodes) {
        if (!node.isVisible) continue;

        DrawCircleV(node.currentPos, currentRadius, node.color);
        DrawCircleLines(node.currentPos.x, node.currentPos.y, currentRadius, DARKBLUE);
        
        float scaledFontSize = 25.0f * zoomMultiplier;
        const char* valText = TextFormat("%d", node.value);
        Vector2 tSize = MeasureTextEx(numberFont, valText, scaledFontSize, 1.0f);
        
        // Căn giữa số vào hình tròn
        DrawTextEx(numberFont, valText, { node.currentPos.x - tSize.x / 2.0f, node.currentPos.y - tSize.y / 2.0f }, scaledFontSize, 1.0f, WHITE);
    }
}

void HeapState::drawHorizontalArray()
{
    float scaledSize = arrayNodeSize * zoomMultiplier;
    
    // Tiêu đề mảng
    DrawTextEx(listFont, "Array Representation:", { arrayStartX, arrayStartY - 40.0f }, 25.0f * zoomMultiplier, 1.0f, DARKGRAY);

    for (size_t i = 0; i < visualNodes.size(); i++) {
        const auto& node = visualNodes[i];
        if (!node.isVisible) continue;

        Rectangle rect = { node.arrayTarget.x, node.arrayTarget.y, scaledSize, scaledSize };
        
        // Vẽ viền và nền
        DrawRectangleRec(rect, node.color);
        DrawRectangleLinesEx(rect, 2.0f, BLACK);

        // In giá trị ở giữa ô vuông
        float fontSize = 22.0f * zoomMultiplier;
        const char* valText = TextFormat("%d", node.value);
        Vector2 tSize = MeasureTextEx(numberFont, valText, fontSize, 1.0f);
        DrawTextEx(numberFont, valText, { rect.x + (scaledSize - tSize.x) / 2.0f, rect.y + (scaledSize - tSize.y) / 2.0f }, fontSize, 1.0f, BLACK);

        // In chỉ số Index bên dưới ô vuông
        const char* idxText = TextFormat("[%d]", i);
        Vector2 idxSize = MeasureTextEx(numberFont, idxText, 18.0f * zoomMultiplier, 1.0f);
        DrawTextEx(numberFont, idxText, { rect.x + (scaledSize - idxSize.x) / 2.0f, rect.y + scaledSize + 5.0f }, 18.0f * zoomMultiplier, 1.0f, DARKGRAY);
    }
}

void HeapState::drawSourceCodeBox()
{
    float panelX = (float)GetScreenWidth() - 400.0f;
    float panelY = 150.0f;
    float panelWidth = 350.0f;
    float panelHeight = 350.0f;

    // Nền mờ cho khung Code
    DrawRectangle(panelX, panelY, panelWidth, panelHeight, Fade(LIGHTGRAY, 0.8f));
    DrawRectangleLines(panelX, panelY, panelWidth, panelHeight, DARKGRAY);

    // Tiêu đề thuật toán
    DrawRectangle(panelX, panelY, panelWidth, 40.0f, DARKBLUE);
    DrawTextEx(listFont, currentOperationName.c_str(), { panelX + 10.0f, panelY + 10.0f }, 22.0f, 1.0f, WHITE);

    // In các dòng Pseudo-code
    float textY = panelY + 50.0f;
    for (int i = 0; i < (int)pseudoCodeLines.size(); i++) {
        // Highlight đỏ nếu dòng này đang được thực thi
        if (i == activeCodeLine) {
            DrawRectangle(panelX, textY - 2.0f, panelWidth, 25.0f, Fade(YELLOW, 0.5f)); // Block highlight vàng nhạt
            DrawTextEx(numberFont, pseudoCodeLines[i].c_str(), { panelX + 15.0f, textY }, 20.0f, 1.0f, RED); // Chữ đỏ
        } else {
            DrawTextEx(numberFont, pseudoCodeLines[i].c_str(), { panelX + 15.0f, textY }, 20.0f, 1.0f, BLACK);
        }
        textY += 25.0f;
    }
}

void HeapState::drawStepControls()
{
    float ctrlX = (float)GetScreenWidth() - 400.0f;
    float ctrlY = 520.0f;

    // Vẽ nút Toggle Chế độ tự động / Thủ công
    const char* modeText = isManualStepMode ? "Mode: MANUAL" : "Mode: AUTO";
    Color modeColor = isManualStepMode ? ORANGE : LIME;
    if (DrawButtonText({ctrlX, ctrlY}, modeText, 160.0f, 40.0f, false)) {
        isManualStepMode = !isManualStepMode;
    }

    // Nếu đang ở chế độ thủ công, hiển thị thêm nút Next Step
    if (isManualStepMode) {
        if (DrawButtonText({ctrlX + 170.0f, ctrlY}, "Next Step >>", 180.0f, 40.0f, false)) {
            // Tự động gọi ép hàm handleAnimationStep chạy 1 nhịp
            handleAnimationStep();
        }
    }
}

// =========================================================
// HÀM VẼ SUB-MENU (TƯƠNG TỰ LINKED LIST)
// =========================================================

void HeapState::DrawSubMenuContent()
{
    float mainHeight = 45.0f, gap = 8.0f;
    float subX = controlBtnPos.x + (float)controlTex.width + 15.0f + 125.0f + gap;
    float startY = controlBtnPos.y;

    switch (activeMainOp) 
    {
        case OP_SLOT1: // Create
            if (DrawButtonText({subX, startY}, "Empty", 90, mainHeight, false)) onExecuteOp(OP_SLOT1); // Sẽ xử lý sau nếu input empty
            if (DrawButtonText({subX + 98, startY}, "User Defined", 160, mainHeight, isCreateUserDefOpen)) isCreateUserDefOpen = !isCreateUserDefOpen;
            if (DrawButtonText({subX + 266, startY}, "Random", 110, mainHeight, false)) {
                heap.clear();
                std::vector<int> randData;
                for(int i=0; i < GetRandomValue(5, 15); i++) randData.push_back(GetRandomValue(1, 99));
                heap.buildHeap(randData);
                syncVisuals();
            }
            if (isCreateUserDefOpen) {
                if (DrawTextBox({subX + 98, startY + mainHeight + gap}, inputBuffers[0], activeInputFocus == 0, 230, mainHeight, cursorIndex, textScrollX, cursorVisible)) activeInputFocus = 0;
                if (DrawButtonText({subX + 336, startY + mainHeight + gap}, "GO", 50, mainHeight, false)) onExecuteOp(OP_SLOT1);
            }
            break;

        case OP_SLOT2: // Insert
            DrawLabel({subX, startY + mainHeight + gap}, "Value=");
            if (DrawTextBox({subX + 80, startY + mainHeight + gap}, inputBuffers[1], activeInputFocus == 1, 100, mainHeight, cursorIndex, textScrollX, cursorVisible)) activeInputFocus = 1;
            if (DrawButtonText({subX + 190, startY + mainHeight + gap}, "GO", 50, mainHeight, false)) onExecuteOp(OP_SLOT2);
            break;

        case OP_SLOT3: // Search
            DrawLabel({subX, startY + 2 * (mainHeight + gap)}, "Value=");
            if (DrawTextBox({subX + 80, startY + 2 * (mainHeight + gap)}, inputBuffers[2], activeInputFocus == 2, 100, mainHeight, cursorIndex, textScrollX, cursorVisible)) activeInputFocus = 2;
            if (DrawButtonText({subX + 190, startY + 2 * (mainHeight + gap)}, "GO", 50, mainHeight, false)) onExecuteOp(OP_SLOT3);
            break;
            
        case OP_SLOT4: // Update
            DrawLabel({subX, startY + 3 * (mainHeight + gap)}, "Idx=");
            if (DrawTextBox({subX + 50, startY + 3 * (mainHeight + gap)}, inputBuffers[3], activeInputFocus == 3, 60, mainHeight, cursorIndex, textScrollX, cursorVisible)) activeInputFocus = 3;
            DrawLabel({subX + 120, startY + 3 * (mainHeight + gap)}, "Val=");
            if (DrawTextBox({subX + 170, startY + 3 * (mainHeight + gap)}, inputBuffers[4], activeInputFocus == 4, 60, mainHeight, cursorIndex, textScrollX, cursorVisible)) activeInputFocus = 4;
            if (DrawButtonText({subX + 240, startY + 3 * (mainHeight + gap)}, "GO", 50, mainHeight, false)) onExecuteOp(OP_SLOT4);
            break;

        case OP_SLOT5: // Extract Top
            if (DrawButtonText({subX, startY + 4 * (mainHeight + gap)}, "POP ROOT", 120, mainHeight, false)) onExecuteOp(OP_SLOT5);
            break;

        default: break;
    }
}