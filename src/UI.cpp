#include "UI.h"
#include <iostream>
// BackGround 
back_ground::back_ground(const char *imagePath, Vector2 imagePosition, const char* imagePath02)
{
    Image image = LoadImage(imagePath);
    ImageResize(&image, 1000, 1000);
    texture = LoadTextureFromImage(image);
    UnloadImage(image);
    position = imagePosition;

    image = LoadImage(imagePath02);
    ImageResize(&image, 1000, 1000);
    texture02 = LoadTextureFromImage(image);
    UnloadImage(image);
}

back_ground::~back_ground()
{
    UnloadTexture(texture);
    UnloadTexture(texture02);
}

void back_ground::Draw()
{
    if (!type) DrawTextureV(texture, position, WHITE);
    else DrawTextureV(texture02, position, WHITE);
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
    Rectangle rect = {position.x, position.y, static_cast<double>(texture.width), static_cast<double>(texture.height)};
    if (!type)
    {
        if (CheckCollisionPointRec(mousePos, rect)) DrawTextureV(ChangeTexture, position, WHITE);
        else DrawTextureV(texture, position, WHITE);
    }
    else 
    {
        if (CheckCollisionPointRec(mousePos, rect)) DrawTextureV(ChangeTexture02, position, WHITE);
        else DrawTextureV(texture02, position, WHITE);
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
