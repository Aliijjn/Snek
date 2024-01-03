#include <Windows.h>
#include <intrin.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

#define BLOCK_SIZE 60
#define THIN_SIZE 32
#define OFFSET_CENTRE ((BLOCK_SIZE - THIN_SIZE) / 4)
#define OFFSET_SIZE ((BLOCK_SIZE - THIN_SIZE) / 2)
#define BLOCK_COUNT 16
#define FOOD_COUNT 3
#define GAP 4
#define WIDTH 1024
#define HEIGHT 1024
#define SCR_WIDTH 1224
#define SCR_HEIGHT 1224
#define WHITE (pixel) { 0, 0, 0 }
#define BLACK (pixel) { 255, 255, 255 }

int windowMessageHandler(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
WNDCLASSA window_class = { .lpfnWndProc = windowMessageHandler,.lpszClassName = "class",.lpszMenuName = "class" };
BITMAPINFO bmi = { sizeof(BITMAPINFOHEADER), WIDTH, HEIGHT,1,32,BI_RGB };
HWND window;
HDC windowDC;

typedef struct 
{
    unsigned char b, g, r, a;
}pixel;

typedef struct
{
    unsigned char pos;
    pixel colour;
}food;

enum
{
    LEFT = -1, UP = BLOCK_COUNT, DOWN = -BLOCK_COUNT, RIGHT = 1
};

enum
{
    EMPTY, WALL
};

pixel vram[WIDTH * HEIGHT];
unsigned char grid[BLOCK_COUNT * BLOCK_COUNT];

unsigned char snake[BLOCK_COUNT * BLOCK_COUNT];
unsigned char snakesize = 1;
food candy[FOOD_COUNT];
unsigned char random_colour;
char snakedirection = UP;
bool changeDirection;

int windowMessageHandler(HWND window, UINT msg, WPARAM wParam, LPARAM lParam) 
{
    switch (msg)
    {
    case WM_KEYDOWN:
        switch (wParam)
        {
        case 'A':
            if (snakedirection != RIGHT && !changeDirection)
            {
                snakedirection = LEFT;
                changeDirection = true;
            }
            break;
        case 'W':
            if (snakedirection != DOWN && !changeDirection)
            {
                snakedirection = UP;
                changeDirection = true;
            }
            break;
        case 'S':
            if (snakedirection != UP && !changeDirection)
            {
                snakedirection = DOWN;
                changeDirection = true;
            }
            break;
        case 'D':
            if (snakedirection != LEFT && !changeDirection)
            {
                snakedirection = RIGHT;
                changeDirection = true;
            }
            break;
        }
        break;
    }
    return DefWindowProcA(window, msg, wParam, lParam);
}

int tRand(int range)
{
    unsigned int x = __rdtsc();
    x += (x << 10);
    x ^= (x >> 6);
    x += (x << 3);
    x ^= (x >> 11);
    x += (x << 15);
    return (x % range);
}

void drawrectangle(int x, int y, int width, int height, pixel colour)
{
    int tempwidth = width;
    width += x;
    height += y;
    
    for (; y < height; y++)
    {
        for (; x < width; x++)
        {
            vram[x + y * WIDTH] = colour;
        }
        x -= tempwidth;
    }
}

void drawmiddlerectangle(int x, int y, int width, int height, pixel colour)
{
    x -= width / 2;
    y -= height / 2;
    drawrectangle(x, y, width, height, colour);
}

bool checkinsnake(unsigned char pos)
{
    if (grid[pos] == WALL)
    {
        return (true);
    }
    for (int i = 0; i < snakesize; i++)
    {
        if (snake[i] == pos)
        {
            return (true);
        }
    }
    return (false);
}

void respawncandy(int candy_ID)
{
    const pixel colours[] =
    {
        { 255, 148, 0 },
        { 255, 255, 0 },
        { 0, 255, 0 },
        { 0, 0, 255 },
        { 160, 0, 128 },
        { 220, 0, 255 },
        { 0, 128, 255 },
        { 0, 255, 255 },
    };
    do
    {
        candy[candy_ID].pos = tRand(256);
    } while (checkinsnake(candy[candy_ID].pos));
    candy[candy_ID].colour = colours[random_colour];
    random_colour = (random_colour + 1 + tRand(2)) % 8;
}

#define BRICK_WIDTH 28
#define BRICK_HEIGHT 12
#define BRICK_HALF_WIDTH 12
#define BRICK_GAP 4
#define BRICK_COLOUR (pixel) { 0, 0, 0 }

void draw_brick(int x, int y)
{
    drawrectangle(x, y, BLOCK_SIZE, BLOCK_SIZE, (pixel) { 255, 255, 255 });
    
    drawrectangle(x, y, BRICK_WIDTH, BRICK_HEIGHT, BRICK_COLOUR);
    drawrectangle(x + BRICK_WIDTH + BRICK_GAP, y, BRICK_WIDTH, BRICK_HEIGHT, BRICK_COLOUR);
    drawrectangle(x, y + BRICK_HEIGHT + BRICK_GAP, BRICK_HALF_WIDTH, BRICK_HEIGHT, BRICK_COLOUR);
    drawrectangle(x + BRICK_HALF_WIDTH + BRICK_GAP, y + BRICK_HEIGHT + BRICK_GAP, BRICK_WIDTH, BRICK_HEIGHT, BRICK_COLOUR);
    drawrectangle(x + BRICK_HALF_WIDTH + BRICK_WIDTH + BRICK_GAP * 2, y + BRICK_HEIGHT + BRICK_GAP, BRICK_HALF_WIDTH, BRICK_HEIGHT, BRICK_COLOUR);
    y += (BRICK_HEIGHT + BRICK_GAP) * 2;
    drawrectangle(x, y, BRICK_WIDTH, BRICK_HEIGHT, BRICK_COLOUR);
    drawrectangle(x + BRICK_WIDTH + BRICK_GAP, y, BRICK_WIDTH, BRICK_HEIGHT, BRICK_COLOUR);
    drawrectangle(x, y + BRICK_HEIGHT + BRICK_GAP, BRICK_HALF_WIDTH, BRICK_HEIGHT, BRICK_COLOUR);
    drawrectangle(x + BRICK_HALF_WIDTH + BRICK_GAP, y + BRICK_HEIGHT + BRICK_GAP, BRICK_WIDTH, BRICK_HEIGHT, BRICK_COLOUR);
    drawrectangle(x + BRICK_HALF_WIDTH + BRICK_WIDTH + BRICK_GAP * 2, y + BRICK_HEIGHT + BRICK_GAP, BRICK_HALF_WIDTH, BRICK_HEIGHT, BRICK_COLOUR);
}

void init()
{
    for (int i = 1; i < BLOCK_COUNT - 1; i += 2)
    {
        for (int j = 1; j < BLOCK_COUNT - 1; j += 2)
        {
            if (tRand(6) == 0)
            {
                grid[i * BLOCK_COUNT + j] = WALL;
            }
        }
    }
    snake[0] = tRand(128) * 2;
    random_colour = tRand(8);
    for (int i = 0; i < FOOD_COUNT; i++)
    {
        respawncandy(i);
    }
    drawrectangle(0, 0, WIDTH, HEIGHT, (pixel) { 255, 255, 255 });
    for (int i = 0; i < BLOCK_COUNT * BLOCK_COUNT; i++)
    {
        int rectangle_x = i % BLOCK_COUNT * (BLOCK_SIZE + GAP);
        int rectangle_y = i / BLOCK_COUNT * (BLOCK_SIZE + GAP);
        if (grid[i] == WALL)
        {
            draw_brick(rectangle_x, rectangle_y);
            continue;
        }
        drawrectangle(rectangle_x, rectangle_y, BLOCK_SIZE, BLOCK_SIZE, (pixel) { 0, 0, 0 });
    }
}

void physics()
{
    int swap;

    if (checkinsnake(snake[0] + snakedirection))
    {
        Sleep(2000);
        exit(0);
    }

    swap = snake[0];
    if (snakedirection == RIGHT && snake[0] % BLOCK_COUNT == BLOCK_COUNT - 1)
    {
        snake[0] += snakedirection - BLOCK_COUNT;
    }
    else if (snakedirection == LEFT && snake[0] % BLOCK_COUNT == 0)
    {
        snake[0] += snakedirection + BLOCK_COUNT;
    }
    else
    {
        snake[0] += snakedirection;
    }

    for (int i = 1; i < snakesize; i++)
    {
        int swap2 = snake[i];
        snake[i] = swap;
        swap = swap2;
    }
    for (int i = 0; i < FOOD_COUNT; i++)
    {
        if (snake[0] == candy[i].pos)
        {
            respawncandy(i);
            snake[snakesize] = swap;
            snakesize++;
        }
    }
}

void draw()
{  
    init();
    for (;;)
    {
        physics();

        int snake_x = snake[0] % BLOCK_COUNT * (BLOCK_SIZE + GAP);
        int snake_y = snake[0] / BLOCK_COUNT * (BLOCK_SIZE + GAP);
        drawrectangle(snake_x, snake_y, BLOCK_SIZE, BLOCK_SIZE, (pixel) { 255, 255, 255 });
        drawmiddlerectangle(snake_x - 12 + BLOCK_SIZE / 2, snake_y + BLOCK_SIZE / 2, 4, 8, (pixel) { 0, 0, 0 });
        drawmiddlerectangle(snake_x + 12 + BLOCK_SIZE / 2, snake_y + BLOCK_SIZE / 2, 4, 8, (pixel) { 0, 0, 0 });

        snake_x = snake[1] % BLOCK_COUNT * (BLOCK_SIZE + GAP) + BLOCK_SIZE / 2;
        snake_y = snake[1] / BLOCK_COUNT * (BLOCK_SIZE + GAP) + BLOCK_SIZE / 2;

        drawmiddlerectangle(snake_x, snake_y, BLOCK_SIZE, BLOCK_SIZE, (pixel) { 0, 0, 0 });        
        
        if (snakesize >= 1)
        {
            if (snake[0] - snake[2] == 2 || snake[0] - snake[2] == -2 || snakesize == 2 && (snake[0] - snake[1] == 1 || snake[0] - snake[1] == -1))
            {
                drawmiddlerectangle(snake_x, snake_y, BLOCK_SIZE, THIN_SIZE, (pixel) { 255, 255, 255 });
            }
            else if (snake[0] - snake[2] == BLOCK_COUNT * 2 || snake[0] - snake[2] == -BLOCK_COUNT * 2 || snakesize == 2 && (snake[0] - snake[1] == BLOCK_COUNT || snake[0] - snake[1] == -BLOCK_COUNT))
            {
                drawmiddlerectangle(snake_x, snake_y, THIN_SIZE, BLOCK_SIZE, (pixel) { 255, 255, 255 });
            }
            else if (snake[1] + 1 == snake[0] && snake[1] + BLOCK_COUNT == snake[2] || snake[1] + 1 == snake[2] && snake[1] + BLOCK_COUNT == snake[0])
            {
                drawmiddlerectangle(snake_x, snake_y + OFFSET_CENTRE, THIN_SIZE, BLOCK_SIZE - OFFSET_SIZE, (pixel) { 255, 255, 255 });
                drawmiddlerectangle(snake_x + OFFSET_CENTRE, snake_y, BLOCK_SIZE - OFFSET_SIZE, THIN_SIZE, (pixel) { 255, 255, 255 });
            }
            else if (snake[1] - 1 == snake[0] && snake[1] + BLOCK_COUNT == snake[2] || snake[1] - 1 == snake[2] && snake[1] + BLOCK_COUNT == snake[0])
            {
                drawmiddlerectangle(snake_x, snake_y + OFFSET_CENTRE, THIN_SIZE, BLOCK_SIZE - OFFSET_SIZE, (pixel) { 255, 255, 255 });
                drawmiddlerectangle(snake_x - OFFSET_CENTRE, snake_y, BLOCK_SIZE - OFFSET_SIZE, THIN_SIZE, (pixel) { 255, 255, 255 });
            }
            else if (snake[1] + 1 == snake[0] && snake[1] - BLOCK_COUNT == snake[2] || snake[1] + 1 == snake[2] && snake[1] - BLOCK_COUNT == snake[0])
            {
                drawmiddlerectangle(snake_x, snake_y - OFFSET_CENTRE, THIN_SIZE, BLOCK_SIZE - OFFSET_SIZE, (pixel) { 255, 255, 255 });
                drawmiddlerectangle(snake_x + OFFSET_CENTRE, snake_y, BLOCK_SIZE - OFFSET_SIZE, THIN_SIZE, (pixel) { 255, 255, 255 });
            }
            else if (snake[1] - 1 == snake[0] && snake[1] - BLOCK_COUNT == snake[2] || snake[1] - 1 == snake[2] && snake[1] - BLOCK_COUNT == snake[0])
            {
                drawmiddlerectangle(snake_x, snake_y - OFFSET_CENTRE, THIN_SIZE, BLOCK_SIZE - OFFSET_SIZE, (pixel) { 255, 255, 255 });
                drawmiddlerectangle(snake_x - OFFSET_CENTRE, snake_y, BLOCK_SIZE - OFFSET_SIZE, THIN_SIZE, (pixel) { 255, 255, 255 });
            }
        }
        
        for (int i = 0; i < FOOD_COUNT; i++)
        {
            int candy_x = candy[i].pos % BLOCK_COUNT * (BLOCK_SIZE + GAP) + BLOCK_SIZE / 2;
            int candy_y = candy[i].pos / BLOCK_COUNT * (BLOCK_SIZE + GAP) + BLOCK_SIZE / 2;
            drawmiddlerectangle(candy_x, candy_y, BLOCK_SIZE / 2, BLOCK_SIZE / 2, candy[i].colour);
        }
        StretchDIBits(windowDC, 0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, WIDTH, HEIGHT, vram, &bmi, 0, SRCCOPY);
        
        snake_x = snake[snakesize - 1] % BLOCK_COUNT * (BLOCK_SIZE + GAP);
        snake_y = snake[snakesize - 1] / BLOCK_COUNT * (BLOCK_SIZE + GAP);
        drawrectangle(snake_x, snake_y, BLOCK_SIZE, BLOCK_SIZE, (pixel) { 0, 0, 0 });
        changeDirection = false;
        Sleep(166);
    }
}

void main() {
    RegisterClassA(&window_class);
    window = CreateWindowExA(0, "class", "hello", WS_POPUP | WS_VISIBLE, 0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, window_class.hInstance, 0);

    MSG message;
#pragma comment(lib,"Winmm.lib")
    timeBeginPeriod(1);
    windowDC = GetDC(window);
    CreateThread(0, 0, draw, 0, 0, 0);

    while (GetMessageA(&message, window, 0, 0)) {
        TranslateMessage(&message);
        DispatchMessageA(&message);
    }
}