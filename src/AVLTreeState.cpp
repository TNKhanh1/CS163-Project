#include "AVLTreeState.h"

AVLTreeState::AVLTreeState() : DataStructureState()
{
    NextState = (int)STATE_AVLTREE;
    activeInputFocus = -1;
    previousInputFocus = -1;
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
    
    if (activeInputFocus != previousInputFocus) {
        cursorIndex = (activeInputFocus == -1) ? 0 : inputBuffers[activeInputFocus].length();
        textScrollX = 0.0f; cursorVisible = true;
        previousInputFocus = activeInputFocus;
    }

    if (activeInputFocus != -1) {
        HandleTextInput(inputBuffers[activeInputFocus], false);
    }

    // Calculate perfect target coordinates before gliding them
    updateTargetLayouts(const_cast<Node*>(avl[cur]->rootCall()), 900.0f, 150.0f, 350.0f * zoomMultiplier);
    updateNodePositions(const_cast<Node*>(avl[cur]->rootCall()), deltaTime);

    if (currentTask == TASK_TRAVERSE_INSERT) {
    animTimer -= deltaTime;
    
    if (animTimer <= 0.0f) {
        if (searchTargetValue < searchPointer->key) {
            if (searchPointer->left != nullptr) {
                searchPointer = searchPointer->left;
                const_cast<Node*>(searchPointer)->color = YELLOW;
                animTimer = 0.4f;
            } else {
                avl[cur]->insert(searchTargetValue);
                resetNodeColors(const_cast<Node*>(avl[cur]->rootCall()));
                
                const Node* newObj = avl[cur]->search(searchTargetValue);
                if (newObj != nullptr) const_cast<Node*>(newObj)->color = GREEN;
                currentTask = TASK_HIGHLIGHT_NEW;
                animTimer = 1.0f;
            }
        } 
        else if (searchTargetValue > searchPointer->key) {
            if (searchPointer->right != nullptr) {
                searchPointer = searchPointer->right;
                const_cast<Node*>(searchPointer)->color = YELLOW;
                animTimer = 0.4f; 
            } else {
                avl[cur]->insert(searchTargetValue);
                resetNodeColors(const_cast<Node*>(avl[cur]->rootCall()));
                
                const Node* newObj = avl[cur]->search(searchTargetValue);
                if (newObj != nullptr) const_cast<Node*>(newObj)->color = GREEN;
                currentTask = TASK_HIGHLIGHT_NEW;
                animTimer = 1.0f;
            }
        } 
        else {
            resetNodeColors(const_cast<Node*>(avl[cur]->rootCall()));
            currentTask = TASK_NONE;
            inputErrorMsg = "Value already in tree!";
            inputErrorTimer = 2.0f;
            currentErrorSlot = 0;
        }
    }
}

    if (currentTask == TASK_HIGHLIGHT_NEW) {
        animTimer -= deltaTime; // Tick down the clock
        
        if (animTimer <= 0.0f) {
            // Time is up! Reset the colors and clear the task.
            resetNodeColors(const_cast<Node*>(avl[cur]->rootCall()));
            currentTask = TASK_WAIT_FOR_BALANCE;
        }
    }
    else if (currentTask == TASK_WAIT_FOR_BALANCE) {
    animTimer -= deltaTime;
    
    if (animTimer <= 0.0f) {
        // Time is up! Try to do exactly ONE rotation.
        bool didRotate = avl[cur]->balance();
        
        if (didRotate) {
            // A rotation happened! Wait 1 second before doing the next one
            // (This perfectly separates Right-Left rotations into two steps!)
            animTimer = 0.5f; 
        } else {
            // No rotations needed, tree is fully balanced! Finish up.
            currentTask = TASK_NONE;
            animTimer = 1.5f;
        }
    }
}
}

void AVLTreeState::draw()
{
    DataStructureState::drawSharedUI();

    const char* titleText = "AVL TREE";
    Vector2 titleSize = MeasureTextEx(listFont, titleText, 55, 6.5f);
    DrawTextEx(listFont, titleText, { (1800.0f - titleSize.x) / 2.0f, 20.0f }, 55, 6.5f, BLACK);

    drawNode(avl[cur]->rootCall());

    DrawTextureV(controlTex, controlBtnPos, WHITE);
    DrawSideMenuFrame({"Insert", "Delete", "Search", "Clear", "Undo", "Redo"});
}

