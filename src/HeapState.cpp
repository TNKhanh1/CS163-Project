#include "HeapState.h"
#include "State.h"
#include <iostream>
#include <cmath>
#include "raymath.h"

HeapState::HeapState() : DataStructureState()
{
    NextState = (int)STATE_HEAP; 

    controlTex.id = 0; 

    controlBtnPos = { 30.0f, 750.0f }; 
    isDraggingControlBtn = false;
    isClickingControlBtn = false;
    isPanelOpen = false;
    panelAnimProgress = 0.0f;
    activeSubPanel = HEAP_SUB_NONE;
    activeInput = HEAP_INP_NONE;
    previousActiveInput = HEAP_INP_NONE;

    cursorIndex = 0;
    cursorBlinkTimer = 0.0f;
    cursorVisible = true;
    textScrollX = 0.0f;
}

HeapState::~HeapState()
{
    heap.clear();
    if (controlTex.id != 0) UnloadTexture(controlTex); 
}

void HeapState::loadAssets()
{
    DataStructureState::loadAssets(); 
    
    if (controlTex.id != 0) {
        UnloadTexture(controlTex);
    }

    Image ctrlImg = LoadImage("assets/control.png");
    ImageResize(&ctrlImg, 75, 75); 
    controlTex = LoadTextureFromImage(ctrlImg);
    UnloadImage(ctrlImg);
}

bool HeapState::IsValidInputString(const std::string& str, HeapInput type)
{
    int currentDigitCount = 0;
    for (size_t i = 0; i < str.length(); i++)
    {
        char c = str[i];
        if (c == '-') {
            if (i != 0) return false;
        }
        else if (c >= '0' && c <= '9') {
            currentDigitCount++;
            if (currentDigitCount > 4) return false; 
        }
        else {
            return false; 
        }
    }
    return true;
}

void HeapState::HandleTextInput(std::string& text, HeapInput type)
{
    if ((IsKeyPressed(KEY_LEFT) || IsKeyPressedRepeat(KEY_LEFT)) && cursorIndex > 0) cursorIndex--;
    if ((IsKeyPressed(KEY_RIGHT) || IsKeyPressedRepeat(KEY_RIGHT)) && cursorIndex < (int)text.length()) cursorIndex++;

    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT) || IsKeyPressedRepeat(KEY_LEFT) || IsKeyPressedRepeat(KEY_RIGHT)) {
        cursorVisible = true; cursorBlinkTimer = 0.0f;
    }

    if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) {
        if (cursorIndex > 0) {
            text.erase(cursorIndex - 1, 1);
            cursorIndex--;
            cursorVisible = true; cursorBlinkTimer = 0.0f;
        }
    }
    if (IsKeyPressed(KEY_DELETE) || IsKeyPressedRepeat(KEY_DELETE)) {
        if (cursorIndex < (int)text.length()) {
            text.erase(cursorIndex, 1);
            cursorVisible = true; cursorBlinkTimer = 0.0f;
        }
    }

    int keyCode = GetKeyPressed();
    while (keyCode > 0) 
    {
        char charToAdd = 0;

        if (keyCode >= KEY_ZERO && keyCode <= KEY_NINE) charToAdd = '0' + (keyCode - KEY_ZERO);
        else if (keyCode >= KEY_KP_0 && keyCode <= KEY_KP_9) charToAdd = '0' + (keyCode - KEY_KP_0);
        else if (keyCode == KEY_MINUS || keyCode == KEY_KP_SUBTRACT) charToAdd = '-';
        
        if (charToAdd != 0) {
            std::string temp = text;
            temp.insert(cursorIndex, 1, charToAdd);

            if (IsValidInputString(temp, type)) {
                text = temp;
                cursorIndex++;
                cursorVisible = true; cursorBlinkTimer = 0.0f;
            }
        }
        keyCode = GetKeyPressed();
    }
    while (GetCharPressed() > 0) {} 
}

