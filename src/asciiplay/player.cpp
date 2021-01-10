static char LOGO[1473] =
"                                      $$    $$              "
"                                                            "
"      @@@$        @@$$      @@@$      @$    @$              "
"    @$    $$    @$    $$  @$    @$    @$    @$              "
"          @$    @$        #$          @$    @$              "
"      @@@@@$      @@$$    =#          @$    @$              "
"    @$    @$          $$  #$          @$    @$              "
"    $$    @$    $$    $$  @$    @$    @$    @$              "
"    $@@$  @$      @@@$      @@@$      @$    @$              "
"                                                            "
"                                                            "
"$@$@$@$@$@$@$@$@$@$@$@$@$@$@$@$@$@$@$@$@$@$$!$;#:#~*        "
"                                  $$                        "
"                                  $$                        "
"                      $@$@@$      $$      @@@$    #$      *$"
"                      @$    @$    $$    @$    $$    $$  $$  "
"                      $$    $$    $$          @$    @$  @$  "
"                      $$    #$    $$      @@@@@$    $$  $$  "
"                      $$    $$    $$    @$    @$      @$    "
"                      @$    @$    $$    $$    @$      @$    "
"                      @@$@@$      $$    $@@$  @$      @$    "
"                      $$                              *#    "
"                      $$                            @$      ";

#define LOGO_W 60
#define LOGO_H 23

#include "player.h"
#include "options.h"
#include "types.h"
#include "version.h"
#include <chrono>
#include <iostream>

#include <curses.h>

namespace chrono = std::chrono;

#define STB_VORBIS_HEADER_ONLY
#include "extras/stb_vorbis.c"    /* Enables Vorbis decoding. */

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

/* stb_vorbis implementation must come after the implementation of miniaudio. */
#undef STB_VORBIS_HEADER_ONLY
#include "extras/stb_vorbis.c"

// color pair codes
#define UI_COLOR 1
#define UI_HIGHLIGHT_COLOR 2

Player::Player(Video &v, Options &o)
: video(v),
  filename(o.input),
  opts(o)
{
    frameInterval = 1/video.fps;
    // play the video right away if it's loaded
    if (video.loaded)
        state = Playing;
}

Player::~Player()
{
    ma_decoder_uninit(&decoder);
    ma_device_uninit(&device);
}

// miniaudio data callback
void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    ma_decoder* pDecoder = (ma_decoder*)pDevice->pUserData;
    if (pDecoder == NULL) {
        return;
    }

    ma_decoder_read_pcm_frames(pDecoder, pOutput, frameCount);

    (void)pInput;
}

bool Player::init()
{
    // initialize ncurses
    setlocale(LC_ALL, "");
    initscr(); cbreak(); noecho();
    nonl();
    intrflush(stdscr, FALSE);
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    notimeout(stdscr, TRUE);
    curs_set(0);
    mousemask(BUTTON1_PRESSED | BUTTON1_RELEASED |
              BUTTON2_PRESSED | BUTTON2_RELEASED | REPORT_MOUSE_POSITION, NULL);

    // colors
    start_color();
    // TODO: load these from a config file
    init_pair(UI_COLOR, COLOR_BLACK, COLOR_WHITE);
    init_pair(UI_HIGHLIGHT_COLOR, COLOR_BLACK, COLOR_GREEN);

    Audio* audio = video.audio;
    if (audio)
    {
        // initialize miniaudio
        ma_device_config deviceConfig;
        ma_result result;

        result = ma_decoder_init_memory(audio->data, audio->length, NULL, &decoder);
        if (result != MA_SUCCESS) {
            return false;
        }

        deviceConfig = ma_device_config_init(ma_device_type_playback);
        deviceConfig.playback.format   = decoder.outputFormat;
        deviceConfig.playback.channels = decoder.outputChannels;
        deviceConfig.sampleRate        = decoder.outputSampleRate;
        deviceConfig.dataCallback      = data_callback;
        deviceConfig.pUserData         = &decoder;

        if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS) {
            std::cout << "FATAL: Failed to open playback device." << std::endl;
            ma_decoder_uninit(&decoder);
            return false;
        }

        audioReady = true;
    }

    return true;
}

// Audio controls
void Player::playAudio()
{
    if (audioReady)
        ma_device_start(&device);
}

