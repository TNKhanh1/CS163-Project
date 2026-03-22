#include "LinkedListState.h"

LinkedListState::LinkedListState()
{
	head = nullptr;
	startX = 100.0f;
	startY = 500.0f;
	nodeSpacing = 150.0f;
	nodeRadius = 40.0f;
}

LinkedListState::~LinkedListState()
{
	clearList();
}

void LinkedListState::loadAssets()
{

}

void LinkedListState::insertNode(int value)
{
	LLNode* newNode = new LLNode();
	newNode->value = value;
	newNode->next = nullptr;
	newNode->color = SKYBLUE;
	
	// Spawn the node slightly off to the side for animation
	newNode->position = { startX, startY - 200.0f }; 
	
	if (head == nullptr) 
	{
		head = newNode;
	} 
	else 
	{
		LLNode* temp = head;
		while (temp->next != nullptr) 
		{
			temp = temp->next;
		}
		temp->next = newNode;
	}
	
	updateTargetPositions();
}

void LinkedListState::deleteNode(int value)
{
	if (head == nullptr) return;
	
	if (head->value == value) 
	{
		LLNode* temp = head;
		head = head->next;
		delete temp;
		updateTargetPositions();
		return;
	}
	
	LLNode* temp = head;
	while (temp->next != nullptr && temp->next->value != value) 
	{
		temp = temp->next;
	}
	
	if (temp->next != nullptr) 
	{
		LLNode* nodeToDelete = temp->next;
		temp->next = temp->next->next;
		delete nodeToDelete;
		updateTargetPositions();
	}
}

void LinkedListState::clearList()
{
	LLNode* current = head;
	while (current != nullptr) 
	{
		LLNode* next = current->next;
		delete current;
		current = next;
	}
	head = nullptr;
}

void LinkedListState::updateTargetPositions()
{
	LLNode* current = head;
	float currentX = startX;
	
	while (current != nullptr) 
	{
		current->targetPosition = { currentX, startY };
		currentX += nodeSpacing;
		current = current->next;
	}
}

void LinkedListState::update(float deltaTime)
{

}

void LinkedListState::draw()
{

}