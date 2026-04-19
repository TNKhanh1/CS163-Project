#pragma once
#include "DataStructureState.h"
#include "State.h"
#include "MST.h"

class MSTState : public DataStructureState
{
public:
    MSTState();
    ~MSTState();

    void loadAssets() override;
    void update(float deltaTime) override;
    void draw() override;

private:
    MST currentGraph;

    int activeInputFocus;
    int previousInputFocus;




    std::map<int, Vector2> nodePositions; // Lưu vị trí các đỉnh trên màn hình
    std::vector<Edge> targetMST;          // Lưu kết quả Kruskal/Prim
    int currentAnimIndex;                 
    enum AnimType { NONE, KRUSKAL, PRIM } currentAnimType;

    void updateNodePositions();           
    
    void DrawSubMenuContent() override;
    void onExecuteOp(MainOp op) override;
    void handleAnimationStep() override;
};