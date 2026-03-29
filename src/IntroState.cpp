#include "IntroState.h"

IntroState::IntroState() {
    WindowWidth = 1800;
    WindowHeight = 1000;
    NextState = Intro; 
    
    introBg = nullptr;
    startBtn = nullptr;
    bubbleSpawnTimer = 0.0f;
    introDuration = 1.2;
    isFadingOut = false;
    transitionAlpha = 0.0f;
    isInitialized = false;
    rng.seed(std::random_device{}());
}

IntroState::~IntroState() {
    if (introBg) delete introBg;
    if (startBtn) delete startBtn;
    UnloadTexture(bubbleTex);
    UnloadFont(introFont);
}

void IntroState::loadAssets() {
    int fontSize = 90;
    introFont = LoadFontEx("assets/FONT.ttf", fontSize, 0, 0);
    SetTextureFilter(introFont.texture, TEXTURE_FILTER_BILINEAR);

    introBg = new back_ground("assets/backgroundintro01.jpg", {0.0f, 0.0f}, "assets/backgroundintro01.jpg", true);
    bubbleTex = LoadTexture("assets/bubble.png");

    startBtn = new button("assets/start01.png", "assets/start01.png", "assets/start01.png", "assets/start01.png", {0, 0}, 120);
    Vector2 startBtnPos = { (float)WindowWidth / 2.0f - 170.0f, (float)WindowHeight / 2.0f + 120.0f };
    startBtn->SetPosition(startBtnPos);
}

void IntroState::update(float deltaTime) {
    if (!isInitialized) {
        startTime = GetTime();
        isInitialized = true;
    }

    double currentTime = GetTime() - startTime;
    Vector2 mousePos = GetMousePosition();

    bool isHoveringStart = false;
    Vector2 startBtnPos = { (float)WindowWidth / 2.0f - 170.0f, (float)WindowHeight / 2.0f + 120.0f };
    Rectangle startBtnBounds = { startBtnPos.x, startBtnPos.y, 320.0f, 120.0f };

    if (currentTime >= introDuration && !isFadingOut) {
        if (CheckCollisionPointRec(mousePos, startBtnBounds)) {
            isHoveringStart = true;
        }

        if (startBtn->isPressed(mousePos, IsMouseButtonPressed(MOUSE_BUTTON_LEFT))) {
            isFadingOut = true;
        }
    }

    if (isFadingOut) {
        transitionAlpha += 500.0f * deltaTime;
        if (transitionAlpha >= 255.0f) {
            transitionAlpha = 255.0f;
            NextState = Menu; // Chuyển sang màn hình Menu
            return;
        }
    }

    // Logic xử lý Bong bóng (Mouse Click)
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !isHoveringStart && !isFadingOut) {
        bool clickedBubble = false;
        bool pre_sign = 0;

        for (int i = bubbles.size() - 1; i >= 0; i--) {
            BUBBLE& b = bubbles[i];
            float radius = (bubbleTex.width * b.scale) / 2.0f;
            Vector2 center = { b.position.x + radius, b.position.y + radius };

            if (CheckCollisionPointCircle(mousePos, center, radius)) {
                clickedBubble = true;
                float diameter = radius * 2.0f;

                if (diameter > 65.0f) {
                    int numSmallBubbles = GetRandomValue(1, 4) + 1;
                    for (int j = 0; j < numSmallBubbles; j++) {
                        BUBBLE smallB;
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

        if (!clickedBubble) {
            BUBBLE newB;
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

    // Logic sinh và di chuyển bong bóng
    bubbleSpawnTimer += deltaTime;
    if (bubbleSpawnTimer > 0.25f) {
        bubbleSpawnTimer = 0.0f;
        std::uniform_int_distribution<int> chanceDist(1, 100);
        if (chanceDist(rng) <= 11) {
            BUBBLE b;
            std::uniform_real_distribution<float> scaleDist(0.1f, 0.36f);
            b.scale = scaleDist(rng);

            std::uniform_real_distribution<float> xDist(0.0f, 1800.0f);
            b.position.x = xDist(rng);

            if(b.position.x > 600.0f && b.position.x < 900.0f) b.position.x -= 180.0f;
            else if(b.position.x > 900.0f && b.position.x < 1100.0f) b.position.x += 180.0f;

            b.position.y = 1000.0f;

            std::uniform_real_distribution<float> speedDist(35.0f, 100.0f);
            b.speed = speedDist(rng);

            std::uniform_real_distribution<float> wobbleDist(0.0f, 3.141f);
            b.wobbleOffset = wobbleDist(rng);
            bubbles.push_back(b);
        }
    }

    for (int i = 0; i < bubbles.size(); i++) {
        bubbles[i].position.y -= bubbles[i].speed * deltaTime;
        bubbles[i].position.x += sin(GetTime() * 2.0f + bubbles[i].wobbleOffset) * 0.15f;

        if (bubbles[i].position.y < -150.0f) {
            bubbles.erase(bubbles.begin() + i);
            i--;
        }
    }
}

void IntroState::draw() {
    Vector2 mousePos = GetMousePosition();
    double currentTime = GetTime() - startTime;
    const char* text = "DATA STRUCTURES VISUALIZER";
    int textLength = strlen(text);
    int fontSize = 90;
    float spacing = 5.0f;

    Vector2 textSize = MeasureTextEx(introFont, text, fontSize, spacing);
    Vector2 startPos = { (float)WindowWidth / 2.0f - textSize.x / 2.0f - 20,
                         (float)WindowHeight / 2.0f - textSize.y / 2.0f - 280 };

    int lettersToShow = (int)((currentTime / introDuration) * (textLength + 1));
    if (lettersToShow > textLength) lettersToShow = textLength;

    ClearBackground(BLACK);

    if (introBg) introBg->Draw(mousePos);

    for (const auto& b : bubbles) {
        DrawTextureEx(bubbleTex, b.position, 0.0f, b.scale, Fade(WHITE, 0.8f));
    }

    float currentX = startPos.x;
    for (int i = 0; i < lettersToShow; i++) {
        char charStr[2] = { text[i], '\0' };
        if(i < 16) DrawTextEx(introFont, charStr, {currentX, startPos.y}, fontSize, spacing, BLACK);
        else DrawTextEx(introFont, charStr, {currentX, startPos.y}, fontSize, spacing, BLUE);

        if(text[i] == ' ') currentX += MeasureTextEx(introFont, charStr, fontSize, spacing).x + spacing * 7;
        else currentX += MeasureTextEx(introFont, charStr, fontSize, spacing).x + spacing;
    }

    if (currentTime >= introDuration && startBtn) {
        startBtn->Draw(mousePos);
    }

    if (transitionAlpha > 0.0f) {
        DrawRectangle(0, 0, WindowWidth, WindowHeight, { 0, 0, 0, (unsigned char)transitionAlpha });
    }
}