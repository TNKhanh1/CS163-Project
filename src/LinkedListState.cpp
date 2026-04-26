#include "LinkedListState.h"
#include <iostream>
#include <cmath>
#include "raymath.h"

LinkedListState::LinkedListState() : DataStructureState()
{
	head = nullptr;
	startX = 100.0f;
	startY = 450.0f;
	nodeSpacing = 150.0f;
	nodeRadius = 40.0f;

	NextState = (int)STATE_LINKEDLIST;

	activeInputFocus = -1;
	previousInputFocus = -1;
	isCreateUserDefOpen = false;

	searchPointer = nullptr;
	currentTask = LL_TASK_NONE;
	previousZoomMultiplier = 1.0f;

	activeCodeLine = -1;
	pseudoCode = {
		"// Select an operation from",
		"// the control panel to see",
		"// the algorithm execution."
	};
}

LinkedListState::~LinkedListState()
{
	clearList();
}

void LinkedListState::loadAssets()
{
	// Base class now handles Background, Home Button, Fonts, and Control Texture
	DataStructureState::loadAssets(); 
}

void LinkedListState::update(float deltaTime)
{
	Vector2 mousePos = GetMousePosition();
	
	// 1. Update shared UI logic
	DataStructureState::updateSharedUI(deltaTime, mousePos);
	DataStructureState::updateControlPanel(deltaTime, mousePos);

	if (zoomMultiplier != previousZoomMultiplier) {
		updateTargetPositions();
		previousZoomMultiplier = zoomMultiplier;
	}

	// 2. Input Handling using inherited inputBuffers
	if (activeInputFocus != previousInputFocus) {
		cursorIndex = (activeInputFocus == -1) ? 0 : inputBuffers[activeInputFocus].length();
		textScrollX = 0.0f; 
		cursorBlinkTimer = 0.0f;
		cursorVisible = true;
		previousInputFocus = activeInputFocus;
	}

	if (activeInputFocus != -1) {
		HandleTextInput(inputBuffers[activeInputFocus], (activeInputFocus == 0));
	}
	// 3. Node movement animation
	LLNode* currentAnim = head;
	while (currentAnim != nullptr) 
	{
		currentAnim->position = Vector2Lerp(currentAnim->position, currentAnim->targetPosition, deltaTime * 8.0f);
		currentAnim = currentAnim->next;
	}
}

void LinkedListState::onExecuteOp(MainOp op)
{
	history.clear();
	try {
		switch (op) {
			case OP_SLOT1: // Create
				if (!inputBuffers[0].empty()) {
					clearList();                           
					std::string temp = "";
					for (char c : inputBuffers[0]) {
						if (c == ',') { 
							if (!temp.empty()) { insertNode(std::stoi(temp)); temp = ""; } 
						} else { 
							temp += c; 
						}
					}
					if (!temp.empty()) insertNode(std::stoi(temp));
					inputBuffers[0].clear(); 
					isCreateUserDefOpen = false;
				} 
				break;
			case OP_SLOT2: // Insert
				if (!inputBuffers[2].empty() && !inputBuffers[3].empty()) {
					insertNodeAtIndex(std::stoi(inputBuffers[2]), std::stoi(inputBuffers[3]));
					inputBuffers[2].clear(); 
					inputBuffers[3].clear();
				} 
				break;
			case OP_SLOT3: // Search
				if (!inputBuffers[1].empty()) { 
					searchNode(std::stoi(inputBuffers[1])); 
					inputBuffers[1].clear(); 
				} 
				break;
			case OP_SLOT4: // Delete
				if (!inputBuffers[4].empty()) { 
					deleteNodeAtIndex(std::stoi(inputBuffers[4])); 
					inputBuffers[4].clear(); 
				} 
				break;
			default: 
				break;
		}
	} catch (...) {
		inputErrorMsg = "Invalid Input!";
		inputErrorTimer = 2.0f;
	}
	activeInputFocus = -1; // Reset focus after execution
}

