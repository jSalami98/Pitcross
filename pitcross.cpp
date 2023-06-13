/**
 * pitcross.cpp
 * @author John Salazar
 * @version 0.6.9
 * @date March 3, 2023
*/

//Windows
//g++ -o pitcross.exe pitcross.cpp -luser32 -lgdi32 -lopengl32 -lgdiplus -lShlwapi -ldwmapi -lstdc++fs -static -std=c++17
//Linux
//g++ -o pitcross pitcross.cpp -lX11 -lGL -lpthread -lpng -lstdc++fs -std=c++17

/*
	Pitcross/Nonogram binary compression format - breakdown
	the first 8 bits represent the width of the puzzle: 0x4, 0b00000100
	the next 8 bits represent the hight of the puzzle: 0x4, 0b00000100
	the remaining bits represent the state of the puzzle starting from the upper-left bit moving rightwards and then downwards:
		puzzle: ----, binary: 0000
		puzzle: -**-, binary: 0110
		puzzle: -**-, binary: 0110
		puzzle: ----, binary: 0000
		all toghther: 0b0000011001100000

		Example puzzle 1
			----
			-**-
			-**-
			----
	the above puzzle can be stored in binary as:
	0b00000100000001000000011001100000
	
		Example puzzle 2
			-*-*-
			*-*-*
			*---*
			-*-*-
			--*--
	the above puzzle can be stored in binary as:
	0b00000101000001010101010101100010101000100

		Example Puzzle 3
			*******
			*-----*
			*-***-*
			*-----*
			*******
	the above puzzle can be stored in binary as:
	0b000001110000010111111111000001101110110000011111111
	
*/

#define SCREENSPACE_X 400		//how big the screen is in the x direction
#define SCREENSPACE_Y 400		//how big the screen is in the y direction

#define OLC_PGE_APPLICATION
#define DEBUG_PITCROSS

#include <vector>
#include <algorithm>
#include "olcPixelGameEngine.h"
#include "gameboard.h"

class Example : public olc::PixelGameEngine
{
public:

    bool winFlag = false;
	bool menuFlag = true;
	
	int xSpaces;	//10;		//how many squares in the x axis of the play area
    int ySpaces;	//10;		//how many squares in the y axis of the play area

	int xSpaceSize; 	//how wide (x) a square is
	int ySpaceSize;	//how tall (y) a square is

	std::string keyString;

	std::vector<std::vector<spaceState>> gridSpace;
	std::vector<std::vector<spaceState>> keySpace;

	std::vector<std::string> pitcrossEventLog;
	std::vector<std::string> hintsVectror;
	std::vector<std::vector<spaceState>> last;

