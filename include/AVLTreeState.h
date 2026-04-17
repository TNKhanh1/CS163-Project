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

};