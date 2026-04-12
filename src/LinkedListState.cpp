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

	NextState = (int)LinkedList;

	activeInputFocus = -1;
	previousInputFocus = -1;
	isCreateUserDefOpen = false;

	searchPointer = nullptr;
	currentTask = TASK_NONE;
	previousZoomMultiplier = 1.0f;
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
	if (currentTask == TASK_SEARCH && searchPointer) {
		if (searchPointer->value == searchTargetValue) {
			searchPointer->color = ORANGE; 
			isAnimating = false;
		} else {
			searchPointer->color = LIGHTGRAY; 
			searchPointer = searchPointer->next;
			if (searchPointer) searchPointer->color = YELLOW;
			else {
				currentErrorSlot = 2; // SEARCH row
				inputErrorMsg = "Value not found!";
				inputErrorTimer = 2.5f;
				isAnimating = false;
			}
		}
	} 
	else if (currentTask == TASK_DELETE_INDEX || currentTask == TASK_INSERT_INDEX) {
		if (searchCurrentIndex < searchTargetIndex - 1 && searchPointer && searchPointer->next) {
			searchPointer->color = LIGHTGRAY;
			searchPointer = searchPointer->next;
			if (searchPointer) searchPointer->color = YELLOW;
			searchCurrentIndex++;
		} else {
			if (currentTask == TASK_DELETE_INDEX) {
				if (searchCurrentIndex == searchTargetIndex - 1 && searchPointer && searchPointer->next) {
					LLNode* toDelete = searchPointer->next;
					searchPointer->next = toDelete->next;
					delete toDelete;
				} else {
					currentErrorSlot = 3; // DELETE row
					inputErrorMsg = "Index out of bounds!";
					inputErrorTimer = 2.5f;
				}
			} 
			else if (currentTask == TASK_INSERT_INDEX) {
				if (searchCurrentIndex == searchTargetIndex - 1 && searchPointer) {
					LLNode* newNode = new LLNode{searchTargetValue, {startX, startY - 200}, {0,0}, searchPointer->next, SKYBLUE};
					searchPointer->next = newNode;
				} else {
					currentErrorSlot = 1; // INSERT row
					inputErrorMsg = "Index out of bounds!";
					inputErrorTimer = 2.5f;
				}
			}
			updateTargetPositions();
			isAnimating = false;
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

	// 4. Control Panel UI
	DrawTextureV(controlTex, controlBtnPos, WHITE);
	DrawSideMenuFrame({"Create", "Insert", "Search", "Delete"}); // Delegates to base class
}

void LinkedListState::searchNode(int value)
{
	if (head == nullptr) { currentErrorSlot = 2; inputErrorMsg = "List is empty!"; inputErrorTimer = 2.5f; return; }
	resetNodeColors();
	searchTargetValue = value; searchPointer = head; searchPointer->color = YELLOW;
	isAnimating = true; currentTask = TASK_SEARCH; animTimer = 0.0f;
}

void LinkedListState::deleteNodeAtIndex(int index)
{
	if (head == nullptr) { currentErrorSlot = 3; inputErrorMsg = "List is empty!"; inputErrorTimer = 2.5f; return; }
	if (index < 0) { currentErrorSlot = 3; inputErrorMsg = "Invalid index!"; inputErrorTimer = 2.5f; return; }
	resetNodeColors();
	if (index == 0) {
		LLNode* temp = head; head = head->next; delete temp; updateTargetPositions();
	} else {
		searchTargetIndex = index; searchCurrentIndex = 0; searchPointer = head; searchPointer->color = YELLOW;
		isAnimating = true; currentTask = TASK_DELETE_INDEX; animTimer = 0.0f;
	}
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

void LinkedListState::insertNodeAtIndex(int index, int value)
{
	if (index < 0) { currentErrorSlot = 1; inputErrorMsg = "Invalid index!"; inputErrorTimer = 2.5f; return; }
	resetNodeColors();
	if (index == 0 || head == nullptr) {
		LLNode* newNode = new LLNode{ value, {startX, startY - 200.0f}, {0,0}, head, SKYBLUE };
		head = newNode; updateTargetPositions();
	} else {
		searchTargetIndex = index; searchTargetValue = value; searchCurrentIndex = 0;
		searchPointer = head; searchPointer->color = YELLOW; isAnimating = true;
		currentTask = TASK_INSERT_INDEX; animTimer = 0.0f;
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