	Example(int xSpace_Local, int ySpace_Local, std::string keyString_Local)
	{
		sAppName = "PitCross++";

		xSpaces = xSpace_Local;
		ySpaces = ySpace_Local;

		xSpaceSize = (2 * SCREENSPACE_X) / (3 * xSpaces);
		ySpaceSize = (2 * SCREENSPACE_Y) / (3 * ySpaces);

		keyString = keyString_Local;

		//spaceState gridSpace[xSpaces][ySpaces] = {};
		//spaceState keySpace[xSpaces][ySpaces] = {};

		//gridSpace = new spaceState[xSpaces][ySpaces];

	}



public:
	bool OnUserCreate() override
	{
		// Called once at the start, so create things here
		
		if(SCREENSPACE_X % xSpaces != 0)
		{
			std::cerr << "	~Warning - MISMACH!~	Screen Space and Game Space (x) do not match!!!" << std::endl;
		}
		if(SCREENSPACE_Y % ySpaces != 0)
		{
			std::cerr << "	~Warning - MISMACH!~	Screen Space and Game Space (y) do not match!!!" << std::endl;
		}

		//Initialise gridSpace matrix
		/*
		for(int i = 0; i < xSpaces; i++)
		{
			for(int j = 0; j < ySpaces; j++)
			{
				gridSpace[i][j] = unknown;
				std::cout << "(" << i << "," << j << ") ";
			}
			std::cout << std::endl;
		}
		*/
		
		std::vector<spaceState> tempSpace;

		for(int j = 0; j < ySpaces; j++)
		{
			tempSpace.push_back(unknown);
		}
		for(int i = 0; i < xSpaces; i++)
		{
			gridSpace.push_back(tempSpace);
			keySpace.push_back(tempSpace);
			last.push_back(tempSpace);
		}



		//Initialise keySpace matrix
		loadKeySpace(keySpace, keyString);	//00000100000001000000011001100000
		pitcrossEventLog.push_back("Puzzle \"Test Puzzle 1\" has been loaded");

		findHintsFromKey(hintsVectror, keySpace);
		pitcrossEventLog.push_back("Generated hints");

		std::cout << "Hello World!" << std::endl;

		#ifdef DEBUG_PITCROSS

		for(const auto a : hintsVectror)
		{
			std::cout << a << std::endl;
		}

		for(int i = 0; i < xSpaces; i++)
		{
			for(int j = 0; j < ySpaces; j++)
			{
				std::cout << (keySpace[i][j] == filledIn? "* " : "- ");
			}
			std::cout << std::endl;
		}

		#endif
		
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		// called once per frame
		
		//Handle win
		if(checkWinCondition(gridSpace, keySpace) && !winFlag)
		{
			winFlag = true;
		}

		#ifdef DEBUG_PITCROSS
	
		//static spaceState last[xSpaces][ySpaces];
		

		if(!checkWinCondition(gridSpace, last))
		{
			for(int i = 0; i < xSpaces; i++)
			{
				for(int j = 0; j < ySpaces; j++)
				{
					std::cout << (gridSpace[i][j] == filledIn? "* " : "- ");
				}

				std::cout << "\t|\t";

				for(int j = 0; j < ySpaces; j++)
				{
					std::cout << (keySpace[i][j] == filledIn? "* " : "- ");
				}

				std::cout << std::endl;

			}

			std::cout << std::endl;

			for(int i = 0; i < xSpaces; i++)
			{
				for(int j = 0; j < ySpaces; j++)
				{
					last[i][j] = gridSpace[i][j];
				}
			}
		}
	
		#endif
		
		//blank the screen
		Clear(olc::BLACK);

		//find what space mouse is above
		int32_t MouseX = GetMouseX();
		int32_t MouseY = GetMouseY();
		getFillPointsFromMousePoints(MouseX, MouseY, xSpaceSize, ySpaceSize);

		

		//controll
		if(winFlag || (MouseX < 0 || MouseY < 0))	//avoid crashing by not accessing the "gridSpace" matrix if the user clicks out of bounds
		{
			//Do nothing
		}
			/*	~Fix!~	Replace the '- 2' with a dynamic way of finding the offset*/
    	else if(GetMouse(olc::Mouse::LEFT).bPressed)
    	{
        	gridSpace[(MouseY / ySpaceSize) /*- 2*/][(MouseX / xSpaceSize) /*- 2*/] = filledIn;
			
			std::stringstream temp;
			temp << "Space " << MouseX/xSpaceSize << ", " << MouseY/ySpaceSize << " has been filled in"; 
			pitcrossEventLog.push_back(temp.str());
			//std::cout << "Space " << MouseX/xSpaceSize << ", " << MouseY/ySpaceSize << " has been filled in" << std::endl;
		}
	   	else if(GetMouse(olc::Mouse::RIGHT).bPressed)
       	{
            gridSpace[(MouseY / ySpaceSize) /*- 2*/][(MouseX / xSpaceSize) /*- 2*/] = crossedOut;
			
			std::stringstream temp;
			temp << "Space " << MouseX/xSpaceSize << ", " << MouseY/ySpaceSize << " has been crossed out"; 
			pitcrossEventLog.push_back(temp.str());
	   	}
		else if(GetMouse(olc::Mouse::MIDDLE).bPressed)
		{
			gridSpace[(MouseY / ySpaceSize) /*- 2*/][(MouseX / xSpaceSize) /*- 2*/] = unknown;
			
			std::stringstream temp;
			temp << "Space " << MouseX/xSpaceSize << ", " << MouseY/ySpaceSize << " has been eraced"; 
			pitcrossEventLog.push_back(temp.str());
		}
	   	else if(GetKey(olc::ENTER).bPressed)		//fill all
	   	{
			for(int i = 0; i < xSpaces; i++)
			{
				for(int j = 0; j < ySpaces; j++)
				{
					gridSpace[i][j] = filledIn;
				}
			}
		}	
		else if(GetKey(olc::SPACE).bPressed)	//clear all
	   	{
			for(int i = 0; i < xSpaces; i++)
			{
				for(int j = 0; j < ySpaces; j++)
				{
					gridSpace[i][j] = unknown;
				}
			}
		}
		else if(GetKey(olc::TAB).bPressed)		//swap all
	   	{
			for(int i = 0; i < xSpaces; i++)
			{
				for(int j = 0; j < ySpaces; j++)
				{
					if(gridSpace[i][j] == filledIn) //gridSpace[i][j] = !gridSpace[i][j].status;
					{
						gridSpace[i][j] = unknown;
					} 
					else if(gridSpace[i][j] == unknown)
					{
						gridSpace[i][j] = filledIn;
					}
				}
			}
		}

		//draw the known in spaces
		for(int i = 0; i < xSpaces; i++)
		{
			for(int j = 0; j < ySpaces; j++)
			{
				if(gridSpace[i][j] == filledIn)
				{
					//FillRect(i * xSpaceSize + SCREENSPACE_X / 6, j * ySpaceSize + SCREENSPACE_Y / 6, xSpaceSize, ySpaceSize, winFlag? olc::YELLOW : olc::GREEN);
					FillRect(j * ySpaceSize + SCREENSPACE_Y / 6, i * xSpaceSize + SCREENSPACE_X / 6, ySpaceSize, xSpaceSize, winFlag? olc::YELLOW : olc::GREEN);
				}
				else if(gridSpace[i][j] == crossedOut)
				{
					//FillRect(i * xSpaceSize + SCREENSPACE_X / 6, j * ySpaceSize + SCREENSPACE_Y / 6, xSpaceSize, ySpaceSize, winFlag? olc::BLACK : olc::RED);
					FillRect(j * ySpaceSize + SCREENSPACE_Y / 6, i * xSpaceSize + SCREENSPACE_X / 6, ySpaceSize, xSpaceSize, winFlag? olc::BLACK : olc::RED);
				}
			}
		}

		//draw the highlighted space
		if(!winFlag && (MouseX >= 0 && MouseY >= 0))
		{
			FillRect(MouseX + SCREENSPACE_X / 6, MouseY + SCREENSPACE_Y / 6, 
				xSpaceSize, ySpaceSize, olc::GREY);
		}

		//draw the lines
		for(float i = 1; i < ySpaces; i++)
       	{
            DrawLine(SCREENSPACE_X / 6, i * ySpaceSize + SCREENSPACE_Y / 6, 5 * SCREENSPACE_X / 6, i * ySpaceSize + SCREENSPACE_Y / 6, olc::Pixel{0xF0, 0xF0, 0xF0});
       	}
		for(float i = 1; i < xSpaces; i++)
		{
			DrawLine(i * xSpaceSize + SCREENSPACE_X / 6, SCREENSPACE_Y / 6, i * xSpaceSize + SCREENSPACE_X / 6, 5 * SCREENSPACE_Y / 6, olc::Pixel{0xF0, 0xF0, 0xF0});
		}

		//Draw game border - 1/6th of the screen in all directions
		//*
		FillRect(0, 0, SCREENSPACE_X, SCREENSPACE_Y / 6, olc::Pixel{0xF0, 0xF0, 0xF0});							//North
		FillRect((SCREENSPACE_X / 6) + (xSpaces * xSpaceSize) /*5 * SCREENSPACE_X / 6*/, 0, SCREENSPACE_X /*/ 6*/, SCREENSPACE_Y, olc::Pixel{0xF0, 0xF0, 0xF0});		//East
		FillRect(0, (SCREENSPACE_Y / 6) + (ySpaces * ySpaceSize) /*5 * SCREENSPACE_Y / 6*/, SCREENSPACE_X, SCREENSPACE_Y /*/ 6*/, olc::Pixel{0xF0, 0xF0, 0xF0});		//South
		FillRect(0, 0, SCREENSPACE_X / 6, SCREENSPACE_Y, olc::Pixel{0xF0, 0xF0, 0xF0});							//West
		//*/
		
		//Draw Hints
		//cols - need to figure out how to draw sideways
		//*
		for(int i = 0; i < ySpaces; i++)
		{
			DrawString((SCREENSPACE_X / 6) + (xSpaceSize / 2) + (i * xSpaceSize), SCREENSPACE_Y / 12, hintsVectror[ySpaces + i].empty()? "0" : hintsVectror[i], olc::BLACK);
		}
		//*/

		//rows
		for(int i = 0; i < xSpaces; i++)
		{
			DrawString(SCREENSPACE_X / 12, (SCREENSPACE_Y / 6) + (ySpaceSize / 2) + (i * ySpaceSize), hintsVectror[ySpaces + i].empty()? "0" : hintsVectror[ySpaces + i], olc::BLACK);
		}

		#ifdef DEBUG_PITCROSS

		//prints a log of events to the screen (not working right)
		if(pitcrossEventLog.size() > 5) //std::vector<std::string>::iterator
		{
			pitcrossEventLog.erase(pitcrossEventLog.begin());
		}

		for(int i = 0; i < pitcrossEventLog.size(); i++)
		{
			DrawString(SCREENSPACE_X / 6, (5 * SCREENSPACE_Y / 6) + (i * 10), pitcrossEventLog[i], olc::BLACK, 1);
		}

		#endif

       return true;
	}
};


int main()
{
	Example demo(5, 5, "00000101000001010101010101100010101000100");
	if (demo.Construct(SCREENSPACE_X, SCREENSPACE_Y, 1, 1))
		demo.Start();

	return 0;
}
