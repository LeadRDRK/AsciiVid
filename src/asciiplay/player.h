#pragma once
#include <types.h>
#include "miniaudio.h"

class Options;

class Player
{
public:
    Player(Video &v, Options &o);
    ~Player();
    bool init();
    void play();
    void pause();
    void stop();
    void main();

private:
    Video& video;
    Options& opts;
    std::string filename;
    float frameInterval = 0;

    // audio stuff
    ma_device device;
    ma_decoder decoder;
    bool audioReady;
    // controls
    void playAudio();
    void pauseAudio();
    void stopAudio();

    // playback states
    enum PlaybackState
    {
        None = -1,
        Playing,
        Paused,
        Stopped
    };
    PlaybackState state = Stopped;

    // video rendering
    void renderVideoFrame(int screenH, int screenW, int frameNum);

    // ui drawing stuff
    enum CMType {
        CM_NONE = -1,
        CM_FILE,
        CM_TOOLS,
        CM_VIDEO,
        CM_AUDIO,
        CM_HELP
    };
    struct MouseState;
    CMType activeMenu = CM_NONE;
    PlaybackState queuedState = None;

    bool handleInput(int* keyPressed, MouseState* mouse);
    void drawLogo(int screenH, int screenW);
    void drawContextMenu(CMType type, MouseState &mouse);
    void finishContextMenu(MouseState &mouse);
    void tweenMenuBar(int screenW, int tweenStep);
    void drawMenuBar(int screenW, MouseState &mouse, int tweenStep);
    void tweenControls(int screenH, int screenW, int tweenStep);
    void drawControls(int screenH, int screenW, MouseState &mouse);
    void tweenUi(int screenH, int screenW, int tweenStep);
};

struct Player::MouseState
{
    int x = 0;
    int y = 0;
    bool leftDown = false;
    bool rightDown = false;
};
