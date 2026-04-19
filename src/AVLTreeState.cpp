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
    updateTargetLayouts(const_cast<Node*>(avl.rootCall()), 900.0f, 150.0f, 350.0f * zoomMultiplier);
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

    DrawTextureV(controlTex, controlBtnPos, WHITE);
    DrawSideMenuFrame({"Insert", "Delete", "Search"});
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
                onExecuteOp(OP_SLOT1);
            }
            break;

        case OP_SLOT2: // Delete
            // Shift the Y position down for the 2nd slot
            DrawLabel({subX, startY + mainHeight + gap}, "Value=");

            if (DrawTextBox({subX + 80, startY + mainHeight + gap}, inputBuffers[2], activeInputFocus == 2, 100, mainHeight, cursorIndex, textScrollX, cursorVisible)) activeInputFocus = 2;

            if (DrawButtonText({subX + 190, startY + mainHeight + gap}, "GO", 50, mainHeight, false)) {
                onExecuteOp(OP_SLOT2);
            }
            break;

        case OP_SLOT3: // Search
            // Shift the Y position down for the 3rd slot
            DrawLabel({subX, startY + 2 * (mainHeight + gap)}, "Value=");

            if (DrawTextBox({subX + 80, startY + 2 * (mainHeight + gap)}, inputBuffers[3], activeInputFocus == 3, 100, mainHeight, cursorIndex, textScrollX, cursorVisible)) activeInputFocus = 3; 
            
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
	std::string currentInput = "";
	int slotIndex = 0;

	// Determine which buffer we are looking at based on the active menu slot
	if (op == OP_SLOT1) { currentInput = inputBuffers[1]; slotIndex = 1; }
	else if (op == OP_SLOT2) { currentInput = inputBuffers[2]; slotIndex = 2; }
	else if (op == OP_SLOT3) { currentInput = inputBuffers[3]; slotIndex = 3; }

	// 1. Check for empty input
	if (currentInput.empty()) {
		currentErrorSlot = slotIndex; 
		inputErrorMsg = "Value cannot be empty!";
		inputErrorTimer = 2.5f;
		return;
	}

	try {
		int value = std::stoi(currentInput);
		
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
			inputBuffers[1].clear();
		}
		else if (op == OP_SLOT2) {
			avl.delNode(value);
			inputBuffers[2].clear();
		}
		else if (op == OP_SLOT3) {
			const Node* foundNode = avl.search(value);
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

        // Clear the text box on success
        activeInputFocus = -1;

	} catch (...) {
		currentErrorSlot = slotIndex;
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