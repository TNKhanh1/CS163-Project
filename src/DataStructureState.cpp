#include "DataStructureState.h"
#include "raymath.h"
#include <cmath>

DataStructureState::DataStructureState()
{
	bg = nullptr;
	homeBtn = nullptr;
	NextState = -1;
	activeMainOp = OP_NONE; // Initialize to no active operation

	inputErrorMsg = "";
	inputErrorTimer = 0.0f;
	currentErrorSlot = 0;

	speedSliderBounds = { 1500.0f, 870.0f, 200.0f, 10.0f };
	animSpeedMultiplier = 1.0f;
	isDraggingSpeedSlider = false;

	zoomSliderBounds = { 1500.0f, 940.0f, 200.0f, 10.0f };
	zoomMultiplier = 1.0f;
	isDraggingZoomSlider = false;
	
	animTimer = 0.0f;
	isAnimating = false;
	isAnimFinished = false;

	isAutoPlay = true;
	stepForwardRequested = false;
	stepBackwardRequested = false;
	controlBtnPos = { 30.0f, 750.0f };
	isDraggingControlBtn = false;
	isClickingControlBtn = false;
	isPanelOpen = false;
	panelAnimProgress = 0.0f;
	dragOffset = { 0, 0 };

	cursorIndex = 0;
	cursorBlinkTimer = 0.0f;
	textScrollX = 0.0f;
	cursorVisible = true;
	clearInputBuffers();
}

DataStructureState::~DataStructureState()
{
	if (bg)
	{
		delete bg;
	}
	if (homeBtn)
	{
		delete homeBtn;
	}
	UnloadFont(listFont);
	UnloadFont(numberFont);
	UnloadTexture(controlTex);
}

void DataStructureState::loadAssets()
{
	bg = new back_ground("assets/background01.jpg", {0.0f, 0.0f}, "assets/background01.jpg", false);
	homeBtn = new button("assets/home.png", "assets/home.png", "assets/home.png", "assets/home.png", {10.0f, 10.0f}, 80.0);

	listFont = LoadFontEx("assets/FONT2.ttf", 50, 0, 0);
	SetTextureFilter(listFont.texture, TEXTURE_FILTER_BILINEAR);

	numberFont = LoadFontEx("assets/FONT3.ttf", 22, 0, 0);
	SetTextureFilter(numberFont.texture, TEXTURE_FILTER_BILINEAR);

	Image ctrlImg = LoadImage("assets/control.png");
	ImageResize(&ctrlImg, 75, 75);
	controlTex = LoadTextureFromImage(ctrlImg);
	UnloadImage(ctrlImg);
}

