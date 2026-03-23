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

    int height(Node* root);

    int getBalance(Node* root);

    Node* rightRotate(Node* y);

    Node* leftRotate(Node* x);

    void insertTo(Node*& node, int key);

public:
    AVLTree();

    ~AVLTree();

    void insert(int k);

    Node* search(int k);

    void delNode(int k);

    Node* rootCall();
};