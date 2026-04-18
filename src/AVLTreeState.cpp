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
    
    if (activeMainOp != OP_NONE) {
        HandleTextInput(inputBuffer, false); // false = single integer, no commas
    }

    // 2. Handle Text Cursor Blinking
    cursorBlinkTimer += deltaTime;
    if (cursorBlinkTimer >= 0.5f) {
        cursorVisible = !cursorVisible;
        cursorBlinkTimer = 0.0f;
    }

    updateNodePositions(const_cast<Node*>(avl.rootCall()), deltaTime);

    if (currentTask == TASK_HIGHLIGHT_NEW) {
        animTimer -= deltaTime; // Tick down the clock
        
        if (animTimer <= 0.0f) {
            // Time is up! Reset the colors and clear the task.
            resetNodeColors(const_cast<Node*>(avl.rootCall()));
            currentTask = TASK_NONE;
        }
    }
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
    if (activeMainOp == OP_NONE) return;

    // Calculate base positions relative to the main control panel
    float subX = controlBtnPos.x + 320.0f; // Offset to the right of the main buttons
    float startY = controlBtnPos.y;        // Align with the top button
    float mainHeight = 45.0f;
    float gap = 8.0f;                      // Gap between menu items

    switch (activeMainOp) {
        case OP_SLOT1: // Insert
            DrawLabel({subX, startY}, "Value=");
            
            // Draw the text box
            DrawTextBox({subX + 80, startY}, inputBuffer, true, 100, mainHeight, cursorIndex, textScrollX, cursorVisible);
            
            // Draw the "GO" button. If clicked, manually trigger onExecuteOp!
            if (DrawButtonText({subX + 190, startY}, "GO", 50, mainHeight, false)) {
                onExecuteOp(OP_SLOT1);
            }
            break;

        case OP_SLOT2: // Delete
            // Shift the Y position down for the 2nd slot
            DrawLabel({subX, startY + mainHeight + gap}, "Value=");
            
            DrawTextBox({subX + 80, startY + mainHeight + gap}, inputBuffer, true, 100, mainHeight, cursorIndex, textScrollX, cursorVisible);
            
            if (DrawButtonText({subX + 190, startY + mainHeight + gap}, "GO", 50, mainHeight, false)) {
                onExecuteOp(OP_SLOT2);
            }
            break;

        case OP_SLOT3: // Search
            // Shift the Y position down for the 3rd slot
            DrawLabel({subX, startY + 2 * (mainHeight + gap)}, "Value=");
            
            DrawTextBox({subX + 80, startY + 2 * (mainHeight + gap)}, inputBuffer, true, 100, mainHeight, cursorIndex, textScrollX, cursorVisible);
            
            if (DrawButtonText({subX + 190, startY + 2 * (mainHeight + gap)}, "GO", 50, mainHeight, false)) {
                onExecuteOp(OP_SLOT3);
            }
            break;
            
        default:
            break;
    }
}

void AVLTreeState::onExecuteOp(MainOp op)
{
    if (inputBuffer.empty()) {
        currentErrorSlot = (int)op - 1;
        inputErrorMsg = "Value cannot be empty!";
        inputErrorTimer = 2.5f;
        return;
    }

    try {
        int value = std::stoi(inputBuffer);
        
        // Reset colors before starting a new operation!
        resetNodeColors(const_cast<Node*>(avl.rootCall()));

        if (op == OP_SLOT1) { // OP_INSERT
            avl.insert(value);
            
            // 1. Find the node we just inserted
            const Node* newObj = avl.search(value);
            if (newObj != nullptr) {
                // 2. Color it GREEN so it stands out!
                const_cast<Node*>(newObj)->color = GREEN;
                
                // 3. Start the animation timer for 1.5 seconds
                currentTask = TASK_HIGHLIGHT_NEW;
                animTimer = 1.5f; 
            }
        }
        else if (op == OP_SLOT2) {
            avl.delNode(value);
        }
        else if (op == OP_SLOT3) {
            const Node* foundNode = avl.search(value);
            if (foundNode != nullptr) {
                // Instantly highlight the found node GREEN for now
                const_cast<Node*>(foundNode)->color = GREEN; 
            } else {
                currentErrorSlot = (int)op - 1;
                inputErrorMsg = "Value not found!";
                inputErrorTimer = 2.5f;
            }
        }

        // Clear the text box on success
        inputBuffer = "";
        cursorIndex = 0;
        textScrollX = 0.0f;

    } catch (...) {
        currentErrorSlot = (int)op - 1;
        inputErrorMsg = "Invalid Number!";
        inputErrorTimer = 2.5f;
    }
}

void AVLTreeState::resetNodeColors(Node* node) {
    if (node == nullptr) return;
    
    // Reset to default color (using SKYBLUE to match LinkedList)
    node->color = SKYBLUE;
    
    resetNodeColors(node->left);
    resetNodeColors(node->right);
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

    DrawCircleV(node->position, 20.0f, node->color);
    DrawCircleLines(node->position.x, node->position.y, 20.0f, BLUE);

    const char* text = TextFormat("%d", node->key);
    int fontSize = 20;
    int textWidth = MeasureText(text, fontSize);
    
    DrawText(text, node->position.x - (textWidth / 2.0f), node->position.y - (fontSize / 2.0f), fontSize, BLACK);
}