#include "../include/AVLTree.h"   
#include <algorithm>

Node :: Node(int key): key(key), left(nullptr), right(nullptr), height(1) {}
Node :: ~Node() {
        delete left;
        delete right;
}

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

    balancingRotation(node);
}

void AVLTree :: remove(Node*& node, int key) {
    if (node == nullptr) return;

    if (key < node->key)
        remove(node->left, key);
    else if (key > node->key)
        remove(node->right, key);
    else {
        if (node->right == nullptr || node->left == nullptr) {
            Node* cur = node;
            node = (node->left == nullptr)? node->right : node->left;
            cur->left = nullptr;
            cur->right = nullptr;
            delete cur;
        }
        else {
            Node* cur = node->right;
            while (cur->left != nullptr) cur = cur->left;
            node->key = cur->key;
            remove(node->right, cur->key);
        }
    }

    if (node == nullptr) return;

    node->height = 1 + std::max(height(node->left), height(node->right));

    balancingRotation(node);
}

void AVLTree :: balancingRotation(Node*& node) {
    int balance = getBalance(node);

    if (balance > 1 && getBalance(node->left) >= 0) node = rightRotate(node);

    else if (balance < -1 && getBalance(node->right) <= 0) node = leftRotate(node);

    else if (balance > 1 && getBalance(node->left) < 0) {
        node->left = leftRotate(node->left);
        node = rightRotate(node);
    }

    else if (balance < -1 && getBalance(node->right) > 0) {
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

const Node* AVLTree :: search(int k) const {
    Node* cur = root;
    while (cur != nullptr) {
        if (cur->key > k) cur = cur->left;
        else if (cur->key < k) cur = cur->right;
        else break;
    }
    return cur;
}

void AVLTree :: delNode(int k) {
    remove(root, k);
}

const Node* AVLTree :: rootCall() const {
    return root;
}