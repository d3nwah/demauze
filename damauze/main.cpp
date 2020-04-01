#include <SDL.h>
#include <iostream>

using namespace std;

SDL_Window* theWindow;
SDL_Surface* theSurface;
SDL_Renderer* theRenderer;

const int MAZE_WIDTH=8, MAZE_HEIGHT=6;

int SCREEN_WIDTH= 400, SCREEN_HEIGHT=400;

int dx[4] = { 0,1,0,-1 }; // up, right, down, left (from top down)
int dy[4] = { 1,0,-1,0 };

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
	{1,1,0,1,1,1,0,1},
	{1,0,0,0,0,0,0,1},
	{1,1,1,1,1,1,0,1},
	{1,1,1,1,1,1,1,1}
};

class player
{public:
	int px = 1, py=1;
	int dir = 0;
	void move(int val) { if (maze[py + val * dy[dir]][px + val * dx[dir]] != 1) { px += val * dx[dir]; py += val * dy[dir]; printLevel = true; } }
	void turn(int val) { dir = (dir + val + 4) % 4; std::cout << "dir set to<<" << dir << "\n"; printLevel = true; }
}p;

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
					/* Look for a keypress */
				case SDL_KEYDOWN:
					/* Check the SDLKey values and move change the coords */
					switch (e.key.keysym.sym) {
					case SDLK_LEFT:
						p.turn(1);
						break;
					case SDLK_RIGHT:
						p.turn(-1);
						break;
					case SDLK_UP:
						p.move(1);
						break;
					case SDLK_DOWN:
						p.move(-1);
						break;
					default:
						break;
					}
				}
			}
			SDL_SetRenderDrawColor(theRenderer, 255, 255, 255, 255);

			SDL_RenderClear(theRenderer);

			//RENDERING GOES HERE
			SDL_SetRenderDrawColor(theRenderer, 0, 0, 0, 255);
			bool hitWall = false; //will send a "blast" forward which will countinously render the scene as it goes until it hits a wall
			
			int maxFrame = SCREEN_HEIGHT;
			int frame = maxFrame; //the view will get progresively smaller as it goes forward

			int bx = p.px, by = p.py; //blast coords start from the player
				

			while (!hitWall)
			{
				int perDif = frame / 5; //the perspective difference between advancements
				
				if (maze[by][bx] == 1)
				{
					hitWall = true;
					//only one end wall will be rendered
					SDL_RenderDrawLine(theRenderer, (maxFrame - frame) / 2, (maxFrame - frame) / 2, (maxFrame - frame) / 2 + frame, (maxFrame - frame) / 2);
					SDL_RenderDrawLine(theRenderer, (maxFrame - frame) / 2, (maxFrame - frame) / 2+frame, (maxFrame - frame) / 2 + frame, (maxFrame - frame) / 2+frame);

					//the trick for this whole rendering process. The endmost wall will stretch to the left and/or right on a split section to look like it's blocking the view
					//this helps because no complex geometry has to be further drawn beyond this closing point
					if (maze[by + dy[(p.dir - 1 + 4) % 4] + dy[(p.dir - 2 + 4) % 4]][bx + dx[(p.dir - 1 + 4) % 4]+ dx[(p.dir - 2 + 4) % 4]] == 1)
						SDL_RenderDrawLine(theRenderer, (maxFrame - frame) / 2 + frame, (maxFrame - frame) / 2, (maxFrame - frame) / 2 + frame, (maxFrame - frame) / 2+frame);
					if (maze[by + dy[(p.dir + 1) % 4] + dy[(p.dir + 2) % 4]][bx + dx[(p.dir + 1)]+ dx[(p.dir + 2) % 4]] == 1)
						SDL_RenderDrawLine(theRenderer, (maxFrame - frame) / 2, (maxFrame - frame) / 2, (maxFrame - frame) / 2, (maxFrame - frame) / 2 + frame);

					continue;	//escape point. Get out of the loop safely
				}
				
				if (maze[by + dy[(p.dir - 1+4) % 4]][bx + dx[(p.dir - 1+4)%4]] == 1)	//draw right side walls
				{
					int l1x = maxFrame - (maxFrame - frame) / 2, l1y = (maxFrame - frame) / 2;
					int l2x = maxFrame - (maxFrame - frame) / 2, l2y = (maxFrame - frame) / 2 + frame;

					SDL_RenderDrawLine(theRenderer, l1x, l1y, l1x -perDif, l1y + perDif);
					SDL_RenderDrawLine(theRenderer, l2x, l2y , l2x - perDif, l2y- perDif);

					if (maze[by + dy[(p.dir - 1 + 4) % 4] + dy[(p.dir - 2 + 4) % 4]][bx + dx[(p.dir - 1 + 4) % 4] + dx[(p.dir - 2 + 4) % 4]] == 0)//add split line if gap
						SDL_RenderDrawLine(theRenderer, l1x, l1y, l1x, l2y);

				}
				else	//fill with wall break on the right
				{
					int lx1 = maxFrame - (maxFrame - frame) / 2, ly1 = (maxFrame - frame) / 2;

					SDL_RenderDrawLine(theRenderer, lx1, ly1, lx1 , ly1 + frame);
					SDL_RenderDrawLine(theRenderer, lx1 - perDif, ly1 + perDif, lx1, ly1 + perDif );
					SDL_RenderDrawLine(theRenderer, lx1 - perDif, ly1 - perDif+frame, lx1, ly1 - perDif+frame);

				}

				if (maze[by + dy[(p.dir + 1) % 4]][bx + dx[(p.dir + 1)]] == 1)			//draw left side walls
				{
					int l1x = (maxFrame - frame) / 2, l1y = (maxFrame - frame) / 2;
					int l2x = (maxFrame - frame) / 2, l2y = (maxFrame - frame) / 2 + frame;

					SDL_RenderDrawLine(theRenderer,l1x ,l1y , l1x+perDif , l1y+perDif);
					SDL_RenderDrawLine(theRenderer, l2x ,l2y , l2x+perDif, l2y-perDif);

					if (maze[by + dy[(p.dir + 1) % 4] + dy[(p.dir + 2) % 4]][bx + dx[(p.dir + 1)] + dx[(p.dir + 2) % 4]] == 0)	//add split line if gap
						SDL_RenderDrawLine(theRenderer, l1x, l1y, l1x, l2y);
				}

				else	//fill with wall break on the left
				{
					int lx1 =  (maxFrame - frame) / 2, ly1 = (maxFrame - frame) / 2;

					SDL_RenderDrawLine(theRenderer, lx1, ly1, lx1, ly1 + frame);
					SDL_RenderDrawLine(theRenderer, lx1 + perDif, ly1 + perDif, lx1, ly1 + perDif);
					SDL_RenderDrawLine(theRenderer, lx1 + perDif, ly1 - perDif + frame, lx1, ly1 - perDif + frame);	

				}


				if(!hitWall)
				bx += dx[p.dir], by += dy[p.dir];
				frame -= perDif * 2;	//shrink frame as the view advances
			}

			if (printLevel)
			{
				for (int i = 0; i < MAZE_HEIGHT; i++)
				{
					for (int j = 0; j < MAZE_WIDTH; j++)
					{
						if (i == p.py and j == p.px) {
							if (p.dir == 0)std::cout << "v"; else if (p.dir == 1)std::cout << ">"; else if (p.dir == 2)std::cout << "^"; else  std::cout << "<";
						}
						else if (i == by && j == bx) cout << "x";
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