void HeapState::update(float deltaTime)
{
    Vector2 mousePos = GetMousePosition();
    
    DataStructureState::updateSharedUI(deltaTime, mousePos);

    Rectangle controlBtnBounds = { controlBtnPos.x, controlBtnPos.y, (float)controlTex.width, (float)controlTex.height };
    
    if (CheckCollisionPointRec(mousePos, controlBtnBounds) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
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
            if (isClickingControlBtn && CheckCollisionPointRec(mousePos, controlBtnBounds)) {
                isPanelOpen = !isPanelOpen; 
            }
        }
        isClickingControlBtn = false; 
    }

    float animSpeed = 4.0f; 
    if (isPanelOpen) {
        panelAnimProgress += deltaTime * animSpeed;
        if (panelAnimProgress > 1.0f) panelAnimProgress = 1.0f;
    } else {
        panelAnimProgress -= deltaTime * animSpeed;
        if (panelAnimProgress < 0.0f) {
            panelAnimProgress = 0.0f; activeSubPanel = HEAP_SUB_NONE; 
            activeInput = HEAP_INP_NONE; 
        }
    }

    if (activeInput != previousActiveInput) {
        if (activeInput == HEAP_INP_INSERT_VAL) cursorIndex = inputInsertVal.length();
        
        textScrollX = 0.0f; 
        cursorBlinkTimer = 0.0f;
        cursorVisible = true;
        previousActiveInput = activeInput;
    }

    if (activeInput != HEAP_INP_NONE) {
        cursorBlinkTimer += deltaTime;
        if (cursorBlinkTimer >= 0.5f) { 
            cursorVisible = !cursorVisible;
            cursorBlinkTimer = 0.0f;
        }

        if (activeInput == HEAP_INP_INSERT_VAL) HandleTextInput(inputInsertVal, activeInput);
        
        if (IsKeyPressed(KEY_ENTER)) {
            try {
                if (activeInput == HEAP_INP_INSERT_VAL && !inputInsertVal.empty()) {
                    heap.insert(std::stoi(inputInsertVal));
                    inputInsertVal.clear();
                }
            } catch (...) {}
            activeInput = HEAP_INP_NONE;
        }
    }
}

bool HeapState::DrawButtonText(Vector2 pos, const char* text, float width, float height, bool isSelected)
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

bool HeapState::DrawTextBox(Vector2 pos, std::string& text, bool isActive, float width, float height)
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
        std::string textBeforeCursor = text.substr(0, cursorIndex);
        float cursorOffsetX = MeasureTextEx(numberFont, textBeforeCursor.c_str(), fontSize, 1.0f).x;
        float maxVisibleWidth = width - (padding * 2);

        if (cursorOffsetX - textScrollX > maxVisibleWidth) {
            textScrollX = cursorOffsetX - maxVisibleWidth;
        } 
        else if (cursorOffsetX - textScrollX < 0) {
            textScrollX = cursorOffsetX;
        }
    } else {
        textScrollX = 0; 
    }

    BeginScissorMode((int)pos.x, (int)pos.y, (int)width, (int)height);
    
    Vector2 textPos = { pos.x + padding - textScrollX, textDrawY };
    DrawTextEx(numberFont, text.c_str(), textPos, fontSize, 1.0f, WHITE);

    if (isActive && cursorVisible) {
        std::string textBeforeCursor = text.substr(0, cursorIndex);
        float cursorX = pos.x + padding - textScrollX + MeasureTextEx(numberFont, textBeforeCursor.c_str(), fontSize, 1.0f).x;
        DrawLineEx({cursorX, textDrawY}, {cursorX, textDrawY + textHeight}, 2.0f, WHITE);
    }

    EndScissorMode();

    float startX = controlBtnPos.x + controlTex.width + 15.0f;
    BeginScissorMode((int)startX, 0, GetScreenWidth(), GetScreenHeight());

    if (isHovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) return true; 
    return false;
}

void HeapState::DrawLabel(Vector2 pos, const char* text)
{
    float fontSize = 22.0f;
    DrawTextEx(listFont, text, {pos.x, pos.y + 10.0f}, fontSize, 1.0f, BLACK);
}

