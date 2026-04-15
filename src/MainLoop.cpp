#include <MainLoop.h>

MainLoop::MainLoop()
{
    WindowWidth = 1800;
    WindowHeight = 1000;
    current_state = STATE_INTRO; 
}

MainLoop::~MainLoop()
{
    
}

void MainLoop::LoadAssets()
{
    int fontSize = 100;
    Font loadingFont = LoadFontEx("assets/FONT2.ttf", fontSize, 0, 0);
    SetTextureFilter(loadingFont.texture, TEXTURE_FILTER_BILINEAR);

    BeginDrawing();
    ClearBackground(RAYWHITE); 
    const char* text = "LOADING.....";
    float spacing = 5.0f;
    Vector2 textSize = MeasureTextEx(loadingFont, text, fontSize, spacing);
    Vector2 textPos = {
        (float)WindowWidth / 2.0f - textSize.x / 2.0f - 20,
        (float)WindowHeight / 2.0f - textSize.y / 2.0f - 20
    };
    DrawTextEx(loadingFont, text, textPos, fontSize, spacing, BLACK); 
    EndDrawing(); 

    // Nạp toàn bộ tài nguyên cho các state
    intro.loadAssets();
    menu.loadAssets();
    linkedList.loadAssets();
    avlTree.loadAssets();
    heap.loadAssets();
    mst.loadAssets();
    UnloadFont(loadingFont); 
}


void MainLoop::run()
{
    InitWindow(WindowWidth, WindowHeight, "Data Structures Visualizer"); 
    SetTargetFPS(60); 
    LoadAssets(); 
    
    float fadeInAlpha = 255.0f; 

    while (!WindowShouldClose()) 
    {
        float deltaTime = GetFrameTime();
        update(deltaTime); 

        BeginDrawing(); 
        draw(); 

        if (fadeInAlpha > 0.0f)
        {
            fadeInAlpha -= 500.0f * deltaTime;
            if (fadeInAlpha < 0.0f) fadeInAlpha = 0.0f;
            DrawRectangle(0, 0, WindowWidth, WindowHeight, { 0, 0, 0, (unsigned char)fadeInAlpha });
        }

        EndDrawing(); 
    }

    CloseWindow();
}

void MainLoop::update(float delta_time)
{
    if (current_state == STATE_INTRO)
    {
        intro.update(delta_time);
    }
    else if (current_state == STATE_MENU)
    {
        menu.update(delta_time);
    }
    else if (current_state == STATE_LINKEDLIST)
    {
        linkedList.update(delta_time); 
    }
    else if (current_state == STATE_AVLTREE)
    {
        avlTree.update(delta_time); 
    }
    else if (current_state == STATE_HEAP)
    {
        heap.update(delta_time); 
    }
    else if (current_state == STATE_MST) 
    {
         mst.update(delta_time); 
    }
}

void MainLoop::draw()
{
    if (current_state == STATE_INTRO)
    {
        intro.draw();
        current_state = intro.NextState;
        intro.NextState = STATE_INTRO;        
    }
    else if (current_state == STATE_MENU)
    {
        menu.draw();
        current_state = menu.NextState; 
        menu.NextState = STATE_MENU;          
    }
    else if (current_state == STATE_LINKEDLIST)
    {
        linkedList.draw();
        current_state = (State)linkedList.NextState;
        linkedList.NextState = (int)STATE_LINKEDLIST;     
    }
    else if (current_state == STATE_AVLTREE)
    {
        avlTree.draw();
        current_state = (State)avlTree.NextState;
        avlTree.NextState = (int)STATE_AVLTREE;     
        
    }
    else if (current_state == STATE_HEAP)
    {
        heap.draw();
        current_state = (State)heap.NextState;
        heap.NextState = (int)STATE_HEAP;     
    }
    else if (current_state == STATE_MST)
    {
        mst.draw();
        current_state = (State)mst.NextState;
        mst.NextState = (int)STATE_MST;     
    }
}

// Các chức năng dự phòng 
void MainLoop::BackState() {}
void MainLoop::NextState() {}   