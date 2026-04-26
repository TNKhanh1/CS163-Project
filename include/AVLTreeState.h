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
    int searchPointerKey;
    AVLTree* treeCopy;
};

struct AVLOpSnapshot { // For undoing individual operations without animation
    AVLTree* treeCopy;
    int pCode;
    std::vector<AVLSnapshot*> steps;
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

    // Manual/Auto mode
    bool hasNextStep() override;
    bool hasPrevStep() override;

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

    // AVL-specific state variables for manual animation control
    AVLSnapshot* captureStep();
    void applyStep(AVLSnapshot* s);
    void undoState() override;
    void redoState();
    void handleAnimationStep() override;
    void snapNodePositions(Node* node);

    int pCode = -1;
    std::vector<AVLOpSnapshot*> opHistory;
    int stepIndex = -1;
    int opIndex = -1;

    // Tasks for highlighting and pseudo-code control
    static const int TASK_NONE = 0;
    static const int TASK_HIGHLIGHT_NEW = 1;
    static const int TASK_TRAVERSE_INSERT = 2;
    static const int TASK_WAIT_FOR_BALANCE = 3;
    static const int TASK_TRAVERSE_SEARCH = 4;
    static const int TASK_TRAVERSE_DELETE = 5;
    static const int TASK_HIGHLIGHT_FOR_DELETE = 6;

    // Task-specific step functions
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