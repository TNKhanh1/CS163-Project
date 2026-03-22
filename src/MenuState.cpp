#include "MenuState.h"
#include <cstddef>
#include <math.h> 
GradientButton::GradientButton(Rectangle bounds, std::string text)
{
    this->bounds = bounds;
    this->text = text;
    this->hoverProgress = 0.0f; 
}

Color GradientButton::LerpColor(Color c1, Color c2, float t)
{
    return {
        (unsigned char)(c1.r + (c2.r - c1.r) * t),
        (unsigned char)(c1.g + (c2.g - c1.g) * t),
        (unsigned char)(c1.b + (c2.b - c1.b) * t),
        255
    };
}

void GradientButton::update(float deltaTime)
{
    Vector2 mousePos = GetMousePosition();
    
    if (CheckCollisionPointRec(mousePos, bounds))
    {
        hoverProgress += deltaTime / 0.4f; 
        if (hoverProgress > 1.0f) hoverProgress = 1.0f;
    }
    else
    {
        hoverProgress -= deltaTime / 0.4f;
        if (hoverProgress < 0.0f) hoverProgress = 0.0f;
    }
}

bool GradientButton::isClicked()
{
    Vector2 mousePos = GetMousePosition();
    return CheckCollisionPointRec(mousePos, bounds) && IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
}

void GradientButton::draw(Font font)
{
    Color greenColor = { 52, 143, 80, 255 };  //#348F50
    Color blueColor  = { 86, 180, 211, 255 }; //#56B4D3

    float r = 15.0f; 
    float w = bounds.width;
    float h = bounds.height;
    float x = bounds.x;
    float y = bounds.y;

    auto GetGradColor = [&](float nx) -> Color {
        float gx = nx + hoverProgress; 
        if (gx <= 1.0f) return LerpColor(greenColor, blueColor, gx);
        else return LerpColor(blueColor, greenColor, gx - 1.0f);
    };

    float nx1 = r / w;
    float nx2 = (w - r) / w;

    Color colorLeft  = GetGradColor(0.0f); 
    Color colorRight = GetGradColor(1.0f); 
    Color colorR1    = GetGradColor(nx1);  
    Color colorR2    = GetGradColor(nx2);  

    Rectangle shadowRec = { x - 5, y - 5, w + 10, h + 10 };
    DrawRectangleRounded(shadowRec, 0.5f, 20, { 200, 200, 200, 100 });

    int midX = (int)(x + r);
    int midY = (int)y;
    int midW = (int)(w - 2 * r);
    int midH = (int)h;

    float peakNx = 1.0f - hoverProgress; 

    if (peakNx > nx1 && peakNx < nx2) {
        int part1W = (int)((peakNx - nx1) * w);
        int part2W = midW - part1W; 
        
        DrawRectangleGradientH(midX, midY, part1W, midH, colorR1, blueColor);
        DrawRectangleGradientH(midX + part1W - 1, midY, part2W + 1, midH, blueColor, colorR2);
    } else {
        DrawRectangleGradientH(midX, midY, midW, midH, colorR1, colorR2);
    }

    DrawRectangleGradientH((int)x, (int)(y + r), (int)r + 1, (int)(h - 2 * r), colorLeft, colorR1);
    DrawRectangleGradientH((int)(x + w - r) - 1, (int)(y + r), (int)r + 1, (int)(h - 2 * r), colorR2, colorRight);

    DrawCircleSector({ x + r, y + r }, r, 180, 270, 16, colorLeft);
    DrawCircleSector({ x + r, y + h - r }, r, 90, 180, 16, colorLeft);
    DrawCircleSector({ x + w - r, y + r }, r, 270, 360, 16, colorRight);
    DrawCircleSector({ x + w - r, y + h - r }, r, 0, 90, 16, colorRight);

    float fontSize = 30.0f;
    float spacing = 2.2f;
    Vector2 textSize = MeasureTextEx(font, text.c_str(), fontSize, spacing);
    Vector2 textPos = {
        x + (w - textSize.x) / 2.0f,
        y + (h - textSize.y) / 2.0f
    };
    
    DrawTextEx(font, text.c_str(), textPos, fontSize, spacing, WHITE);
}

MenuState::MenuState()
{
    bg = nullptr; 
    bubbleSpawnTimer = 0.0f;
}

MenuState::~MenuState()
{
    if (bg != nullptr) delete bg;
    UnloadFont(menuFont); 
    UnloadTexture(bubbleTex); 
}

void MenuState::loadAssets()
{
    bg = new back_ground("assets/background01.jpg", {0.0f, 0.0f}, "assets/background01.jpg", true);
    
    menuFont = LoadFontEx("assets/FONT2.ttf", 60, 0, 0);
    SetTextureFilter(menuFont.texture, TEXTURE_FILTER_BILINEAR);

    bubbleTex = LoadTexture("assets/bubble.png");

    float btnWidth = 420.0f;
    float btnHeight = 100.0f;
    float startX = (1600.0f - 420.0f) / 2 - 20; 
    float startY = 280.0f; 
    float gap = 160.0f;    

    buttons.push_back(GradientButton({startX, startY, btnWidth, btnHeight}, "LINKED LIST"));
    buttons.push_back(GradientButton({startX, startY + gap, btnWidth, btnHeight}, "HEAP"));
    buttons.push_back(GradientButton({startX, startY + gap * 2, btnWidth, btnHeight}, "AVL TREE"));
    buttons.push_back(GradientButton({startX, startY + gap * 3, btnWidth, btnHeight}, "MINIMUM SPANNING TREE"));
}

void MenuState::update(float deltaTime)
{
    bool isMouseOverButton = false;
    for (auto& btn : buttons)
    {
        btn.update(deltaTime);
        
        if (CheckCollisionPointRec(GetMousePosition(), btn.bounds)) {
            isMouseOverButton = true;
        }

        if (btn.isClicked())
        {
            if (btn.text == "LINKED LIST") {
            }
            if (btn.text == "HEAP") {
            }
            if (btn.text == "AVL TREE") {
            }
            if (btn.text == "MINIMUM SPANNING TREE") {
            }
        }
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !isMouseOverButton)
    {
        Vector2 mousePos = GetMousePosition();
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
                        if(dx < 0) dx -= 5.0f;
                        else dx += 5.0f;
                        smallB.position.y = center.y + dx - (bubbleTex.height * smallB.scale) / 2.0f;
                        

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
            std::uniform_real_distribution<float> scaleDist(0.1f, 0.36f);
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
        if (GetRandomValue(1, 100) <= 11) 
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
}
void MenuState::draw()
{

    if (bg != nullptr) bg->Draw(GetMousePosition());


    for (const auto& b : bubbles)
    {
        DrawTextureEx(bubbleTex, b.position, 0.0f, b.scale, Fade(WHITE, 0.8f));
    }


    const char* titleText = "DATA STRUCTURES VISUALIZER";
    float titleFontSize = 80.0f; 
    float titleSpacing = 5.0f;
    
    Vector2 titleSize = MeasureTextEx(menuFont, titleText, titleFontSize, titleSpacing);
    Vector2 titlePos = {
        1600.0f / 2.0f - titleSize.x / 2.0f, 
        100.0f                               
    };

    DrawTextEx(menuFont, titleText, {titlePos.x + 4, titlePos.y + 4}, titleFontSize, titleSpacing, Fade(WHITE, 0.5f));
    DrawTextEx(menuFont, titleText, titlePos, titleFontSize, titleSpacing, BLACK);
    
    for (auto& btn : buttons)
    {
        btn.draw(menuFont);
    }
}