#include "AVLTreeState.h"
#include <random>


static const std::vector<std::string> pseudoTable[] = {
    { "// Select an operation..." },                          // 0: idle
    { "Node* insert(Node* node, int key) {",                 // 1: insert
        "    if (node == NULL) return new Node(key);",
        "    if (key < node->key)",
        "        node->left = insert(node->left, key);",
        "    else if (key > node->key)",
        "        node->right = insert(node->right, key);",
        "    ",
        "    return balance(node);",
        "}" },
    { "Node* deleteNode(Node* node, int key) {",             // 2: delete
        "    if (node == NULL) return node;",
        "    if (key < node->key) ",
        "        node->left = deleteNode(node->left, key);",
        "    else if (key > node->key) ",
        "        node->right = deleteNode(node->right, key);",
        "    else { /* Delete leaf or swap successor */ }",
        "    ",
        "    return balance(node);",
        "}" },
    { "Node* search(Node* node, int key) {",                 // 3: search
        "    if (node == NULL || node->key == key)",
        "        return node;",
        "    if (key < node->key)",
        "        return search(node->left, key);",
        "    return search(node->right, key);",
        "}" },
    { "Node* rightRotate(Node* y) {",                        // 4: right rotate
        "    Node* x = y->left;",
        "    Node* sub = x->right;",
        "    x->right = y;   // MAIN SWAP",
        "    y->left = sub;  // REATTACH SUBTREE",
        "    updateHeights();",
        "    return x;",
        "}" },
    { "Node* leftRotate(Node* x) {",                         // 5: left rotate
        "    Node* y = x->right;",
        "    Node* sub = y->left;",
        "    y->left = x;    // MAIN SWAP",
        "    x->right = sub; // REATTACH SUBTREE",
        "    updateHeights();",
        "    return y;",
        "}" },
    { "// Tree is perfectly balanced!",                      // 6: balanced
        "// Waiting for next operation..." },
    { "// Random AVL Tree Generated",                        // 7: random
        "// with 10-20 nodes"
    },
};

AVLTreeState::AVLTreeState() : DataStructureState()
{
    NextState = (int)STATE_AVLTREE;
    activeInputFocus = -1;
    previousInputFocus = -1;

    avl = new AVLTree();
    currentTask = TASK_NONE;
    pCode = 0;
    activeCodeLine = -1;
    searchPointer = nullptr;
    animTimer = 0.0f;
    searchTargetValue = 0;

    saveState();
}

AVLTreeState::~AVLTreeState()
{
    delete avl;
    for (auto& snap : snapHistory) {
        delete snap.treeCopy;
    }
    snapHistory.clear();
}

void AVLTreeState::loadAssets()
{
    DataStructureState::loadAssets(); 
    
}

void AVLTreeState::update(float deltaTime)
{   
    pseudoCode = pseudoTable[pCode];
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

    bool stepTriggered = false;

    // 1. Check UI Buttons (OUTSIDE of the TASK_NONE check so they always work)
    if (stepBackwardRequested) {
        undoState();
        stepBackwardRequested = false;
    }

    if (stepForwardRequested) {
        if (currentSnapIndex < (int)snapHistory.size() - 1) {
            redoState(); // Scrub forward from timeline
        } else if (currentTask != TASK_NONE) {
            stepTriggered = true; // Calculate next fresh step
        }
        stepForwardRequested = false;
    }

    // 2. Check AutoPlay Timer
    if (isAutoPlay) {
        animTimer -= deltaTime;
        if (animTimer <= 0.0f) {
            if (currentSnapIndex < (int)snapHistory.size() - 1) {
                redoState(); // Auto-scrub forward through timeline
            } else if (currentTask != TASK_NONE) {
                stepTriggered = true; // Auto-calculate next fresh step
            }
            animTimer = 0.5f; // Reset for next frame
        }
    }

    // 3. Route the Trigger (ONLY if a fresh step is needed)
    if (stepTriggered && currentTask != TASK_NONE) {
        switch (currentTask) {
            case TASK_TRAVERSE_INSERT:   stepTraverseInsert(); break;
            case TASK_TRAVERSE_SEARCH:   stepTraverseSearch(); break;
            case TASK_TRAVERSE_DELETE:   stepTraverseDelete(); break;
            case TASK_HIGHLIGHT_NEW:     stepHighlightNew(); break;
            case TASK_HIGHLIGHT_FOR_DELETE: stepHighlightForDelete(); break;
            case TASK_WAIT_FOR_BALANCE:  stepWaitForBalance(); break;
        }
    }

    // Calculate perfect target coordinates before gliding them
    updateTargetLayouts(const_cast<Node*>(avl->rootCall()), 900.0f, 150.0f, 350.0f * zoomMultiplier);
    updateNodePositions(const_cast<Node*>(avl->rootCall()), deltaTime);
}

