#pragma once
#include <raylib.h>
#include <raymath.h>
#include <vector>
#include <string>
#include <iostream>
#include <cmath>
#include <UI.h>
#include <State.h>

struct LLNode 
{
	int value;
	Vector2 position;
	Vector2 targetPosition;
	LLNode* next;
	Color color;
};

//các Enum cho UI
enum ActiveSubPanel { SUB_NONE, SUB_CREATE, SUB_INSERT, SUB_SEARCH, SUB_DELETE };
enum ActiveInput { INP_NONE, INP_CREATE, INP_SEARCH, INP_INSERT_IDX, INP_INSERT_VAL, INP_DELETE_IDX };

class LinkedListState
{
public:
	LinkedListState();
	~LinkedListState();
	
	void update(float deltaTime);
	void draw();
	void loadAssets();

	void insertNode(int value);
	void deleteNode(int value);
	void clearList();
    void insertNodeAtIndex(int index, int value);
	void deleteNodeAtIndex(int index);
	void searchNode(int value);

	State NextState;

private:
	LLNode* head;
	Font listFont;
	Font numberFont;
	
	// Layout configs
	float startX;
	float startY;
	float nodeSpacing;
	float nodeRadius;
	
	void updateTargetPositions();

	back_ground* bg;
    button* homeBtn;

    //Các biến trạng thái cho UI Control Panel
    Texture2D controlTex;
    Vector2 controlBtnPos;
    bool isDraggingControlBtn;
    Vector2 dragOffset;
    bool isClickingControlBtn;

    bool isPanelOpen;
    float panelAnimProgress; 

    ActiveSubPanel activeSubPanel;
    ActiveInput activeInput;
    ActiveInput previousActiveInput; // Để theo dõi khi nào đổi Textbox

    std::string inputCreate;
    std::string inputSearch;
    std::string inputInsertIdx;
    std::string inputInsertVal;
    std::string inputDeleteIdx;

    bool isCreateUserDefOpen; 

    int cursorIndex;          // Vị trí con trỏ trong chuỗi
    float cursorBlinkTimer;   // Hẹn giờ nhấp nháy
    bool cursorVisible;       // Trạng thái hiện/ẩn của con trỏ
    float textScrollX;        // Vị trí cuộn ngang của Textbox

    //Các hàm hỗ trợ vẽ UI
    bool IsValidInputString(const std::string& str, ActiveInput type); // Hàm kiểm tra logic nhập
    void HandleTextInput(std::string& text, ActiveInput type); // Cập nhật để nhận type
    bool DrawButtonText(Vector2 pos, const char* text, float width, float height, bool isSelected = false);
    bool DrawTextBox(Vector2 pos, std::string& text, bool isActive, float width, float height);
    void DrawLabel(Vector2 pos, const char* text);
};