void HeapState::draw()
{
    DataStructureState::drawSharedUI();

    const char* titleText = "HEAP VISUALIZATION";
    DrawTextEx(listFont, titleText, { (1800.0f - MeasureTextEx(listFont, titleText, 55, 6.5f).x) / 2.0f, 20.0f }, 55, 6.5f, BLACK);

    // TODO: Draw Heap Tree Here (Chưa vẽ cây theo yêu cầu)

    Vector2 mousePos = GetMousePosition();
    Rectangle controlBtnBounds = { controlBtnPos.x, controlBtnPos.y, (float)controlTex.width, (float)controlTex.height };
    
    Color controlColor = CheckCollisionPointRec(mousePos, controlBtnBounds) ? Fade(WHITE, 0.7f) : WHITE;
    DrawTextureV(controlTex, controlBtnPos, controlColor);
    
    if (panelAnimProgress > 0.0f) 
    {
        float easedProgress = sin(panelAnimProgress * PI / 2.0f); 

        float mainItemWidth = 125.0f; 
        float mainItemHeight = 45.0f;
        float gap = 8.0f; 

        float startX = controlBtnPos.x + controlTex.width + 15.0f;
        float startY = controlBtnPos.y; 
        
        BeginScissorMode((int)startX, 0, GetScreenWidth(), GetScreenHeight());
        float panelX = startX - mainItemWidth * (1.0f - easedProgress);

        const char* mainItems[] = {"Create", "Insert", "Delete"};
        HeapSubPanel itemSubPanels[] = {HEAP_SUB_CREATE, HEAP_SUB_INSERT, HEAP_SUB_DELETE};

        for (int i = 0; i < 3; i++) {
            float itemY = startY + i * (mainItemHeight + gap);
            bool isSelected = (activeSubPanel == itemSubPanels[i]);

            bool isClicked = DrawButtonText({panelX, itemY}, mainItems[i], mainItemWidth, mainItemHeight, isSelected);

            if (isClicked && panelAnimProgress >= 1.0f) {
                if (activeSubPanel == itemSubPanels[i]) activeSubPanel = HEAP_SUB_NONE; 
                else { activeSubPanel = itemSubPanels[i]; }
            }
        }

        if (activeSubPanel != HEAP_SUB_NONE) 
        {
            float subX = panelX + mainItemWidth + gap; 
            float labelFontSize = 22.0f; 

            if (activeSubPanel == HEAP_SUB_CREATE) {
                float sy = startY + 0 * (mainItemHeight + gap); 
                float cx = subX;

                if (DrawButtonText({cx, sy}, "Empty", 90, mainItemHeight)) { 
                    heap.clear();
                }
                cx += 90 + gap;

                if (DrawButtonText({cx, sy}, "Random", 110, mainItemHeight)) { 
                    heap.clear();
                    int NumNode = GetRandomValue(1, 15);
                    std::vector<int> randomData;
                    for(int i = 0; i < NumNode; i++) randomData.push_back(GetRandomValue(1, 99));
                    heap.buildHeap(randomData);
                }
            }
            else if (activeSubPanel == HEAP_SUB_INSERT) {
                float sy = startY + 1 * (mainItemHeight + gap); 
                float cx = subX;

                DrawLabel({cx, sy}, "Value =");
                cx += MeasureTextEx(listFont, "Value =", labelFontSize, 1.0f).x + gap;

                if (DrawTextBox({cx, sy}, inputInsertVal, activeInput == HEAP_INP_INSERT_VAL, 100, mainItemHeight)) activeInput = HEAP_INP_INSERT_VAL;
                cx += 100 + gap;

                if (DrawButtonText({cx, sy}, "GO", 50, mainItemHeight)) {
                    if (!inputInsertVal.empty()) {
                        try { heap.insert(std::stoi(inputInsertVal)); inputInsertVal.clear(); activeInput = HEAP_INP_NONE; } catch (...) {}
                    }
                }
            }
            else if (activeSubPanel == HEAP_SUB_DELETE) {
                float sy = startY + 2 * (mainItemHeight + gap);
                float cx = subX;

                if (DrawButtonText({cx, sy}, "POP", 90, mainItemHeight)) {
                    if (!heap.isEmpty()) {
                        try { heap.extractTop(); } catch (...) {} 
                    }
                }
            }
        }

        if (!inputErrorMsg.empty() && inputErrorTimer > 0.0f) {
            DrawTextEx(numberFont, inputErrorMsg.c_str(), { panelX + mainItemWidth + gap + 90.0f, startY + 200.0f }, 18.0f, 1.0f, RED);
        }

        EndScissorMode();    
    }
}

void HeapState::DrawSubMenuContent() {
    // TODO: Implement heap submenu drawing
}

void HeapState::onExecuteOp(MainOp op) {
    // TODO: Implement heap operation execution
}