void DataStructureState::updateSharedUI(float deltaTime, Vector2 mousePos)
{
	if (homeBtn != nullptr && homeBtn->isPressed(mousePos, IsMouseButtonPressed(MOUSE_LEFT_BUTTON)))
	{
		NextState = 0;
		return;
	}

	// 1. Centralized Animation & Undo Trigger
	
	if (!overridesStepHandling)
	{
		if (isAutoPlay)
		{
			// AUTO MODE: Progress animation automatically based on timer
			if (isAnimating && !isAnimFinished && CheckStepReady(deltaTime, 0.7f))
			{
				saveState();
				handleAnimationStep();
			}
		}
		else 
		{
			// MANUAL MODE
			if (stepForwardRequested)
			{
				// Only allow forward steps if an animation is actually running
				if (isAnimating && !isAnimFinished)
				{
					saveState();
					handleAnimationStep();
				}
				stepForwardRequested = false;
			}
			
			if (stepBackwardRequested)
			{
				// You can always step backwards as long as there's history, even if not currently animating
				undoState();
				stepBackwardRequested = false;
			}
		}
	}

	// 2. Centralized ENTER Key Logic
	if (IsKeyPressed(KEY_ENTER) && activeMainOp != OP_NONE)
	{
		onExecuteOp(activeMainOp);
	}

	// 3. Error Message Timer
	if (inputErrorTimer > 0.0f)
	{
		inputErrorTimer -= deltaTime;
		if (inputErrorTimer <= 0.0f)
		{
			inputErrorMsg = "";
		}
	}

	// 4. Speed Slider Logic
	Rectangle speedSliderHitBox = { speedSliderBounds.x, speedSliderBounds.y - 15.0f, speedSliderBounds.width, 40.0f };
	if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mousePos, speedSliderHitBox))
	{
		isDraggingSpeedSlider = true;
	}
	if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
	{
		isDraggingSpeedSlider = false;
	}

	if (isDraggingSpeedSlider)
	{
		float progress = Clamp((mousePos.x - speedSliderBounds.x) / speedSliderBounds.width, 0.0f, 1.0f);
		animSpeedMultiplier = 0.2f + (progress * 3.8f);
	}

	// 5. Zoom Slider Logic
	Rectangle zoomHitBox = { zoomSliderBounds.x, zoomSliderBounds.y - 15.0f, zoomSliderBounds.width, 40.0f };
	if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mousePos, zoomHitBox))
	{
		isDraggingZoomSlider = true;
	}
	if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
	{
		isDraggingZoomSlider = false;
	}

	if (isDraggingZoomSlider)
	{
		float progress = Clamp((mousePos.x - zoomSliderBounds.x) / zoomSliderBounds.width, 0.0f, 1.0f);
		zoomMultiplier = 0.2f + (progress * 1.8f);
	}

	if (isWaitingForDrop) {
        if (IsFileDropped()) {
            FilePathList droppedFiles = LoadDroppedFiles();
            if (droppedFiles.count > 0) {
            
                if (processDroppedFile(droppedFiles.paths[0])) {
                    inputErrorMsg = "SUCCESS: FILE LOADED";
                    isWaitingForDrop = false; 
                }
            }
            UnloadDroppedFiles(droppedFiles);
        }
    }
}

void DataStructureState::DrawSideMenuFrame(const std::vector<std::string>& labels)
{
	if (panelAnimProgress <= 0.0f)
	{
		return;
	}

	float easedProgress = sin(panelAnimProgress * PI / 2.0f);
	float mainWidth = 125.0f, mainHeight = 45.0f, gap = 8.0f;
	float startX = controlBtnPos.x + (float)controlTex.width + 15.0f;
	float startY = controlBtnPos.y;
	
	BeginScissorMode((int)startX, 0, GetScreenWidth(), GetScreenHeight()); // Reveal the menu sliding from behind the gear
	float panelX = startX - mainWidth * (1.0f - easedProgress);

	// DYNAMIC LOOP: Draw as many buttons as the child requested
	for (size_t i = 0; i < labels.size(); i++)
	{
		float itemY = startY + (float)i * (mainHeight + gap);
		bool isSelected = ((int)activeMainOp == (int)i + 1);

		if (DrawButtonText({panelX, itemY}, labels[i].c_str(), mainWidth, mainHeight, isSelected))
		{
			// Toggle the operation slot
			if (activeMainOp == (MainOp)(i + 1))
			{
				activeMainOp = OP_NONE;
			}
			else
			{
				activeMainOp = (MainOp)(i + 1);
			}

			inputErrorMsg = "";
			inputErrorTimer = 0.0f;
		}
	}

	// If a slot is active, let the child fill in the unique textboxes/labels
	if (activeMainOp != OP_NONE)
	{
		DrawSubMenuContent();
	}

	EndScissorMode();
}

