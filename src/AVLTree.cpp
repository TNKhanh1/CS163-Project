#include "..\\include\\AVLTree.h"   
#include <algorithm>

Node :: Node(int key): key(key), left(nullptr), right(nullptr), height(1) {}
Node :: ~Node() {
        if (left) delete left;
        if (right) delete right;
}

int AVLTree :: height(Node* root) {
    if (root == nullptr)
        return 0;
    return root->height;
}

int AVLTree :: getBalance(Node* root) {
    if (root == nullptr)
        return 0;
    return height(root->left) - height(root->right);
}

Node* AVLTree :: rightRotate(Node* y) {
    Node* x = y->left;
    Node* sub = x->right;

    x->right = y;
    y->left = sub;

    y->height = std::max(height(y->left), height(y->right)) + 1;
    x->height = std::max(height(x->left), height(x->right)) + 1;

    return x;
}

Node* AVLTree :: leftRotate(Node* x) {
    Node* y = x->right;
    Node* sub = y->left;

    y->left = x;
    x->right = sub;

    x->height = std::max(height(x->left), height(x->right)) + 1;
    y->height = std::max(height(y->left), height(y->right)) + 1;

    return y;
}

void AVLTree :: insertTo(Node*& node, int key) {
    if (node == nullptr) {
        node = new Node(key);
        return;
    }

    if (key < node->key)
        insertTo(node->left, key);
    else if (key > node->key)
        insertTo(node->right, key);
    else return;

    node->height = 1 + std::max(height(node->left), height(node->right));

    int balance = getBalance(node);

    if (balance > 1 && key < node->left->key)
        node = rightRotate(node);

    if (balance < -1 && key > node->right->key)
        node = leftRotate(node);

    if (balance > 1 && key > node->left->key) {
        node->left = leftRotate(node->left);
        node = rightRotate(node);
    }

    if (balance < -1 && key < node->right->key) {
        node->right = rightRotate(node->right);
        node = leftRotate(node);
    }
}

AVLTree :: AVLTree(): root(nullptr) {}

AVLTree :: ~AVLTree() {
    if (root != nullptr) {
        delete root;
    }
}

void AVLTree :: insert(int k) {
    insertTo(root,k);
}

Node* AVLTree :: search(int k);

void AVLTree :: delNode(int k);

Node* AVLTree :: rootCall() {
    return root;
}