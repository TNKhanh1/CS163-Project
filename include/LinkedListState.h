#pragma once
#include "DataStructureState.h"
#include <raylib.h>
#include <raymath.h>
#include <vector>
#include <string>
#include <iostream>
#include <cmath>
#include <UI.h>
#include <State.h>

// Enums for UI state
enum ActiveSubPanel { SUB_NONE, SUB_CREATE, SUB_INSERT, SUB_SEARCH, SUB_DELETE };
enum ActiveInput { INP_NONE, INP_CREATE, INP_SEARCH, INP_INSERT_IDX, INP_INSERT_VAL, INP_DELETE_IDX };
enum PendingTask { TASK_NONE, TASK_SEARCH, TASK_INSERT_INDEX, TASK_DELETE_INDEX };

struct LLNode {
	int value;
	Vector2 position;
	Vector2 targetPosition;
	LLNode* next;
	Color color;
};

class LinkedListState : public DataStructureState 
{
public:
	LinkedListState();
	~LinkedListState();

	void loadAssets() override;
	void update(float deltaTime) override;
	void draw() override;

protected:
	// Implementation of DataStructureState hooks
	void handleAnimationStep() override;
	void DrawSubMenuContent() override;
	void onExecuteOp(MainOp op) override;

private:
	// Linked list data
	LLNode* head;
	float startX, startY;
	float nodeSpacing, nodeRadius;

	// Menu state
	int activeInputFocus; // -1: None, 0: Create, 1: Search, 2: InsIdx, 3: InsVal, 4: DelIdx
	int previousInputFocus;
	bool isCreateUserDefOpen;

    // LL anim
	LLNode* searchPointer;
	int searchTargetValue, searchTargetIndex, searchCurrentIndex;
	PendingTask currentTask;
	float previousZoomMultiplier;

    // Helper functions for linked list operations and UI
	void insertNode(int value);
	void insertNodeAtIndex(int index, int value);
	void deleteNodeAtIndex(int index);
	void searchNode(int value);
	void clearList();
	void updateTargetPositions();
	void resetNodeColors();
	
	// NOTE: IsValidInputString and HandleTextInput were REMOVED because they are now inherited from DataStructureState.
};