void DataStructureState::drawSharedUI()
{
	Vector2 mousePos = GetMousePosition();
	if (bg != nullptr)
	{
		bg->Draw(mousePos);
	}
	if (homeBtn != nullptr)
	{
		homeBtn->Draw(mousePos);
	}

	// Render animation speed slider
	DrawTextEx(listFont, "Animation Speed", { speedSliderBounds.x, speedSliderBounds.y - 30.0f }, 22.0f, 1.0f, BLACK);
	DrawRectangleRounded(speedSliderBounds, 1.0f, 8, LIGHTGRAY);
	float fillWidth = ((animSpeedMultiplier - 0.2f) / 3.8f) * speedSliderBounds.width;
	DrawRectangleRounded({ speedSliderBounds.x, speedSliderBounds.y, fillWidth, speedSliderBounds.height }, 1.0f, 8, SKYBLUE);
	DrawCircle((int)(speedSliderBounds.x + fillWidth), (int)(speedSliderBounds.y + speedSliderBounds.height / 2.0f), 12.0f, DARKBLUE);
	DrawTextEx(numberFont, TextFormat("%.1fx", animSpeedMultiplier), { speedSliderBounds.x + speedSliderBounds.width + 15.0f, speedSliderBounds.y - 6.0f }, 22.0f, 1.0f, BLACK);

	// Render zoom slider
	DrawTextEx(listFont, "Zoom", { zoomSliderBounds.x, zoomSliderBounds.y - 30.0f }, 22.0f, 1.0f, BLACK);
	DrawRectangleRounded(zoomSliderBounds, 1.0f, 8, LIGHTGRAY);
	float zoomFillWidth = ((zoomMultiplier - 0.2f) / 1.8f) * zoomSliderBounds.width;
	DrawRectangleRounded({ zoomSliderBounds.x, zoomSliderBounds.y, zoomFillWidth, zoomSliderBounds.height }, 1.0f, 8, SKYBLUE);
	DrawCircle((int)(zoomSliderBounds.x + zoomFillWidth), (int)(zoomSliderBounds.y + zoomSliderBounds.height / 2.0f), 12.0f, DARKBLUE);
	DrawTextEx(numberFont, TextFormat("%d%%", (int)(zoomMultiplier * 100)), { zoomSliderBounds.x + zoomSliderBounds.width + 15.0f, zoomSliderBounds.y - 6.0f }, 22.0f, 1.0f, BLACK);

	// Step-by-step control buttons
	float buttonWidth = 150.0f;
	float stepBtnWidth = 100.0f;
	float gap = 15.0f;
	
	// Position the mode toggle to the left of the Zoom slider
	float modeToggleX = zoomSliderBounds.x - buttonWidth - gap;
	float nextBtnX = modeToggleX - stepBtnWidth - gap;
	float backBtnX = nextBtnX - stepBtnWidth - gap;
	
	// Center vertically relative to the zoom slider height
	float stepY = zoomSliderBounds.y - 12.0f;
	
	// 1. Mode Toggle Button
	const char* modeLabel = isAutoPlay ? "AUTO" : "MANUAL";
	if (DrawButtonText({modeToggleX, stepY}, modeLabel, buttonWidth, 35, !isAutoPlay))
	{
		isAutoPlay = !isAutoPlay;
	}
	
	// 2. Next & Back Buttons (Only visible during Manual mode + Animating)
	if (!isAutoPlay)
	{
		// NEXT Button: Only shows up if there is an active animation
		if (DrawButtonText({nextBtnX, stepY}, "Next >>", stepBtnWidth, 35, false))
		{
			if (hasNextStep())
			{
				stepForwardRequested = true;
			}
		}
		// BACK Button: Only shows up if there is history to go back to
		if (DrawButtonText({backBtnX, stepY}, "<< Back", stepBtnWidth, 35, false))
		{
			if (hasPrevStep())
			{
				stepBackwardRequested = true;
			}
		}
	}

	// Render active error message if exists
	if (!inputErrorMsg.empty() && inputErrorTimer > 0.0f)
	{
		float menuStartX = controlBtnPos.x + 75.0f + 15.0f;
		float errorY = controlBtnPos.y + ((currentErrorSlot) * (45.0f + 8.0f)) + 50.0f;
		DrawTextEx(numberFont, inputErrorMsg.c_str(), { menuStartX + 135.0f, errorY }, 24.0f, 1.0f, RED);
	}

	Vector2 importBtnPos = { 110.0f, 30.0f }; 
    if (DrawButtonText(importBtnPos, "Import .txt File", 180, 40, isWaitingForDrop)) {
        isWaitingForDrop = !isWaitingForDrop;
    }
}

