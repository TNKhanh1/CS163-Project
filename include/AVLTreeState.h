#pragma once
#include "AVLTree.h"

class AVLTreeState : public DataStructureState 
{
public:
    AVLTreeState();
    ~AVLTreeState();

    void loadAssets() override;
    void update(float deltaTime) override;
    void draw() override;

protected:
	void DrawSubMenuContent() override;
	void onExecuteOp(MainOp op) override;
    
private:
    AVLTree avl;
    std::string inputBuffer;

    const int TASK_NONE = 0;
    const int TASK_HIGHLIGHT_NEW = 1;
    
    // Animation & Traversal State
    int currentTask; // Matches LinkedList's TASK_NONE, TASK_SEARCH, etc.
    float animTimer;
    const Node* searchPointer;
    int searchTargetValue;

    // Helper functions
    void resetNodeColors(Node* node);
    
    void updateNodePositions(Node* node, float deltaTime);
    void drawNode(const Node* node);

};