#pragma once
#include <raylib.h>
#include <raymath.h>
#include <vector>
#include <string>
#include "UI.h"

struct LLNode 
{
	int value;
	Vector2 position;
	Vector2 targetPosition;
	LLNode* next;
	Color color;
};

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

private:
	LLNode* head;
	Font listFont;
	
	//Layout configs
	float startX;
	float startY;
	float nodeSpacing;
	float nodeRadius;
	
	void updateTargetPositions();
};