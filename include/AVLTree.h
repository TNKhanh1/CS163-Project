#pragma once
#include <DataStructureState.h>
#include <raylib.h>
#include <raymath.h>

inline float twoPower(int height) {
    if (height == 0) return 1.0f;
    if (height > 0) return twoPower(height - 1)*2.0f;
    else return twoPower(height + 1)*0.5f;
}

struct Node {
    Node(int key);
    Node(int key, float h, float lvl);
    ~Node();

    Vector2 position;
    Vector2 targetPosition;
    Color color = SKYBLUE;

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
    int balancingRotation(Node*& node);

    void remove(Node*& node, int key, float coord);

    Node* insertTo(Node*& node, int key, float coord, float h = 0.0f, float w = 0.0f);

    void rebellion(Node*& node, float side, float coord);

    void downHeight(Node*& node, float mul, float coord, float side);
    void upHeight(Node*& node, float mul, float coord, float side);

    int fixHeights(Node* node);

    Node* copyHelper(const Node* node);

public:
    AVLTree();
    ~AVLTree();

    const Node* rootCall() const;

    const Node* search(int k) const;

    void insert(int k);

    int balance();

    void delNode(int k);

    void clear();

    void copyTree(AVLTree& avl);
};