void LinkedListState::handleAnimationStep()
{
	if (currentTask == LL_TASK_SEARCH && searchPointer) {
		activeCodeLine = 1; // while loop condition
		if (searchPointer->value == searchTargetValue) {
			searchPointer->color = ORANGE; 
			activeCodeLine = 3; // return temp;
			isAnimating = false; isAnimFinished = true;
		} else {
			searchPointer->color = LIGHTGRAY; 
			searchPointer = searchPointer->next;
			if (searchPointer) {
				searchPointer->color = YELLOW;
				activeCodeLine = 4; // temp = temp->next;
			} else {
				currentErrorSlot = 2; // SEARCH row
				inputErrorMsg = "Value not found!";
				inputErrorTimer = 2.5f;
				activeCodeLine = 6; // return null;
				isAnimating = false; isAnimFinished = true;
			}
		}
	} 
	else if (currentTask == LL_TASK_DELETE_INDEX || currentTask == LL_TASK_INSERT_INDEX) {
		if (searchCurrentIndex < searchTargetIndex - 1 && searchPointer && searchPointer->next) {
			searchPointer->color = LIGHTGRAY;
			searchPointer = searchPointer->next;
			if (searchPointer) searchPointer->color = YELLOW;
			searchCurrentIndex++;
			activeCodeLine = 2; // temp = temp->next;
		} else {
			if (currentTask == LL_TASK_DELETE_INDEX) {
				if (searchCurrentIndex == searchTargetIndex - 1 && searchPointer && searchPointer->next) {
					LLNode* toDelete = searchPointer->next;
					searchPointer->next = toDelete->next;
					delete toDelete;
					activeCodeLine = 4; // temp->next = toDelete->next;
				} else {
					currentErrorSlot = 3; // DELETE row
					inputErrorMsg = "Index out of bounds!";
					inputErrorTimer = 2.5f;
				}
			} 
			else if (currentTask == LL_TASK_INSERT_INDEX) {
				if (searchCurrentIndex == searchTargetIndex - 1 && searchPointer) {
					LLNode* newNode = new LLNode{searchTargetValue, {startX, startY - 200}, {0,0}, searchPointer->next, SKYBLUE};
					searchPointer->next = newNode;
					activeCodeLine = 4; // newNode->next = temp->next;
				} else {
					currentErrorSlot = 1; // INSERT row
					inputErrorMsg = "Index out of bounds!";
					inputErrorTimer = 2.5f;
				}
			}
			updateTargetPositions();
			isAnimating = false; isAnimFinished = true;
			resetNodeColors();
		}
	}
}

void LinkedListState::DrawSubMenuContent()
{
	float mainHeight = 45.0f, gap = 8.0f;
	float subX = controlBtnPos.x + (float)controlTex.width + 15.0f + 125.0f + gap;
	float startY = controlBtnPos.y;

	switch (activeMainOp) 
	{
		case OP_SLOT1: // Create
			if (DrawButtonText({subX, startY}, "Empty", 90, mainHeight, false)) clearList();
			if (DrawButtonText({subX + 98, startY}, "User Defined", 160, mainHeight, isCreateUserDefOpen)) isCreateUserDefOpen = !isCreateUserDefOpen;
			if (DrawButtonText({subX + 266, startY}, "Random", 110, mainHeight, false)) {
				clearList();
				for(int i = 0; i < GetRandomValue(3, 7); i++) insertNode(GetRandomValue(1, 99));
			}
			if (isCreateUserDefOpen) {
				if (DrawTextBox({subX + 98, startY + mainHeight + gap}, inputBuffers[0], activeInputFocus == 0, 230, mainHeight, cursorIndex, textScrollX, cursorVisible)) activeInputFocus = 0;
				if (DrawButtonText({subX + 336, startY + mainHeight + gap}, "GO", 50, mainHeight, false)) onExecuteOp(OP_SLOT1);
			}
			break;

		case OP_SLOT2: // Insert
			DrawLabel({subX, startY + mainHeight + gap}, "Index=");
			if (DrawTextBox({subX + 80, startY + mainHeight + gap}, inputBuffers[2], activeInputFocus == 2, 70, mainHeight, cursorIndex, textScrollX, cursorVisible)) activeInputFocus = 2;
			
			DrawLabel({subX + 160, startY + mainHeight + gap}, "Value=");
			if (DrawTextBox({subX + 240, startY + mainHeight + gap}, inputBuffers[3], activeInputFocus == 3, 70, mainHeight, cursorIndex, textScrollX, cursorVisible)) activeInputFocus = 3;
			
			if (DrawButtonText({subX + 320, startY + mainHeight + gap}, "GO", 50, mainHeight, false)) onExecuteOp(OP_SLOT2);
			break;

		case OP_SLOT3: // Search
			DrawLabel({subX, startY + 2 * (mainHeight + gap)}, "Value=");
			if (DrawTextBox({subX + 80, startY + 2 * (mainHeight + gap)}, inputBuffers[1], activeInputFocus == 1, 120, mainHeight, cursorIndex, textScrollX, cursorVisible)) activeInputFocus = 1;
			if (DrawButtonText({subX + 210, startY + 2 * (mainHeight + gap)}, "GO", 50, mainHeight, false)) onExecuteOp(OP_SLOT3);
			break;

		case OP_SLOT4: // Delete
			DrawLabel({subX, startY + 3 * (mainHeight + gap)}, "Index=");
			if (DrawTextBox({subX + 80, startY + 3 * (mainHeight + gap)}, inputBuffers[4], activeInputFocus == 4, 120, mainHeight, cursorIndex, textScrollX, cursorVisible)) activeInputFocus = 4;
			if (DrawButtonText({subX + 210, startY + 3 * (mainHeight + gap)}, "GO", 50, mainHeight, false)) onExecuteOp(OP_SLOT4);
			break;
		default: 
			break;
	}
}

