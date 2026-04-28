#include "AVLTreeState.h"
#include <fstream>
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
    activeInputFocus  = -1;
    previousInputFocus = -1;

    avl = new AVLTree();
    currentTask       = TASK_NONE;
    pCode             = 0;
    activeCodeLine    = -1;
    searchPointer     = nullptr;
    animTimer         = 0.0f;
    searchTargetValue = 0;
    overridesStepHandling = true;

    // Push the empty tree as the baseline op[0]
    AVLOpSnapshot* fresh = new AVLOpSnapshot();
    fresh->treeCopy = new AVLTree();
    fresh->pCode    = 0;
    opHistory.push_back(fresh);
    opIndex   = 0;
    stepIndex = -1;
}

AVLTreeState::~AVLTreeState()
{
    delete avl;
    for (auto* o : opHistory) {
        for (auto* s : o->steps) { delete s->treeCopy; delete s; }
        delete o->treeCopy; delete o;
    }
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
        textScrollX = 0.0f;
        cursorVisible = true;
        previousInputFocus = activeInputFocus;
    }

    if (activeInputFocus != -1) {
        HandleTextInput(inputBuffers[activeInputFocus], false);
    }

    // Back button
    if (stepBackwardRequested) {
        undoState();
        stepBackwardRequested = false;
    }

    // Next button
    if (stepForwardRequested) {
    bool atLiveTip = (opIndex == (int)opHistory.size() - 1)
                  && (currentTask != TASK_NONE)
                  && (stepIndex == -1 || stepIndex == (int)opHistory[opIndex]->steps.size() - 1);

    if (atLiveTip) {
        handleAnimationStep();
    } else {
        redoState();
    }
    stepForwardRequested = false;
}

    // AutoPlay
    if (isAutoPlay) {
    bool atLiveTip = (opIndex == (int)opHistory.size() - 1)
                  && (currentTask != TASK_NONE)
                  && (stepIndex == -1 || stepIndex == (int)opHistory[opIndex]->steps.size() - 1);
    bool hasAnythingToPlay = atLiveTip || hasNextStep();

    if (hasAnythingToPlay) {
        animTimer -= deltaTime * animSpeedMultiplier;
        if (animTimer <= 0.0f) {
            if (atLiveTip) {
                handleAnimationStep();
            } else {
                redoState();
            }
            animTimer = 0.5f;
        }
    }
}

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

    drawPseudoCode();
    drawDropZone();
}

bool AVLTreeState::checkBuffer(std::string& currentInput, int id) {
    if (currentInput.empty() || currentInput == "-") {
		currentErrorSlot = id; 
		inputErrorMsg = "Value cannot be empty!";
		inputErrorTimer = 2.5f;
		return false;
	}
    return true;
}

