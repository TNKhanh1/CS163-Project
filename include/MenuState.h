#pragma once
#include <raylib.h>
#include <raymath.h>
#include <UI.h>
#include <vector>
#include <string>
#include <random>
#include <State.h>
// Cấu trúc lưu trữ thông tin của từng bong bóng
struct Bubble {
    Vector2 position;
    float speed;
    float scale;
    float wobbleOffset; 
};

// Class đại diện cho nút bấm với hiệu ứng CSS Gradient
class GradientButton 
{
    public:
        Rectangle bounds;
        std::string text;
        float hoverProgress;

        GradientButton(Rectangle bounds, std::string text);
        
        void update(float deltaTime);
        void draw(Font font);
        bool isClicked();
        
    private:
        // Hàm nội suy màu để tạo hiệu ứng chuyển gradient
        Color LerpColor(Color c1, Color c2, float t);
};

class MenuState
{
    public:
        MenuState();
        ~MenuState();
        
        void update(float deltaTime); 
        void draw();
        void loadAssets();
        
    private:
        back_ground* bg;
        Font menuFont;
        std::vector<GradientButton> buttons; 

        // Biến quản lý bong bóng
        Texture2D bubbleTex;
        std::vector<Bubble> bubbles;
        float bubbleSpawnTimer;
        std::mt19937 rng;
};