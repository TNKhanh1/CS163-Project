# Data Structure Visualization Project

A graphical application for visualizing and interacting with fundamental data structures. Built with Raylib for high-performance 2D rendering.

---

## Table of Contents

1. [Getting Started](#getting-started)
2. [Application Overview](#application-overview)
3. [Linked List](#linked-list)
4. [AVL Tree](#avl-tree)
5. [Binary Heap](#binary-heap)
6. [Minimum Spanning Tree (MST)](#minimum-spanning-tree-mst)
7. [Controls & Shortcuts](#controls--shortcuts)

---

## Getting Started

### Running the Application

1. Navigate to the project folder
2. Double-click **App.exe** to launch the application
3. The application window will open with the intro screen

### Building from Source (Optional)

If you need to rebuild the application:

```bash
g++ -g src/main.cpp src/UI.cpp src/MainLoop.cpp src/DataStructureState.cpp src/IntroState.cpp src/MenuState.cpp src/LinkedListState.cpp src/AVLTree.cpp src/AVLTreeState.cpp src/HeapState.cpp src/Heap.cpp src/MSTState.cpp src/MST.cpp -o App.exe -O2 -static -Iinclude -Llib -lraylib -lopengl32 -lgdi32 -lwinmm -Wl,--subsystem,windows
```

---

## Application Overview

The application provides an interactive visualization environment with:

- **Main Menu**: Access to four different data structures
- **Visual Canvas**: Real-time animation of data structure operations — pannable and zoomable, with the option to reset the view just in case
- **Control Panel**: Buttons for Create, Insert, Search, Delete operations
- **Code View**: Shows the current operation being executed
- **Step-by-Step Mode**: Manual or automatic animation control

### Navigation

- **ESC**: Return to previous menu
- **SPACE**: Pause/Resume animation
- **Arrow Keys**: Navigate between options

---

## Linked List

A linear data structure where elements are stored in nodes, with each node pointing to the next.

### Features

| Operation | Description |
|-----------|-------------|
| **Create** | Initialize a new linked list with optional initial values |
| **Insert** | Add a new node at a specific position or at the end |
| **Search** | Find a value in the list and highlight its path |
| **Delete** | Remove a node at a specific position |

### How to Use

1. Select **Linked List** from the main menu
2. Use the submenu buttons to perform operations:
   - **Create**: Enter a comma-separated list of values (e.g., `10,20,30`)
   - **Insert**: Enter position index and value
   - **Search**: Enter a value to search for
   - **Delete**: Enter the position index to remove

### Visual Indicators

- **Blue nodes**: Normal state
- **Green highlight**: Currently processing
- **Red highlight**: Search target found

---

## AVL Tree

A self-balancing binary search tree where the height difference between left and right subtrees is at most 1 for every node.

### Features

| Operation | Description |
|-----------|-------------|
| **Insert** | Add a value with automatic rebalancing (rotations) |
| **Search** | Find a value and visualize the search path |
| **Delete** | Remove a value with rebalancing |

### How to Use

1. Select **AVL Tree** from the main menu
2. Use the control panel:
   - **Insert**: Enter a value to add to the tree
   - **Search**: Enter a value to find
   - **Delete**: Enter a value to remove

### Animation Modes

- **Auto Mode**: Complete animation plays automatically
- **Manual Mode**: Press SPACE to advance each step

### Visual Indicators

- **Node colors**: Indicate balance factor
- **Rotations**: Animated left/right rotations shown step-by-step
- **Code highlight**: Shows current operation in pseudocode

---

## Binary Heap

A complete binary tree that satisfies the heap property: parent node is always **smaller** than its children (min-heap). The smallest element is always at the root.

### Features

| Operation | Description |
|-----------|-------------|
| **Insert** | Add a value and heapify up to maintain heap property |
| **Search** | Find a value in the heap |
| **Extract** | Remove and return the root value, then heapify |
| **Update** | Modify a value and rebalance the heap |

### How to Use

1. Select **Heap** from the main menu
2. Operations available:
   - **Insert**: Enter a value to add
   - **Search**: Enter a value to find
   - **Extract**: Remove the root element
   - **Update**: Change a value at specific index

### Visual Indicators

- Tree is displayed with root (minimum value) at top, children below
- Array representation shown below the tree visualization
- Swapping operations are animated
- **Root element**: Smallest value, displayed at the top

---

## Minimum Spanning Tree (MST)

Visualizes graph algorithms (Kruskal's and Prim's) for finding the minimum spanning tree of a weighted graph.

### Features

| Algorithm | Description |
|-----------|-------------|
| **Kruskal's** | Edge-based algorithm using Union-Find |
| **Prim's** | Vertex-based algorithm growing MST from a source |

### How to Use

1. Select **MST** from the main menu
2. Choose algorithm: **Kruskal** or **Prim**
3. Add nodes and edges:
   - **Add Node**: Click on canvas to add vertex
   - **Add Edge**: Select two nodes to connect with weight
4. Click **Run** to visualize the algorithm

### Input File Format

You can also load a graph from a text file with the following format:

```
0 1 4
0 2 3
1 2 1
1 3 2
2 3 4
3 4 2
4 5 6
```

Where:
- lines: `source destination weight`

### Visual Indicators

- **White nodes**: Unvisited
- **Yellow nodes**: Part of MST
- **Green edges**: Included in MST
- **Red edges**: Currently being processed

---

## Controls & Shortcuts

| Key | Action |
|-----|--------|
| **ESC** | Go back / Cancel operation |
| **SPACE** | Pause/Resume animation |
| **ENTER** | Confirm input |
| **BACKSPACE** | Delete last character |
| **1-9** | Quick select operations |

### Mouse Controls

- **Left Click**: Select buttons, nodes, or confirm
- **Right Click**: Cancel selection
- **Right/Middle Hold + Drag**: Pan the visualization canvas

---

## Troubleshooting

### Application Won't Start

- Ensure **App.exe** is in the project root folder
- Verify all required DLL files are present
- Try running as Administrator

### Visual Glitches

- Window resize may require restart
- Ensure display scaling is set to 100%

### Performance Issues

- Close other heavy applications
- Reduce window size for better performance

---

## Project Structure

```
Project/
├── App.exe              # Compiled executable
├── data.txt             # Sample data file
├── include/             # Header files
│   ├── LinkedListState.h
│   ├── AVLTreeState.h
│   ├── HeapState.h
│   └── MSTState.h
├── src/                 # Source files
│   ├── main.cpp
│   ├── UI.cpp
│   └── ...
└── lib/                 # Raylib library
```

---

## Credits

Built with [Raylib](https://www.raylib.com/) - A simple and easy-to-use library to enjoy video game programming.

---

*For questions or issues, please refer to the source code comments or your course instructor.*

---