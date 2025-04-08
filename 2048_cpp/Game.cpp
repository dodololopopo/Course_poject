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

std::tuple<int, int> GetRandomPosition(std::vector<Tile> tiles)
{
    int row = 0;
    int col = 0;
    std::string key;
    while (true)
    {
        row = dist(gen);
        col = dist(gen);
        key = std::to_string(row) + std::to_string(col);
        if (tiles.size() != 0)
        {
            if (std::find_if(tiles.begin(), tiles.end(), [&](const Tile& t) {return t.key == key; }) != tiles.end()) continue;
            else return { row, col };
        }
        else
            break;
    }
    return { row, col };
}

void DrawScore()
{
    std::string s = "SCORE " + std::to_string(score);
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, s.c_str(), strlen(s.c_str()), FONT_COLOR);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

    int textWidth = textSurface->w;
    int textHeight = textSurface->h;

    SDL_DestroySurface(textSurface);

    SDL_FRect textRect{ 10, HEIGHT + 10, (float)textWidth, (float)textHeight };
    SDL_RenderTexture(renderer, textTexture, nullptr, &textRect);
    SDL_DestroyTexture(textTexture);
}

void DrawGrid()
{
    SDL_SetRenderDrawColor(renderer, 187, 173, 160, 255);

    for (int row{ 0 }; row <= ROWS; row++)
    {
        int y = row * RECT_WIDTH;
        SDL_FRect line;
        line.x = 0;
        row == 0 ? line.y = (float)y : (row == ROWS ? line.y = (float)(y - 10) : line.y = (float)(y - 5));
        line.w = WIDTH;
        line.h = OUTLINE_THIKNESS;
        SDL_RenderFillRect(renderer, &line);
    }

    for (int col{ 0 }; col <= COLS; col++)
    {
        int x = col * RECT_HIGHT;
        SDL_FRect line;
        col == 0 ? line.x = (float)x : (col == COLS ? line.x = (float)(x - 10) : line.x = (float)(x - 5));
        line.y = 0;
        line.w = OUTLINE_THIKNESS;
        line.h = HEIGHT;
        SDL_RenderFillRect(renderer, &line);
    }

    DrawScore();
}
void DrawMain(std::vector<Tile>& tiles)
{
    // Clear the screen
    SDL_SetRenderDrawColor(renderer, 205, 192, 180, 255);
    SDL_RenderClear(renderer);

    for (auto tile : tiles)
    {
        tile.draw();
    }
    DrawGrid();
    // Display
    SDL_RenderPresent(renderer);

}

void GenerateTiles(std::vector<Tile>& tiles)
{
    for (int _{ 0 }; _ < 2; _++)
    {
        int row, col;
        std::tie(row, col) = GetRandomPosition(tiles);
        tiles.emplace_back(Tile(2, row, col));
    }
}
int main()
{
    window = nullptr;
    renderer = nullptr;
    font = nullptr;

    SDL_Init(SDL_INIT_VIDEO);
    if (!SDL_CreateWindowAndRenderer("2048", WIDTH, WINDOW_HIGHT, 0, &window, &renderer)) {
        std::cerr << "Window Creation Failed: " << std::endl;
        SDL_Quit();
        return -1;
    }

    if (!TTF_Init())
    {
        std::cerr << "SDL_ttf Init Failed: " << std::endl;
        SDL_Quit();
        return -1;
    }


    font = TTF_OpenFont("ComicSansMS.ttf", 50);
    if (!font) {
        std::cerr << "Failed to Load Font: " << SDL_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    SDL_Event e;
    bool running = true;
    Direction dir = RIGHT;

    // Variables for FPS calculation
    Uint64 frameStart;
    Uint64 frameTime;

    std::vector<Tile> tiles;
    tiles.reserve(16);
    GenerateTiles(tiles);

    Gameplay::tutorial();
    while (running)
    {
        frameStart = SDL_GetTicks(); // Start time of the frame
        // Check input
        /*while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_EVENT_QUIT) { running = false; }
            if (e.type == SDL_EVENT_KEY_DOWN)
            {
                if (e.key.key == SDLK_LEFT)
                {
                    if (MoveTiles(tiles, LEFT, frameStart, frameTime) == "Lost") {
                        Gameplay::lose();
                        SDL_Quit();
                        exit(0);
                    }
                }
                if (e.key.key == SDLK_RIGHT)
                {
                    if (MoveTiles(tiles, RIGHT, frameStart, frameTime) == "Lost") {
                        Gameplay::lose();
                        SDL_Quit();
                        exit(0);
                    }
                }
                if (e.key.key == SDLK_UP)
                {
                    if (MoveTiles(tiles, UP, frameStart, frameTime) == "Lost") {
                        Gameplay::lose();
                        SDL_Quit();
                        exit(0);
                    }
                }
                if (e.key.key == SDLK_DOWN)
                {
                    if (MoveTiles(tiles, DOWN, frameStart, frameTime) == "Lost") {
                        Gameplay::lose();
                        SDL_Quit();
                        exit(0);
                    }
                }
            }
        }*/

        DrawMain(tiles);

        for (const auto& tile : tiles) {
            if (tile.value == 2048) {
                SDL_Delay(1000);
                Gameplay::win(); // Вызываем функцию победы
                running = false; // Останавливаем игровой цикл
                break; // Выходим из цикла, так как мы уже нашли плитку
            }
        }

        // Calculate frame time and introduce a delay if necessary
        frameTime = SDL_GetTicks() - frameStart; // Time taken to render the frame
        if (FRAME_DELAY > frameTime) {
            SDL_Delay((Uint32)(FRAME_DELAY - frameTime)); // Wait for the remaining time
        }
    }

    return 0;

}