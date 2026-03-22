#include <MainLoop.h>
MainLoop::MainLoop()
{
    WindowWidth = 1600;
    WindowHeight = 1000;
    current_state = Menu; 
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

    menu.loadAssets();

    UnloadFont(loadingFont);
}

void MainLoop::Intro()
{
    int fontSize = 100;
    Font introFont = LoadFontEx("assets/FONT.ttf", fontSize, 0, 0); 
    SetTextureFilter(introFont.texture, TEXTURE_FILTER_BILINEAR); 

    back_ground introBg("assets/backgroundintro01.jpg", {0.0f, 0.0f}, "assets/backgroundintro01.jpg", true);

    Texture2D bubbleTex = LoadTexture("assets/bubble.png");
    std::vector<Bubble> bubbles;
    float bubbleSpawnTimer = 0.0f;
    std::mt19937 rng(std::random_device{}());

    double startTime = GetTime();
    const char* text = "DATA STRUCTURES VISUALIZER";
    int textLength = strlen(text);
    
    double introDuration = 1.5;
    float spacing = 5.0f;

    Vector2 textSize = MeasureTextEx(introFont, text, fontSize, spacing);
    Vector2 startPos = { (float)WindowWidth / 2.0f - textSize.x / 2.0f - 20, 
                         (float)WindowHeight / 2.0f - textSize.y / 2.0f - 200 };

    button startBtn("assets/start01.png", "assets/start01.png", "assets/start01.png", "assets/start01.png", {0, 0}, 120);
    Vector2 startBtnPos = { (float)WindowWidth / 2.0f - 160.0f, (float)WindowHeight / 2.0f + 90.0f };
    startBtn.SetPosition(startBtnPos);

    Rectangle startBtnBounds = { startBtnPos.x, startBtnPos.y, 320.0f, 120.0f };

    bool isFadingOut = false;
    float transitionAlpha = 0.0f;

    while (!WindowShouldClose())
    {
        float deltaTime = GetFrameTime(); 
        double currentTime = GetTime() - startTime;
        Vector2 mousePos = GetMousePosition();

        int lettersToShow = (int)((currentTime / introDuration) * (textLength + 1));
        if (lettersToShow > textLength) lettersToShow = textLength;

        // --- 3. XỬ LÝ CLICK NÚT START ---
        bool isHoveringStart = false;
        
        // Chỉ cho phép thao tác với nút START nếu chưa bắt đầu Fade Out
        if (currentTime >= introDuration && !isFadingOut)
        {
            if (CheckCollisionPointRec(mousePos, startBtnBounds)) {
                isHoveringStart = true;
            }

            if (startBtn.isPressed(mousePos, IsMouseButtonPressed(MOUSE_BUTTON_LEFT)))
            {
                isFadingOut = true;
            }
        }

        if (isFadingOut)
        {
            transitionAlpha += 500.0f * deltaTime; 
            if (transitionAlpha >= 255.0f) {
                transitionAlpha = 255.0f;
                break;
            }
        }

       
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !isHoveringStart && !isFadingOut)
        {
            bool clickedBubble = false;
            bool pre_sign = 0;

            for (int i = bubbles.size() - 1; i >= 0; i--)
            {
                Bubble& b = bubbles[i];
                float radius = (bubbleTex.width * b.scale) / 2.0f;
                Vector2 center = { b.position.x + radius, b.position.y + radius };

                if (CheckCollisionPointCircle(mousePos, center, radius))
                {
                    clickedBubble = true;
                    float diameter = radius * 2.0f;

                    if (diameter > 65.0f) 
                    {
                        int numSmallBubbles = GetRandomValue(1, 4) + 1;
                        for (int j = 0; j < numSmallBubbles; j++)
                        {
                            Bubble smallB;
                            std::uniform_real_distribution<float> Raito(0.3f, 0.7f);
                            smallB.scale = b.scale * Raito(rng); 
                            
                            std::uniform_real_distribution<float> offsetDist(-30.0f, 30.0f);
                            std::uniform_real_distribution<float> Delta(5.0f, 30.0f);
                            float dx = offsetDist(rng);
                            if(pre_sign == 1 && dx >= 0) dx *= -1;
                            else if(pre_sign == 0 && dx < 0) dx *= -1;
                            if(dx >= 0) pre_sign = 1;
                            else pre_sign = 0;
                            if(dx < 0) dx -= Delta(rng);
                            else dx += Delta(rng);
                            smallB.position.x = center.x + dx - (bubbleTex.width * smallB.scale) / 2.0f;
                            dx = offsetDist(rng);
                            if(dx < 0) dx -= 8.0f;
                            else dx += 8.0f;
                            smallB.position.y = center.y + dx - (bubbleTex.height * smallB.scale) / 2.0f;
                            
                            // Bóng nhỏ bay nhanh hơn một chút
                            std::uniform_real_distribution<float> speedDist(50.0f, 100.0f);
                            smallB.speed = speedDist(rng);
                            
                            std::uniform_real_distribution<float> wobbleDist(0.0f, 3.141f);
                            smallB.wobbleOffset = wobbleDist(rng);
                            
                            bubbles.push_back(smallB);
                        }
                    }
                    bubbles.erase(bubbles.begin() + i);
                    break; 
                }
            }

            if (!clickedBubble)
            {
                Bubble newB;
                std::uniform_real_distribution<float> scaleDist(0.1f, 0.33f);
                newB.scale = scaleDist(rng);
                float radius = (bubbleTex.width * newB.scale) / 2.0f;
                
                newB.position.x = mousePos.x - radius;
                newB.position.y = mousePos.y - radius;
                
                std::uniform_real_distribution<float> speedDist(35.0f, 100.0f);
                newB.speed = speedDist(rng);
                std::uniform_real_distribution<float> wobbleDist(0.0f, 3.141f);
                newB.wobbleOffset = wobbleDist(rng);
                
                bubbles.push_back(newB);
            }
        }

        bubbleSpawnTimer += deltaTime;
        if (bubbleSpawnTimer > 0.25f) 
        {
            bubbleSpawnTimer = 0.0f;
            std::uniform_int_distribution<int> chanceDist(1, 100);
            if (chanceDist(rng) <= 11) 
            {
                Bubble b;
                std::uniform_real_distribution<float> scaleDist(0.1f, 0.36f);
                b.scale = scaleDist(rng); 
                
                std::uniform_real_distribution<float> xDist(0.0f, 1600.0f);
                b.position.x = xDist(rng);
                
                if(b.position.x > 600.0f && b.position.x < 800.0f) b.position.x -= 180.0f;
                else if(b.position.x > 800.0f && b.position.x < 1000.0f) b.position.x += 180.0f;
                
                b.position.y = 1000.0f; 
                
                std::uniform_real_distribution<float> speedDist(35.0f, 100.0f);
                b.speed = speedDist(rng); 
                
                std::uniform_real_distribution<float> wobbleDist(0.0f, 3.141f);
                b.wobbleOffset = wobbleDist(rng);
                bubbles.push_back(b);
            }
        }

        for (int i = 0; i < bubbles.size(); i++)
        {
            bubbles[i].position.y -= bubbles[i].speed * deltaTime;
            bubbles[i].position.x += sin(GetTime() * 2.0f + bubbles[i].wobbleOffset) * 0.15f;

            if (bubbles[i].position.y < -200.0f)
            {
                bubbles.erase(bubbles.begin() + i);
                i--; 
            }
        }

        BeginDrawing();
        ClearBackground(BLACK); 
        
        introBg.Draw(mousePos);

        for (const auto& b : bubbles)
        {
            DrawTextureEx(bubbleTex, b.position, 0.0f, b.scale, Fade(WHITE, 0.8f));
        }

        float currentX = startPos.x;
        for (int i = 0; i < lettersToShow; i++)
        {
            char charStr[2] = { text[i], '\0' };
            if(i < 16) DrawTextEx(introFont, charStr, {currentX, startPos.y}, fontSize, spacing, BLACK);
            else DrawTextEx(introFont, charStr, {currentX, startPos.y}, fontSize, spacing, BLUE);
            
            if(text[i] == ' ') currentX += MeasureTextEx(introFont, charStr, fontSize, spacing).x + spacing * 7;
            else currentX += MeasureTextEx(introFont, charStr, fontSize, spacing).x + spacing;
        }

        if (currentTime >= introDuration)
        {
            startBtn.Draw(mousePos);
        }

        if (transitionAlpha > 0.0f)
        {
            DrawRectangle(0, 0, WindowWidth, WindowHeight, { 0, 0, 0, (unsigned char)transitionAlpha });
        }

        EndDrawing();
    }

    UnloadTexture(bubbleTex); 
    UnloadFont(introFont);
}

void MainLoop::run()
{
    InitWindow(WindowWidth, WindowHeight, "Data Structures Visualizer");
    SetTargetFPS(60);
    LoadAssets();
    
    Intro();           
    
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
    if (current_state == Menu)
    {
        menu.update(delta_time);
    }
}
void MainLoop::draw()
{
    if(current_state == Menu)
    {
        menu.draw();
    }
}

void MainLoop::BackState()
{
}

void MainLoop::NextState()
{
}