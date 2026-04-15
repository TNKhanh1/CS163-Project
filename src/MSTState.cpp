#include "MSTState.h"
#include <iostream>
#include <string>

MSTState::MSTState() : DataStructureState()
{
    NextState = (int)STATE_MST;

    activeInputFocus = -1;
    previousInputFocus = -1;
}

MSTState::~MSTState()
{
    currentGraph.clear();
}

void MSTState::loadAssets()
{
    DataStructureState::loadAssets();
}

void MSTState::update(float deltaTime)
{
    Vector2 mousePos = GetMousePosition();

    DataStructureState::updateSharedUI(deltaTime, mousePos);
    DataStructureState::updateControlPanel(deltaTime, mousePos);

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

void MSTState::onExecuteOp(MainOp op)
{
    try
    {
        switch (op)
        {
            case OP_SLOT1:
            {
                break;
            }

            case OP_SLOT2:
            {
                if (!inputBuffers[0].empty() && !inputBuffers[1].empty() && !inputBuffers[2].empty())
                {
                    int u = std::stoi(inputBuffers[0]);
                    int v = std::stoi(inputBuffers[1]);
                    int w = std::stoi(inputBuffers[2]);

                    currentGraph.insertEdge(u, v, w);

                    inputBuffers[0].clear();
                    inputBuffers[1].clear();
                    inputBuffers[2].clear();
                }
                break;
            }

            case OP_SLOT3:
            {
                currentGraph.kruskal();
                break;
            }

            case OP_SLOT4:
            {
                if (!inputBuffers[3].empty())
                {
                    int startNode = std::stoi(inputBuffers[3]);
                    currentGraph.prim(startNode);
                    inputBuffers[3].clear();
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

void MSTState::DrawSubMenuContent()
{
    float mainHeight = 45.0f, gap = 8.0f;
    
    float subX = controlBtnPos.x + (float)controlTex.width + 15.0f + 125.0f + gap;
    float startY = controlBtnPos.y;

    switch (activeMainOp) 
    {
        case OP_SLOT1: // Create
            if (DrawButtonText({subX, startY}, "Empty", 90, mainHeight, false)) 
            {
                currentGraph.clear();
            }
            if (DrawButtonText({subX + 98, startY}, "Random", 110, mainHeight, false))
            {
                currentGraph.generateRandomGraph(GetRandomValue(4, 8), GetRandomValue(5, 12));
            }
            break;

        case OP_SLOT2: // Insert
            DrawLabel({subX, startY + mainHeight + gap}, "u=");
            if (DrawTextBox({subX + 40, startY + mainHeight + gap}, inputBuffers[0], activeInputFocus == 0, 60, mainHeight, cursorIndex, textScrollX, cursorVisible)) activeInputFocus = 0;
            
            DrawLabel({subX + 110, startY + mainHeight + gap}, "v=");
            if (DrawTextBox({subX + 150, startY + mainHeight + gap}, inputBuffers[1], activeInputFocus == 1, 60, mainHeight, cursorIndex, textScrollX, cursorVisible)) activeInputFocus = 1;

            DrawLabel({subX + 220, startY + mainHeight + gap}, "w=");
            if (DrawTextBox({subX + 260, startY + mainHeight + gap}, inputBuffers[2], activeInputFocus == 2, 60, mainHeight, cursorIndex, textScrollX, cursorVisible)) activeInputFocus = 2;
            
            if (DrawButtonText({subX + 330, startY + mainHeight + gap}, "GO", 50, mainHeight, false)) onExecuteOp(OP_SLOT2);
            break;

        case OP_SLOT3: // Kruskal's
            if (DrawButtonText({subX, startY + 2 * (mainHeight + gap)}, "RUN KRUSKAL", 180, mainHeight, false)) 
            {
                onExecuteOp(OP_SLOT3);
            }
            break;

        case OP_SLOT4: // Prim's
            DrawLabel({subX, startY + 3 * (mainHeight + gap)}, "Start =");
            if (DrawTextBox({subX + 90, startY + 3 * (mainHeight + gap)}, inputBuffers[3], activeInputFocus == 3, 70, mainHeight, cursorIndex, textScrollX, cursorVisible)) activeInputFocus = 3;
            
            if (DrawButtonText({subX + 170, startY + 3 * (mainHeight + gap)}, "GO", 50, mainHeight, false)) onExecuteOp(OP_SLOT4);
            break;

        default: 
            break;
    }
}

void MSTState::draw()
{
    DataStructureState::drawSharedUI();

    const char* titleText = "MINIMUM SPANNING TREE";

    DrawTextEx(
        listFont,
        titleText,
        {(1800.0f - MeasureTextEx(listFont, titleText, 55, 6.5f).x) / 2.0f, 20.0f},
        55,
        6.5f,
        BLACK
    );


    DrawTextureV(controlTex, controlBtnPos, WHITE);

    DrawSideMenuFrame({"Create", "Insert", "Kruskal", "Prim"});
}