#pragma once 
#include "raylib.h"
#include <string>
class back_ground
{
    public:
        bool type = 0; 
        back_ground(const char* imagePath, Vector2 imagePosition, const char* imagePath02, bool isParallax = false);
        ~back_ground();
        void Draw(Vector2 mousePos = {-1.0f, -1.0f});
    private:
        Texture2D texture;
        Texture2D texture02;
        Vector2 position;
        Vector2 currentOffset;
        bool isParallax;
};

class button 
{
    public:
        bool type = 0;
        button(const char *imagePath, const char *ChangeImagePath, const char *imagePath02, const char *ChangeImagePath02, Vector2 imagePosition, double scale);
        ~button();
        void Draw(Vector2 mousePos);
        bool isPressed(Vector2 mousePos, bool mousePressed);
        void SetPosition(Vector2 newPosition)
        {
            this->position = newPosition;
        }
    private:
        Texture2D texture;
        Vector2 position;
        Texture2D ChangeTexture;
        Texture2D texture02;
        Texture2D ChangeTexture02;
};

class audio
{
    public:
        audio(const char *soundPath);
        ~audio();
        void Play(bool clickState);
    private:
        Sound Audio;
};

class slider 
{
public:
    slider(Rectangle bounds, float minVal, float maxVal, float startVal);
    void Update(Vector2 mousePos);
    void Draw(Font font, Color fill = SKYBLUE);
    float GetValue() const { return currentValue; }

private:
    Rectangle bounds;
    float minValue;
    float maxValue;
    float currentValue;
    bool isDragging;
};

class error_label 
{
public:
    error_label();
    void Set(std::string msg, float duration = 2.5f);
    void Update(float deltaTime);
    void Draw(Font font, Vector2 pos, float size);

private:
    std::string message;
    float timer;
};

