#include <MainLoop.h>

MainLoop::MainLoop()
{
    WindowWidth = 1800;
    WindowHeight = 1000;
    current_state = Intro; 
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

    UnloadFont(loadingFont); 
}


void MainLoop::run()
{
    InitWindow(WindowWidth, WindowHeight, "Data Structures Visualizer"); 
    SetTargetFPS(60); 
    LoadAssets(); 
    
    // Đã xóa hàm Intro() chặn luồng ở đây
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
    if (current_state == Intro)
    {
        intro.update(delta_time);
    }
    else if (current_state == Menu)
    {
        menu.update(delta_time);
    }
    else if (current_state == LinkedList)
    {
        linkedList.update(delta_time); 
    }
}

void MainLoop::draw()
{
    if (current_state == Intro)
    {
        intro.draw();
        current_state = intro.NextState;
        intro.NextState = Intro;        
    }
    else if (current_state == Menu)
    {
        menu.draw();
        current_state = menu.NextState; 
        menu.NextState = Menu;          
    }
    else if (current_state == LinkedList)
    {
        linkedList.draw();
        current_state = (State)linkedList.NextState;
        linkedList.NextState = (int)LinkedList;     
    }
}

// Các chức năng dự phòng
void MainLoop::BackState() {}
void MainLoop::NextState() {}