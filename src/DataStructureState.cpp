#include "DataStructureState.h"
#include "raymath.h"
#include <cmath>

DataStructureState::DataStructureState()
{
	bg = nullptr;
	homeBtn = nullptr;
	NextState = -1; 

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
	if (bg) delete bg;
	if (homeBtn) delete homeBtn;
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

	// Load and prepare control button texture
	Image ctrlImg = LoadImage("assets/control.png");
	ImageResize(&ctrlImg, 75, 75); 
	controlTex = LoadTextureFromImage(ctrlImg);
	UnloadImage(ctrlImg);
}

void DataStructureState::update(float deltaTime) {}
void DataStructureState::draw() {}

// Shared control panel logic and shared UI drawing tools
void DataStructureState::updateControlPanel(float deltaTime, Vector2 mousePos)
{
	Rectangle bounds = { controlBtnPos.x, controlBtnPos.y, (float)controlTex.width, (float)controlTex.height };
	
	if (CheckCollisionPointRec(mousePos, bounds) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
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
			if (isClickingControlBtn && CheckCollisionPointRec(mousePos, bounds)) {
				isPanelOpen = !isPanelOpen; 
			}
		}
		isClickingControlBtn = false; 
	}

	float animSpeed = 4.0f; 
	if (isPanelOpen) {
		panelAnimProgress = fminf(1.0f, panelAnimProgress + deltaTime * animSpeed);
	} else {
		panelAnimProgress = fmaxf(0.0f, panelAnimProgress - deltaTime * animSpeed);
	}
}

// Shared input validation for text boxes
bool DataStructureState::IsValidInputString(const std::string& text, bool isCreateInput)
{
	if (isCreateInput && text.length() > 30) return false;
	if (!text.empty() && text[0] == ',') return false;

	int currentDigitCount = 0;
	for (size_t i = 0; i < text.length(); i++) {
		char c = text[i];
		if (c == '-') {
			if (i != 0 && text[i-1] != ',') return false;
		}
		else if (c == ',') {
			if (!isCreateInput) return false;
			if (i > 0 && text[i-1] == ',') return false;
			currentDigitCount = 0; 
		}
		else if (c >= '0' && c <= '9') {
			currentDigitCount++;
			if (currentDigitCount > 4) return false; 
		}
		else return false; 
	}
	return true;
}

// Shared text input handling for text boxes
void DataStructureState::HandleTextInput(std::string& text, bool isCreateInput)
{
	if ((IsKeyPressed(KEY_LEFT) || IsKeyPressedRepeat(KEY_LEFT)) && cursorIndex > 0) cursorIndex--;
	if ((IsKeyPressed(KEY_RIGHT) || IsKeyPressedRepeat(KEY_RIGHT)) && cursorIndex < (int)text.length()) cursorIndex++;

	if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) {
		if (cursorIndex > 0) {
			text.erase(cursorIndex - 1, 1);
			cursorIndex--;
		}
	}
	if (IsKeyPressed(KEY_DELETE) || IsKeyPressedRepeat(KEY_DELETE)) {
		if (cursorIndex < (int)text.length()) text.erase(cursorIndex, 1);
	}

	int keyCode = GetKeyPressed();
	while (keyCode > 0) {
		char charToAdd = 0;
		if (keyCode >= KEY_ZERO && keyCode <= KEY_NINE) charToAdd = '0' + (keyCode - KEY_ZERO);
		else if (keyCode >= KEY_KP_0 && keyCode <= KEY_KP_9) charToAdd = '0' + (keyCode - KEY_KP_0);
		else if (keyCode == KEY_MINUS || keyCode == KEY_KP_SUBTRACT) charToAdd = '-';
		else if (keyCode == KEY_COMMA && isCreateInput) charToAdd = ',';
		
		if (charToAdd != 0) {
			std::string temp = text;
			temp.insert(cursorIndex, 1, charToAdd);
			if (IsValidInputString(temp, isCreateInput)) {
				text = temp;
				cursorIndex++;
			}
		}
		keyCode = GetKeyPressed();
	}
}

void DataStructureState::updateSharedUI(float deltaTime, Vector2 mousePos)
{
	if (homeBtn != nullptr && homeBtn->isPressed(mousePos, IsMouseButtonPressed(MOUSE_LEFT_BUTTON))) {
		NextState = 0;
		return;
	}

	if (isAnimating && CheckStepReady(deltaTime, 0.7f)) {
		handleAnimationStep(); 
	}

	// Sliders logic
	Rectangle speedSliderHitBox = { speedSliderBounds.x, speedSliderBounds.y - 15.0f, speedSliderBounds.width, 40.0f };
	if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mousePos, speedSliderHitBox)) isDraggingSpeedSlider = true;
	if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) isDraggingSpeedSlider = false;

	if (isDraggingSpeedSlider) {
		float progress = Clamp((mousePos.x - speedSliderBounds.x) / speedSliderBounds.width, 0.0f, 1.0f);
		animSpeedMultiplier = 0.2f + (progress * 3.8f); 
	}

	if (inputErrorTimer > 0.0f) {
		inputErrorTimer -= deltaTime;
		if (inputErrorTimer <= 0.0f) inputErrorMsg = "";
	}

	Rectangle zoomHitBox = { zoomSliderBounds.x, zoomSliderBounds.y - 15.0f, zoomSliderBounds.width, 40.0f };
	if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mousePos, zoomHitBox)) isDraggingZoomSlider = true;
	if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) isDraggingZoomSlider = false;

	if (isDraggingZoomSlider) {
		float progress = Clamp((mousePos.x - zoomSliderBounds.x) / zoomSliderBounds.width, 0.0f, 1.0f);
		zoomMultiplier = 0.2f + (progress * 1.8f); 
	}
}

