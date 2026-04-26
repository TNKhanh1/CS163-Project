#include "MSTState.h"
#include <iostream>
#include <string>
#include <cmath>

MSTState::MSTState() : DataStructureState()
{
    NextState = (int)STATE_MST;

    activeInputFocus = -1;
    previousInputFocus = -1;

    currentAnimIndex = 0;
    currentAnimType = NONE;

    // Khởi tạo trạng thái ban đầu của mã giả
    animPhase = 0;
    activeCodeLine = -1;
    pseudoCode = {
        "// Select an algorithm from",
        "// the control panel to see",
        "// its execution step-by-step."
    };
}

MSTState::~MSTState()
{
    currentGraph.clear();
    pseudoCode.clear();
}

void MSTState::loadAssets()
{
    DataStructureState::loadAssets();
}

void MSTState::updateNodePositions()
{
    nodePositions.clear();

    const auto& vertices = currentGraph.getVertices();
    if (vertices.empty())
    {
        return;
    }

    int numNodes = vertices.size();
    float angleStep = 2.0f * PI / numNodes;
    float angle = 0.0f;

    Vector2 center = {1800.0f / 2.0f - 150.0f, 1000.0f / 2.0f - 50.0f};
    float radius = 300.0f * zoomMultiplier;

    nodePositions.clear();
    for (int v : vertices)
    {
        nodePositions[v] =
        {
            static_cast<float>(center.x + cos(angle) * radius),
            static_cast<float>(center.y + sin(angle) * radius)
        };
        angle += angleStep;
    }
}

void MSTState::update(float deltaTime)
{
    Vector2 mousePos = GetMousePosition();

    DataStructureState::updateSharedUI(deltaTime, mousePos);
    DataStructureState::updateControlPanel(deltaTime, mousePos);

    updateNodePositions();

    if (activeInputFocus != previousInputFocus)
    {
        cursorIndex = (activeInputFocus == -1) ? 0 : inputBuffers[activeInputFocus].length();
        textScrollX = 0.0f;
        cursorBlinkTimer = 0.0f;
        cursorVisible = true;
        previousInputFocus = activeInputFocus;
    }

    if (activeInputFocus != -1)
    {
        HandleTextInput(inputBuffers[activeInputFocus], false);
    }
}

void MSTState::saveState()
{
    history.push_back({currentAnimIndex, animPhase, activeCodeLine});
}

void MSTState::undoState()
{    
    if (!history.empty())
    {
        MSTSnapshot lastState = history.back();
        history.pop_back();

        currentAnimIndex = lastState.animIndex;
        animPhase = lastState.animPhase;
        activeCodeLine = lastState.activeCodeLine;
    }
}

void MSTState::onExecuteOp(MainOp op)
{
    try
    {
        switch (op)
        {
            case OP_SLOT1:
            case OP_SLOT2:
            {
                if (op == OP_SLOT2 && !inputBuffers[0].empty() && !inputBuffers[1].empty() && !inputBuffers[2].empty())
                {
                    int u = std::stoi(inputBuffers[0]);
                    int v = std::stoi(inputBuffers[1]);
                    int w = std::stoi(inputBuffers[2]);
                    currentGraph.insertEdge(u, v, w);
                    inputBuffers[0].clear();
                    inputBuffers[1].clear();
                    inputBuffers[2].clear();
                }

                isAnimating = false;
                currentAnimType = NONE;
                targetMST.clear();
                activeCodeLine = -1;
                break;
            }

            case OP_SLOT3: // KRUSKAL
            {
                targetMST = currentGraph.kruskal();
                currentAnimIndex = 0;
                currentAnimType = KRUSKAL;
                isAnimating = true;
                history.clear();
                animTimer = 0.0f;
                animPhase = 0;      // Reset phase
                activeCodeLine = 1;

                pseudoCode = {
                    "vector<Edge> kruskal() {",
                    "    sort(edges);",
                    "    for (Edge e : edges) {",
                    "        if (find(e.u) != find(e.v)) {",
                    "            union(e.u, e.v);",
                    "            mst.push_back(e);",
                    "        }",
                    "    }",
                    "    return mst;",
                    "}"
                };
                break;
            }

            case OP_SLOT4: // PRIM
            {
                if (!inputBuffers[3].empty())
                {
                    int startNode = std::stoi(inputBuffers[3]);
                    targetMST = currentGraph.prim(startNode);
                    currentAnimIndex = 0;
                    currentAnimType = PRIM;
                    isAnimating = true;
                    history.clear();
                    animTimer = 0.0f;
                    animPhase = 0;      // Reset phase
                    activeCodeLine = 1; 
                    inputBuffers[3].clear();

                    pseudoCode = {
                        "vector<Edge> prim(int start) {",
                        "    pq.push(edges_from(start));",
                        "    while (!pq.empty()) {",
                        "        Edge e = pq.pop();",
                        "        if (!visited(e.v)) {",
                        "            mst.push_back(e);",
                        "            visited(e.v) = true;",
                        "            pq.push(edges_from(e.v));",
                        "        }",
                        "    }",
                        "    return mst;",
                        "}"
                    };
                }
                break;
            }

            default:
            {
                break;
            }
        }
    }
    catch (...)
    {
        inputErrorMsg = "Invalid Input!";
        inputErrorTimer = 2.0f;
    }

    activeInputFocus = -1;
}

