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
    Node(int key);
    Node(int key, int h, float lvl);
    ~Node();

    Vector2 position;
    Vector2 targetPosition;
    Color color;

    int key;
    Node* left;
    Node* right;
    int height;
};

class AVLTree {
private:
    Node* root;

    int getBalance(Node* root) const;

    int height(Node* root) const;

    Node* rightRotate(Node* y, float coord);
    Node* leftRotate(Node* x, float coord);
    void balancingRotation(Node*& node, float coord);

    void remove(Node*& node, int key, float coord);

    void insertTo(Node*& node, int key, float coord, int h = 0, float w = 0.0);

    void rebellion(Node*& node, float side, float coord);

    void downHeight(Node*& node, float mul, float coord, float side);
    void upHeight(Node*& node, float mul, float coord, float side);

public:
    AVLTree();
    ~AVLTree();

    const Node* rootCall() const;

    const Node* search(int k) const;

    void insert(int k);

    void delNode(int k);
};