#pragma once
#include "DataStructureState.h"
#include <raylib.h>
#include <raymath.h>
#include <vector>
#include <string>
#include <iostream>
#include "Heap.h"

enum HeapSubPanel { HEAP_SUB_NONE, HEAP_SUB_CREATE, HEAP_SUB_INSERT, HEAP_SUB_DELETE };
enum HeapInput { HEAP_INP_NONE, HEAP_INP_INSERT_VAL };

class HeapState : public DataStructureState 
{
public:
    HeapState();
    ~HeapState();

    void loadAssets() override;
    void update(float deltaTime) override;
    void draw() override;

private:
    Heap heap; 

    Texture2D controlTex;
    Vector2 controlBtnPos;
    Vector2 dragOffset;
    bool isDraggingControlBtn, isClickingControlBtn, isPanelOpen;
    float panelAnimProgress;
    
    HeapSubPanel activeSubPanel;
    HeapInput activeInput, previousActiveInput;

    std::string inputInsertVal;
    int cursorIndex;
    float cursorBlinkTimer, textScrollX;
    bool cursorVisible;

    bool IsValidInputString(const std::string& str, HeapInput type);
    void HandleTextInput(std::string& text, HeapInput type);
    bool DrawButtonText(Vector2 pos, const char* text, float width, float height, bool isSelected = false);
    bool DrawTextBox(Vector2 pos, std::string& text, bool isActive, float width, float height);
    void DrawLabel(Vector2 pos, const char* text);
};