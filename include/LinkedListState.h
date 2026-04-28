#pragma once
#include <DataStructureState.h>
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
enum PendingTask { LL_TASK_NONE, LL_TASK_SEARCH, LL_TASK_INSERT_INDEX, LL_TASK_DELETE_INDEX, LL_TASK_UPDATE_INDEX };

struct LLNode {
	int value;
	Vector2 position;
	Vector2 targetPosition;
	LLNode* next;
	Color color;
};

// Snapshot structures for undo/redo
struct LLNodeSnapshot {
	int value;
	Color color;
};

struct LLStateSnapshot {
	std::vector<LLNodeSnapshot> nodes;
	int activeCodeLine;
	PendingTask currentTask;
	int searchPointerIndex;
	int searchTargetValue;
	int searchTargetIndex;
	int searchCurrentIndex;
};

class LinkedListState : public DataStructureState 
{
public:
	LinkedListState();
	~LinkedListState();

	void loadAssets() override;
	void update(float deltaTime) override;
	void draw() override;
	bool hasPrevStep() override { return !history.empty(); }

// Implementation of DataStructureState hooks
protected:
	void handleAnimationStep() override;
	void DrawSubMenuContent() override;
	void onExecuteOp(MainOp op) override;

	// State management for undo/redo
	void saveState() override;
	void undoState() override;

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


	// History for undo/redo
	std::vector<LLStateSnapshot> history;
	
    // Helper functions for linked list operations and UI
	void insertNode(int value);
	void insertNodeAtIndex(int index, int value);
	void deleteNodeAtIndex(int index);
	void searchNode(int value);
	void clearList();
	void updateTargetPositions();
	void resetNodeColors();
	void updateNodeAtIndex(int index, int value);

	bool processDroppedFile(const std::string& filePath) override;
};