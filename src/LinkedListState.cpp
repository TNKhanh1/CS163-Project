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

	//Khá»Ÿi táº¡o tráº¡ng thÃ¡i UI 
	controlBtnPos = { 30.0f, 750.0f }; 
	isDraggingControlBtn = false;
	isClickingControlBtn = false;
	isPanelOpen = false;
	panelAnimProgress = 0.0f;
	activeSubPanel = SUB_NONE;
	activeInput = INP_NONE;
	previousActiveInput = INP_NONE;
	isCreateUserDefOpen = false;

	// Khá»Ÿi táº¡o TextBox state
	cursorIndex = 0;
	cursorBlinkTimer = 0.0f;
	cursorVisible = true;
	textScrollX = 0.0f;

	// Initialize Animation State
	searchPointer = nullptr;
	currentTask = TASK_NONE;
	isAnimating = false;
}

LinkedListState::~LinkedListState()
{
	clearList();
	UnloadTexture(controlTex); 
}

void LinkedListState::loadAssets()
{
	// Load shared assets via base class (Background, Home Button, Fonts)
	DataStructureState::loadAssets(); 

	Image ctrlImg = LoadImage("assets/control.png");
	ImageResize(&ctrlImg, 75, 75); 
	controlTex = LoadTextureFromImage(ctrlImg);
	UnloadImage(ctrlImg);
}

// LOGIC TEXTBOX
bool LinkedListState::IsValidInputString(const std::string& str, ActiveInput type)
{
	if (type == INP_CREATE && str.length() > 30) return false;

	if (!str.empty() && str[0] == ',') return false;

	int currentDigitCount = 0;
	for (size_t i = 0; i < str.length(); i++)
	{
		char c = str[i];
		if (c == '-') {
			if (i != 0 && str[i-1] != ',') return false;
		}
		else if (c == ',') {
			if (type != INP_CREATE) return false;

			if (i > 0 && str[i-1] == ',') return false;

			currentDigitCount = 0; 
		}
		else if (c >= '0' && c <= '9') {
			currentDigitCount++;
			if (currentDigitCount > 4) return false; 
		}
		else {
            return false; 
        }
	}
	return true;
}

void LinkedListState::HandleTextInput(std::string& text, ActiveInput type)
{
	if ((IsKeyPressed(KEY_LEFT) || IsKeyPressedRepeat(KEY_LEFT)) && cursorIndex > 0) cursorIndex--;
	if ((IsKeyPressed(KEY_RIGHT) || IsKeyPressedRepeat(KEY_RIGHT)) && cursorIndex < (int)text.length()) cursorIndex++;

	if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT) || IsKeyPressedRepeat(KEY_LEFT) || IsKeyPressedRepeat(KEY_RIGHT)) {
		cursorVisible = true; cursorBlinkTimer = 0.0f;
	}

	if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) {
		if (cursorIndex > 0) {
			text.erase(cursorIndex - 1, 1);
			cursorIndex--;
			cursorVisible = true; cursorBlinkTimer = 0.0f;
		}
	}
	if (IsKeyPressed(KEY_DELETE) || IsKeyPressedRepeat(KEY_DELETE)) {
		if (cursorIndex < (int)text.length()) {
			text.erase(cursorIndex, 1);
			cursorVisible = true; cursorBlinkTimer = 0.0f;
		}
	}

    //QuÃ©t PhÃ­m Váº­t LÃ½
    int keyCode = GetKeyPressed();
	while (keyCode > 0) 
	{
		char charToAdd = 0;

		if (keyCode >= KEY_ZERO && keyCode <= KEY_NINE) charToAdd = '0' + (keyCode - KEY_ZERO);
		else if (keyCode >= KEY_KP_0 && keyCode <= KEY_KP_9) charToAdd = '0' + (keyCode - KEY_KP_0);
		else if (keyCode == KEY_MINUS || keyCode == KEY_KP_SUBTRACT) charToAdd = '-';
		else if (keyCode == KEY_COMMA) charToAdd = ',';
		
		if (charToAdd != 0) {
			std::string temp = text;
			temp.insert(cursorIndex, 1, charToAdd);

			if (IsValidInputString(temp, type)) {
				text = temp;
				cursorIndex++;
				cursorVisible = true; cursorBlinkTimer = 0.0f;
			}
		}
		keyCode = GetKeyPressed();
	}
	while (GetCharPressed() > 0) {} 
}


