#include <MainLoop.h>

MainLoop::MainLoop()
{
    WindowWidth = 1800;
    WindowHeight = 1000;
    current_state = STATE_INTRO; 
    isMusicOn = true;
    musicBtnBounds = { (float)WindowWidth - 180.0f, 20.0f, 150.0f, 40.0f };
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
    InitAudioDevice();
    SetTargetFPS(60); 
    LoadAssets(); 
    bgMusic = LoadMusicStream("assets/background_music.mp3");
    SetMusicVolume(bgMusic, 0.7f);
    PlayMusicStream(bgMusic);
    
    float fadeInAlpha = 255.0f; 

    while (!WindowShouldClose()) 
    {
        float deltaTime = GetFrameTime();
        UpdateMusicStream(bgMusic);
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
    UnloadMusicStream(bgMusic);
    CloseAudioDevice();
    CloseWindow();
}

void MainLoop::update(float delta_time)
{

    bool isHovering = CheckCollisionPointRec(GetMousePosition(), musicBtnBounds);

    if (isHovering)
    {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            isMusicOn = !isMusicOn;
            
            if (isMusicOn)
            {
                ResumeMusicStream(bgMusic);
            }
            else
            {
                PauseMusicStream(bgMusic);
            }
        }
    }


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

    if (current_state == STATE_INTRO)
    {
        intro.draw();
        current_state = intro.NextState;
        intro.NextState = STATE_INTRO;        
    }

    

    bool isHovering = CheckCollisionPointRec(GetMousePosition(), musicBtnBounds);
    
    Color baseColor;
    if (isMusicOn)
    {
        baseColor = DARKGREEN;
    }
    else
    {
        baseColor = MAROON;
    }
    
    Color btnColor;
    if (isHovering)
    {
        btnColor = Fade(baseColor, 0.65f);
    }
    else
    {
        btnColor = baseColor;
    }

    DrawRectangleRounded(musicBtnBounds, 0.2f, 10, btnColor);
    DrawRectangleRoundedLines(musicBtnBounds, 0.2f, 10, BLACK);
    
    const char* text;
    if (isMusicOn)
    {
        text = "Music: ON";
    }
    else
    {
        text = "Music: OFF";
    }

    int fontSize = 20;
    int textWidth = MeasureText(text, fontSize);
    
    Vector2 textPos = {
        musicBtnBounds.x + (musicBtnBounds.width - textWidth) / 2.0f,
        musicBtnBounds.y + (musicBtnBounds.height - fontSize) / 2.0f
    };
    
    DrawText(text, textPos.x, textPos.y, fontSize, WHITE);


}


// Các chức năng dự phòng 
void MainLoop::BackState() {}
void MainLoop::NextState() {}   