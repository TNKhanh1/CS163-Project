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

private:
    // Linked list data
    LLNode* head;
    float startX, startY;
    float nodeSpacing, nodeRadius;

    //Các biến trạng thái cho UI Control Panel
    Texture2D controlTex;
    Vector2 controlBtnPos;
    Vector2 dragOffset;
    bool isDraggingControlBtn, isClickingControlBtn, isPanelOpen;
    float panelAnimProgress;
    
    ActiveSubPanel activeSubPanel;
    ActiveInput activeInput, previousActiveInput;
    bool isCreateUserDefOpen;

    // Textbox state
    std::string inputCreate, inputSearch, inputInsertIdx, inputInsertVal, inputDeleteIdx;
    int cursorIndex;
    float cursorBlinkTimer, textScrollX;
    bool cursorVisible;

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
    
    bool IsValidInputString(const std::string& str, ActiveInput type);
    void HandleTextInput(std::string& text, ActiveInput type);
    bool DrawButtonText(Vector2 pos, const char* text, float width, float height, bool isSelected = false);
    bool DrawTextBox(Vector2 pos, std::string& text, bool isActive, float width, float height);
    void DrawLabel(Vector2 pos, const char* text);
};