void Player::pauseAudio()
{
    if (audioReady)
        ma_device_stop(&device);
}

void Player::stopAudio()
{
    if (audioReady)
        ma_device_stop(&device);
    // also unload the audio decoder
    ma_decoder_uninit(&decoder);
}

// Player controls
void Player::play()
{
    state = Playing;
    playAudio();
}

void Player::pause()
{
    state = Paused;
    pauseAudio();
}

void Player::stop()
{
    state = Stopped;
    stopAudio();
    // force a screen redraw
    clear();
}

void Player::renderVideoFrame(int screenH, int screenW, int frameNum)
{
    auto fr = video.frames[frameNum];
    int type = fr.index();
    int offsetY = (screenH - video.height)/2;
    int offsetX = (screenW - video.width)/2;

    switch (type)
    {
    case 0: // I-frame
    {
        IFrame frame = std::get<IFrame>(fr);
        int pixel = 0;
        for (int y = 0; y < video.height; y++)
        {
            for (int x = 0; x < video.width; x++)
            {
                CharacterIndex chr = frame[pixel];
                mvaddch(offsetY + y, offsetX + x, video.charset[chr]);
                pixel++;
            }
        }
        break;
    }
    case 1: // P-frame
    {
        PFrame frame = std::get<PFrame>(fr);
        for (int n = 0; n < frame.size(); n++)
        {
            auto entry = frame[n];
            CharacterIndex chr = entry.first;
            Position pos = entry.second;
            mvaddch(offsetY + pos.y, offsetX + pos.x, video.charset[chr]);
        }
        break;
    }
    }
    // reset the cursor
    move(0, 0);
}

/* UI DRAWING FUNCTIONS */

// useful stuff
static const char* fadeChars[4] = {"▓", "▒", "░", " "};

// Logo
void Player::drawLogo(int screenH, int screenW)
{
    for (int i = 0; i < LOGO_H; i++)
    {
        int offset = LOGO_W * i;
        int y = (screenH - 1)/2 + (i - LOGO_H/2);
        int x = (screenW - LOGO_W)/2;
        mvprintw(y, x, "%.*s", LOGO_W, LOGO + offset);
    }
    move(0, 0);
}

// Context menu
typedef std::vector<const char*> CMEntry;
static std::vector<CMEntry> CMEntryList {
    {"Open", "Exit"}, // CM_FILE
    {"Media Info", "Settings"}, // CM_TOOLS
    {"Play", "Pause", "Stop"}, // CM_VIDEO
    {"Mute"}, // CM_AUDIO
    {"About"} // CM_HELP
};

inline void drawMenuEntry(int y, int x, const char* label)
{
    mvprintw(y, x, " %-13s", label);
}

void Player::drawContextMenu(CMType type, MouseState &mouse)
{
    int curX = getcurx(stdscr);
    int curY = getcury(stdscr);

    CMEntry entry = CMEntryList[type];
    for (int i = 0; i < entry.size(); i++)
        drawMenuEntry(curY + 1 + i, curX, entry[i]);
    activeMenu = type;

    move(curY, curX);
}

void Player::finishContextMenu(MouseState &mouse)
{
    int curX = getcurx(stdscr);
    int curY = getcury(stdscr);

    CMEntry entry = CMEntryList[activeMenu];
    for (int y = 0; y < entry.size(); y++)
    {
        int entryY = curY + 1 + y;
        if (mouse.leftDown)
        {
            // check if the entry was clicked on
            if (mouse.x >= curX && mouse.x <= curX + 13 &&
                mouse.y == entryY)
            {
                // highlight the entry
                attroff(COLOR_PAIR(UI_COLOR));
                attron(COLOR_PAIR(UI_HIGHLIGHT_COLOR));
                drawMenuEntry(entryY, curX, entry[y]);
                attroff(COLOR_PAIR(UI_HIGHLIGHT_COLOR));
                attron(COLOR_PAIR(UI_COLOR));
                // returning here; the menu bar drawing function
                // will call this again later and the entry will be
                // removed after the left mouse has been released
                move(curY, curX);
                return;
            }
        }
        move(entryY, curX);
        for (int i = 0; i < 14; i++)
            delch();
    }
    activeMenu = (CMType)CM_NONE; // mingw complains without the cast

    move(curY, curX);
}