void MSTState::handleAnimationStep()
{
    if (currentAnimType == KRUSKAL)
    {
        if (currentAnimIndex < (int)targetMST.size())
        {
            // Chạy từng dòng code của Kruskal
            if (animPhase == 0) { activeCodeLine = 2; animPhase = 1; }      // Dòng: for (Edge e : edges)
            else if (animPhase == 1) { activeCodeLine = 3; animPhase = 2; } // Dòng: if (find != find)
            else if (animPhase == 2) { activeCodeLine = 4; animPhase = 3; } // Dòng: union
            else if (animPhase == 3) { 
                activeCodeLine = 5;                                         // Dòng: mst.push_back(e)
                currentAnimIndex++; // Thực sự vẽ cạnh lên đồ thị
                animPhase = 0;      // Quay lại vòng lặp cho cạnh tiếp theo
            }
        }
        else
        {
            activeCodeLine = 8; // Dòng: return mst;
            isAnimating = false;
        }
    }
    else if (currentAnimType == PRIM)
    {
        if (currentAnimIndex < (int)targetMST.size())
        {
            // Chạy từng dòng code của Prim
            if (animPhase == 0) { activeCodeLine = 2; animPhase = 1; }      // Dòng: while (!pq.empty())
            else if (animPhase == 1) { activeCodeLine = 3; animPhase = 2; } // Dòng: Edge e = pq.pop()
            else if (animPhase == 2) { activeCodeLine = 4; animPhase = 3; } // Dòng: if (!visited)
            else if (animPhase == 3) { 
                activeCodeLine = 5;                                         // Dòng: mst.push_back(e)
                currentAnimIndex++; // Thực sự vẽ cạnh lên đồ thị
                animPhase = 4; 
            }
            else if (animPhase == 4) { activeCodeLine = 6; animPhase = 5; } // Dòng: visited = true
            else if (animPhase == 5) { activeCodeLine = 7; animPhase = 0; } // Dòng: pq.push(edges)
        }
        else
        {
            activeCodeLine = 10; // Dòng: return mst;
            isAnimating = false;
        }
    }
    else
    {
        isAnimating = false;
        activeCodeLine = -1;
    }
}

