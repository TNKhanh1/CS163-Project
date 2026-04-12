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

	activeSubPanel = SUB_NONE;
	activeInput = INP_NONE;
	previousActiveInput = INP_NONE;
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
	if (activeInput != previousActiveInput) {
	
		cursorIndex = (activeInput == INP_NONE) ? 0 : inputBuffers[activeInput - 1].length();
		textScrollX = 0.0f; 
		cursorBlinkTimer = 0.0f;
		cursorVisible = true;
		previousActiveInput = activeInput;
	}

	if (activeInput != INP_NONE) {
		HandleTextInput(inputBuffers[activeInput - 1], (activeInput == INP_CREATE));
		
		if (IsKeyPressed(KEY_ENTER)) {
			try {
				int idx = activeInput - 1;
				switch (activeInput) {
					case INP_CREATE: 
						if (!inputBuffers[idx].empty()) { insertNode(std::stoi(inputBuffers[idx])); inputBuffers[idx].clear(); }
						break;
					case INP_SEARCH:
						if (!inputBuffers[idx].empty()) { searchNode(std::stoi(inputBuffers[idx])); inputBuffers[idx].clear(); }
						break;
					case INP_INSERT_IDX:
					case INP_INSERT_VAL:
						if (!inputBuffers[2].empty() && !inputBuffers[3].empty()) {
							insertNodeAtIndex(std::stoi(inputBuffers[2]), std::stoi(inputBuffers[3]));
							inputBuffers[2].clear(); inputBuffers[3].clear();
						}
						break;
					case INP_DELETE_IDX:
						if (!inputBuffers[idx].empty()) { deleteNodeAtIndex(std::stoi(inputBuffers[idx])); inputBuffers[idx].clear(); }
						break;
					default: break;
				}
			} catch (...) {}
			activeInput = INP_NONE;
		}
	}

	// 3. Node movement animation
	LLNode* currentAnim = head;
	while (currentAnim != nullptr) 
	{
		currentAnim->position = Vector2Lerp(currentAnim->position, currentAnim->targetPosition, deltaTime * 8.0f);
		currentAnim = currentAnim->next;
	}
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
	if (panelAnimProgress > 0.0f) 
	{
		float easedProgress = sin(panelAnimProgress * PI / 2.0f); 
		float mainWidth = 125.0f, mainHeight = 45.0f, gap = 8.0f; 
		float startX = controlBtnPos.x + (float)controlTex.width + 15.0f;
		float startY = controlBtnPos.y; 
		
		BeginScissorMode((int)startX, 0, GetScreenWidth(), GetScreenHeight());
		float panelX = startX - mainWidth * (1.0f - easedProgress);

		const char* labels[] = {"Create", "Insert", "Search", "Delete"};
		ActiveSubPanel subPanels[] = {SUB_CREATE, SUB_INSERT, SUB_SEARCH, SUB_DELETE};

		for (int i = 0; i < 4; i++) {
			float itemY = startY + i * (mainHeight + gap);
			if (DrawButtonText({panelX, itemY}, labels[i], mainWidth, mainHeight, (activeSubPanel == subPanels[i]))) {
				if (activeSubPanel == subPanels[i]) activeSubPanel = SUB_NONE; 
				else { activeSubPanel = subPanels[i]; isCreateUserDefOpen = false; }
			}
		}

		float subX = panelX + mainWidth + gap; 
		switch (activeSubPanel) 
		{
			case SUB_CREATE: {
				float sy = startY; 
				if (DrawButtonText({subX, sy}, "Empty", 90, mainHeight, false)) clearList();
				if (DrawButtonText({subX + 98, sy}, "User Defined", 160, mainHeight, isCreateUserDefOpen)) isCreateUserDefOpen = !isCreateUserDefOpen; 
				if (DrawButtonText({subX + 266, sy}, "Random", 110, mainHeight, false)) { 
					clearList();
					int n = GetRandomValue(1, 9); 
					for(int i = 0; i < n; i++) insertNode(GetRandomValue(1, 99));
				}
				if (isCreateUserDefOpen) {
					if (DrawTextBox({subX + 98, sy + mainHeight + gap}, inputBuffers[0], activeInput == INP_CREATE, 230, mainHeight, cursorIndex, textScrollX, cursorVisible)) activeInput = INP_CREATE;
					if (DrawButtonText({subX + 336, sy + mainHeight + gap}, "GO", 50, mainHeight, false)) {
						if (!inputBuffers[0].empty()) {
							clearList();                           
							std::string temp = "";
							for (char c : inputBuffers[0]) {
								if (c == ',') { if (!temp.empty()) { insertNode(std::stoi(temp)); temp = ""; } } 
								else { temp += c; }
							}
							if (!temp.empty()) insertNode(std::stoi(temp));
							inputBuffers[0].clear(); activeInput = INP_NONE; isCreateUserDefOpen = false;
						}
					}
				}
				break;
			}
			case SUB_INSERT: {
				float sy = startY + mainHeight + gap;
				DrawLabel({subX, sy}, "Idx =");
				if (DrawTextBox({subX + 60, sy}, inputBuffers[2], activeInput == INP_INSERT_IDX, 80, mainHeight, cursorIndex, textScrollX, cursorVisible)) activeInput = INP_INSERT_IDX;
				DrawLabel({subX + 150, sy}, "Val =");
				if (DrawTextBox({subX + 210, sy}, inputBuffers[3], activeInput == INP_INSERT_VAL, 80, mainHeight, cursorIndex, textScrollX, cursorVisible)) activeInput = INP_INSERT_VAL;
				if (DrawButtonText({subX + 300, sy}, "GO", 50, mainHeight, false)) {
					if (!inputBuffers[2].empty() && !inputBuffers[3].empty()) {
						insertNodeAtIndex(std::stoi(inputBuffers[2]), std::stoi(inputBuffers[3])); 
						inputBuffers[2].clear(); inputBuffers[3].clear(); activeInput = INP_NONE;
					}
				}
				break;
			}
			case SUB_SEARCH: {
				float sy = startY + 2 * (mainHeight + gap);
				DrawLabel({subX, sy}, "Val =");
				if (DrawTextBox({subX + 60, sy}, inputBuffers[1], activeInput == INP_SEARCH, 120, mainHeight, cursorIndex, textScrollX, cursorVisible)) activeInput = INP_SEARCH;
				if (DrawButtonText({subX + 190, sy}, "GO", 50, mainHeight, false)) {
					if (!inputBuffers[1].empty()) { searchNode(std::stoi(inputBuffers[1])); inputBuffers[1].clear(); activeInput = INP_NONE; }
				}
				break;
			}
			case SUB_DELETE: {
				float sy = startY + 3 * (mainHeight + gap);
				DrawLabel({subX, sy}, "Idx =");
				if (DrawTextBox({subX + 60, sy}, inputBuffers[4], activeInput == INP_DELETE_IDX, 120, mainHeight, cursorIndex, textScrollX, cursorVisible)) activeInput = INP_DELETE_IDX;
				if (DrawButtonText({subX + 190, sy}, "GO", 50, mainHeight, false)) {
					if (!inputBuffers[4].empty()) { deleteNodeAtIndex(std::stoi(inputBuffers[4])); inputBuffers[4].clear(); activeInput = INP_NONE; }
				}
				break;
			}
			default: break;
		}
		EndScissorMode();    
	}
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