void LinkedListState::update(float deltaTime)
{
	Vector2 mousePos = GetMousePosition();
	
	// 1. Update base UI (interactions, speed slider, error messages)
	DataStructureState::updateSharedUI(deltaTime, mousePos);

	// 2. Traversal animation logic
	if (isAnimating) {
		// Use CheckStepReady from base class to control speed via the slider
		if (DataStructureState::CheckStepReady(deltaTime, 0.7f)) {
			
			if (currentTask == TASK_SEARCH) {
				if (searchPointer) {
					if (searchPointer->value == searchTargetValue) {
						searchPointer->color = ORANGE; 
						isAnimating = false;
					} else {
						searchPointer->color = LIGHTGRAY; 
						searchPointer = searchPointer->next;
						if (searchPointer) searchPointer->color = YELLOW;
						else {
							inputErrorMsg = "Value not found!";
							inputErrorTimer = 2.5f;
							isAnimating = false;
						}
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
					if (currentTask == TASK_DELETE_INDEX && searchPointer && searchPointer->next) {
						LLNode* toDelete = searchPointer->next;
						searchPointer->next = toDelete->next;
						delete toDelete;
					} 
					else if (currentTask == TASK_INSERT_INDEX && searchPointer) {
						LLNode* newNode = new LLNode{searchTargetValue, {startX, startY - 200}, {0,0}, searchPointer->next, SKYBLUE};
						searchPointer->next = newNode;
					}
					updateTargetPositions();
					isAnimating = false;
					resetNodeColors();
				}
			}
		}
	}

	// 3. Control panel button logic
	Rectangle controlBtnBounds = { controlBtnPos.x, controlBtnPos.y, (float)controlTex.width, (float)controlTex.height };
	
	if (CheckCollisionPointRec(mousePos, controlBtnBounds) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
		isDraggingControlBtn = true;
		isClickingControlBtn = true; 
		dragOffset = { mousePos.x - controlBtnPos.x, mousePos.y - controlBtnPos.y };
	}

	if (isDraggingControlBtn) {
		if (isClickingControlBtn && Vector2Distance(mousePos, {controlBtnPos.x + dragOffset.x, controlBtnPos.y + dragOffset.y}) > 3.0f) {
			isClickingControlBtn = false; 
		}
		controlBtnPos.x = mousePos.x - dragOffset.x;
		controlBtnPos.y = mousePos.y - dragOffset.y;
	}

	if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
		if (isDraggingControlBtn) {
			isDraggingControlBtn = false; 
			if (isClickingControlBtn && CheckCollisionPointRec(mousePos, controlBtnBounds)) {
				isPanelOpen = !isPanelOpen; 
			}
		}
		isClickingControlBtn = false; 
	}

	float animSpeed = 4.0f; 
	if (isPanelOpen) {
		panelAnimProgress += deltaTime * animSpeed;
		if (panelAnimProgress > 1.0f) panelAnimProgress = 1.0f;
	} else {
		panelAnimProgress -= deltaTime * animSpeed;
		if (panelAnimProgress < 0.0f) {
			panelAnimProgress = 0.0f; activeSubPanel = SUB_NONE; 
		}
	}

    //Cáº­p nháº­t Textbox & Con trá»
	if (activeInput != previousActiveInput) {
		if (activeInput == INP_CREATE) cursorIndex = inputCreate.length();
		else if (activeInput == INP_SEARCH) cursorIndex = inputSearch.length();
		else if (activeInput == INP_INSERT_IDX) cursorIndex = inputInsertIdx.length();
		else if (activeInput == INP_INSERT_VAL) cursorIndex = inputInsertVal.length();
		else if (activeInput == INP_DELETE_IDX) cursorIndex = inputDeleteIdx.length();
		
		textScrollX = 0.0f; 
		cursorBlinkTimer = 0.0f;
		cursorVisible = true;
		previousActiveInput = activeInput;
	}

	if (activeInput != INP_NONE) {
		cursorBlinkTimer += deltaTime;
		if (cursorBlinkTimer >= 0.5f) { 
			cursorVisible = !cursorVisible;
			cursorBlinkTimer = 0.0f;
		}

		if (activeInput == INP_CREATE) HandleTextInput(inputCreate, activeInput);
		else if (activeInput == INP_SEARCH) HandleTextInput(inputSearch, activeInput);
		else if (activeInput == INP_INSERT_IDX) HandleTextInput(inputInsertIdx, activeInput);
		else if (activeInput == INP_INSERT_VAL) HandleTextInput(inputInsertVal, activeInput);
		else if (activeInput == INP_DELETE_IDX) HandleTextInput(inputDeleteIdx, activeInput);
		
		if (IsKeyPressed(KEY_ENTER)) {
			try {
				if (activeInput == INP_CREATE && !inputCreate.empty()) {
					insertNode(std::stoi(inputCreate)); 
                    					inputCreate.clear();
				}
				else if (activeInput == INP_SEARCH && !inputSearch.empty()) {
					searchNode(std::stoi(inputSearch));
					inputSearch.clear();
				}
				else if ((activeInput == INP_INSERT_VAL || activeInput == INP_INSERT_IDX) 
						 && !inputInsertIdx.empty() && !inputInsertVal.empty()) {
					insertNodeAtIndex(std::stoi(inputInsertIdx), std::stoi(inputInsertVal));
					inputInsertIdx.clear(); 
					inputInsertVal.clear();
				}
				else if (activeInput == INP_DELETE_IDX && !inputDeleteIdx.empty()) {
					deleteNodeAtIndex(std::stoi(inputDeleteIdx));
					inputDeleteIdx.clear();
				}
			} catch (...) {}
			activeInput = INP_NONE;
		}
	}

	// 5. Node movement animation towards target positions
	float moveSpeed = 8.0f;
	LLNode* currentAnim = head;
	while (currentAnim != nullptr) 
	{
		currentAnim->position.x = Lerp(currentAnim->position.x, currentAnim->targetPosition.x, deltaTime * moveSpeed);
		currentAnim->position.y = Lerp(currentAnim->position.y, currentAnim->targetPosition.y, deltaTime * moveSpeed);
		currentAnim = currentAnim->next;
	}
}

