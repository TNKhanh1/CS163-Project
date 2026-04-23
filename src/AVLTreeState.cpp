#include "AVLTreeState.h"

AVLTreeState::AVLTreeState() : DataStructureState()
{
    NextState = (int)STATE_AVLTREE;
    activeInputFocus = -1;
    previousInputFocus = -1;

    for (int i = 0; i < 3; i++) {
        avl[i] = new AVLTree();
    }
}

AVLTreeState::~AVLTreeState()
{
    for (int i = 0; i < 3; i++) {
        delete avl[i];
    }   
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
        if (activeCodeLine == 0) {
                if (searchTargetValue < searchPointer->key) {
                    activeCodeLine = 3; // Highlight "node->left = insert(...)"
                    animTimer = 0.8f;
                } 
                else if (searchTargetValue > searchPointer->key) {
                    activeCodeLine = 5; // Highlight "node->right = insert(...)"
                    animTimer = 0.8f;
                }
                else {
                    // Duplicate found (cancel insertion)
                    const_cast<Node*>(searchPointer)->color = SKYBLUE;
                    currentTask = TASK_NONE;
                    activeCodeLine = -1;
                }
            }
            else if (activeCodeLine == 3 || activeCodeLine == 5) {
                
                // Reset current node color
                const_cast<Node*>(searchPointer)->color = SKYBLUE; 

                if (searchTargetValue < searchPointer->key) {
                    if (searchPointer->left != nullptr) {
                        // Jump to the left child!
                        searchPointer = searchPointer->left;
                        const_cast<Node*>(searchPointer)->color = YELLOW;
                        activeCodeLine = 0; // Loop back to the function signature!
                        animTimer = 0.8f;
                    } else {
                        // Reached the bottom, insert it!
                        activeCodeLine = 0; // Loop back to the function signature!
                        animTimer = 0.8f;
                        currentTask = TASK_HIGHLIGHT_NEW;
                    }
                } 
                else if (searchTargetValue > searchPointer->key) {
                    if (searchPointer->right != nullptr) {
                        // Jump to the right child!
                        searchPointer = searchPointer->right;
                        const_cast<Node*>(searchPointer)->color = YELLOW;
                        activeCodeLine = 0; // Loop back to the function signature!
                        animTimer = 0.8f;
                    } else {
                        // Reached the bottom, insert it!
                        activeCodeLine = 0; // Loop back to the function signature!
                        animTimer = 0.8f;
                        currentTask = TASK_HIGHLIGHT_NEW;
                    }
                }
            }
    }
}

    if (currentTask == TASK_HIGHLIGHT_NEW) {
        animTimer -= deltaTime; // Tick down the clock
        
        if (animTimer <= 0.0f) {
            // Time is up! Reset the colors and clear the task.
            if (activeCodeLine == 0) {
                // We just highlighted the signature. Now highlight the base case!
                activeCodeLine = 1; // "if (node == NULL) return new Node(key);"
                animTimer = 0.8f;
            } 
            else if (activeCodeLine == 1) {
                // Time to actually insert the node into the backend!
                avl[cur]->insert(searchTargetValue);
                
                // Find it and color it green
                const Node* newObj = avl[cur]->search(searchTargetValue);
                if (newObj != nullptr) const_cast<Node*>(newObj)->color = GREEN;
                
                // Move on to balancing
                currentTask = TASK_WAIT_FOR_BALANCE;
                animTimer = 0.6f;
            }
        }
    }
    else if (currentTask == TASK_WAIT_FOR_BALANCE) {
    animTimer -= deltaTime;
    
    if (animTimer <= 0.0f) {
        // Time is up! Try to do exactly ONE rotation.
        int didRotate = avl[cur]->balance();
        
        if (didRotate != 0) {
        // A rotation happened! Wait 1.5 second before doing the next one
        animTimer = 1.5f;
        // (This perfectly separates Right-Left rotations into two steps!)
        if (didRotate == 1 || didRotate == 4) {
                pseudoCode = {
                    "Node* rightRotate(Node* y) {",
                    "    Node* x = y->left;",
                    "    Node* sub = x->right;",
                    "    x->right = y;   // MAIN SWAP",
                    "    y->left = sub;  // REATTACH SUBTREE",
                    "    updateHeights();",
                    "    return x;",
                    "}"
                };
                activeCodeLine = 3; // Highlight the "x->right = y" line!
            }
            // If we did a LEFT rotate (RR Case, or first half of LR Case)
            else if (didRotate == 2 || didRotate == 3) {
                pseudoCode = {
                    "Node* leftRotate(Node* x) {",
                    "    Node* y = x->right;",
                    "    Node* sub = y->left;",
                    "    y->left = x;    // MAIN SWAP",
                    "    x->right = sub; // REATTACH SUBTREE",
                    "    updateHeights();",
                    "    return y;",
                    "}"
                };
                activeCodeLine = 3; // Highlight the "y->left = x" line!
            }
        } 
        else {
            // Tree is completely balanced! Clear task and reset the code box.
            currentTask = TASK_NONE;
            animTimer = 0.0f;
            pseudoCode = {
                "// Tree is perfectly balanced!",
                "// Waiting for next operation..."
            };
            activeCodeLine = -1; // Turn off highlight
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
    DrawSideMenuFrame({"Insert", "Delete", "Search", "Clear"});

    if (!pseudoCode.empty()) {
        // 1. Define Box Size and Position (Bottom Right Corner)
        float pcWidth = 450.0f;
        float pcHeight = 350.0f;
        float pcX = GetScreenWidth() - pcWidth - 20.0f;  
        float pcY = 50.0f; // Above the control panel

        // 2. Draw Background and Border
        DrawRectangle(pcX, pcY, pcWidth, pcHeight, Fade(LIGHTGRAY, 0.8f));
        DrawRectangleLines(pcX, pcY, pcWidth, pcHeight, DARKGRAY);

        // 3. Draw Title
        DrawTextEx(listFont, "Source Code:", {pcX + 15.0f, pcY + 15.0f}, 25.0f, 1.0f, DARKBLUE);

        // 4. Draw the Code Lines with Highlight
        float lineHeight = 28.0f;
        float startY = pcY + 60.0f;
        
        for (int i = 0; i < (int)pseudoCode.size(); i++) {
            Color textCol = BLACK;
            
            // If this is the active line, draw the yellow highlight and make text red!
            if (i == activeCodeLine) {
                DrawRectangle(pcX + 5.0f, startY + i * lineHeight - 2.0f, pcWidth - 10.0f, lineHeight, Fade(YELLOW, 0.6f));
                textCol = RED;
            }
            
            // Draw the actual line of code (respects the spaces for indentation)
            DrawTextEx(numberFont, pseudoCode[i].c_str(), {pcX + 20.0f, startY + i * lineHeight}, 18.0f, 1.0f, textCol);
        }
    }
}

bool AVLTreeState::checkBuffer(std::string& currentInput, int id) {
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
                        onExecuteOp(OP_SLOT1);
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
                        onExecuteOp(OP_SLOT2);
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
                        onExecuteOp(OP_SLOT3);
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
                onExecuteOp(OP_SLOT4);
            }
            break;

        // case OP_SLOT5: // Undo
        //     // Shift the Y position down for the 3rd slot

        //     onExecuteOp(OP_SLOT5);
        //     break;

        // case OP_SLOT6: // Redo
        //     // Shift the Y position down for the 3rd slot

        //     onExecuteOp(OP_SLOT6);
        //     break;
            
        default:
            break;
    }
}

