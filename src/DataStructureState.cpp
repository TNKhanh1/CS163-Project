#include "DataStructureState.h"

DataStructureState::DataStructureState()
{
	bg = nullptr;
	homeBtn = nullptr;
	NextState = -1; 

	// Shared error
	inputErrorMsg = "";
	inputErrorTimer = 0.0f;

	// Shared anim and slider
	speedSliderBounds = { 1500.0f, 870.0f, 200.0f, 10.0f };
	animSpeedMultiplier = 1.0f;
	isDraggingSpeedSlider = false;

	zoomSliderBounds = { 1500.0f, 940.0f, 200.0f, 10.0f };
	zoomMultiplier = 1.0f; 
	isDraggingZoomSlider = false;
	
	animTimer = 0.0f;
	isAnimating = false;
}

DataStructureState::~DataStructureState()
{
	if (bg) delete bg;
	if (homeBtn) delete homeBtn;
	UnloadFont(listFont); 
	UnloadFont(numberFont);
}

void DataStructureState::loadAssets()
{
	// 1. Load background and home
	bg = new back_ground("assets/background01.jpg", {0.0f, 0.0f}, "assets/background01.jpg", false);
	homeBtn = new button("assets/home.png", "assets/home.png", "assets/home.png", "assets/home.png", {10.0f, 10.0f}, 80.0);

	// 2. Load shared fonts
	int fontSize = 50;
	listFont = LoadFontEx("assets/FONT2.ttf", fontSize, 0, 0);
	SetTextureFilter(listFont.texture, TEXTURE_FILTER_BILINEAR);

	int numFontSize = 22;
	numberFont = LoadFontEx("assets/FONT3.ttf", numFontSize, 0, 0);
	SetTextureFilter(numberFont.texture, TEXTURE_FILTER_BILINEAR);
}

void DataStructureState::update(float deltaTime) {}
void DataStructureState::draw() {}

void DataStructureState::updateSharedUI(float deltaTime, Vector2 mousePos)
{
	// 1. Home button
	if (homeBtn != nullptr && homeBtn->isPressed(mousePos, IsMouseButtonPressed(MOUSE_LEFT_BUTTON))) {
		NextState = 0;
		return;
	}

	// 2. Speed slider
	Rectangle sliderHitBox = { speedSliderBounds.x, speedSliderBounds.y - 15.0f, speedSliderBounds.width, 40.0f };
	if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mousePos, sliderHitBox)) {
		isDraggingSpeedSlider = true;
    }
	if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
		isDraggingSpeedSlider = false;
	}

	if (isDraggingSpeedSlider) {
		float progress = (mousePos.x - speedSliderBounds.x) / speedSliderBounds.width;
		if (progress < 0.0f) progress = 0.0f;
		if (progress > 1.0f) progress = 1.0f;
		animSpeedMultiplier = 0.2f + (progress * 3.8f); // Maps to 0.2x - 4.0x speed
	}

	// 3. Error message timer
	if (inputErrorTimer > 0.0f) {
		inputErrorTimer -= deltaTime;
		if (inputErrorTimer <= 0.0f) inputErrorMsg = "";
	}

	Rectangle zoomHitBox = { zoomSliderBounds.x, zoomSliderBounds.y - 15.0f, zoomSliderBounds.width, 40.0f };
	if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mousePos, zoomHitBox)) {
		isDraggingZoomSlider = true;
    }
	if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
		isDraggingZoomSlider = false;
	}

	if (isDraggingZoomSlider) {
		float progress = (mousePos.x - zoomSliderBounds.x) / zoomSliderBounds.width;
		if (progress < 0.0f) progress = 0.0f;
		if (progress > 1.0f) progress = 1.0f;
		zoomMultiplier = 0.2f + (progress * 1.8f); 
	}

}

void DataStructureState::drawSharedUI()
{
	Vector2 mousePos = GetMousePosition();

	// 1. Draw background and home button
	if (bg != nullptr) bg->Draw(mousePos);
	if (homeBtn != nullptr) homeBtn->Draw(mousePos);

	// 2. Draw speed slider
	DrawTextEx(listFont, "Animation Speed", { speedSliderBounds.x, speedSliderBounds.y - 30.0f }, 22.0f, 1.0f, BLACK);
	DrawRectangleRounded(speedSliderBounds, 1.0f, 8, LIGHTGRAY);
	
	float fillWidth = ((animSpeedMultiplier - 0.2f) / 3.8f) * speedSliderBounds.width;
	Rectangle filledTrack = { speedSliderBounds.x, speedSliderBounds.y, fillWidth, speedSliderBounds.height };
	DrawRectangleRounded(filledTrack, 1.0f, 8, SKYBLUE);
	
	float handleX = speedSliderBounds.x + fillWidth;
	float handleY = speedSliderBounds.y + (speedSliderBounds.height / 2.0f);
	DrawCircle((int)handleX, (int)handleY, 12.0f, DARKBLUE);
	
	const char* speedText = TextFormat("%.1fx", animSpeedMultiplier);
	DrawTextEx(numberFont, speedText, { speedSliderBounds.x + speedSliderBounds.width + 15.0f, speedSliderBounds.y - 6.0f }, 22.0f, 1.0f, BLACK);

	//ZOOM
	DrawTextEx(listFont, "Zoom", { zoomSliderBounds.x, zoomSliderBounds.y - 30.0f }, 22.0f, 1.0f, BLACK);
	DrawRectangleRounded(zoomSliderBounds, 1.0f, 8, LIGHTGRAY);

	float zoomFillWidth = ((zoomMultiplier - 0.2f) / 1.8f) * zoomSliderBounds.width;
	Rectangle zoomFilledTrack = { zoomSliderBounds.x, zoomSliderBounds.y, zoomFillWidth, zoomSliderBounds.height };


	DrawRectangleRounded(zoomFilledTrack, 1.0f, 8, SKYBLUE); 

	float zoomHandleX = zoomSliderBounds.x + zoomFillWidth;
	float zoomHandleY = zoomSliderBounds.y + (zoomSliderBounds.height / 2.0f);

	DrawCircle((int)zoomHandleX, (int)zoomHandleY, 12.0f, DARKBLUE);

	const char* zoomText = TextFormat("%d%%", (int)(zoomMultiplier * 100));
	DrawTextEx(numberFont, zoomText, { zoomSliderBounds.x + zoomSliderBounds.width + 15.0f, zoomSliderBounds.y - 6.0f }, 22.0f, 1.0f, BLACK);
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