// CÃC HÃ€M Há»– TRá»¢ Váº¼ UI
bool LinkedListState::DrawButtonText(Vector2 pos, const char* text, float width, float height, bool isSelected)
{
	Rectangle bounds = {pos.x, pos.y, width, height};
	bool isHovered = CheckCollisionPointRec(GetMousePosition(), bounds);
	
	Color bgColor = (isHovered || isSelected) ? BLACK : Color{ 102, 191, 255, 255 }; 
	DrawRectangleRec(bounds, bgColor);

	float fontSize = 22.0f;
	Vector2 textSize = MeasureTextEx(listFont, text, fontSize, 1.0f);
	Vector2 textPos = { pos.x + (width - textSize.x) / 2.0f, pos.y + (height - textSize.y) / 2.0f };
	DrawTextEx(listFont, text, textPos, fontSize, 1.0f, WHITE);
	
	return (isHovered && IsMouseButtonReleased(MOUSE_LEFT_BUTTON));
}

bool LinkedListState::DrawTextBox(Vector2 pos, std::string& text, bool isActive, float width, float height)
{
	Rectangle bounds = { pos.x, pos.y, width, height };
	bool isHovered = CheckCollisionPointRec(GetMousePosition(), bounds);

	DrawRectangleRec(bounds, BLACK); 
	DrawRectangleLinesEx(bounds, 2.0f, isActive ? RED : DARKGRAY); 

	float fontSize = 22.0f; 
	float padding = 8.0f;
	float textHeight = MeasureTextEx(numberFont, "0", fontSize, 1.0f).y; 
	float textDrawY = pos.y + (height - textHeight) / 2.0f; 
	
    // TÃ­nh toÃ¡n cuá»™n ngang
    if (isActive) {
		std::string textBeforeCursor = text.substr(0, cursorIndex);
		float cursorOffsetX = MeasureTextEx(numberFont, textBeforeCursor.c_str(), fontSize, 1.0f).x;
		float maxVisibleWidth = width - (padding * 2);

		if (cursorOffsetX - textScrollX > maxVisibleWidth) {
			textScrollX = cursorOffsetX - maxVisibleWidth;
		} 
		else if (cursorOffsetX - textScrollX < 0) {
			textScrollX = cursorOffsetX;
		}
	} else {
		textScrollX = 0; 
	}

    // Váº¼ CHá»® VÃ€ CON TRá»Ž Báº°NG NUMBER FONT
    BeginScissorMode((int)pos.x, (int)pos.y, (int)width, (int)height);
	
	Vector2 textPos = { pos.x + padding - textScrollX, textDrawY };
	DrawTextEx(numberFont, text.c_str(), textPos, fontSize, 1.0f, WHITE);

	if (isActive && cursorVisible) {
		std::string textBeforeCursor = text.substr(0, cursorIndex);
		float cursorX = pos.x + padding - textScrollX + MeasureTextEx(numberFont, textBeforeCursor.c_str(), fontSize, 1.0f).x;
		DrawLineEx({cursorX, textDrawY}, {cursorX, textDrawY + textHeight}, 2.0f, WHITE);
	}

	EndScissorMode();

    // KHÃ”I PHá»¤C SCISSOR MODE Cá»¦A MENU CHÃNH
    	float startX = controlBtnPos.x + controlTex.width + 15.0f;
	BeginScissorMode((int)startX, 0, GetScreenWidth(), GetScreenHeight());

	if (isHovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) return true; 
	return false;
}


