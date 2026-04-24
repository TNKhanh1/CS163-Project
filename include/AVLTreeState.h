#pragma once
#include "AVLTree.h"
#include <UI.h>
#include <State.h>
#include <stack>

// Snapshot struct for undo
struct AVLSnapshot {
    int currentTask;
    int activeCodeLine;
    int searchTargetValue;
    int pCode;
    AVLTree* treeCopy; // deep copy of the tree
};

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
    // AVLTree* avl[3];
    // int cur = 0;
    // int latest;
    // stack<int> valOp;
    // stack<int> listOp;
    // stack<int> redoOp;
    // int bound = 0;
    // int cur = 0;
    
    AVLTree* avl;
    int previousInputFocus;
    int activeInputFocus;

    void saveState() override;
    void undoState() override;
    void handleAnimationStep() override;

    std::vector<std::string> pseudoCode;
    int pCode = -1;
    int activeCodeLine;
    std::vector<AVLSnapshot> snapHistory;

    const int TASK_NONE = 0;
    const int TASK_HIGHLIGHT_NEW = 1;
    const int TASK_TRAVERSE_INSERT = 2;
    const int TASK_WAIT_FOR_BALANCE = 3;
    const int TASK_TRAVERSE_SEARCH = 4;
    const int TASK_TRAVERSE_DELETE = 5;
    const int TASK_HIGHLIGHT_FOR_DELETE = 6;

    void stepTraverseInsert();
    void stepTraverseSearch();
    void stepTraverseDelete();
    void stepHighlightNew();
    void stepHighlightForDelete();
    void stepWaitForBalance();
    
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