void DataStructureState::drawSharedUI()
{
	Vector2 mousePos = GetMousePosition();
	if (bg != nullptr) bg->Draw(mousePos);
	if (homeBtn != nullptr) homeBtn->Draw(mousePos);

	// Render sliders
	DrawTextEx(listFont, "Animation Speed", { speedSliderBounds.x, speedSliderBounds.y - 30.0f }, 22.0f, 1.0f, BLACK);
	DrawRectangleRounded(speedSliderBounds, 1.0f, 8, LIGHTGRAY);
	float fillWidth = ((animSpeedMultiplier - 0.2f) / 3.8f) * speedSliderBounds.width;
	DrawRectangleRounded({ speedSliderBounds.x, speedSliderBounds.y, fillWidth, speedSliderBounds.height }, 1.0f, 8, SKYBLUE);
	DrawCircle((int)(speedSliderBounds.x + fillWidth), (int)(speedSliderBounds.y + speedSliderBounds.height / 2.0f), 12.0f, DARKBLUE);
	DrawTextEx(numberFont, TextFormat("%.1fx", animSpeedMultiplier), { speedSliderBounds.x + speedSliderBounds.width + 15.0f, speedSliderBounds.y - 6.0f }, 22.0f, 1.0f, BLACK);

	DrawTextEx(listFont, "Zoom", { zoomSliderBounds.x, zoomSliderBounds.y - 30.0f }, 22.0f, 1.0f, BLACK);
	DrawRectangleRounded(zoomSliderBounds, 1.0f, 8, LIGHTGRAY);
	float zoomFillWidth = ((zoomMultiplier - 0.2f) / 1.8f) * zoomSliderBounds.width;
	DrawRectangleRounded({ zoomSliderBounds.x, zoomSliderBounds.y, zoomFillWidth, zoomSliderBounds.height }, 1.0f, 8, SKYBLUE); 
	DrawCircle((int)(zoomSliderBounds.x + zoomFillWidth), (int)(zoomSliderBounds.y + zoomSliderBounds.height / 2.0f), 12.0f, DARKBLUE);
	DrawTextEx(numberFont, TextFormat("%d%%", (int)(zoomMultiplier * 100)), { zoomSliderBounds.x + zoomSliderBounds.width + 15.0f, zoomSliderBounds.y - 6.0f }, 22.0f, 1.0f, BLACK);

	// Error message
	if (!inputErrorMsg.empty() && inputErrorTimer > 0.0f) {
		float menuStartX = 30.0f + 75.0f + 15.0f; 
		float menuStartY = 750.0f;
		float errorY = menuStartY + (currentErrorSlot * (45.0f + 8.0f)) + 50.0f;
		DrawTextEx(numberFont, inputErrorMsg.c_str(), { menuStartX + 135.0f, errorY }, 24.0f, 1.0f, RED);
	}
}

bool DataStructureState::CheckStepReady(float deltaTime, float stepDuration)
{
	animTimer += deltaTime * animSpeedMultiplier;
	if (animTimer >= stepDuration) {
		animTimer = 0.0f;
		return true;
	}
	return false;
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
	
	if (isActive) {
		std::string textBeforeCursor = text.substr(0, cursorIdx);
		float cursorOffsetX = MeasureTextEx(numberFont, textBeforeCursor.c_str(), fontSize, 1.0f).x;
		float maxVisibleWidth = width - (padding * 2);

		if (cursorOffsetX - scrollX > maxVisibleWidth) {
			scrollX = cursorOffsetX - maxVisibleWidth;
		} 
		else if (cursorOffsetX - scrollX < 0) {
			scrollX = cursorOffsetX;
		}
	} else {
		scrollX = 0; 
	}

	BeginScissorMode((int)pos.x, (int)pos.y, (int)width, (int)height);
	
	Vector2 textPos = { pos.x + padding - scrollX, textDrawY };
	DrawTextEx(numberFont, text.c_str(), textPos, fontSize, 1.0f, WHITE);

	if (isActive && cursorVis) {
		std::string textBeforeCursor = text.substr(0, cursorIdx);
		float cursorX = pos.x + padding - scrollX + MeasureTextEx(numberFont, textBeforeCursor.c_str(), fontSize, 1.0f).x;
		DrawLineEx({cursorX, textDrawY}, {cursorX, textDrawY + textHeight}, 2.0f, WHITE);
	}

	EndScissorMode();

	if (isHovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) return true; 
	return false;
}

void DataStructureState::DrawLabel(Vector2 pos, const char* text)
{
	float fontSize = 22.0f;
	DrawTextEx(listFont, text, {pos.x, pos.y + 10.0f}, fontSize, 1.0f, BLACK);
}