void LinkedListState::DrawLabel(Vector2 pos, const char* text)
{
	float fontSize = 22.0f;
	DrawTextEx(listFont, text, {pos.x, pos.y + 10.0f}, fontSize, 1.0f, BLACK);
}

void LinkedListState::draw()
{
	// Draw base UI (background, home button, speed slider, error messages)
	DataStructureState::drawSharedUI();

	const char* titleText = "SINGLY LINKED LIST";
	DrawTextEx(listFont, titleText, { (1800.0f - MeasureTextEx(listFont, titleText, 55, 6.5f).x) / 2.0f, 20.0f }, 55, 6.5f, BLACK);

	// Draw the linked list
	LLNode* curr = head;
	while (curr != nullptr) 
	{
		if (curr->next != nullptr) 
		{
			Vector2 startPos = { curr->position.x + nodeRadius, curr->position.y };
			Vector2 endPos = { curr->next->position.x - nodeRadius, curr->next->position.y };
			DrawLineEx(startPos, endPos, 3.0f, DARKGRAY);
			
			float arrowSize = 12.0f;
			DrawTriangle(endPos, 
						 { endPos.x - arrowSize, endPos.y - arrowSize * 0.7f },
						 { endPos.x - arrowSize, endPos.y + arrowSize * 0.7f }, 
						 DARKGRAY);
		}

		DrawCircleV(curr->position, nodeRadius, curr->color);
		DrawCircleLines(curr->position.x, curr->position.y, nodeRadius, DARKBLUE);
		DrawTextEx(numberFont, TextFormat("%d", curr->value), { curr->position.x - 12, curr->position.y - 11 }, 25, 1, WHITE);
		curr = curr->next;
	}

	// Draw Control Panel
	Vector2 mousePos = GetMousePosition();
	Rectangle controlBtnBounds = { controlBtnPos.x, controlBtnPos.y, (float)controlTex.width, (float)controlTex.height };
	
	// Kiá»ƒm tra náº¿u chuá»™t náº±m trong khu vá»±c nÃºt thÃ¬ lÃ m má» 30% (cÃ²n 0.7f), ngÆ°á»£c láº¡i giá»¯ nguyÃªn
	Color controlColor = CheckCollisionPointRec(mousePos, controlBtnBounds) ? Fade(WHITE, 0.7f) : WHITE;
	
	DrawTextureV(controlTex, controlBtnPos, controlColor);
	if (panelAnimProgress > 0.0f) 
	{
		float easedProgress = sin(panelAnimProgress * PI / 2.0f); 

		float mainItemWidth = 125.0f; 
		float mainItemHeight = 45.0f;
		float gap = 8.0f; 

		float startX = controlBtnPos.x + controlTex.width + 15.0f;
		float startY = controlBtnPos.y; 
		
		BeginScissorMode((int)startX, 0, GetScreenWidth(), GetScreenHeight());
		float panelX = startX - mainItemWidth * (1.0f - easedProgress);

		const char* mainItems[] = {"Create", "Insert", "Search", "Delete"};
		ActiveSubPanel itemSubPanels[] = {SUB_CREATE, SUB_INSERT, SUB_SEARCH, SUB_DELETE};

		for (int i = 0; i < 4; i++) {
			float itemY = startY + i * (mainItemHeight + gap);
			bool isSelected = (activeSubPanel == itemSubPanels[i]);

			bool isClicked = DrawButtonText({panelX, itemY}, mainItems[i], mainItemWidth, mainItemHeight, isSelected);

			if (isClicked && panelAnimProgress >= 1.0f) {
				if (activeSubPanel == itemSubPanels[i]) activeSubPanel = SUB_NONE; 
				else { activeSubPanel = itemSubPanels[i]; isCreateUserDefOpen = false; }
			}
		}

		if (activeSubPanel != SUB_NONE) 
		{
			float subX = panelX + mainItemWidth + gap; 
			float labelFontSize = 22.0f; 

			if (activeSubPanel == SUB_CREATE) {
				float sy = startY + 0 * (mainItemHeight + gap); 
				float cx = subX;

				if (DrawButtonText({cx, sy}, "Empty", 90, mainItemHeight)) { 
					clearList();
				}
				cx += 90 + gap;

				if (DrawButtonText({cx, sy}, "User Defined List", 230, mainItemHeight, isCreateUserDefOpen)) {
					isCreateUserDefOpen = !isCreateUserDefOpen; 
				}
				cx += 230 + gap;

				if (DrawButtonText({cx, sy}, "Random", 110, mainItemHeight)) { 
					clearList();
					int NumNode = GetRandomValue(1, 9);
					for(int i = 0; i < NumNode; i++) {
						insertNode(GetRandomValue(1, 99));
					}
				}

				if (isCreateUserDefOpen) {
					float ud_sx = subX + 90 + gap;
					float ud_sy = sy + mainItemHeight + gap;

					if (DrawTextBox({ud_sx, ud_sy}, inputCreate, activeInput == INP_CREATE, 230, mainItemHeight)) {
						activeInput = INP_CREATE;
					}
					ud_sx += 230 + gap;
					if (DrawButtonText({ud_sx, ud_sy}, "GO", 50, mainItemHeight)) {
						if (!inputCreate.empty()) {
							clearList();                           
							std::string temp = "";
							for (char c : inputCreate) {
								if (c == ',') {
									if (!temp.empty()) {
										insertNode(std::stoi(temp));
										temp = "";
									}
								} else {
									temp += c;
								}
							}
							if (!temp.empty()) insertNode(std::stoi(temp));
							inputCreate.clear();
							activeInput = INP_NONE;
							isCreateUserDefOpen = false;
						}
					}
				}
			}
			else if (activeSubPanel == SUB_INSERT) {
				float sy = startY + 1 * (mainItemHeight + gap); 
				float cx = subX;

				DrawLabel({cx, sy}, "Index =");
				cx += MeasureTextEx(listFont, "Index =", labelFontSize, 1.0f).x + gap;

				if (DrawTextBox({cx, sy}, inputInsertIdx, activeInput == INP_INSERT_IDX, 100, mainItemHeight)) activeInput = INP_INSERT_IDX;
				cx += 100 + gap;

				DrawLabel({cx, sy}, "Value =");
				cx += MeasureTextEx(listFont, "Value =", labelFontSize, 1.0f).x + gap;

				if (DrawTextBox({cx, sy}, inputInsertVal, activeInput == INP_INSERT_VAL, 100, mainItemHeight)) activeInput = INP_INSERT_VAL;
				cx += 100 + gap;

				if (DrawButtonText({cx, sy}, "GO", 50, mainItemHeight)) {
					if (!inputInsertIdx.empty() && !inputInsertVal.empty()) {
						try { insertNodeAtIndex(std::stoi(inputInsertIdx), std::stoi(inputInsertVal)); inputInsertIdx.clear(); inputInsertVal.clear(); activeInput = INP_NONE; } catch (...) {}
					}
				}
			}
			else if (activeSubPanel == SUB_SEARCH) {
				float sy = startY + 2 * (mainItemHeight + gap); 
				float cx = subX;

				DrawLabel({cx, sy}, "Value =");
				cx += MeasureTextEx(listFont, "Value =", labelFontSize, 1.0f).x + gap;

				if (DrawTextBox({cx, sy}, inputSearch, activeInput == INP_SEARCH, 120, mainItemHeight)) activeInput = INP_SEARCH;
				cx += 120 + gap;

				if (DrawButtonText({cx, sy}, "GO", 50, mainItemHeight)) {
					if (!inputSearch.empty()) {
						try { searchNode(std::stoi(inputSearch)); inputSearch.clear(); activeInput = INP_NONE; } catch (...) {}
					}
				}
			}
			else if (activeSubPanel == SUB_DELETE) {
				float sy = startY + 3 * (mainItemHeight + gap); 
				float cx = subX;

				DrawLabel({cx, sy}, "Index =");
				cx += MeasureTextEx(listFont, "Index =", labelFontSize, 1.0f).x + gap;

				if (DrawTextBox({cx, sy}, inputDeleteIdx, activeInput == INP_DELETE_IDX, 120, mainItemHeight)) activeInput = INP_DELETE_IDX;
				cx += 120 + gap;

				if (DrawButtonText({cx, sy}, "GO", 50, mainItemHeight)) {
					if (!inputDeleteIdx.empty()) {
						try { deleteNodeAtIndex(std::stoi(inputDeleteIdx)); inputDeleteIdx.clear(); activeInput = INP_NONE; } catch (...) {}
					}
				}
			}
		}

		// Draw input error message if exists
		if (!inputErrorMsg.empty() && inputErrorTimer > 0.0f) {
			DrawTextEx(numberFont, inputErrorMsg.c_str(), { panelX + mainItemWidth + gap + 90.0f, startY + 200.0f }, 18.0f, 1.0f, RED);
		}

		EndScissorMode();    
	}
}

