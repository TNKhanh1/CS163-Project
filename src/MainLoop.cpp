#include <raylib.h>
#include <raymath.h>
#include <cstring>
#include <iostream>
#include "MainLoop.h"

MainLoop::MainLoop()
{
    WindowHeight = 1000;
    WindowWidth = 1000;
    current_state = Menu; 
}

MainLoop::~MainLoop()
{
}

void MainLoop::run()
{
    InitWindow(WindowWidth, WindowHeight, "Data Structure Visualizer");
    
    SetTargetFPS(60);

    while (!WindowShouldClose()) 
    {
        // float deltaTime = GetFrameTime();
        
        // update(deltaTime);

        BeginDrawing();
        
        // ClearBackground(RAYWHITE); 

        // draw();

        EndDrawing();
    }

    CloseWindow();
}

void MainLoop::update(float delta_time)
{
}

void MainLoop::draw()
{
}

void MainLoop::BackState()
{
}

void MainLoop::NextState()
{
}