void DataStructureState::updateControlPanel(float deltaTime, Vector2 mousePos)
{
	Rectangle bounds = { controlBtnPos.x, controlBtnPos.y, (float)controlTex.width, (float)controlTex.height };
	
	if (CheckCollisionPointRec(mousePos, bounds) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
	{
		isDraggingControlBtn = true;
		isClickingControlBtn = true;
		dragOffset = { mousePos.x - controlBtnPos.x, mousePos.y - controlBtnPos.y };
	}

	if (isDraggingControlBtn)
	{
		if (isClickingControlBtn && Vector2Distance(mousePos, {controlBtnPos.x + dragOffset.x, controlBtnPos.y + dragOffset.y}) > 3.0f)
		{
			isClickingControlBtn = false;
		}
		controlBtnPos.x = mousePos.x - dragOffset.x;
		controlBtnPos.y = mousePos.y - dragOffset.y;
	}

	if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
	{
		if (isDraggingControlBtn)
		{
			isDraggingControlBtn = false;
			if (isClickingControlBtn && CheckCollisionPointRec(mousePos, bounds))
			{
				isPanelOpen = !isPanelOpen;
			}
		}
		isClickingControlBtn = false;
	}

	float animSpeed = 4.0f;
	if (isPanelOpen)
	{
		panelAnimProgress = fminf(1.0f, panelAnimProgress + deltaTime * animSpeed);
	}
	else
	{
		panelAnimProgress = fmaxf(0.0f, panelAnimProgress - deltaTime * animSpeed);
	}
}

bool DataStructureState::CheckStepReady(float deltaTime, float stepDuration)
{
	animTimer += deltaTime * animSpeedMultiplier;
	if (animTimer >= stepDuration)
	{
		animTimer = 0.0f;
		return true;
	}
	return false;
}

void DataStructureState::HandleTextInput(std::string& text, bool isCreateInput)
{
	if ((IsKeyPressed(KEY_LEFT) || IsKeyPressedRepeat(KEY_LEFT)) && cursorIndex > 0)
	{
		cursorIndex--;
	}
	if ((IsKeyPressed(KEY_RIGHT) || IsKeyPressedRepeat(KEY_RIGHT)) && cursorIndex < (int)text.length())
	{
		cursorIndex++;
	}

	if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE))
	{
		if (cursorIndex > 0)
		{
			text.erase(cursorIndex - 1, 1);
			cursorIndex--;
		}
	}
	if (IsKeyPressed(KEY_DELETE) || IsKeyPressedRepeat(KEY_DELETE))
	{
		if (cursorIndex < (int)text.length())
		{
			text.erase(cursorIndex, 1);
		}
	}

	int keyCode = GetKeyPressed();
	while (keyCode > 0)
	{
		char charToAdd = 0;
		if (keyCode >= KEY_ZERO && keyCode <= KEY_NINE)
		{
			charToAdd = '0' + (keyCode - KEY_ZERO);
		}
		else if (keyCode >= KEY_KP_0 && keyCode <= KEY_KP_9)
		{
			charToAdd = '0' + (keyCode - KEY_KP_0);
		}
		else if (keyCode == KEY_MINUS || keyCode == KEY_KP_SUBTRACT)
		{
			charToAdd = '-';
		}
		else if (keyCode == KEY_COMMA && isCreateInput)
		{
			charToAdd = ',';
		}
		
		if (charToAdd != 0)
		{
			std::string temp = text;
			temp.insert(cursorIndex, 1, charToAdd);
			if (IsValidInputString(temp, isCreateInput))
			{
				text = temp;
				cursorIndex++;
			}
		}
		keyCode = GetKeyPressed();
	}
}

