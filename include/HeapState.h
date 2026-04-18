#pragma once
#include "Heap.h"
#include "DataStructureState.h"
#include <vector>
#include <string>

// Cấu trúc để quản lý visual cho từng Node trên cả Cây và Mảng
struct HeapNodeVisual {
    int value;
    Vector2 treePos;    // Tọa độ trên cây
    Vector2 arrayPos;   // Tọa độ trong mảng
    Vector2 currentPos; // Tọa độ hiển thị hiện tại (dùng để Lerp)
    Color color;
};

class HeapState : public DataStructureState 
{
public:
    HeapState();
    ~HeapState();

    void loadAssets() override;
    void update(float deltaTime) override;
    void draw() override;

protected:
    // Thực thi các hook từ DataStructureState
    void DrawSubMenuContent() override;
    void onExecuteOp(MainOp op) override;
    void handleAnimationStep() override;

private:
    // Dữ liệu lõi
    BinaryHeap heap;
    std::vector<HeapNodeVisual> visualNodes;
    
    // Quản lý Animation & Step-by-step 
    std::vector<HeapStep> animationQueue;
    int currentStepIdx;
    int activeCodeLine; 
    bool isStepByStepMode;

    // Pseudo-code để hiển thị và highlight 
    std::vector<std::string> currentPseudoCode;

    // Các thông số điều chỉnh giao diện
    float nodeRadius;
    float levelHeight;
    float arrayStartX, arrayStartY;

    // Hàm bổ trợ logic visualization
    void syncVisuals();         // Cập nhật số lượng node visual khớp với data
    void updateTargetLayout();  // Tính toán lại treePos và arrayPos cho toàn bộ node
    void startAnimation(const std::vector<HeapStep>& steps, const std::vector<std::string>& code);
    
    // Các hàm vẽ thành phần riêng biệt
    void drawTree();
    void drawArray();
    void drawCodePanel();
    void drawStepControls(); // Nút Next/Prev/Final cho chế độ step-by-step 

    // Menu state
    int activeInputFocus;
    bool isCreateUserDefOpen;
};