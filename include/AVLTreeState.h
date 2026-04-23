#pragma once
#include "AVLTree.h"
#include <UI.h>
#include <State.h>

class AVLTreeState : public DataStructureState 
{
public:
    AVLTreeState();
    ~AVLTreeState();

    void loadAssets() override;
    void update(float deltaTime) override;
    void draw() override;
    bool checkBuffer(std::string& currentInput, int id);

protected:
	void DrawSubMenuContent() override;
	void onExecuteOp(MainOp op) override;
    
private:
    AVLTree* avl[3];
    int cur = 0;
    int previousInputFocus;
    int activeInputFocus;
    int latest;
    int valOp[3] = {0, 0, 0};
    int undoBound = 0;
    int undoOp[3] = {0, 0, 0};
    int redoBound = 0;

    std::vector<std::string> pseudoCode;
    int activeCodeLine;

    const int TASK_NONE = 0;
    const int TASK_HIGHLIGHT_NEW = 1;
    const int TASK_TRAVERSE_INSERT = 2;
    const int TASK_WAIT_FOR_BALANCE = 3;
    
    // Animation & Traversal State
    int currentTask; // Matches LinkedList's TASK_NONE, TASK_SEARCH, etc.
    float animTimer;
    const Node* searchPointer;
    int searchTargetValue;

    // Helper functions
    void resetNodeColors(Node* node);
    
    void updateNodePositions(Node* node, float deltaTime);
    void drawNode(const Node* node);
    void updateTargetLayouts(Node* node, float x, float y, float hGap);

};