void MSTState::DrawSubMenuContent()
{
    float mainHeight = 45.0f;
    float gap = 8.0f;
    float subX = controlBtnPos.x + (float)controlTex.width + 15.0f + 125.0f + gap;
    float startY = controlBtnPos.y;

    switch (activeMainOp)
    {
        case OP_SLOT1:
        {
            if (DrawButtonText({subX, startY}, "Empty", 90, mainHeight, false))
            {
                currentGraph.clear();
                onExecuteOp(OP_SLOT1);
            }
            if (DrawButtonText({subX + 98, startY}, "Random", 110, mainHeight, false))
            {
                currentGraph.generateRandomGraph(GetRandomValue(4, 8), GetRandomValue(5, 12));
                onExecuteOp(OP_SLOT1);
            }
            break;
        }

        case OP_SLOT2:
        {
            DrawLabel({subX, startY + mainHeight + gap}, "u=");
            if (DrawTextBox({subX + 40, startY + mainHeight + gap}, inputBuffers[0], activeInputFocus == 0, 60, mainHeight, cursorIndex, textScrollX, cursorVisible))
            {
                activeInputFocus = 0;
            }

            DrawLabel({subX + 110, startY + mainHeight + gap}, "v=");
            if (DrawTextBox({subX + 150, startY + mainHeight + gap}, inputBuffers[1], activeInputFocus == 1, 60, mainHeight, cursorIndex, textScrollX, cursorVisible))
            {
                activeInputFocus = 1;
            }

            DrawLabel({subX + 220, startY + mainHeight + gap}, "w=");
            if (DrawTextBox({subX + 260, startY + mainHeight + gap}, inputBuffers[2], activeInputFocus == 2, 60, mainHeight, cursorIndex, textScrollX, cursorVisible))
            {
                activeInputFocus = 2;
            }

            if (DrawButtonText({subX + 330, startY + mainHeight + gap}, "GO", 50, mainHeight, false))
            {
                onExecuteOp(OP_SLOT2);
            }
            break;
        }

        case OP_SLOT3:
        {
            if (DrawButtonText({subX, startY + 2 * (mainHeight + gap)}, "RUN KRUSKAL", 180, mainHeight, false))
            {
                onExecuteOp(OP_SLOT3);
            }
            break;
        }

        case OP_SLOT4:
        {
            DrawLabel({subX, startY + 3 * (mainHeight + gap)}, "Start =");
            if (DrawTextBox({subX + 90, startY + 3 * (mainHeight + gap)}, inputBuffers[3], activeInputFocus == 3, 70, mainHeight, cursorIndex, textScrollX, cursorVisible))
            {
                activeInputFocus = 3;
            }

            if (DrawButtonText({subX + 170, startY + 3 * (mainHeight + gap)}, "GO", 50, mainHeight, false))
            {
                onExecuteOp(OP_SLOT4);
            }
            break;
        }

        default:
        {
            break;
        }
    }
}