bool AVLTreeState::hasNextStep() {
    if (opIndex < 0) return false;
    // There's a next op to jump to
    if (opIndex < (int)opHistory.size() - 1) return true;
    // Within this op's cached steps
    if (stepIndex >= 0 && stepIndex < (int)opHistory[opIndex]->steps.size() - 1) return true;
    // At a cached step's end, final state exists
    if (stepIndex == (int)opHistory[opIndex]->steps.size() - 1 && !opHistory[opIndex]->steps.empty()) return true;
    // At live tip with animation still running
    if (stepIndex == -1 && currentTask != TASK_NONE) return true;
    return false;
}
bool AVLTreeState::hasPrevStep() {
    if (opIndex < 0) return false;
    if (opIndex > 0) return true;
    if (stepIndex > 0) return true;
    if (stepIndex == 0) return true;
    if (stepIndex == -1 && !opHistory[opIndex]->steps.empty()) return true;
    return false;
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
            DrawLabel({subX - 85, startY}, "Value=");

            // Draw the text box
            if (DrawTextBox({subX - 10, startY}, inputBuffers[1], activeInputFocus == 1, 100, mainHeight, cursorIndex, textScrollX, cursorVisible)) activeInputFocus = 1;

            // Draw the "GO" button. If clicked, manually trigger onExecuteOp!
            if (DrawButtonText({subX + 100, startY}, "GO", 50, mainHeight, false)) {
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
            DrawLabel({subX - 85, startY + mainHeight + gap}, "Value=");

            if (DrawTextBox({subX - 10, startY + mainHeight + gap}, inputBuffers[2], activeInputFocus == 2, 100, mainHeight, cursorIndex, textScrollX, cursorVisible)) activeInputFocus = 2;

            if (DrawButtonText({subX + 100, startY + mainHeight + gap}, "GO", 50, mainHeight, false)) {
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
            DrawLabel({subX - 85, startY + 2 * (mainHeight + gap)}, "Value=");

            if (DrawTextBox({subX - 10, startY + 2 * (mainHeight + gap)}, inputBuffers[3], activeInputFocus == 3, 100, mainHeight, cursorIndex, textScrollX, cursorVisible)) activeInputFocus = 3; 
            
            if (DrawButtonText({subX + 100, startY + 2 * (mainHeight + gap)}, "GO", 50, mainHeight, false)) {
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

            if (DrawButtonText({subX - 85, startY + 3 * (mainHeight + gap)}, "ACCEPT CLEAR", 160, mainHeight, false)) {
                onExecuteOp(OP_SLOT4);
            }
            break;

        case OP_SLOT5: // Random
            // Shift the Y position down for the 5th slot

            if (DrawButtonText({subX  - 85, startY + 4 * (mainHeight + gap)}, "GENERATE RANDOM", 210, mainHeight, false)) {
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

void AVLTreeState::onExecuteOp(MainOp op) {
    // 1. Jump to the absolute latest op's final state (ignores where opIndex is)
    if (!opHistory.empty()) {
        AVLOpSnapshot* latest = opHistory.back();
        avl->clear(); avl->copyTree(*latest->treeCopy);
        pCode = latest->pCode;
        opIndex = (int)opHistory.size() - 1;
    }
    currentTask = TASK_NONE;
    isAnimating = false; isAnimFinished = true;
    searchPointer = nullptr; stepIndex = -1;

    // 2. Clear wipes opHistory entirely (only case where we delete)
    if (op == OP_SLOT4) {
        avl->clear();
        for (auto* o : opHistory) {
            for (auto* s : o->steps) { delete s->treeCopy; delete s; }
            delete o->treeCopy; delete o;
        }
        opHistory.clear();
        pCode = 0; activeCodeLine = -1; opIndex = -1; stepIndex = -1;
        currentTask = TASK_NONE; isAnimating = false; isAnimFinished = true;
        // Push empty tree as baseline
        AVLOpSnapshot* fresh = new AVLOpSnapshot();
        fresh->treeCopy = new AVLTree();
        fresh->pCode = 0;
        opHistory.push_back(fresh);
        opIndex = 0;
        inputErrorMsg = ""; activeInputFocus = -1;
        return;
    }

    // 3. Random — instant op, no animation steps
    if (op == OP_SLOT5) {
        avl->clear();
        std::mt19937 gen(std::random_device{}());
        std::uniform_int_distribution<> distNodes(10, 20);
        std::uniform_int_distribution<> distValues(1, 100);
        int n = distNodes(gen);
        for (int i = 0; i < n; i++) {
            avl->insert(distValues(gen));
            while (avl->balance() != 0) {}
        }
        Node* r = const_cast<Node*>(avl->rootCall());
        updateTargetLayouts(r, 900.0f, 150.0f, 350.0f * zoomMultiplier);
        snapNodePositions(r);
        pCode = 7; activeCodeLine = -1;
        currentTask = TASK_NONE; isAnimating = false;
        AVLOpSnapshot* randOp = new AVLOpSnapshot();
        randOp->treeCopy = new AVLTree();
        randOp->treeCopy->copyTree(*avl);
        randOp->pCode = 7;
        opHistory.push_back(randOp);  // append, never delete old ops
        opIndex = (int)opHistory.size() - 1;
        inputErrorMsg = ""; activeInputFocus = -1;
        return;
    }

    // 4. Insert / Delete / Search — create new op entry, steps filled by handleAnimationStep
    AVLOpSnapshot* newOp = new AVLOpSnapshot();
    newOp->treeCopy = new AVLTree();
    newOp->treeCopy->copyTree(*avl); // placeholder, updated when op finishes
    newOp->pCode = pCode;
    opHistory.push_back(newOp);      // always append
    opIndex = (int)opHistory.size() - 1;
    stepIndex = -1;

    resetNodeColors(const_cast<Node*>(avl->rootCall()));
    int id = (op == OP_SLOT1) ? 1 : (op == OP_SLOT2) ? 2 : 3;
    if (!checkBuffer(inputBuffers[id], id)) return;
    int value = std::stoi(inputBuffers[id]);
    isAnimating = true; isAnimFinished = false;

    if (op == OP_SLOT1) {
        pCode = 1; activeCodeLine = 0; searchTargetValue = value;
        if (avl->rootCall() == nullptr) { currentTask = TASK_HIGHLIGHT_NEW; }
        else {
            searchPointer = avl->rootCall();
            const_cast<Node*>(searchPointer)->color = YELLOW;
            currentTask = TASK_TRAVERSE_INSERT;
        }
        inputBuffers[1].clear();
    }
    else if (op == OP_SLOT2) {
        pCode = 2; activeCodeLine = 0;
        if (avl->rootCall() == nullptr) {
            currentErrorSlot = 2; inputErrorMsg = "Tree is empty!"; inputErrorTimer = 2.5f;
        } else {
            searchTargetValue = value;
            searchPointer = avl->rootCall();
            const_cast<Node*>(searchPointer)->color = YELLOW;
            currentTask = TASK_TRAVERSE_DELETE;
        }
        inputBuffers[2].clear();
    }
    else if (op == OP_SLOT3) {
        pCode = 3; activeCodeLine = 0;
        if (avl->rootCall() == nullptr) {
            currentErrorSlot = 3; inputErrorMsg = "Value not found!"; inputErrorTimer = 2.5f;
        } else {
            searchTargetValue = value;
            searchPointer = avl->rootCall();
            const_cast<Node*>(searchPointer)->color = YELLOW;
            currentTask = TASK_TRAVERSE_SEARCH;
        }
        inputBuffers[3].clear();
    }
    activeInputFocus = -1;
}

AVLSnapshot* AVLTreeState::captureStep() {
    AVLSnapshot* s = new AVLSnapshot();
    s->currentTask       = currentTask;
    s->activeCodeLine    = activeCodeLine;
    s->searchTargetValue = searchTargetValue;
    s->pCode             = pCode;
    s->searchPointerKey  = (searchPointer != nullptr) ? searchPointer->key : -9999;
    s->treeCopy          = new AVLTree();
    s->treeCopy->copyTree(*avl);
    return s;
}

void AVLTreeState::applyStep(AVLSnapshot* s) {
    avl->clear();
    avl->copyTree(*s->treeCopy);
    currentTask       = s->currentTask;
    activeCodeLine    = s->activeCodeLine;
    searchTargetValue = s->searchTargetValue;
    pCode             = s->pCode;
    searchPointer     = (s->searchPointerKey != -9999)
                        ? avl->search(s->searchPointerKey) : nullptr;
    isAnimating    = (currentTask != TASK_NONE);
    isAnimFinished = !isAnimating;
}

void AVLTreeState::undoState() {
    if (opIndex < 0) return;
    AVLOpSnapshot* cur = opHistory[opIndex];

    if (stepIndex > 0) {
        stepIndex--;
        applyStep(cur->steps[stepIndex]);
    }
    else if (stepIndex == 0) {
        // Go to previous op's final state
        stepIndex = -1;
        if (opIndex > 0) {
            opIndex--;
            AVLOpSnapshot* prev = opHistory[opIndex];
            avl->clear(); avl->copyTree(*prev->treeCopy);
            pCode = prev->pCode;
            currentTask = TASK_NONE; activeCodeLine = -1;
            searchPointer = nullptr;
            isAnimating = false; isAnimFinished = true;
        }
    }
    else { // stepIndex == -1, at final state
        if (!cur->steps.empty()) {
            // Step into last animation step of this op
            stepIndex = (int)cur->steps.size() - 1;
            applyStep(cur->steps[stepIndex]);
        }
        else if (opIndex > 0) {
            // No steps (e.g. Random op), jump to previous op
            opIndex--;
            AVLOpSnapshot* prev = opHistory[opIndex];
            avl->clear(); avl->copyTree(*prev->treeCopy);
            pCode = prev->pCode;
            currentTask = TASK_NONE; activeCodeLine = -1;
            searchPointer = nullptr;
            isAnimating = false; isAnimFinished = true;
        }
    }
}

void AVLTreeState::redoState() {
    if (opIndex < 0) return;
    AVLOpSnapshot* cur = opHistory[opIndex];

    if (stepIndex >= 0 && stepIndex < (int)cur->steps.size() - 1) {
        // Scrub forward within cached steps
        stepIndex++;
        applyStep(cur->steps[stepIndex]);
    }
    else if (stepIndex >= 0 && stepIndex == (int)cur->steps.size() - 1) {
        // At last cached step — go to this op's final state
        stepIndex = -1;
        avl->clear(); avl->copyTree(*cur->treeCopy);
        pCode = cur->pCode;
        currentTask = TASK_NONE; activeCodeLine = -1;
        searchPointer = nullptr;
        isAnimating = false; isAnimFinished = true;
    }
    else if (stepIndex == -1 && opIndex < (int)opHistory.size() - 1) {
        // At final state — move to next op
        opIndex++;
        cur = opHistory[opIndex];
        if (!cur->steps.empty()) {
            // Land on first step of the next op
            stepIndex = 0;
            applyStep(cur->steps[0]);
        } else {
            // No steps (Random/Clear) — show final state directly
            stepIndex = -1;
            avl->clear(); avl->copyTree(*cur->treeCopy);
            pCode = cur->pCode;
            currentTask = TASK_NONE; activeCodeLine = -1;
            searchPointer = nullptr;
            isAnimating = false; isAnimFinished = true;
        }
    }
    // else: at live tip final state with no next op — nothing to do
}

void AVLTreeState::handleAnimationStep() {
    // Only compute fresh steps at the live tip
    if (opIndex != (int)opHistory.size() - 1) return;
    if (stepIndex >= 0 && stepIndex < (int)opHistory[opIndex]->steps.size() - 1) return;

    if (currentTask == TASK_TRAVERSE_INSERT)           stepTraverseInsert();
    else if (currentTask == TASK_TRAVERSE_SEARCH)      stepTraverseSearch();
    else if (currentTask == TASK_TRAVERSE_DELETE)      stepTraverseDelete();
    else if (currentTask == TASK_HIGHLIGHT_NEW)        stepHighlightNew();
    else if (currentTask == TASK_HIGHLIGHT_FOR_DELETE) stepHighlightForDelete();
    else if (currentTask == TASK_WAIT_FOR_BALANCE)     stepWaitForBalance();

    AVLOpSnapshot* curOp = opHistory[opIndex];
    curOp->steps.push_back(captureStep());
    stepIndex = (int)curOp->steps.size() - 1;

    if (currentTask == TASK_NONE) {
        isAnimating = false; isAnimFinished = true;
        curOp->treeCopy->clear();
        curOp->treeCopy->copyTree(*avl);
        curOp->pCode = pCode;
        stepIndex = -1;
    }
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
        } 
        else if (searchTargetValue > searchPointer->key) {
            activeCodeLine = 5; // Highlight "node->right = insert(...)"
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
            } else {
                // Reached the bottom, insert it!
                activeCodeLine = 0; // Loop back to the function signature!
                currentTask = TASK_HIGHLIGHT_NEW;
            }
        } 
        else if (searchTargetValue > searchPointer->key) {
            if (searchPointer->right != nullptr) {
                // Jump to the right child!
                searchPointer = searchPointer->right;
                const_cast<Node*>(searchPointer)->color = YELLOW;
                activeCodeLine = 0; // Loop back to the function signature!
            } else {
                // Reached the bottom, insert it!
                activeCodeLine = 0; // Loop back to the function signature!
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
        } 
        else if (searchTargetValue > searchPointer->key) {
            activeCodeLine = 5; // Highlight "return search(node->right, key);"
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
        }
        else if (searchTargetValue < searchPointer->key) {
            activeCodeLine = 3; // Highlight "node->left = deleteNode(...)"
        } 
        else if (searchTargetValue > searchPointer->key) {
            activeCodeLine = 5; // Highlight "node->right = deleteNode(...)"
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
    }
}
void AVLTreeState::stepHighlightForDelete() {
    // Time to delete the node
    avl->delNode(searchTargetValue);
    
    // Move to balancing phase
    currentTask = TASK_WAIT_FOR_BALANCE;
    activeCodeLine = -1;
}
void AVLTreeState::stepWaitForBalance() {
    // Try to do exactly ONE rotation.
    int didRotate = avl->balance();
    
    if (didRotate != 0) {
        // A rotation happened! Show the function signature first
        if (didRotate == 1 || didRotate == 4) {
            pCode = 4;
            activeCodeLine = 0; // Highlight function signature first
        }
        // If we did a LEFT rotate (RR Case, or first half of LR Case)
        else if (didRotate == 2 || didRotate == 3) {
            pCode = 5;
            activeCodeLine = 0; // Highlight function signature first
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




bool AVLTreeState::processDroppedFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file) { 
        inputErrorMsg = "ERROR: CANNOT OPEN FILE"; 
        return false; 
    }

    std::vector<int> temp;
    int val;
    while (file >> val) {
        temp.push_back(val);
        if (temp.size() > 20) { 
            inputErrorMsg = "ERROR: MAX 20 NODES EXCEEDED"; 
            return false; 
        }
    }
    
    if (temp.empty()) return false;

    avl->clear();
    for (auto* o : opHistory) {
        for (auto* s : o->steps) { delete s->treeCopy; delete s; }
        delete o->treeCopy; delete o;
    }
    opHistory.clear(); 
    
    pCode = 0; activeCodeLine = -1; opIndex = -1; stepIndex = -1;
    currentTask = TASK_NONE; isAnimating = false; isAnimFinished = true;

    for (int i = 0; i < (int)temp.size(); i++) {
        avl->insert(temp[i]); 
        while (avl->balance() != 0) {} 
    }
    
    AVLOpSnapshot* dropOp = new AVLOpSnapshot();
    dropOp->treeCopy = new AVLTree();
    dropOp->treeCopy->copyTree(*avl);
    dropOp->pCode = 0;
    opHistory.push_back(dropOp);
    opIndex = 0;
    
    Node* rootNode = const_cast<Node*>(avl->rootCall());
    
    updateTargetLayouts(rootNode, 900.0f, 150.0f, 350.0f * zoomMultiplier);
    
    snapNodePositions(rootNode); 
    
    inputErrorMsg = ""; 
    activeInputFocus = -1;
    return true;
}