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

    void DrawSubMenuContent() override;
    
    void onExecuteOp(MainOp op) override;
};