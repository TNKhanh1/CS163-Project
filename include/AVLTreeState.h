#pragma once
#include "DataStructureState.h"
#include <raylib.h>
#include <State.h>

class AVLTreeState : public DataStructureState 
{
public:
    AVLTreeState();
    ~AVLTreeState();

    void loadAssets() override;
    void update(float deltaTime) override;
    void draw() override;

private:

};