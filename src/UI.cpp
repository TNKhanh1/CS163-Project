#include "UI.h"
#include <iostream>
// BackGround 
back_ground::back_ground(const char *imagePath, Vector2 imagePosition, const char* imagePath02, bool isParallax)
{
    this->isParallax = isParallax;
    this->position = imagePosition;
    this->currentOffset = {0.0f, 0.0f};


    int w = isParallax ? 1700 : 1600;
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
    Rectangle rect = {position.x, position.y, static_cast<double>(texture.width), static_cast<double>(texture.height)};
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