bool checkBuffer(string& currentInput, int id) const {
    if (currentInput.empty()) {
		currentErrorSlot = id; 
		inputErrorMsg = "Value cannot be empty!";
		inputErrorTimer = 2.5f;
		return false;
	}
    return true;
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
            if (DrawTextBox({subX + 80, startY}, inputBuffers[1], activeInputFocus == 1, 100, mainHeight, cursorIndex, textScrollX, cursorVisible)) activeInputFocus = 1;

            // Draw the "GO" button. If clicked, manually trigger onExecuteOp!
            if (DrawButtonText({subX + 190, startY}, "GO", 50, mainHeight, false)) {
                if (checkBuffer(inputBuffers[1], 1)) {
                    try {		
                        int value = std::stoi(inputBuffers[1]);
                        onExecuteOp(OP_SLOT1, value);
                    }
                    catch (...) {
                        currentErrorSlot = 1;
                        inputErrorMsg = "Invalid Number!";
                        inputErrorTimer = 2.5f;
                    }
                }
            }
            break;

        case OP_SLOT2: // Delete
            // Shift the Y position down for the 2nd slot
            DrawLabel({subX, startY + mainHeight + gap}, "Value=");

            if (DrawTextBox({subX + 80, startY + mainHeight + gap}, inputBuffers[2], activeInputFocus == 2, 100, mainHeight, cursorIndex, textScrollX, cursorVisible)) activeInputFocus = 2;

            if (DrawButtonText({subX + 190, startY + mainHeight + gap}, "GO", 50, mainHeight, false)) {
                if (checkBuffer(inputBuffers[2], 2)) {
                    try {		
                        int value = std::stoi(inputBuffers[2]);
                        onExecuteOp(OP_SLOT2, value);
                    }
                    catch (...) {
                        currentErrorSlot = 2;
                        inputErrorMsg = "Invalid Number!";
                        inputErrorTimer = 2.5f;
                    }
                }
            }
            break;

        case OP_SLOT3: // Search
            // Shift the Y position down for the 3rd slot
            DrawLabel({subX, startY + 2 * (mainHeight + gap)}, "Value=");

            if (DrawTextBox({subX + 80, startY + 2 * (mainHeight + gap)}, inputBuffers[3], activeInputFocus == 3, 100, mainHeight, cursorIndex, textScrollX, cursorVisible)) activeInputFocus = 3; 
            
            if (DrawButtonText({subX + 190, startY + 2 * (mainHeight + gap)}, "GO", 50, mainHeight, false)) {
                if (checkBuffer(inputBuffers[3], 3)) {
                    try {		
                        int value = std::stoi(inputBuffers[3]);
                        onExecuteOp(OP_SLOT3, value);
                    }
                    catch (...) {
                        currentErrorSlot = 3;
                        inputErrorMsg = "Invalid Number!";
                        inputErrorTimer = 2.5f;
                    }
                }
            }
            break;

        case OP_SLOT4: // Clear
            // Shift the Y position down for the 3rd slot

            if (DrawButtonText({subX + 90, startY + 3 * (mainHeight + gap)}, "ACCEPT CLEAR", 150, mainHeight, false)) {
                onExecuteOp(OP_SLOT4, 0);
            }
            break;

        case OP_SLOT5: // Undo
            // Shift the Y position down for the 3rd slot

            onExecuteOp(OP_SLOT5, 0);
            break;

        case OP_SLOT6: // Redo
            // Shift the Y position down for the 3rd slot

            onExecuteOp(OP_SLOT6, 0);
            break;
            
        default:
            break;
    }
}

