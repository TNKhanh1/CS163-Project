#include<raylib.h>
#include<raymath.h>
#include<iostream>
#include<UI.h>
#include<vector>
using namespace std;
enum State
{
    Menu,
    Setting,
    LinkList,
    Heap,
    AVLTree,
    MST
};
class MainLoop
{
    public:
    MainLoop();
    ~MainLoop();


    State current_state;
    vector<State> history_state;


    void update(float delta_time);
    void run();
    void draw();
    void BackState();
    void NextState();

    
    private:
        int WindowHeight;
        int WindowWidth;
        void LoadAssets();
};