// Menu bar
#define MENU_COUNT 5
static const char* menuEntries[MENU_COUNT] = {"File", "Tools", "Video", "Audio", "Help"};

void Player::tweenMenuBar(int screenW, int tweenStep)
{
    for (int i = 0; i < screenW; i++)
        mvaddstr(0, i, fadeChars[tweenStep]);
}

void Player::drawMenuBar(int screenW, MouseState &mouse, int tweenStep)
{
    // set the color
    attron(COLOR_PAIR(UI_COLOR));

    // background
    for (int i = 0; i < screenW; i++)
        mvaddch(0, i, ' ');

    move(0, 1);
    for (int i = 0; i < MENU_COUNT; i++)
    {
        if (activeMenu == i)
            finishContextMenu(mouse);
        
        bool highlight = false;
        if (mouse.y == 0)
        {
            int cursorX = getcurx(stdscr);
            if (mouse.x >= cursorX && mouse.x <= cursorX + strlen(menuEntries[i]))
            {
                highlight = true;
                drawContextMenu((CMType)i, mouse);
            }
        }
        if (highlight)
        {
            attroff(COLOR_PAIR(UI_COLOR));
            attron(COLOR_PAIR(UI_HIGHLIGHT_COLOR));
            printw(" %s ", menuEntries[i]);
            attroff(COLOR_PAIR(UI_HIGHLIGHT_COLOR));
            attron(COLOR_PAIR(UI_COLOR));
        }
        else printw(" %s ", menuEntries[i]);
        addstr(" "); // padding
    }

    // clear the color
    attroff(COLOR_PAIR(UI_COLOR));
    // if (settings.color == CONSOLE)
    use_default_colors();
}

// Controls
void Player::tweenControls(int screenH, int screenW, int tweenStep)
{
    // clear the previous stuff
    for (int i = screenH; i >= screenH - 4; i--)
    {
        move(i, 0);
        clrtoeol();
    }

    int y = screenH - (4 - tweenStep);

    // top line
    move(y, 0);
    for (int i = 0; i < screenW; i++)
        addstr(fadeChars[tweenStep]);
    
    // buttons
    attron(COLOR_PAIR(UI_COLOR));
    mvaddstr(y + 2, screenW/2 - 9,
            (state == Playing ? "⏸ Pause " : " ▶ Play "));
    mvaddstr(y + 2, screenW/2 + 1, " ⏹ Stop ");
    attroff(COLOR_PAIR(UI_COLOR));
}

static const char* noFileStr = "No file loaded.";
void Player::drawControls(int screenH, int screenW, MouseState &mouse)
{
    // check if there's a state queued and apply it
    if (!mouse.leftDown && queuedState != None)
    {
        switch (queuedState)
        {
        case None:
            break;
        case Playing: play();
            break;
        case Paused: pause();
            break;
        case Stopped: stop();
            break;
        }
        queuedState = None;
    }

    attron(COLOR_PAIR(UI_COLOR));
    int y = screenH - 4;
    
    // top line
    move(y, 0);
    for (int i = 0; i < screenW; i++)
        addch(' ');
    
    if (!filename.empty())
        mvaddstr(y, (screenW - filename.length())/2, filename.c_str());
    else
        mvaddstr(y, (screenW - strlen(noFileStr))/2, noFileStr);

    // controls
    // TODO: improve this section somehow?
    int btnY = y + 2;
    int playX = screenW/2 - 9;
    const char* playLabel = state == Playing ? "⏸ Pause " : " ▶ Play ";
    if (mouse.x >= playX && mouse.x <= playX + 8 && mouse.leftDown)
    {
        attroff(COLOR_PAIR(UI_COLOR));
        attron(COLOR_PAIR(UI_HIGHLIGHT_COLOR));
        mvaddstr(btnY, playX, playLabel);
        attroff(COLOR_PAIR(UI_HIGHLIGHT_COLOR));
        attron(COLOR_PAIR(UI_COLOR));
        // queue the state to be changed after the mouse is released
        if (video.loaded)
            queuedState = state == Playing ? Paused : Playing;
    } else mvaddstr(btnY, playX, playLabel);

    int stopX = screenW/2 + 1;
    if (mouse.x >= stopX && mouse.x <= stopX + 8 && mouse.leftDown)
    {
        attroff(COLOR_PAIR(UI_COLOR));
        attron(COLOR_PAIR(UI_HIGHLIGHT_COLOR));
        mvaddstr(btnY, stopX, " ⏹ Stop ");
        attroff(COLOR_PAIR(UI_HIGHLIGHT_COLOR));
        attron(COLOR_PAIR(UI_COLOR));
        queuedState = Stopped;
    } else mvaddstr(btnY, stopX, " ⏹ Stop ");

    attroff(COLOR_PAIR(UI_COLOR));
}