void AVLTreeState::onExecuteOp(MainOp op, int value)
{
    
    if (op == OP_SLOT4) {
        // 1. Wipe the tree data
        cur = (cur + 1) % 2;
        if (avl[cur] != nullptr) avl[cur]->clear();
        latest = (latest + 1) % 3;
        undoOp[latest] = 4;
        if (undoBound < 3) undoBound++;
        if (redoBound > 0) redoBound = 0;
        avl[cur]->clear();

        // 2. Reset the visualizer states so it doesn't look for nodes that no longer exist
        currentTask = TASK_NONE;
        animTimer = 0.0f;

        // 3. Clear any error messages or active inputs
        inputErrorMsg = "";
        activeInputFocus = -1;
        return;
    }
    else if (op == OP_SLOT5) {
        if (bound > 0) {
            if (undoOp[latest] == 1) {
                onExecuteOp(static_cast<MainOp>(2), valOp[latest]);
                latest = (latest + 2) % 3;
                redoBound++;
                undoBound--;
            }
            else if (undoOp[latest] == 2) {
                onExecuteOp(static_cast<MainOp>(1), valOp[latest]);
                latest = (latest + 2) % 3;
                redoBound++;
                undoBound--;
            }
            else if (undoOp[latest] == 4) {
                avl[cur]->copyTree(secureTree[--secureNum]);
                secureTree[secureNum].clear();
                latest = (latest + 2) % 3;
                redoBound++;
                undoBound--;
                searchPointer = nullptr;
            }
        }

        currentTask = TASK_NONE;
        animTimer = 0.0f;
        searchPointer = nullptr;
        inputErrorMsg = "";
        activeInputFocus = -1;
        return;
    }
    else if (op == OP_SLOT6) {
        if (redoBound > 0) {
            latest = (latest + 1) % 3;
            if (undoOp[latest] == 1) {
                onExecuteOp(static_cast<MainOp>(1), valOp[latest]);
                redoBound--;
                undoBound++;
            }
            else if (undoOp[latest] == 2) {
                onExecuteOp(static_cast<MainOp>(2), valOp[latest]);
                redoBound--;
                undoBound++;
            }
            else if (undoOp[latest] == 4) {
                cur = (cur + 1) % 2;
                redoBound--;
                undoBound++;
            }
        }

        currentTask = TASK_NONE;
        animTimer = 0.0f;
        searchPointer = nullptr;
        inputErrorMsg = "";
        activeInputFocus = -1;
        return;
    }

			
    // Reset colors before starting a new operation!
    resetNodeColors(const_cast<Node*>(avl[cur]->rootCall()));
    int curOp = 0;

    if (op == OP_SLOT1) { // OP_INSERT
        if (avl[cur]->rootCall() == nullptr) {
            avl[cur]->insert(value);
            const Node* newObj = avl[cur]->search(value);
            if (newObj != nullptr) {
                const_cast<Node*>(newObj)->color = GREEN;
                currentTask = TASK_HIGHLIGHT_NEW;
                animTimer = 1.5f; 
            }
        }
        else {
            // If nodes exist, start YELLOW search traversal!
            searchTargetValue = value;
            searchPointer = avl[cur]->rootCall();
            const_cast<Node*>(searchPointer)->color = YELLOW;
            
            currentTask = TASK_TRAVERSE_INSERT;
            animTimer = 0.8f; 
        }
        curOp = 1;
        inputBuffers[1].clear();
    }
    else if (op == OP_SLOT2) {
        avl[cur]->delNode(value);
        curOp = 2;
        inputBuffers[2].clear();
    }
    else if (op == OP_SLOT3) {
        const Node* foundNode = avl[cur]->search(value);
        if (foundNode != nullptr) {
            // Instantly highlight the found node GREEN for now
            const_cast<Node*>(foundNode)->color = GREEN; 
            currentTask = TASK_HIGHLIGHT_NEW;
            animTimer = 1.5f; 
        } else {
            currentErrorSlot = slotIndex;
            inputErrorMsg = "Value not found!";
            inputErrorTimer = 2.5f;
        }
        inputBuffers[3].clear();
    }
    if (curOp != 0) {
        latest = (latest + 1) % 3;
        undoOp[latest] = curOp;
        if (undoBound < 3) undoBound++;
        if (redoBound > 0) redoBound = 0;
        valOp[latest] = value;
    }

    // Clear the text box on success
    activeInputFocus = -1;
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
    float scaledRadius = 25.0f * zoomMultiplier;
    float lineWidth = 3.0f * zoomMultiplier;
    if (node->left != nullptr) {
        DrawLineEx(node->position, node->left->position, lineWidth, DARKGRAY);
    }
    if (node->right != nullptr) {
        DrawLineEx(node->position, node->right->position, lineWidth, DARKGRAY);
    }

    drawNode(node->left);
    drawNode(node->right);

    DrawCircleV(node->position, scaledRadius, node->color);
    DrawCircleLines(node->position.x, node->position.y, scaledRadius, DARKBLUE);

    const char* text = TextFormat("%d", node->key);
    float fontSize = 20.0f * zoomMultiplier;
    Vector2 tSize = MeasureTextEx(numberFont, text, fontSize, 1.0f);
    
    DrawTextEx(numberFont, text, { node->position.x - tSize.x / 2.0f, node->position.y - tSize.y / 2.0f }, fontSize, 1.0f, BLACK);
}

void AVLTreeState::updateTargetLayouts(Node* node, float x, float y, float hGap) {
    if (node == nullptr) return;
    node->targetPosition = { x, y };

    // Recursively calculate positions for the left and right children
    float verticalSpacing = 80.0f * zoomMultiplier;
    updateTargetLayouts(node->left, x - hGap, y + verticalSpacing, hGap / 2.0f);
    updateTargetLayouts(node->right, x + hGap, y + verticalSpacing, hGap / 2.0f);
}