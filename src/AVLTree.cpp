#include "../include/AVLTree.h"   
#include <algorithm>

Node :: Node(int key): key(key), left(nullptr), right(nullptr), height(1) {}
Node :: ~Node() {
        delete left;
        delete right;
}
Node :: Node(int key, float h, float w): key(key), left(nullptr), right(nullptr), height(1), position({w + 900.0f, h + 50.0f}), targetPosition({w + 900.0f, h + 50.0f}) {}

int AVLTree :: height(Node* root) const {
    if (root == nullptr)
        return 0;
    return root->height;
}

int AVLTree :: getBalance(Node* root) const {
    if (root == nullptr)
        return 0;
    return height(root->left) - height(root->right);
}

Node* AVLTree :: leftRotate(Node* x, float coord) {
    Node* y = x->right;
    Node* sub = y->left;

    y->left = x;
    x->right = sub;
    rebellion(sub, -1.0f, coord);

    upHeight(y, -1.0f, coord, 0.5f);
    downHeight(x, -1.0f, coord, 0.5f);

    x->height = std::max(height(x->left), height(x->right)) + 1;
    y->height = std::max(height(y->left), height(y->right)) + 1;

    return y;
}

Node* AVLTree :: rightRotate(Node* y, float coord) {
    Node* x = y->left;
    Node* sub = x->right;

    x->right = y;
    y->left = sub;
    rebellion(sub, 1.0f, coord);

    upHeight(x, 1.0f, coord, -0.5f);
    downHeight(y, 1.0f, coord, -0.5f);

    y->height = std::max(height(y->left), height(y->right)) + 1;
    x->height = std::max(height(x->left), height(x->right)) + 1;

    return x;
}

void AVLTree :: balancingRotation(Node*& node, float coord) {
    int balance = getBalance(node);

    if (balance > 1 && getBalance(node->left) >= 0) node = rightRotate(node, coord);

    else if (balance < -1 && getBalance(node->right) <= 0) node = leftRotate(node, coord);

    else if (balance > 1 && getBalance(node->left) < 0) {
        node->left = leftRotate(node->left, coord);
        node = rightRotate(node, coord);
    }

    else if (balance < -1 && getBalance(node->right) > 0) {
        node->right = rightRotate(node->right, coord);
        node = leftRotate(node, coord);
    }
}

void AVLTree :: remove(Node*& node, int key, float coord) {
    if (node == nullptr) return;

    if (key < node->key)
        remove(node->left, key, coord*0.5f);
    else if (key > node->key)
        remove(node->right, key, coord*0.5f);
    else {
        if (node->right == nullptr || node->left == nullptr) {
            Node* cur = node;
            if (node->left == nullptr) {
                node = node->right;
                upHeight(node, -1.0f, coord, 0.5f);
            }
            else  {
                node = node->left;
                upHeight(node, 1.0f, coord, -0.5f);
            }
            cur->left = nullptr;
            cur->right = nullptr;
            delete cur;
        }
        else {
            Node* cur = node->right;
            while (cur->left != nullptr) {
                cur = cur->left;
            }
            node->key = cur->key;
            remove(node->right, cur->key, coord*0.5f);
        }
    }

    if (node == nullptr) return;

    node->height = 1 + std::max(height(node->left), height(node->right));

    balancingRotation(node, coord*4.0f);
}

void AVLTree :: insertTo(Node*& node, int key, float coord, float h, float w) {
    if (node == nullptr) {
        node = new Node(key, h, w);
        return;
    }

    if (key < node->key)
        insertTo(node->left, key, coord*0.5f, h + 50.0f, w - coord);
    else if (key > node->key)
        insertTo(node->right, key, coord*0.5f, h + 50.0f, w + coord);
    else return;

    node->height = 1 + std::max(height(node->left), height(node->right));

    balancingRotation(node, coord*4.0f);
}

void AVLTree :: rebellion(Node*& node, float side, float coord) {
    if (node == nullptr) return;
    node->targetPosition.x = node->targetPosition.x + side*coord;
    rebellion(node->left, side, coord);
    rebellion(node->right, side, coord);
}

void AVLTree :: downHeight(Node*& node, float mul, float coord, float side) {
    if (node == nullptr) return;
    node->targetPosition.x = node->targetPosition.x + (mul - side)*coord;
    node->targetPosition.y += 50.0f;
    downHeight(node->left, mul, coord*0.5f, -0.5f);
    downHeight(node->right, mul, coord*0.5f, 0.5f);
}

void AVLTree :: upHeight(Node*& node, float mul, float coord, float side) {
    if (node == nullptr) return;
    node->targetPosition.x = node->targetPosition.x + (mul + side)*coord;
    node->targetPosition.y -= 50.0f;
    upHeight(node->left, mul, coord*0.5f, -0.5f);
    upHeight(node->right, mul, coord*0.5f, 0.5f);
}

AVLTree :: AVLTree(): root(nullptr) {}

AVLTree :: ~AVLTree() {
    if (root != nullptr) {
        delete root;
    }
}

const Node* AVLTree :: rootCall() const {
    return root;
}

void AVLTree :: insert(int k) {
    insertTo(root,k, twoPower(height(root) - 3)*20.0f);
}

void AVLTree :: delNode(int k) {
    remove(root, k, twoPower(height(root) - 3)*20.0f);
}

const Node* AVLTree :: search(int k) const {
    Node* cur = root;
    while (cur != nullptr) {
        if (cur->key > k) cur = cur->left;
        else if (cur->key < k) cur = cur->right;
        else break;
    }
    return cur;
}