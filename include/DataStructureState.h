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
	virtual bool hasNextStep() { return isAnimating && !isAnimFinished; }
	virtual bool hasPrevStep() { return historyCount > 0; }
	bool overridesStepHandling = false;

	void drawPseudoCode();


protected:
	// Dynamic operation slots
	enum MainOp { 
		OP_NONE = 0, 
		OP_SLOT1, OP_SLOT2, OP_SLOT3, OP_SLOT4, OP_SLOT5, OP_SLOT6 
	};
	MainOp activeMainOp;

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
	bool isAnimFinished;

	//Step-by-step control
	bool isAutoPlay;
    bool stepForwardRequested;
	bool stepBackwardRequested;
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
	std::string inputBuffers[5]; 

	// Shared input error
	std::string inputErrorMsg;
	float inputErrorTimer;
	int currentErrorSlot; 

	int historyCount;

	void beginAnimation()
	{
		isAnimating = true;
		isAnimFinished = false;
		historyCount = 0;
		animTimer = 0.0f;
	}

	void endAnimation()
	{
		isAnimFinished = true;
	}

	// UI Logic & Animation Trigger
	void updateSharedUI(float deltaTime, Vector2 mousePos); 
	void drawSharedUI(); 
	bool CheckStepReady(float deltaTime, float stepDuration);
	void clearInputBuffers() { for (int i = 0; i < 5; i++) inputBuffers[i].clear(); }
	virtual void handleAnimationStep() {} 
	virtual void saveState() {}
	virtual void undoState() {}
	virtual void onModeSwitch(bool toAutoMode) {} // Called when switching between Auto/Manual mode

	// Menu drawing tools
	void DrawSideMenuFrame(const std::vector<std::string>& labels);
	virtual void DrawSubMenuContent() = 0; 
	virtual void onExecuteOp(MainOp op) = 0; 

	// Shared logic methods
	void updateControlPanel(float deltaTime, Vector2 mousePos);
	void HandleTextInput(std::string& text, bool isCreateInput);
	bool IsValidInputString(const std::string& text, bool isCreateInput);

	// Shared UI drawing tools
	bool DrawButtonText(Vector2 pos, const char* text, float width, float height, bool isSelected = false);
	bool DrawTextBox(Vector2 pos, std::string& text, bool isActive, float width, float height, int cursorIdx, float& scrollX, bool cursorVis);
	void DrawLabel(Vector2 pos, const char* text);

	//Pseudocode drawing
	int activeCodeLine;
	std::vector<std::string> pseudoCode;
};