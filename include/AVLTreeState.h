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
    
    void updateNodePositions(Node* node, float deltaTime);
    void drawNode(const Node* node);

};