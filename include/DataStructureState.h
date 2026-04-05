#pragma once
#include "raylib.h"
#include "UI.h"
#include <string>


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
	// All child classes will have access to these automatically
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

	// Shared input error
	std::string inputErrorMsg;
	float inputErrorTimer;

	void updateSharedUI(float deltaTime, Vector2 mousePos); 
	void drawSharedUI(); 
	bool CheckStepReady(float deltaTime, float stepDuration);
};