void AVLTreeState::draw()
{
    DataStructureState::drawSharedUI();

    const char* titleText = "AVL TREE";
    Vector2 titleSize = MeasureTextEx(listFont, titleText, 55, 6.5f);
    DrawTextEx(listFont, titleText, { (1800.0f - titleSize.x) / 2.0f, 20.0f }, 55, 6.5f, BLACK);

    drawNode(avl->rootCall());

    DrawTextureV(controlTex, controlBtnPos, WHITE);
    DrawSideMenuFrame({"Insert", "Delete", "Search", "Clear", "Random"});

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
            // Shift the Y position down for the 4th slot

            if (DrawButtonText({subX + 90, startY + 3 * (mainHeight + gap)}, "ACCEPT CLEAR", 150, mainHeight, false)) {
                onExecuteOp(OP_SLOT4);
            }
            break;

        case OP_SLOT5: // Random
            // Shift the Y position down for the 5th slot

            if (DrawButtonText({subX + 65, startY + 4 * (mainHeight + gap)}, "GENERATE RANDOM", 200, mainHeight, false)) {
                onExecuteOp(OP_SLOT5);
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
    if (!opHistory.empty()) {
        const AVLOpSnapshot& latest = opHistory.back();
        avl->clear();
        avl->copyTree(*latest.treeCopy);
        pCode = latest.pCode;
    }
    opIndex = (int)opHistory.size() - 1;
    
    currentTask = TASK_NONE; // the latest op snapshot is already post-animation
    isAnimating = false;
    isAnimFinished = true;
    searchPointer = nullptr;

    for (auto& s : snapHistory) delete s.treeCopy;
    snapHistory.clear();
    currentSnapIndex = -1;

    if (op == OP_SLOT4) {
        // 1. Wipe the tree data
        avl->clear();
        pCode = 0;
        activeCodeLine = -1;
        for (auto& op : opHistory) {
            delete op.treeCopy;
        }
        opHistory.clear();
        opIndex = -1;

        // 2. Reset the visualizer states so it doesn't look for nodes that no longer exist
        currentTask = TASK_NONE;
        isAnimating = false;
        isAnimFinished = true;
        animTimer = 0.0f;
        currentSnapIndex = -1;

        // 3. Clear any error messages or active inputs
        inputErrorMsg = "";
        activeInputFocus = -1;

        saveOpState();
        return;
    }
    else if (op == OP_SLOT5) {
        // 1. Clear the current tree and move to a new slot
        // cur = (cur + 1) % 3;
        if (avl != nullptr) avl->clear();
        
        // 2. Generate random values (between 1 and 100)
        std::mt19937 gen(std::random_device{}());
        std::uniform_int_distribution<> distNodes(10, 20);
        std::uniform_int_distribution<> distValues(1, 100);
        
        int numNodes = distNodes(gen);
        
        for (int i = 0; i < numNodes; i++) {
            int randomValue = distValues(gen);
            avl->insert(randomValue);

            // 3. Perform balancing to ensure the tree is properly balanced
            while (avl->balance() != 0) {
                // Keep balancing until no rotations occur
            }
        }
        
        Node* r = const_cast<Node*>(avl->rootCall());
        updateTargetLayouts(r, 900.0f, 150.0f, 350.0f * zoomMultiplier);
        snapNodePositions(r);
        pCode = 7; activeCodeLine = -1;
        currentTask = TASK_NONE;
        isAnimating = false; animTimer = 0.0f;
        inputErrorMsg = ""; activeInputFocus = -1;

        // Save the generated tree as a new op
        saveOpState();
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
    resetNodeColors(const_cast<Node*>(avl->rootCall()));
    int id = 0;
    if (op == OP_SLOT1) id = 1;
    else if (op == OP_SLOT2) id = 2;
    else if (op == OP_SLOT3) id = 3;
    int value = std::stoi(inputBuffers[id]);
        
    isAnimating = true;
    isAnimFinished = false;

    if (op == OP_SLOT1) { // OP_INSERT
        pCode = 1;
        activeCodeLine = 0;
        if (avl->rootCall() == nullptr) {
            searchTargetValue = value; 
            currentTask = TASK_HIGHLIGHT_NEW;
            animTimer = 0.5f;
        }
        else {
            // If nodes exist, start YELLOW search traversal!
            searchTargetValue = value;
            searchPointer = avl->rootCall();
            const_cast<Node*>(searchPointer)->color = YELLOW;
            
            currentTask = TASK_TRAVERSE_INSERT;
            animTimer = 0.5f; 
        }
        inputBuffers[1].clear();
    }
    else if (op == OP_SLOT2) {
        pCode = 2;
        activeCodeLine = 0;
        
        if (avl->rootCall() == nullptr) {
            // Empty tree, nothing to delete
            currentErrorSlot = 2;
            inputErrorMsg = "Tree is empty!";
            inputErrorTimer = 2.5f;
        }
        else {
            // Start traversal to find the node
            searchTargetValue = value;
            searchPointer = avl->rootCall();
            const_cast<Node*>(searchPointer)->color = YELLOW;
            
            currentTask = TASK_TRAVERSE_DELETE;
            animTimer = 0.5f; 
        }
        inputBuffers[2].clear();
    }
    else if (op == OP_SLOT3) {
        pCode = 3;
        activeCodeLine = 0;

        if (avl->rootCall() == nullptr) {
            // Empty tree, nothing to find
            currentErrorSlot = 3;
            inputErrorMsg = "Value not found!";
            inputErrorTimer = 2.5f;
        }
        else {
            // Start traversal to find the node
            searchTargetValue = value;
            searchPointer = avl->rootCall();
            const_cast<Node*>(searchPointer)->color = YELLOW;
            
            currentTask = TASK_TRAVERSE_SEARCH;
            animTimer = 0.5f;
        }
        inputBuffers[3].clear();
    }

    // Clear the text box on success
    activeInputFocus = -1;
}

void AVLTreeState::saveState() {

    AVLSnapshot snap;
    snap.currentTask       = currentTask;
    snap.activeCodeLine    = activeCodeLine;
    snap.searchTargetValue = searchTargetValue;
    snap.pCode             = pCode;
    snap.searchPointerKey  = (searchPointer != nullptr) ? searchPointer->key : -9999;
    
    snap.treeCopy = new AVLTree();
    snap.treeCopy->copyTree(*avl);
    
    snapHistory.push_back(snap);
    currentSnapIndex = snapHistory.size() - 1; // Move index to the very end
}

void AVLTreeState::undoState() {
    if (currentSnapIndex > 0) { 
        currentSnapIndex--;
        restoreFromSnapshot(snapHistory[currentSnapIndex]);
    }
    else if (opIndex > 0) {
        opIndex--;
        restoreFromOpSnapshot(opHistory[opIndex]);
    }
}

void AVLTreeState::redoState() {
    // Scrub forward using cached memory
    if (currentSnapIndex < (int)snapHistory.size() - 1) {
        currentSnapIndex++;
        restoreFromSnapshot(snapHistory[currentSnapIndex]);
    }
    else if (opIndex < (int)opHistory.size() - 1) {
        opIndex++;
        restoreFromOpSnapshot(opHistory[opIndex]);
    }
}

void AVLTreeState::saveOpState() {
    AVLOpSnapshot snap;
    snap.treeCopy = new AVLTree();
    snap.treeCopy->copyTree(*avl);
    snap.pCode = pCode;
    opHistory.push_back(snap);
    opIndex = (int)opHistory.size() - 1;
}

void AVLTreeState::handleAnimationStep() {
    // Dispatch to whichever task is active
    if (currentTask == TASK_TRAVERSE_INSERT)      stepTraverseInsert();
    else if (currentTask == TASK_TRAVERSE_SEARCH) stepTraverseSearch();
    else if (currentTask == TASK_TRAVERSE_DELETE) stepTraverseDelete();
    else if (currentTask == TASK_HIGHLIGHT_NEW)   stepHighlightNew();
    else if (currentTask == TASK_HIGHLIGHT_FOR_DELETE) stepHighlightForDelete();
    else if (currentTask == TASK_WAIT_FOR_BALANCE) stepWaitForBalance();
    saveState();
    if (currentTask == TASK_NONE) {
        isAnimating    = false;
        isAnimFinished = true;
        saveOpState();
    }
}

void AVLTreeState::restoreFromSnapshot(const AVLSnapshot& snap) {
    avl->clear();
    avl->copyTree(*snap.treeCopy);

    currentTask        = snap.currentTask;
    activeCodeLine     = snap.activeCodeLine;
    searchTargetValue  = snap.searchTargetValue;
    pCode              = snap.pCode;

    if (currentTask != TASK_NONE && snap.searchPointerKey != -9999) {
        searchPointer = avl->search(snap.searchPointerKey);
    } else {
        searchPointer = nullptr;
    }

    // Sync UI to the timeline state
    if (currentTask == TASK_NONE) {
        isAnimating = false;
        isAnimFinished = true;
    } else {
        isAnimating = true;
        isAnimFinished = false;
    }
}

void AVLTreeState::restoreFromOpSnapshot(const AVLOpSnapshot& op) {
    avl->clear();
    avl->copyTree(*op.treeCopy);
    pCode = op.pCode;
    currentTask    = TASK_NONE;
    activeCodeLine = -1;
    searchPointer  = nullptr;
    isAnimating    = false;
    isAnimFinished = true;

    for (auto& s : snapHistory) delete s.treeCopy;
    snapHistory.clear();
    currentSnapIndex = -1;
}

void AVLTreeState::snapNodePositions(Node* node) {
    if (!node) return;
    node->position = node->targetPosition;
    snapNodePositions(node->left);
    snapNodePositions(node->right);
}

void AVLTreeState::stepTraverseInsert() {
    if (activeCodeLine == 0) {
        if (searchTargetValue < searchPointer->key) {
            activeCodeLine = 3; // Highlight "node->left = insert(...)"
            animTimer = 0.5f;
        } 
        else if (searchTargetValue > searchPointer->key) {
            activeCodeLine = 5; // Highlight "node->right = insert(...)"
            animTimer = 0.5f;
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
                animTimer = 0.5f;
            } else {
                // Reached the bottom, insert it!
                activeCodeLine = 0; // Loop back to the function signature!
                animTimer = 0.5f;
                currentTask = TASK_HIGHLIGHT_NEW;
            }
        } 
        else if (searchTargetValue > searchPointer->key) {
            if (searchPointer->right != nullptr) {
                // Jump to the right child!
                searchPointer = searchPointer->right;
                const_cast<Node*>(searchPointer)->color = YELLOW;
                activeCodeLine = 0; // Loop back to the function signature!
                animTimer = 0.5f;
            } else {
                // Reached the bottom, insert it!
                activeCodeLine = 0; // Loop back to the function signature!
                animTimer = 0.5f;
                currentTask = TASK_HIGHLIGHT_NEW;
            }
        }
    }
}
void AVLTreeState::stepTraverseSearch() {
    if (activeCodeLine == 0) {
        // Check if we found the node or if it's NULL
        if (searchTargetValue == searchPointer->key) {
            // Found it!
            const_cast<Node*>(searchPointer)->color = GREEN;
            currentTask = TASK_NONE;
            activeCodeLine = 2; // Highlight "return node;"
            animTimer = 0.0f;
        }
        else if (searchTargetValue < searchPointer->key) {
            activeCodeLine = 4; // Highlight "return search(node->left, key);"
            animTimer = 0.5f;
        } 
        else if (searchTargetValue > searchPointer->key) {
            activeCodeLine = 5; // Highlight "return search(node->right, key);"
            animTimer = 0.5f;
        }
    }
    else if (activeCodeLine == 4 || activeCodeLine == 5) {
        // Reset current node color
        const_cast<Node*>(searchPointer)->color = SKYBLUE; 

        if (searchTargetValue < searchPointer->key) {
            if (searchPointer->left != nullptr) {
                // Jump to the left child!
                searchPointer = searchPointer->left;
                const_cast<Node*>(searchPointer)->color = YELLOW;
                activeCodeLine = 0; // Loop back to the function signature!
                animTimer = 0.5f;
            } else {
                // Not found!
                currentErrorSlot = 3;
                inputErrorMsg = "Value not found!";
                inputErrorTimer = 2.5f;
                currentTask = TASK_NONE;
                activeCodeLine = -1;
            }
        } 
        else if (searchTargetValue > searchPointer->key) {
            if (searchPointer->right != nullptr) {
                // Jump to the right child!
                searchPointer = searchPointer->right;
                const_cast<Node*>(searchPointer)->color = YELLOW;
                activeCodeLine = 0; // Loop back to the function signature!
                animTimer = 0.5f;
            } else {
                // Not found!
                currentErrorSlot = 3;
                inputErrorMsg = "Value not found!";
                inputErrorTimer = 2.5f;
                currentTask = TASK_NONE;
                activeCodeLine = -1;
            }
        }
    }
}
void AVLTreeState::stepTraverseDelete() {
    if (activeCodeLine == 0) {
        // Check if we found the node
        if (searchTargetValue == searchPointer->key) {
            // Found it! Highlight it and proceed to deletion
            const_cast<Node*>(searchPointer)->color = GREEN;
            currentTask = TASK_HIGHLIGHT_FOR_DELETE;
            activeCodeLine = 6; // Highlight the "else { /* Delete... */" line
            animTimer = 0.5f;
        }
        else if (searchTargetValue < searchPointer->key) {
            activeCodeLine = 3; // Highlight "node->left = deleteNode(...)"
            animTimer = 0.5f;
        } 
        else if (searchTargetValue > searchPointer->key) {
            activeCodeLine = 5; // Highlight "node->right = deleteNode(...)"
            animTimer = 0.5f;
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
                animTimer = 0.5f;
            } else {
                // Not found!
                currentErrorSlot = 2;
                inputErrorMsg = "Value not found!";
                inputErrorTimer = 2.5f;
                currentTask = TASK_NONE;
                activeCodeLine = -1;
            }
        } 
        else if (searchTargetValue > searchPointer->key) {
            if (searchPointer->right != nullptr) {
                // Jump to the right child!
                searchPointer = searchPointer->right;
                const_cast<Node*>(searchPointer)->color = YELLOW;
                activeCodeLine = 0; // Loop back to the function signature!
                animTimer = 0.5f;
            } else {
                // Not found!
                currentErrorSlot = 2;
                inputErrorMsg = "Value not found!";
                inputErrorTimer = 2.5f;
                currentTask = TASK_NONE;
                activeCodeLine = -1;
            }
        }
    }
}
void AVLTreeState::stepHighlightNew() {
    // Time is up! Reset the colors and clear the task.
    if (activeCodeLine == 0) {
        // We just highlighted the signature. Now highlight the base case!
        activeCodeLine = 1; // "if (node == NULL) return new Node(key);"
        animTimer = 0.5f;
    } 
    else if (activeCodeLine == 1) {
        // Time to actually insert the node into the backend!
        avl->insert(searchTargetValue);
        
        // Find it and color it green
        const Node* newObj = avl->search(searchTargetValue);
        if (newObj != nullptr) const_cast<Node*>(newObj)->color = GREEN;
        
        // Move on to balancing
        currentTask = TASK_WAIT_FOR_BALANCE;
        activeCodeLine = -1;
        animTimer = 0.5f;
    }
}
void AVLTreeState::stepHighlightForDelete() {
    // Time to delete the node
    avl->delNode(searchTargetValue);
    
    // Move to balancing phase
    currentTask = TASK_WAIT_FOR_BALANCE;
    activeCodeLine = -1;
    animTimer = 0.5f;
}
void AVLTreeState::stepWaitForBalance() {
    // Try to do exactly ONE rotation.
    int didRotate = avl->balance();
    
    if (didRotate != 0) {
        // A rotation happened! Show the function signature first
        if (didRotate == 1 || didRotate == 4) {
            pCode = 4;
            activeCodeLine = 0; // Highlight function signature first
            animTimer = 0.5f;
        }
        // If we did a LEFT rotate (RR Case, or first half of LR Case)
        else if (didRotate == 2 || didRotate == 3) {
            pCode = 5;
            activeCodeLine = 0; // Highlight function signature first
            animTimer = 0.5f;
        }
    } 
    else {
        // Tree is completely balanced! Clear task and reset the code box.
        currentTask = TASK_NONE;
        animTimer = 0.0f;
        pCode = 6;
        activeCodeLine = -1; // Turn off highlight
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