void LinkedListState::draw()
{
	DataStructureState::drawSharedUI();

	const char* titleText = "SINGLY LINKED LIST";
	DrawTextEx(listFont, titleText, { (1800.0f - MeasureTextEx(listFont, titleText, 55, 6.5f).x) / 2.0f, 20.0f }, 55, 6.5f, BLACK);

	float currentRadius = nodeRadius * zoomMultiplier;
	LLNode* curr = head;
	while (curr != nullptr) 
	{
		// 1. Draw HEAD Indicator
		if (curr == head) {
			float headLabelY = curr->position.y - currentRadius - (45.0f * zoomMultiplier);
			const char* headText = "HEAD";
			float headFontSize = 22.0f * zoomMultiplier;
			Vector2 headTextSize = MeasureTextEx(numberFont, headText, headFontSize, 1.0f);
			DrawTextEx(numberFont, headText, { curr->position.x - headTextSize.x / 2.0f, headLabelY }, headFontSize, 1.0f, DARKGREEN);
			DrawLineEx({curr->position.x, headLabelY + headTextSize.y}, {curr->position.x, curr->position.y - currentRadius - 5}, 2.0f * zoomMultiplier, DARKGREEN);
		}

		// 2. Draw TAIL Indicator
		if (curr->next == nullptr) {
			float tailOffset = (curr == head) ? (65.0f * zoomMultiplier) : (45.0f * zoomMultiplier);
			float tailLabelY = curr->position.y + currentRadius + tailOffset - (20.0f * zoomMultiplier);
			const char* tailText = "TAIL";
			float tailFontSize = 22.0f * zoomMultiplier;
			Vector2 tailTextSize = MeasureTextEx(numberFont, tailText, tailFontSize, 1.0f);
			DrawTextEx(numberFont, tailText, { curr->position.x - tailTextSize.x / 2.0f, tailLabelY }, tailFontSize, 1.0f, MAROON);
			DrawLineEx({curr->position.x, tailLabelY - 5}, {curr->position.x, curr->position.y + currentRadius + 5}, 2.0f * zoomMultiplier, MAROON);
		}

		// 3. Draw Connection Lines
		if (curr->next != nullptr) 
		{
			Vector2 startPos = { curr->position.x + currentRadius, curr->position.y };
			Vector2 endPos = { curr->next->position.x - currentRadius, curr->next->position.y };
			DrawLineEx(startPos, endPos, fmaxf(1.0f, 3.0f * zoomMultiplier), DARKGRAY);
			
			float arrowSize = 12.0f * zoomMultiplier;
			DrawTriangle(endPos, 
						 { endPos.x - arrowSize, endPos.y - arrowSize * 0.7f },
						 { endPos.x - arrowSize, endPos.y + arrowSize * 0.7f }, 
						 DARKGRAY);
		}

		DrawCircleV(curr->position, currentRadius, curr->color);
		DrawCircleLines(curr->position.x, curr->position.y, currentRadius, DARKBLUE);
		
		float scaledFontSize = 25.0f * zoomMultiplier;
		const char* valText = TextFormat("%d", curr->value);
		Vector2 tSize = MeasureTextEx(numberFont, valText, scaledFontSize, 1.0f);
		DrawTextEx(numberFont, valText, { curr->position.x - tSize.x / 2.0f, curr->position.y - tSize.y / 2.0f }, scaledFontSize, 1.0f, WHITE);
		
		curr = curr->next;
	}

	// Pseudocode Panel
	drawPseudoCode();

	// 4. Control Panel UI
	DrawTextureV(controlTex, controlBtnPos, WHITE);
	DrawSideMenuFrame({"Create", "Insert", "Search", "Delete"});
}

