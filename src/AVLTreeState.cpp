#include "AVLTreeState.h"

AVLTreeState::AVLTreeState() : DataStructureState()
{
    NextState = (int)STATE_AVLTREE;
}

AVLTreeState::~AVLTreeState()
{
}

void AVLTreeState::loadAssets()
{
    DataStructureState::loadAssets(); 
    
}

void AVLTreeState::update(float deltaTime)
{
    Vector2 mousePos = GetMousePosition();
    
    DataStructureState::updateSharedUI(deltaTime, mousePos);

    DataStructureState::updateControlPanel(deltaTime, mousePos);

    updateNodePositions(const_cast<Node*>(avl.rootCall()), deltaTime);
}

void AVLTreeState::draw()
{
    DataStructureState::drawSharedUI();

    const char* titleText = "AVL TREE";
    Vector2 titleSize = MeasureTextEx(listFont, titleText, 55, 6.5f);
    DrawTextEx(listFont, titleText, { (1800.0f - titleSize.x) / 2.0f, 20.0f }, 55, 6.5f, BLACK);

    drawNode(avl.rootCall());
}

void AVLTreeState::DrawSubMenuContent()
{

}

void AVLTreeState::onExecuteOp(MainOp op)
{

}

void AVLTreeState::updateNodePositions(Node* node, float deltaTime) {
    if (node == nullptr) return;

    node->position = Vector2Lerp(node->position, node->targetPosition, 10.0f * deltaTime);

    updateNodePositions(node->left, deltaTime);
    updateNodePositions(node->right, deltaTime);
}

void AVLTreeState::drawNode(const Node* node) {
    if (node == nullptr) return;

    if (node->left != nullptr) {
        DrawLineEx(node->position, node->left->position, 3.0f, DARKGRAY);
    }
    if (node->right != nullptr) {
        DrawLineEx(node->position, node->right->position, 3.0f, DARKGRAY);
    }

    drawNode(node->left);
    drawNode(node->right);

    DrawCircleV(node->position, 20.0f, SKYBLUE);
    DrawCircleLines(node->position.x, node->position.y, 20.0f, BLUE);

    const char* text = TextFormat("%d", node->key);
    int fontSize = 20;
    int textWidth = MeasureText(text, fontSize);
    
    DrawText(text, node->position.x - (textWidth / 2.0f), node->position.y - (fontSize / 2.0f), fontSize, BLACK);
}