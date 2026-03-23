#pragma once

struct Node {
    int key;
    Node* left;
    Node* right;
    int height;

    Node(int key);
    ~Node();
};

class AVLTree {
private:
    Node* root;

    int height(Node* root) const;

    int getBalance(Node* root) const;

    Node* rightRotate(Node* y);

    Node* leftRotate(Node* x);

    void insertTo(Node*& node, int key);

    void remove(Node*& node, int key);

    void balancingRotation(Node*& node);

public:
    AVLTree();

    ~AVLTree();

    void insert(int k);

    const Node* search(int k) const;

    void delNode(int k);

    const Node* rootCall() const;
};