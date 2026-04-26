#pragma once
#include "DataStructureState.h"
#include "State.h"
#include "MST.h"
#include <vector>
#include <string>

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

    std::map<int, Vector2> nodePositions; 
    std::vector<Edge> targetMST;          
    int currentAnimIndex;                 
    enum AnimType { NONE, KRUSKAL, PRIM } currentAnimType;
    int animPhase;

    void updateNodePositions();           
    
    void DrawSubMenuContent() override;
    void onExecuteOp(MainOp op) override;
    void handleAnimationStep() override;

    struct MSTSnapshot {
        int animIndex;
        int animPhase;
        int activeCodeLine;
    };
    std::vector<MSTSnapshot> history;

    void saveState() override;
    void undoState() override;
};