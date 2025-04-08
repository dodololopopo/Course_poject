#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <random>

const int WINDOW_HIGHT = 900;
const int WIDTH = 800;
const int HEIGHT = 800;
const int ROWS = 4;
const int COLS = 4;

const int RECT_HIGHT = HEIGHT / ROWS;
const int RECT_WIDTH = WIDTH / ROWS;

const int OUTLINE_THIKNESS = 10;

// Constants for FPS control
const Uint64 FPS = 60; // Desired FPS
const Uint64 FRAME_DELAY = 1000 / FPS; // Time per frame in milliseconds
const int MOVE_VEL = 20;

std::random_device rd;
std::mt19937 gen(rd());
std::bernoulli_distribution dis(0.1); // 10% chance for `true` (4)
std::uniform_int_distribution<int> dist(0, 3); // Range [0, 3]

SDL_Color FONT_COLOR = { 119, 110, 101, 255 };

char SValue[10];

enum Direction
{
    DOWN,
    LEFT,
    RIGHT,
    UP
};

TTF_Font* font;
SDL_Window* window;
SDL_Renderer* renderer;
//SDL_Surface* textSurface;
//SDL_Texture* textTexture;

int score = 0;

const int COLORS[][3] = { {237, 229, 218},
                        {238, 225, 201},
                        {243, 178, 122},
                        {246, 150, 101},
                        {247, 124, 95 },
                        {247, 95, 59 },
                        {237, 208, 115},
                        {237, 204, 99 },
                        {236, 202, 80 },
                        {60,  58,  50} };

class Gameplay
{
public:
    static void tutorial() {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Manual", "Use your arrow-keys to move the tiles. When two tiles with the same number touch, they merge into one!", nullptr);
    }
    static void win() {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Victory", "You win!", nullptr);
    }
    static void lose() {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Defeat", "You lose!", nullptr);
    }
};

class Tile : public Gameplay
{
public:
    int value, row, col, x, y;
    std::string key;

    Tile() {
    }

    Tile(int value, int row, int col)
    {
        this->value = value;
        this->row = row;
        this->col = col;
        this->x = col * RECT_WIDTH;
        this->y = row * RECT_HIGHT;
        setKey(row, col);
    }

    void setKey(int row, int col) {
        this->key = std::to_string(row) + std::to_string(col);
    }
    const int* getColor() const
    {
        const int* color;
        int color_index = static_cast<int>(std::log2(value));

        if (color_index < 9) {
            color = COLORS[color_index];

        }
        else {
            color = COLORS[9];
        }
        return color;
    }

    void draw()
    {
        auto color = getColor();
        SDL_SetRenderDrawColor(renderer, color[0], color[1], color[2], 255);
        SDL_FRect rect{ (float)x, (float)y, (float)RECT_WIDTH, (float)RECT_HIGHT };
        SDL_RenderFillRect(renderer, &rect);

        sprintf_s(SValue, "%d", value);
        SDL_Surface* textSurface = TTF_RenderText_Solid(font, SValue, strlen(SValue), FONT_COLOR);
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

        float textWidth = (float)textSurface->w;
        float textHeight = (float)textSurface->h;

        SDL_DestroySurface(textSurface);

        SDL_FRect textRect;
        textRect.x = (float)(x + (RECT_WIDTH / 2 - textWidth / 2));
        textRect.y = (float)(y + (RECT_HIGHT / 2 - textHeight / 2));
        textRect.w = textWidth;
        textRect.h = textHeight;
        SDL_RenderTexture(renderer, textTexture, nullptr, &textRect);
        SDL_DestroyTexture(textTexture); // Cleanup
    }

    void set_pos(bool ceil = false)
    {
        if (ceil)
        {
            row = (int)std::ceil((double)y / RECT_HIGHT);
            col = (int)std::ceil((double)x / RECT_WIDTH);
            setKey(row, col);
        }
        else
        {
            row = (int)std::floor((double)y / RECT_HIGHT);
            col = (int)std::floor((double)x / RECT_WIDTH);
            setKey(row, col);
        }
    }

    void move(int deltaX, int deltaY)
    {
        x += deltaX;
        y += deltaY;
    }

};