bool DataStructureState::IsValidInputString(const std::string& text, bool isCreateInput)
{
	if (isCreateInput && text.length() > 30)
	{
		return false;
	}
	if (!text.empty() && text[0] == ',')
	{
		return false;
	}

	int currentDigitCount = 0;
	for (size_t i = 0; i < text.length(); i++)
	{
		char c = text[i];
		if (c == '-')
		{
			if (i != 0 && text[i-1] != ',')
			{
				return false;
			}
		}
		else if (c == ',')
		{
			if (!isCreateInput)
			{
				return false;
			}
			if (i > 0 && text[i-1] == ',')
			{
				return false;
			}
			currentDigitCount = 0;
		}
		else if (c >= '0' && c <= '9')
		{
			currentDigitCount++;
			if (currentDigitCount > 4)
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}
	return true;
}

bool DataStructureState::DrawButtonText(Vector2 pos, const char* text, float width, float height, bool isSelected)
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

bool DataStructureState::DrawTextBox(Vector2 pos, std::string& text, bool isActive, float width, float height, int cursorIdx, float& scrollX, bool cursorVis)
{
	Rectangle bounds = { pos.x, pos.y, width, height };
	bool isHovered = CheckCollisionPointRec(GetMousePosition(), bounds);

	DrawRectangleRec(bounds, BLACK);
	DrawRectangleLinesEx(bounds, 2.0f, isActive ? RED : DARKGRAY);

	float fontSize = 22.0f;
	float padding = 8.0f;
	float textHeight = MeasureTextEx(numberFont, "0", fontSize, 1.0f).y;
	float textDrawY = pos.y + (height - textHeight) / 2.0f;
	
	if (isActive)
	{
		std::string textBeforeCursor = text.substr(0, cursorIdx);
		float cursorOffsetX = MeasureTextEx(numberFont, textBeforeCursor.c_str(), fontSize, 1.0f).x;
		float maxVisibleWidth = width - (padding * 2);

		if (cursorOffsetX - scrollX > maxVisibleWidth)
		{
			scrollX = cursorOffsetX - maxVisibleWidth;
		}
		else if (cursorOffsetX - scrollX < 0)
		{
			scrollX = cursorOffsetX;
		}
	}
	else
	{
		scrollX = 0;
	}

	BeginScissorMode((int)pos.x, (int)pos.y, (int)width, (int)height);
	
	Vector2 textPos = { pos.x + padding - scrollX, textDrawY };
	DrawTextEx(numberFont, text.c_str(), textPos, fontSize, 1.0f, WHITE);

	if (isActive && cursorVis)
	{
		std::string textBeforeCursor = text.substr(0, cursorIdx);
		float cursorX = pos.x + padding - scrollX + MeasureTextEx(numberFont, textBeforeCursor.c_str(), fontSize, 1.0f).x;
		DrawLineEx({cursorX, textDrawY}, {cursorX, textDrawY + textHeight}, 2.0f, WHITE);
	}

	EndScissorMode();

	if (isHovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
	{
		return true;
	}
	return false;
}

void DataStructureState::DrawLabel(Vector2 pos, const char* text)
{
	float fontSize = 22.0f;
	DrawTextEx(listFont, text, {pos.x, pos.y + 10.0f}, fontSize, 1.0f, BLACK);
}

void DataStructureState::drawPseudoCode()
{
    if (pseudoCode.empty()) return; // Don't draw if there's no code

    float pcX = 1315.0f, pcY = 150.0f;
    float pcWidth = 450.0f, pcHeight = 450.0f;
    
    // Draw background for pseudocode
    DrawRectangle(pcX - 10, pcY - 10, pcWidth + 40, pcHeight, Fade(LIGHTGRAY, 0.6f));
    DrawRectangleLines(pcX - 10, pcY - 10, pcWidth + 40, pcHeight, DARKGRAY);
    DrawTextEx(listFont, "Source Code:", {pcX, pcY}, 25.0f, 1.0f, DARKBLUE);
    
    // Draw each line of pseudocode with active line highlighting
    float lineHeight = 28.0f;
    float textPadding = 15.0f;
    for (int i = 0; i < (int)pseudoCode.size(); i++) {
        Color textCol = BLACK;
        
        // Highlight active line if it matches the current step
        if (i == activeCodeLine) {
            DrawRectangle(pcX, pcY + 40.0f + i * lineHeight - 2.0f, pcWidth, lineHeight - 2.0f, Fade(YELLOW, 0.5f));
            textCol = RED;
        }
        
        std::string line = pseudoCode[i];
        
        // Simple line wrapping: If the line is too long, split it into two lines at the last space before the cutoff
        if (MeasureTextEx(numberFont, line.c_str(), 18.0f, 1.0f).x > 520.0f) {
            size_t spacePos = line.find_last_of(' ', 60);
            if (spacePos != std::string::npos) {
                std::string first = line.substr(0, spacePos);
                std::string second = line.substr(spacePos + 1);
                DrawTextEx(numberFont, first.c_str(), {pcX + textPadding, pcY + 40.0f + i * lineHeight}, 18.0f, 1.0f, textCol);
                DrawTextEx(numberFont, second.c_str(), {pcX + textPadding, pcY + 40.0f + (i + 0.5f) * lineHeight}, 18.0f, 1.0f, textCol);
            } else {
                DrawTextEx(numberFont, line.c_str(), {pcX + textPadding, pcY + 40.0f + i * lineHeight}, 18.0f, 1.0f, textCol);
            }
        } else {
            DrawTextEx(numberFont, line.c_str(), {pcX + textPadding, pcY + 40.0f + i * lineHeight}, 18.0f, 1.0f, textCol);
        }
    }
}


void DataStructureState::drawDropZone()
{
	// Paste the extracted logic here!
	if (isWaitingForDrop) {
		float rectWidth = 600.0f;
		float rectHeight = 400.0f;
		Vector2 screenCenter = { (float)GetScreenWidth() / 2.0f, (float)GetScreenHeight() / 2.0f };
		Rectangle dropZone = { 
			screenCenter.x - rectWidth / 2.0f, 
			screenCenter.y - rectHeight / 2.0f, 
			rectWidth, 
			rectHeight 
		};

		DrawRectangleRec(dropZone, Fade(BLACK, 0.85f)); 
		DrawRectangleLinesEx(dropZone, 3.0f, RAYWHITE); 

		Rectangle closeBtn = { dropZone.x + dropZone.width - 45, dropZone.y + 10, 35, 35 };
		bool hoverX = CheckCollisionPointRec(GetMousePosition(), closeBtn);
		DrawRectangleRec(closeBtn, hoverX ? RED : MAROON);
		
		DrawTextEx(numberFont, "X", { closeBtn.x + 10, closeBtn.y + 5 }, 25, 2, WHITE);

		const char* msg = "DROP .TXT FILE HERE";
		float fontSize = 40.0f;
		float spacing = 2.0f;
		
		Vector2 textSize = MeasureTextEx(numberFont, msg, fontSize, spacing);
		Vector2 textPos = { screenCenter.x - textSize.x / 2, screenCenter.y - 30 };
		DrawTextEx(numberFont, msg, textPos, fontSize, spacing, RAYWHITE);

		const char* subMsg = "Integers only - Max 20 nodes";
		float subFontSize = 20.0f;
		Vector2 subSize = MeasureTextEx(numberFont, subMsg, subFontSize, 1);
		DrawTextEx(numberFont, subMsg, { screenCenter.x - subSize.x / 2, screenCenter.y + 30 }, subFontSize, 1, LIGHTGRAY);

		if (hoverX && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
			isWaitingForDrop = false;
		}
	}
}
void DataStructureState::update(float deltaTime)
{

}

void DataStructureState::draw()
{

}