#include <SDL.h>
#include <iostream>

using namespace std;

SDL_Window* theWindow;
SDL_Surface* theSurface;
SDL_Renderer* theRenderer;

const int MAZE_WIDTH = 8, MAZE_HEIGHT = 6;

int SCREEN_WIDTH = 400, SCREEN_HEIGHT = 400;

enum class DIR : int
{
    UP,
    RIGHT,
    DOWN,
    LEFT
};

inline int WrapAroundDir(int dir)
{
    return (dir + 4) % 4;
}

bool printLevel=false;

bool PrepareSDL()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        return false;
    }
    else
    {
        theWindow = SDL_CreateWindow("damauze", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (theWindow == NULL)
        {
            return false;
        }
        else
        {
            theRenderer = SDL_CreateRenderer(theWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
            if (theRenderer == NULL)
            {
                return false;
            }
            else
            {
                SDL_SetRenderDrawColor(theRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
            }
        }
    }
    return true;
}
void CloseAll()
{
    SDL_DestroyWindow(theWindow);
    theWindow = NULL;

    SDL_Quit();
}

int maze[MAZE_HEIGHT][MAZE_WIDTH] =
{
    {1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,1},
    {1,1,0,1,1,1,2,1},
    {1,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,0,1},
    {1,1,1,1,1,1,1,1}
};

class Player
{
public:
    std::pair<int, int> pos;
    DIR dirEnum;
    Player() : pos(std::make_pair(1,1)), dirEnum(DIR::UP)
    {}
} p;

int getField(std::pair<int, int> coord)
{
    return maze[coord.first][coord.second];
}

DIR turn(DIR originalDir, DIR turnDir)
{
    return static_cast<DIR>(WrapAroundDir(static_cast<int>(originalDir) + (turnDir == DIR::LEFT ? -1 : 0) + (turnDir == DIR::RIGHT ? 1 : 0) + (turnDir == DIR::DOWN ? -2 : 0)));
}

std::pair<int, int> simulateMove( std::pair<int, int> originalPos, DIR dir, DIR turnOrMoveDirection, bool respectWalls = false )
{
    DIR targetDir = turn(dir, turnOrMoveDirection);
    int y, x;
    y = originalPos.first + (targetDir == DIR::UP ? -1 : 0) + (targetDir == DIR::DOWN ? 1 : 0);
    x = originalPos.second + (targetDir == DIR::RIGHT ? 1 : 0) + (targetDir == DIR::LEFT ? -1 : 0);
    auto newPos = std::make_pair(y, x);

    if (respectWalls)
    {
        if (getField(newPos) == 1)
        {
            return originalPos;
        }
    }

    return newPos;
}

int main(int argc, char* args[])
{
    if (PrepareSDL())
    {
        bool quit = false;
        SDL_Event e;
        while (!quit)
        {
            while (SDL_PollEvent(&e) != 0)
            {
                if (e.type == SDL_QUIT)
                {
                    quit = true;
                }
                switch (e.type) {
                    case SDL_KEYDOWN:
                    {
                        switch (e.key.keysym.sym) {
                            case SDLK_LEFT:
                                p.dirEnum = turn(p.dirEnum, DIR::LEFT);
                                break;
                            case SDLK_RIGHT:
                                p.dirEnum = turn(p.dirEnum, DIR::RIGHT);
                                break;
                            case SDLK_UP:
                            {
                                p.pos = simulateMove(p.pos, p.dirEnum, DIR::UP, true);
                                break;
                            }
                            case SDLK_DOWN:
                            {
                                p.pos = simulateMove(p.pos, p.dirEnum, DIR::DOWN, true);
                                break;
                            }
                            default:
                                break;
                        }

                        printLevel = true;
                    }
                }
            }

            //RENDERING GOES HERE
            SDL_SetRenderDrawColor(theRenderer, 255, 255, 255, 255);
            SDL_RenderClear(theRenderer);
            SDL_SetRenderDrawColor(theRenderer, 0, 0, 0, 255);

            int maxFrame = SCREEN_HEIGHT;
            int frame = maxFrame; //the view will get progresively smaller as it goes forward

            bool hitWall = false; //will send a "blast" forward which will countinously render the scene as it goes until it hits a wall

            std::pair<int, int> b = p.pos;
            while (!hitWall)
            {
                int perDif = frame / 5; //the perspective difference between advancements

                if (getField(b) == 1) //wall the furthest from the player
                {
                    hitWall = true;

                    SDL_RenderDrawLine(theRenderer, (maxFrame - frame) / 2, (maxFrame - frame) / 2, (maxFrame - frame) / 2 + frame, (maxFrame - frame) / 2); //furthest horizontal line on the top
                    SDL_RenderDrawLine(theRenderer, (maxFrame - frame) / 2, (maxFrame - frame) / 2 + frame, (maxFrame - frame) / 2 + frame, (maxFrame - frame) / 2 + frame); //furthest horizontal line on the bottom

                    auto walkerHelperStepBack = simulateMove(b, p.dirEnum, DIR::DOWN);

                    //Furthest vertical line on the right
                    if (getField(simulateMove(walkerHelperStepBack, p.dirEnum, DIR::RIGHT)) == 1)
                    {
                        SDL_RenderDrawLine(theRenderer, (maxFrame - frame) / 2 + frame, (maxFrame - frame) / 2, (maxFrame - frame) / 2 + frame, (maxFrame - frame) / 2 + frame);
                    }

                    //Furthest vertical line on the left
                    if (getField(simulateMove(walkerHelperStepBack, p.dirEnum, DIR::LEFT)) == 1)
                    {
                        SDL_RenderDrawLine(theRenderer, (maxFrame - frame) / 2, (maxFrame - frame) / 2, (maxFrame - frame) / 2, (maxFrame - frame) / 2 + frame);
                    }
                    continue;
                }

                auto walkerHelperStepRight = simulateMove(b, p.dirEnum, DIR::RIGHT);
                if (getField(walkerHelperStepRight) == 1) //draw right side walls
                {
                    int l1x = maxFrame - (maxFrame - frame) / 2, l1y = (maxFrame - frame) / 2;
                    int l2x = maxFrame - (maxFrame - frame) / 2, l2y = (maxFrame - frame) / 2 + frame;

                    SDL_RenderDrawLine(theRenderer, l1x, l1y, l1x - perDif, l1y + perDif); //upper right diagonal line
                    SDL_RenderDrawLine(theRenderer, l2x, l2y , l2x - perDif, l2y - perDif); //lower right diagonal line

                    auto walkerHelperStepBackRight = simulateMove(walkerHelperStepRight, p.dirEnum, DIR::DOWN);
                    if (getField(walkerHelperStepBackRight) == 0)
                    {
                        SDL_RenderDrawLine(theRenderer, l1x, l1y, l1x, l2y); //vertical right split line
                    }
                }
                else //fill with wall break on the right
                {
                    int lx1 = maxFrame - (maxFrame - frame) / 2, ly1 = (maxFrame - frame) / 2;

                    SDL_RenderDrawLine(theRenderer, lx1, ly1, lx1 , ly1 + frame); //vertical right split line
                    SDL_RenderDrawLine(theRenderer, lx1 - perDif, ly1 + perDif, lx1, ly1 + perDif ); //top horizontal split line
                    SDL_RenderDrawLine(theRenderer, lx1 - perDif, ly1 - perDif + frame, lx1, ly1 - perDif+frame); //bottom horizontal split line
                }

                auto walkerHelperStepLeft = simulateMove(b, p.dirEnum, DIR::LEFT);
                if (getField(walkerHelperStepLeft) == 1) //draw left side walls
                {
                    int l1x = (maxFrame - frame) / 2, l1y = (maxFrame - frame) / 2;
                    int l2x = (maxFrame - frame) / 2, l2y = (maxFrame - frame) / 2 + frame;

                    SDL_RenderDrawLine(theRenderer, l1x, l1y, l1x+perDif, l1y+perDif); //upper left diagonal line
                    SDL_RenderDrawLine(theRenderer, l2x, l2y, l2x+perDif, l2y-perDif); //lower left diagonal line

                    auto walkerHelperStepBackLeft = simulateMove(walkerHelperStepLeft, p.dirEnum, DIR::DOWN);
                    if (getField(walkerHelperStepBackLeft) == 0)
                    {
                        SDL_RenderDrawLine(theRenderer, l1x, l1y, l1x, l2y); //vertical left split line
                    }
                }
                else //fill with wall break on the left
                {
                    int lx1 = (maxFrame - frame) / 2, ly1 = (maxFrame - frame) / 2;

                    SDL_RenderDrawLine(theRenderer, lx1, ly1, lx1, ly1 + frame);
                    SDL_RenderDrawLine(theRenderer, lx1 + perDif, ly1 + perDif, lx1, ly1 + perDif);
                    SDL_RenderDrawLine(theRenderer, lx1 + perDif, ly1 - perDif + frame, lx1, ly1 - perDif + frame);
                }

                if (!hitWall)
                {
                    b = simulateMove(b, p.dirEnum, DIR::UP);
                }

                frame -= perDif * 2; //shrink frame as the view advances
            }

            if (printLevel)
            {
                cout << "pdir " << (int) p.dirEnum << endl;
                for (int i = 0; i < MAZE_HEIGHT; i++)
                {
                    for (int j = 0; j < MAZE_WIDTH; j++)
                    {
                        if (i == p.pos.first and j == p.pos.second) {
                            switch (p.dirEnum)
                            {
                                case DIR::UP:
                                {
                                    cout << "^";
                                    break;
                                };
                                case DIR::DOWN:
                                {
                                    cout << "v";
                                    break;
                                };
                                case DIR::LEFT:
                                {
                                    cout << "<";
                                    break;
                                };
                                case DIR::RIGHT:
                                {
                                    cout << ">";
                                    break;
                                };
                                default:
                                {
                                    __debugbreak(); //Wrong direction
                                }
                            }
                        }
                        else if (i == b.first && j == b.second) cout << "x";
                        else if (maze[i][j] == 1) std::cout << "#";
                        else std::cout << " ";
                    }
                    std::cout << endl;
                }
                printLevel = false;
                std::cout << endl;
            }

            SDL_RenderPresent(theRenderer);
            SDL_Delay(1.0/60.0);
        }
    }
    return 0;
}
