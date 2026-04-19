#include "AVLTreeState.h"

AVLTreeState::AVLTreeState() : DataStructureState()
{
    NextState = (int)STATE_AVLTREE;
}

AVLTreeState::~AVLTreeState()
{
}

void AVLTreeState::loadAssets()
{
    DataStructureState::loadAssets(); 
    
}

void AVLTreeState::update(float deltaTime)
{
    Vector2 mousePos = GetMousePosition();
    
    DataStructureState::updateSharedUI(deltaTime, mousePos);

    DataStructureState::updateControlPanel(deltaTime, mousePos);
    
    if (activeMainOp != OP_NONE) {
        HandleTextInput(inputBuffer, false); // false = single integer, no commas
    }

    // 2. Handle Text Cursor Blinking
    cursorBlinkTimer += deltaTime;
    if (cursorBlinkTimer >= 0.5f) {
        cursorVisible = !cursorVisible;
        cursorBlinkTimer = 0.0f;
    }

    updateNodePositions(const_cast<Node*>(avl.rootCall()), deltaTime);

    if (currentTask == TASK_HIGHLIGHT_NEW) {
        animTimer -= deltaTime; // Tick down the clock
        
        if (animTimer <= 0.0f) {
            // Time is up! Reset the colors and clear the task.
            resetNodeColors(const_cast<Node*>(avl.rootCall()));
            currentTask = TASK_NONE;
        }
    }
}

void AVLTreeState::draw()
{
    DataStructureState::drawSharedUI();

    const char* titleText = "AVL TREE";
    Vector2 titleSize = MeasureTextEx(listFont, titleText, 55, 6.5f);
    DrawTextEx(listFont, titleText, { (1800.0f - titleSize.x) / 2.0f, 20.0f }, 55, 6.5f, BLACK);

    drawNode(avl.rootCall());
}

void AVLTreeState::DrawSubMenuContent()
{
    if (activeMainOp == OP_NONE) return;

    // Calculate base positions relative to the main control panel
    float subX = controlBtnPos.x + 320.0f; // Offset to the right of the main buttons
    float startY = controlBtnPos.y;        // Align with the top button
    float mainHeight = 45.0f;
    float gap = 8.0f;                      // Gap between menu items

    switch (activeMainOp) {
        case OP_SLOT1: // Insert
            DrawLabel({subX, startY}, "Value=");
            
            // Draw the text box
            DrawTextBox({subX + 80, startY}, inputBuffer, true, 100, mainHeight, cursorIndex, textScrollX, cursorVisible);
            
            // Draw the "GO" button. If clicked, manually trigger onExecuteOp!
            if (DrawButtonText({subX + 190, startY}, "GO", 50, mainHeight, false)) {
                onExecuteOp(OP_SLOT1);
            }
            break;

        case OP_SLOT2: // Delete
            // Shift the Y position down for the 2nd slot
            DrawLabel({subX, startY + mainHeight + gap}, "Value=");
            
            DrawTextBox({subX + 80, startY + mainHeight + gap}, inputBuffer, true, 100, mainHeight, cursorIndex, textScrollX, cursorVisible);
            
            if (DrawButtonText({subX + 190, startY + mainHeight + gap}, "GO", 50, mainHeight, false)) {
                onExecuteOp(OP_SLOT2);
            }
            break;

        case OP_SLOT3: // Search
            // Shift the Y position down for the 3rd slot
            DrawLabel({subX, startY + 2 * (mainHeight + gap)}, "Value=");
            
            DrawTextBox({subX + 80, startY + 2 * (mainHeight + gap)}, inputBuffer, true, 100, mainHeight, cursorIndex, textScrollX, cursorVisible);
            
            if (DrawButtonText({subX + 190, startY + 2 * (mainHeight + gap)}, "GO", 50, mainHeight, false)) {
                onExecuteOp(OP_SLOT3);
            }
            break;
            
        default:
            break;
    }
}

void AVLTreeState::onExecuteOp(MainOp op)
{
    if (inputBuffer.empty()) {
        currentErrorSlot = (int)op - 1;
        inputErrorMsg = "Value cannot be empty!";
        inputErrorTimer = 2.5f;
        return;
    }

    try {
        int value = std::stoi(inputBuffer);
        
        // Reset colors before starting a new operation!
        resetNodeColors(const_cast<Node*>(avl.rootCall()));

        if (op == OP_SLOT1) { // OP_INSERT
            avl.insert(value);
            
            // 1. Find the node we just inserted
            const Node* newObj = avl.search(value);
            if (newObj != nullptr) {
                // 2. Color it GREEN so it stands out!
                const_cast<Node*>(newObj)->color = GREEN;
                
                // 3. Start the animation timer for 1.5 seconds
                currentTask = TASK_HIGHLIGHT_NEW;
                animTimer = 1.5f; 
            }
        }
        else if (op == OP_SLOT2) {
            avl.delNode(value);
        }
        else if (op == OP_SLOT3) {
            const Node* foundNode = avl.search(value);
            if (foundNode != nullptr) {
                // Instantly highlight the found node GREEN for now
                const_cast<Node*>(foundNode)->color = GREEN; 
            } else {
                currentErrorSlot = (int)op - 1;
                inputErrorMsg = "Value not found!";
                inputErrorTimer = 2.5f;
            }
        }

        // Clear the text box on success
        inputBuffer = "";
        cursorIndex = 0;
        textScrollX = 0.0f;

    } catch (...) {
        currentErrorSlot = (int)op - 1;
        inputErrorMsg = "Invalid Number!";
        inputErrorTimer = 2.5f;
    }
}

void AVLTreeState::resetNodeColors(Node* node) {
    if (node == nullptr) return;
    
    // Reset to default color (using SKYBLUE to match LinkedList)
    node->color = SKYBLUE;
    
    resetNodeColors(node->left);
    resetNodeColors(node->right);
}

void AVLTreeState::updateNodePositions(Node* node, float deltaTime) {
    if (node == nullptr) return;

    node->position = Vector2Lerp(node->position, node->targetPosition, 10.0f * deltaTime);

    updateNodePositions(node->left, deltaTime);
    updateNodePositions(node->right, deltaTime);
}

void AVLTreeState::drawNode(const Node* node) {
    if (node == nullptr) return;

    if (node->left != nullptr) {
        DrawLineEx(node->position, node->left->position, 3.0f, DARKGRAY);
    }
    if (node->right != nullptr) {
        DrawLineEx(node->position, node->right->position, 3.0f, DARKGRAY);
    }

    drawNode(node->left);
    drawNode(node->right);

    DrawCircleV(node->position, 20.0f, node->color);
    DrawCircleLines(node->position.x, node->position.y, 20.0f, BLUE);

    const char* text = TextFormat("%d", node->key);
    int fontSize = 20;
    int textWidth = MeasureText(text, fontSize);
    
    DrawText(text, node->position.x - (textWidth / 2.0f), node->position.y - (fontSize / 2.0f), fontSize, BLACK);
}

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