// input handler
bool Player::handleInput(int* keyPressed, MouseState* mouse)
{
    int c = getch();
    MEVENT event;
    
    switch (c)
    {
    case KEY_MOUSE:
        if (opts.disableMouse) return false;
        if (getmouse(&event) == OK)
        {
            if (event.bstate & BUTTON1_PRESSED)
                mouse->leftDown = true;
            if (event.bstate & BUTTON2_PRESSED)
                mouse->rightDown = true;
            if (event.bstate & BUTTON1_RELEASED)
                mouse->leftDown = false;
            if (event.bstate & BUTTON2_RELEASED)
                mouse->rightDown = false;
            
            mouse->x = event.x;
            mouse->y = event.y;
            return true;
        }
        return false;
        break;
    default:
        if (opts.disableKb || c == ERR) return false;
        *keyPressed = c;
        return true;
    }
}

void Player::tweenUi(int screenH, int screenW, int tweenStep)
{
    tweenMenuBar(screenW, tweenStep);
    tweenControls(screenH, screenW, tweenStep);
}

// Main loop
void Player::main()
{
    // play the audio if the playback state is already set
    if (state == Playing)
        playAudio();

    // screen size
    int screenW = 0,
        screenH = 0;
    getmaxyx(stdscr, screenH, screenW);
    
    // input
    int keyDown = -1;
    MouseState mouse;

    // timing
    auto prevTime = chrono::steady_clock::now();
    float deltaTime = 0;
    float uiTime = 0;
    float uiTweenDelta = 0;
    int uiTweenStep = 0;
    float frameDelta = 0;
    int currentFrame = 0;

    use_default_colors();

    for (;;)
    {
        // timing
        auto currentTime = chrono::steady_clock::now();
        chrono::duration<float> dd = currentTime - prevTime;
        deltaTime = dd.count();
        prevTime = currentTime;
        uiTweenDelta += deltaTime;

        // render video
        if (state == Playing)
        {
            frameDelta += deltaTime;
            if (frameDelta >= frameInterval)
            {
                renderVideoFrame(screenH, screenW, currentFrame);
                frameDelta = frameDelta - frameInterval;
                currentFrame++;
                // end of video reached
                if (currentFrame == video.frames.size())
                    stop();
            }
        }

        if (!opts.disableUi)
        {
            // interface
            if (state == Stopped)
                drawLogo(screenH, screenW);

            if (uiTime < 3) {
                if (uiTweenStep > 0)
                {
                    if (uiTweenDelta >= 0.05) {
                        uiTweenDelta = 0;
                        uiTweenStep--;
                        tweenUi(screenH, screenW, uiTweenStep);
                    }
                } else {
                    drawMenuBar(screenW, mouse, uiTweenStep);
                    drawControls(screenH, screenW, mouse);
                    uiTime += deltaTime;
                }
            } else if (uiTweenStep < 4) {
                if (uiTweenDelta >= 0.05)
                {
                    tweenUi(screenH, screenW, uiTweenStep);
                    uiTweenDelta = 0;
                    uiTweenStep++;
                }
            }
        }
        
        // finally, refresh the screen
        refresh();

        // update/reset stuff for the next frame
        int prevW = screenW,
            prevH = screenH;
        getmaxyx(stdscr, screenH, screenW);
        if (prevW != screenW || prevH != screenH)
            clear();
        keyDown = -1;

        // handle input
        bool gotInput = handleInput(&keyDown, &mouse);
        if (gotInput)
        {
            // reset the ui time if there's input
            uiTime = 0;
            uiTweenDelta = 0;

            // keyboard controls
            if (keyDown == ' ')
            {
                if (state == Playing) pause();
                else play();
            }
        }
    }
}