// Linked list operations

void LinkedListState::searchNode(int value)
{
	if (head == nullptr) {
		inputErrorMsg = "List is empty!";
		inputErrorTimer = 2.5f;
		return;
	}
	resetNodeColors();
	searchTargetValue = value;
	searchPointer = head;
	searchPointer->color = YELLOW;
	isAnimating = true;
	currentTask = TASK_SEARCH;
	animTimer = 0.0f;
}

void LinkedListState::deleteNodeAtIndex(int index)
{
	if (head == nullptr) {
		inputErrorMsg = "List is empty!";
		inputErrorTimer = 2.5f;
		return;
	}
	if (index < 0) {
		inputErrorMsg = "Invalid index!";
		inputErrorTimer = 2.5f;
		return;
	}
	resetNodeColors();

	if (index == 0) {
		LLNode* temp = head;
		head = head->next;
		delete temp;
		updateTargetPositions();
	} else {
		searchTargetIndex = index;
		searchCurrentIndex = 0;
		searchPointer = head;
		searchPointer->color = YELLOW;
		isAnimating = true;
		currentTask = TASK_DELETE_INDEX;
		animTimer = 0.0f;
	}
}

// CÃC HÃ€M Xá»¬ LÃ LINKED LIST 
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
	if (index < 0) {
		inputErrorMsg = "Invalid index!";
		inputErrorTimer = 2.5f;
		return;
	}
	resetNodeColors();

	if (index == 0 || head == nullptr) {
		LLNode* newNode = new LLNode{ value, {startX, startY - 200.0f}, {0,0}, head, SKYBLUE };
		head = newNode;
		updateTargetPositions();
	} else {
		searchTargetIndex = index;
		searchTargetValue = value;
		searchCurrentIndex = 0;
		searchPointer = head;
		searchPointer->color = YELLOW;
		isAnimating = true;
		currentTask = TASK_INSERT_INDEX;
		animTimer = 0.0f;
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

	while (current != nullptr) {
		current->targetPosition = { currentX, startY };
		currentX += nodeSpacing;
		current = current->next;
	}
}