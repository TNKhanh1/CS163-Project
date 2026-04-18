#pragma once
#include <DataStructureState.h>
#include <raylib.h>
#include <raymath.h>

float twoPower(int height) {
    if (height == 0) return 1.0;
    if (height > 0) return twoPower(height - 1)*2.0;
    else return twoPower(height + 1)*0.5;
}

struct Node {
    int key;
    Node* left;
    Node* right;
    int height;

    Vector2 position;
    Vector2 targetPosition;
    Color color;

    Node(int key);
    Node(int key, int h, float lvl);
    ~Node();
};

class AVLTree {
private:
    Node* root;

    int height(Node* root) const;

    int getBalance(Node* root) const;

    Node* rightRotate(Node* y, float coord);

    Node* leftRotate(Node* x, float coord);

    void insertTo(Node*& node, int key, float coord, int h = 0, float w = 0.0);

    void remove(Node*& node, int key, float coord);

    void balancingRotation(Node*& node, float coord);

    void upHeight(Node*& node, float mul, float coord, float side);

    void downHeight(Node*& node, float mul, float coord, float side);

    void rebellion(Node*& node, float side, float coord);

public:
    AVLTree();

    ~AVLTree();

    void insert(int k);

    const Node* search(int k) const;

    void delNode(int k);

    const Node* rootCall() const;
};