// Undo/redo state management
void LinkedListState::saveState()
{
	LLStateSnapshot snap;
	snap.activeCodeLine = activeCodeLine;
	snap.currentTask = currentTask;
	snap.searchTargetValue = searchTargetValue;
	snap.searchTargetIndex = searchTargetIndex;
	snap.searchCurrentIndex = searchCurrentIndex;
	snap.searchPointerIndex = -1;

	// Copy the exact state of every node into the snapshot
	LLNode* curr = head;
	int currentIndex = 0;
	while (curr != nullptr) {
		snap.nodes.push_back({ curr->value, curr->color });
		if (curr == searchPointer) {
			snap.searchPointerIndex = currentIndex;
		}
		curr = curr->next;
		currentIndex++;
	}

	history.push_back(snap);
}

void LinkedListState::undoState()
{
	if (history.empty()) return;

	// Get the last snapshot and remove it from history
	LLStateSnapshot snap = history.back();
	history.pop_back();

	//  Update existing nodes in-place to avoid O(N^2) traversal and to preserve current Vector2 physical screen coordinates.
	LLNode* curr = head;
	LLNode* prev = nullptr;
	size_t snapIndex = 0;

	// Phase A: Update existing nodes
	while (curr != nullptr && snapIndex < snap.nodes.size()) {
		curr->value = snap.nodes[snapIndex].value;
		curr->color = snap.nodes[snapIndex].color;
		
		prev = curr;
		curr = curr->next;
		snapIndex++;
	}

	// Phase B: Snapshot has more nodes (undid a delete operation)
	while (snapIndex < snap.nodes.size()) {
		LLNode* newNode = new LLNode{ 
			snap.nodes[snapIndex].value, 
			{startX, startY - 200.0f}, // Will be corrected by updateTargetPositions
			{0,0}, 
			nullptr, 
			snap.nodes[snapIndex].color 
		};
		
		if (prev == nullptr) {
			head = newNode;
		} else {
			prev->next = newNode;
		}
		prev = newNode;
		snapIndex++;
	}

	// Phase C: Snapshot has fewer nodes (undid an insert operation)
	if (curr != nullptr) {
		if (prev == nullptr) {
			head = nullptr; // The snapshot was completely empty
		} else {
			prev->next = nullptr; // Sever the tie to the excess nodes
		}
		
		// Safely delete the abandoned excess nodes
		while (curr != nullptr) {
			LLNode* temp = curr->next;
			delete curr;
			curr = temp;
		}
	}

	// Recalculate where everything is
	updateTargetPositions(); 

	// Restore the UI State
	activeCodeLine = snap.activeCodeLine;
	currentTask = snap.currentTask;
	searchTargetValue = snap.searchTargetValue;
	searchTargetIndex = snap.searchTargetIndex;
	searchCurrentIndex = snap.searchCurrentIndex;
	
	// Restore the Search Pointer
	searchPointer = nullptr;
	if (snap.searchPointerIndex != -1) {
		LLNode* temp = head;
		for (int i = 0; i < snap.searchPointerIndex; i++) {
			if (temp) temp = temp->next;
		}
		searchPointer = temp;
	}
	
	// Resume animation state
	isAnimating = true;
	isAnimFinished = false;
}


