#include "UI.h"
#include <iostream>
// BackGround 
back_ground::back_ground(const char *imagePath, Vector2 imagePosition, const char* imagePath02, bool isParallax)
{
    this->isParallax = isParallax;
    this->position = imagePosition;
    this->currentOffset = {0.0f, 0.0f};


    int w = isParallax ? 1900 : 1800;
    int h = isParallax ? 1100 : 1000;

    Image image = LoadImage(imagePath);
    ImageResize(&image, w, h);
    texture = LoadTextureFromImage(image);
    UnloadImage(image);

    image = LoadImage(imagePath02);
    ImageResize(&image, w, h);
    texture02 = LoadTextureFromImage(image);
    UnloadImage(image);
}

back_ground::~back_ground()
{
    UnloadTexture(texture);
    UnloadTexture(texture02);
}

void back_ground::Draw(Vector2 mousePos)
{
    Vector2 drawPos = position;

    if (isParallax && mousePos.x != -1.0f) 
    {
        float targetOffsetX = (800.0f - mousePos.x) * 0.06f;
        float targetOffsetY = (500.0f - mousePos.y) * 0.06f;
        float lerpSpeed = 1.3f * GetFrameTime(); 

        currentOffset.x += (targetOffsetX - currentOffset.x) * lerpSpeed;
        currentOffset.y += (targetOffsetY - currentOffset.y) * lerpSpeed;

        drawPos.x = position.x - 50.0f + currentOffset.x;
        drawPos.y = position.y - 50.0f + currentOffset.y;
    }

    if (!type) DrawTextureV(texture, drawPos, WHITE);
    else DrawTextureV(texture02, drawPos, WHITE);
}

// Button
button::button(const char *imagePath, const char *ChangeImagePath, const char *imagePath02, const char *ChangeImagePath02, Vector2 imagePosition, double scale)
{
    Image image = LoadImage(imagePath);
    
    int w = image.width * scale / image.height;
    int h = scale;

    ImageResize(&image, w, h);
    texture = LoadTextureFromImage(image);
    UnloadImage(image);

    image = LoadImage(ChangeImagePath);
    ImageResize(&image, w, h);
    ChangeTexture = LoadTextureFromImage(image);
    UnloadImage(image);

    position = imagePosition;

    image = LoadImage(imagePath02);

    ImageResize(&image, w, h);
    texture02 = LoadTextureFromImage(image);
    UnloadImage(image);

    image = LoadImage(ChangeImagePath02);
    ImageResize(&image, w, h);
    ChangeTexture02 = LoadTextureFromImage(image);
    UnloadImage(image);
}

button::~button()
{
    UnloadTexture(texture);
    UnloadTexture(ChangeTexture);
    UnloadTexture(texture02);
    UnloadTexture(ChangeTexture02);
}


void button::Draw(Vector2 mousePos)
{
    Rectangle rect = { position.x, position.y, (float)texture.width, (float)texture.height };
    
    Color hoverColor = Fade(WHITE, 0.5f); 

    if (!type)
    {
        if (CheckCollisionPointRec(mousePos, rect)) 
        {
            DrawTextureV(ChangeTexture, position, hoverColor);
        }
        else 
        {
            DrawTextureV(texture, position, WHITE);
        }
    }
    else 
    {
        if (CheckCollisionPointRec(mousePos, rect)) 
        {
            DrawTextureV(ChangeTexture02, position, hoverColor);
        }
        else 
        {
            DrawTextureV(texture02, position, WHITE);
        }
    }
}

bool button::isPressed(Vector2 mousePos, bool mousePressed)
{
    Rectangle rect = { position.x, position.y, (float)texture.width, (float)texture.height };
    if (CheckCollisionPointRec(mousePos, rect) && mousePressed) return true;
    return false;
}

audio::audio(const char *soundPath)
{
    Audio = LoadSound(soundPath);
}

audio::~audio()
{
    UnloadSound(Audio);
}

void audio::Play(bool clickState)
{
    if (!clickState) return;
    PlaySound(Audio);
}
// Slider component
slider::slider(Rectangle bounds, float minVal, float maxVal, float startVal)
{
    this->bounds = bounds;
    this->minValue = minVal;
    this->maxValue = maxVal;
    this->currentValue = startVal;
    this->isDragging = false;
}

void slider::Update(Vector2 mousePos)
{
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mousePos, bounds)) {
        isDragging = true;
    }
    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        isDragging = false;
    }

    if (isDragging) {
        float progress = (mousePos.x - bounds.x) / bounds.width;
        if (progress < 0.0f) progress = 0.0f;
        if (progress > 1.0f) progress = 1.0f;
        currentValue = minValue + (progress * (maxValue - minValue));
    }
}

void slider::Draw(Font font, Color fill)
{
    // Draw the track
    DrawRectangleRounded(bounds, 1.0f, 8, LIGHTGRAY);
    
    // Draw the filled part
    float fillPercentage = (currentValue - minValue) / (maxValue - minValue);
    Rectangle filledPart = { bounds.x, bounds.y, bounds.width * fillPercentage, bounds.height };
    DrawRectangleRounded(filledPart, 1.0f, 8, fill);

    // Draw the handle
    float handleX = bounds.x + (bounds.width * fillPercentage);
    DrawCircle((int)handleX, (int)(bounds.y + bounds.height/2.0f), 12.0f, DARKBLUE);
}
// Error label
error_label::error_label()
{
    message = "";
    timer = 0.0f;
}

void error_label::Set(std::string msg, float duration)
{
    message = msg;
    timer = duration;
}

void error_label::Update(float deltaTime)
{
    if (timer > 0.0f) {
        timer -= deltaTime;
        if (timer <= 0.0f) message = "";
    }
}

void error_label::Draw(Font font, Vector2 pos, float size)
{
    if (timer > 0.0f) {
        DrawTextEx(font, message.c_str(), pos, size, 1.0f, RED);
    }
}