void MSTState::draw()
{
    DataStructureState::drawSharedUI();

    const char* titleText = "MINIMUM SPANNING TREE";
    DrawTextEx(listFont, titleText, {(1800.0f - MeasureTextEx(listFont, titleText, 55, 6.5f).x) / 2.0f, 20.0f}, 55, 6.5f, BLACK);

    float nodeRadius = 30.0f * zoomMultiplier;
    float lineThickness = 3.0f * zoomMultiplier;
    float fontSize = 24.0f * zoomMultiplier;

    std::map<std::pair<int, int>, int> pairCount;

    struct EdgeKey
    {
        int u, v, w;
        bool operator<(const EdgeKey& o) const
        {
            if (u != o.u) return u < o.u;
            if (v != o.v) return v < o.v;
            return w < o.w;
        }
    };

    std::map<EdgeKey, Vector2> edgeTextPosMap;

    for (const auto& edge : currentGraph.getEdges())
    {
        if (nodePositions.count(edge.u) && nodePositions.count(edge.v))
        {
            Vector2 p1 = nodePositions[edge.u];
            Vector2 p2 = nodePositions[edge.v];
            DrawLineEx(p1, p2, lineThickness, LIGHTGRAY);

            int minV = std::min(edge.u, edge.v);
            int maxV = std::max(edge.u, edge.v);
            int duplicateIndex = pairCount[{minV, maxV}]++;

            Vector2 startP = (edge.u < edge.v) ? p1 : p2;
            Vector2 endP = (edge.u < edge.v) ? p2 : p1;

            float dx = endP.x - startP.x;
            float dy = endP.y - startP.y;
            float length = sqrt(dx * dx + dy * dy);

            Vector2 textPos = {startP.x + dx * 0.3f, startP.y + dy * 0.3f};

            if (length > 0)
            {
                float nx = -dy / length;
                float ny = dx / length;

                float direction = (duplicateIndex % 2 == 0) ? 1.0f : -1.0f;
                float extraStep = (duplicateIndex / 2) * 25.0f;
                float totalOffset = (18.0f + extraStep) * zoomMultiplier * direction;

                textPos.x += nx * totalOffset;
                textPos.y += ny * totalOffset;
            }

            const char* wText = TextFormat("%d", edge.weight);
            Vector2 tSize = MeasureTextEx(numberFont, wText, fontSize, 1.0f);
            Vector2 finalPos = {textPos.x - tSize.x / 2.0f, textPos.y - tSize.y / 2.0f};

            edgeTextPosMap[{minV, maxV, edge.weight}] = finalPos;

            DrawTextEx(numberFont, wText, {finalPos.x + 1, finalPos.y + 1}, fontSize, 1.0f, RAYWHITE);
            DrawTextEx(numberFont, wText, finalPos, fontSize, 1.0f, BLACK);
        }
    }

    if (currentAnimType != NONE)
    {
        for (int i = 0; i < currentAnimIndex && i < targetMST.size(); i++)
        {
            const auto& edge = targetMST[i];
            if (nodePositions.count(edge.u) && nodePositions.count(edge.v))
            {
                Vector2 p1 = nodePositions[edge.u];
                Vector2 p2 = nodePositions[edge.v];
                DrawLineEx(p1, p2, lineThickness * 1.5f, ORANGE);

                int minV = std::min(edge.u, edge.v);
                int maxV = std::max(edge.u, edge.v);
                Vector2 finalPos = edgeTextPosMap[{minV, maxV, edge.weight}];

                const char* wText = TextFormat("%d", edge.weight);
                DrawTextEx(numberFont, wText, {finalPos.x + 1, finalPos.y + 1}, fontSize + 2.0f, 1.0f, RAYWHITE);
                DrawTextEx(numberFont, wText, finalPos, fontSize + 2.0f, 1.0f, RED);
            }
        }
    }

    for (const auto& pair : nodePositions)
    {
        int vertexId = pair.first;
        Vector2 pos = pair.second;

        Color nodeColor = SKYBLUE;
        Color borderColor = DARKBLUE;

        if (currentAnimType != NONE)
        {
            bool inMST = false;
            for (int i = 0; i < currentAnimIndex; i++)
            {
                if (targetMST[i].u == vertexId || targetMST[i].v == vertexId)
                {
                    inMST = true;
                    break;
                }
            }
            if (inMST)
            {
                nodeColor = GOLD;
                borderColor = ORANGE;
            }
        }

        DrawCircleV(pos, nodeRadius, nodeColor);
        DrawCircleLines((int)pos.x, (int)pos.y, nodeRadius, borderColor);

        const char* vText = TextFormat("%d", vertexId);
        Vector2 textSize = MeasureTextEx(numberFont, vText, fontSize, 1.0f);
        DrawTextEx(numberFont, vText, {pos.x - textSize.x / 2.0f, pos.y - textSize.y / 2.0f}, fontSize, 1.0f, BLACK);
    }

    float pcX = 1315.0f, pcY = 150.0f;
    float pcWidth = 450.0f, pcHeight = 450.0f;
    DrawRectangle(pcX - 10, pcY - 10, pcWidth + 40, pcHeight, Fade(LIGHTGRAY, 0.6f));
    DrawRectangleLines(pcX - 10, pcY - 10, pcWidth + 40, pcHeight, DARKGRAY);
    DrawTextEx(listFont, "Source Code:", {pcX, pcY}, 25.0f, 1.0f, DARKBLUE);
    
    float lineHeight = 28.0f;
    float textPadding = 15.0f;
    for (int i = 0; i < (int)pseudoCode.size(); i++) {
        Color textCol = BLACK;
        if (i == activeCodeLine) {
            DrawRectangle(pcX, pcY + 40.0f + i * lineHeight - 2.0f, pcWidth, lineHeight - 2.0f, Fade(YELLOW, 0.5f));
            textCol = RED;
        }
        
        std::string line = pseudoCode[i];
        if (MeasureTextEx(numberFont, line.c_str(), 18.0f, 1.0f).x > 520.0f) {
            size_t spacePos = line.find_last_of(' ', 60);
            if (spacePos != std::string::npos) {
                std::string first = line.substr(0, spacePos);
                std::string second = line.substr(spacePos + 1);
                DrawTextEx(numberFont, first.c_str(), {pcX + textPadding, pcY + 40.0f + i * lineHeight}, 18.0f, 1.0f, textCol);
                DrawTextEx(numberFont, second.c_str(), {pcX + textPadding, pcY + 40.0f + (i + 0.5f) * lineHeight}, 18.0f, 1.0f, textCol);
            } else {
                DrawTextEx(numberFont, line.c_str(), {pcX + textPadding, pcY + 40.0f + i * lineHeight}, 18.0f, 1.0f, textCol);
            }
        } else {
            DrawTextEx(numberFont, line.c_str(), {pcX + textPadding, pcY + 40.0f + i * lineHeight}, 18.0f, 1.0f, textCol);
        }
    }
    
    DrawTextureV(controlTex, controlBtnPos, WHITE);
    DrawSideMenuFrame({"Create", "Insert", "Kruskal", "Prim"});
}