// CÁC HÀM XỬ LÝ LINKED LIST

void LinkedListState::insertNode(int value)
{
	resetNodeColors();
	LLNode* newNode = new LLNode{ value, {startX, startY - 200.0f}, {0,0}, nullptr, SKYBLUE };
	
	if (head == nullptr) head = newNode;
	else {
		LLNode* temp = head;
		while (temp->next != nullptr) temp = temp->next;
		temp->next = newNode;
	}
	updateTargetPositions(); 
}

void LinkedListState::searchNode(int value)
{
	if (head == nullptr) { currentErrorSlot = 2; inputErrorMsg = "List is empty!"; inputErrorTimer = 2.5f; return; }
	resetNodeColors();
	
	pseudoCode = {
		"Node* temp = head;",
		"while (temp != nullptr) {",
		"    if (temp->value == target)",
		"        return temp;",
		"    temp = temp->next;",
		"}",
		"return null;"
	};
	activeCodeLine = 1;
	
	searchTargetValue = value; searchPointer = head; searchPointer->color = YELLOW;
	isAnimating = true; isAnimFinished = false;
	currentTask = LL_TASK_SEARCH; animTimer = 0.0f;
}

void LinkedListState::deleteNodeAtIndex(int index)
{
	if (head == nullptr) { currentErrorSlot = 3; inputErrorMsg = "List is empty!"; inputErrorTimer = 2.5f; return; }
	if (index < 0) { currentErrorSlot = 3; inputErrorMsg = "Invalid index!"; inputErrorTimer = 2.5f; return; }
	resetNodeColors();
	
	if (index == 0) {
		LLNode* temp = head; head = head->next; delete temp; updateTargetPositions();
	} else {
		pseudoCode = {
			"Node* temp = head;",
			"for (int i = 0; i < index - 1; i++)",
			"    temp = temp->next;",
			"Node* toDelete = temp->next;",
			"temp->next = toDelete->next;",
			"delete toDelete;"
		};
		activeCodeLine = 1;

		searchTargetIndex = index; searchCurrentIndex = 0; searchPointer = head; searchPointer->color = YELLOW;
		isAnimating = true; isAnimFinished = false;
		currentTask = LL_TASK_DELETE_INDEX; animTimer = 0.0f;
	}
}

void LinkedListState::insertNodeAtIndex(int index, int value)
{
	if (index < 0) { currentErrorSlot = 1; inputErrorMsg = "Invalid index!"; inputErrorTimer = 2.5f; return; }
	resetNodeColors();
	
	if (index == 0 || head == nullptr) {
		LLNode* newNode = new LLNode{ value, {startX, startY - 200.0f}, {0,0}, head, SKYBLUE };
		head = newNode; updateTargetPositions();
	} else {
		pseudoCode = {
			"Node* temp = head;",
			"for (int i = 0; i < index - 1; i++)",
			"    temp = temp->next;",
			"Node* newNode = new Node(value);",
			"newNode->next = temp->next;",
			"temp->next = newNode;"
		};
		activeCodeLine = 1;
		
		searchTargetIndex = index; searchTargetValue = value; searchCurrentIndex = 0;
		searchPointer = head; searchPointer->color = YELLOW; 
		isAnimating = true; isAnimFinished = false;
		currentTask = LL_TASK_INSERT_INDEX; animTimer = 0.0f;
	}
}

void LinkedListState::resetNodeColors()
{
	LLNode* curr = head;
	while (curr) { 
		curr->color = SKYBLUE; 
		curr = curr->next; 
	}
}

void LinkedListState::clearList()
{
	LLNode* current = head;
	while (current != nullptr) {
		LLNode* next = current->next;
		delete current;
		current = next;
	}
	head = nullptr;
}

void LinkedListState::updateTargetPositions()
{
	LLNode* current = head;
	float currentX = startX;

	float currentSpacing = nodeSpacing * zoomMultiplier;

	while (current != nullptr) {
		current->targetPosition = { currentX, startY };
		currentX += currentSpacing;
		current = current->next;
	}
}