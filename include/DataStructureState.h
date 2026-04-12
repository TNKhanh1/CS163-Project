#pragma once
#include "raylib.h"
#include "UI.h"
#include <string>
#include <vector>

class DataStructureState 
{
public:
	DataStructureState();
	virtual ~DataStructureState();

	virtual void loadAssets();
	virtual void update(float deltaTime); 
	virtual void draw();

	int NextState;

protected:
	// Shared visual assets
	back_ground* bg;
	button* homeBtn;
	Font listFont;
	Font numberFont;

	// Shared animation timing
	Rectangle speedSliderBounds;
	float animSpeedMultiplier;
	bool isDraggingSpeedSlider;
	float animTimer;
	bool isAnimating;

	Rectangle zoomSliderBounds;
	float zoomMultiplier;
	bool isDraggingZoomSlider;

	// Shared control panel state
	Texture2D controlTex;
	Vector2 controlBtnPos;
	bool isDraggingControlBtn;
	bool isClickingControlBtn;
	bool isPanelOpen;
	float panelAnimProgress;
	Vector2 dragOffset;

	// Shared input/cursor state & Buffers
	int cursorIndex;
	float cursorBlinkTimer;
	float textScrollX;
	bool cursorVisible;
	std::string inputBuffers[5]; // (0: Create, 1: Search, 2: InsertIdx, 3: InsertVal, 4: DeleteIdx)

	// Shared input error
	std::string inputErrorMsg;
	float inputErrorTimer;
	int currentErrorSlot; 

	// UI Logic & Animation Trigger
	void updateSharedUI(float deltaTime, Vector2 mousePos); 
	void drawSharedUI(); 
	bool CheckStepReady(float deltaTime, float stepDuration);
	void clearInputBuffers() { for (int i = 0; i < 5; i++) inputBuffers[i].clear(); }
	virtual void handleAnimationStep() {} 

	// Shared logic methods
	void updateControlPanel(float deltaTime, Vector2 mousePos);
	void HandleTextInput(std::string& text, bool isCreateInput);
	bool IsValidInputString(const std::string& text, bool isCreateInput);

	// Shared UI drawing tools
	bool DrawButtonText(Vector2 pos, const char* text, float width, float height, bool isSelected = false);
	bool DrawTextBox(Vector2 pos, std::string& text, bool isActive, float width, float height, int cursorIdx, float& scrollX, bool cursorVis);
	void DrawLabel(Vector2 pos, const char* text);
};