void AVLTreeState::onExecuteOp(MainOp op)
{
    
    if (op == OP_SLOT4) {
        // 1. Wipe the tree data
        cur = (cur + 1) % 3;
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
    // else if (op == OP_SLOT5) {
    //     if (undoBound > 0) {
    //         if (undoOp[latest] == 1) {
    //             avl[cur]->delNode(valOp[latest]);
    //             latest = (latest + 2) % 3;
    //             redoBound++;
    //             undoBound--;
    //         }
    //         else if (undoOp[latest] == 2) {
    //             avl[cur]->insert(valOp[latest]);
    //             latest = (latest + 2) % 3;
    //             redoBound++;
    //             undoBound--;
    //         }
    //         else if (undoOp[latest] == 4) {
    //             cur = (cur + 2) % 3;
    //             latest = (latest + 2) % 3;
    //             redoBound++;
    //             undoBound--;
    //         }
    //     }

    //     currentTask = TASK_NONE;
    //     animTimer = 0.0f;
    //     searchPointer = nullptr;
    //     inputErrorMsg = "";
    //     activeInputFocus = -1;
    //     return;
    // }
    // else if (op == OP_SLOT6) {
    //     if (redoBound > 0) {
    //         latest = (latest + 1) % 3;
    //         if (undoOp[latest] == 1) {
    //             avl[cur]->insert(valOp[latest]);
    //             redoBound--;
    //             undoBound++;
    //         }
    //         else if (undoOp[latest] == 2) {
    //             avl[cur]->delNode(valOp[latest]);
    //             redoBound--;
    //             undoBound++;
    //         }
    //         else if (undoOp[latest] == 4) {
    //             cur = (cur + 1) % 3;
    //             redoBound--;
    //             undoBound++;
    //         }
    //     }

    //     currentTask = TASK_NONE;
    //     animTimer = 0.0f;
    //     searchPointer = nullptr;
    //     inputErrorMsg = "";
    //     activeInputFocus = -1;
    //     return;
    // }

			
    // Reset colors before starting a new operation!
    resetNodeColors(const_cast<Node*>(avl[cur]->rootCall()));
    int curOp = 0;
    int id = 0;
    if (op == OP_SLOT1) id = 1;
    else if (op == OP_SLOT2) id = 2;
    else if (op == OP_SLOT3) id = 3;
    int value = std::stoi(inputBuffers[id]);

    if (op == OP_SLOT1) { // OP_INSERT
        pseudoCode = {
            "Node* insert(Node* node, int key) {",
            "    if (node == NULL) return new Node(key);",
            "    if (key < node->key)",
            "        node->left = insert(node->left, key);",
            "    else if (key > node->key)",
            "        node->right = insert(node->right, key);",
            "    ",
            "    return balance(node);",
            "}"
        };
        activeCodeLine = 0;
        if (avl[cur]->rootCall() == nullptr) {
            searchTargetValue = value; 
            currentTask = TASK_HIGHLIGHT_NEW;
            animTimer = 0.8f;
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
        pseudoCode = {
            "Node* deleteNode(Node* node, int key) {",
            "    if (node == NULL) return node;",
            "    if (key < node->key) ",
            "        node->left = deleteNode(node->left, key);",
            "    else if (key > node->key) ",
            "        node->right = deleteNode(node->right, key);",
            "    else { /* Delete leaf or swap successor */ }",
            "    ",
            "    return balance(node);",
            "}"
        };
        activeCodeLine = 0;
        currentTask = TASK_WAIT_FOR_BALANCE;
        animTimer = 0.8f;
        avl[cur]->delNode(value);
        curOp = 2;
        inputBuffers[2].clear();
    }
    else if (op == OP_SLOT3) {
        pseudoCode = {
            "Node* search(Node* node, int key) {",
            "    if (node == NULL || node->key == key)",
            "        return node;",
            "    if (key < node->key)",
            "        return search(node->left, key);",
            "    return search(node->right, key);",
            "}"
        };
        activeCodeLine = 0;
        const Node* foundNode = avl[cur]->search(value);
        if (foundNode != nullptr) {
            // Instantly highlight the found node GREEN for now
            const_cast<Node*>(foundNode)->color = GREEN; 
            currentTask = TASK_HIGHLIGHT_NEW;
            animTimer = 1.5f; 
        } else {
            currentErrorSlot = 3;
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