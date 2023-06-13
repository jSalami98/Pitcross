/**
 * gameboard.h
 * @author John Salazar
 * @version 0.75
 * @date March 18, 2023
*/

#include <cstdint>
#include <sstream>

#ifndef GAMEBOARD_H
#define GAMEBOARD_H

/*  defines */
#ifndef SCREENSPACE_X
#define SCREENSPACE_X 150		//how big the screen is in the x direction
#endif

#ifndef SCREENSPACE_Y
#define SCREENSPACE_Y 150       //how big the screen is in the y direction
#endif	


/*  structs */
/**
 * The gameSpace struct holds the status of the spaces in the game board 
*/
enum spaceState
{
    unknown = -1,
	crossedOut,
	filledIn
};

/*  functions   */
void getFillPointsFromMousePoints(int32_t& x, int32_t& y, int32_t w, int32_t h)
{
	//*
	if(x < SCREENSPACE_X / 6 || y < SCREENSPACE_Y / 6 || x > 5 * SCREENSPACE_X / 6 || y > 5 * SCREENSPACE_Y / 6)
	{
		x = -1;
		y = -1;
		return;
	}
	//*/
	
	x -= SCREENSPACE_X / 6;
	y -= SCREENSPACE_Y / 6;

	/*
	x -= w / 2;
	y -= h / 2;
	*/

	//find X
	//x = int32_t(x / (2.0/3.0 * w) * w) + SCREENSPACE_X / 6;
	x = int32_t(x/w) * w;

	//find y
	//y = (int32_t((2 * y) / (3 * h) * h)) + SCREENSPACE_Y / 6;
	y = int32_t(y/h) * h;
}

//template<size_t rows, size_t cols>
void loadKeySpace(std::vector<std::vector<spaceState>> &mat/*spaceState (&mat)[rows][cols]*/, const std::string& binaryString)
{
	
	//find x size
	if(binaryString.size() < 8)
	{
		//goto loadKeySpaceERROR;
		std::cerr << "Error: String too short X" << std::endl;
	}
	int x = std::stoi(binaryString.substr(0,8), nullptr, 2);

	//find y size
	if(binaryString.size() < 16)
	{
		//goto loadKeySpaceERROR;
		std::cerr << "Error: String too short Y" << std::endl;
	}
	int y = std::stoi(binaryString.substr(8,8), nullptr, 2);
	
	for(int i = 0; i < mat.size()/*rows*/; i++)
	{
		for(int j = 0; j < mat[i].size()/*cols*/; j++)
		{
			
			//*
			if(binaryString.size() <= (i * x + j))
			{
				//goto loadKeySpaceERROR;
				std::cerr << "Error: String too short" << std::endl;
				//return;
			}
			
			if(binaryString.c_str()[ i * x + j + 16] == '1')	//binaryString.compare("1", i * x + j) == 0
			{
				mat[i][j] = filledIn;
				std::cout << "found a 1" << std::endl;
			}
			else if(binaryString.c_str()[ i * x + j + 16] == '0')	//binaryString.at(i * x + j) == char("0")
			{
				mat[i][j] = crossedOut;
				std::cout << "found a 1" << std::endl;
			}
			else
			{
				//goto loadKeySpaceERROR;
				std::cerr << "Error: Invalid input" << std::endl;
				//return;
			}
			//*/

			//mat[i][j] = (binaryString[i * x + j] == "1"? filledIn : (binaryString[i * x + j] == "0"? crossedOut : goto loadKeySpaceERROR));
			
			/*
			//hardcoaded test puzzle 1
			if(i == 0 || i == 3)
			{
				mat[i][j] = crossedOut;
			}
			else
			{
				if(j == 0 || j == 3)
				{
					mat[i][j] = crossedOut;
				}
				else
				{
					mat[i][j] = filledIn;
				}
			}
			//*/
		}
	}

	//return;

	for(int i = 0; i < x; i++)
	{
		for(int j = 0; j < y; j++)
		{
			std::cout << (mat[i][j] == filledIn? "* " : "- ");
		}
		std::cout << std::endl;
	}

/*
loadKeySpaceERROR:
	std::cerr << "\t~ERROR! - Incorect puzzle data!~" << std::endl;
		std::cerr << "\t\t AT: " << binaryString << std::endl;
	exit(0);
	return;
*/
}

//template<size_t rows, size_t cols>
bool checkWinCondition(std::vector<std::vector<spaceState>> &play/*const spaceState (&play)[rows][cols]*/, std::vector<std::vector<spaceState>> &key /*const spaceState (&key)[rows][cols]*/)
{
	
	for(int i = 0; i < play.size() /*rows*/; i++)
	{
		for(int j = 0; j < play[i].size() /*cols*/; j++)
		{
			if(((key[i][j] == filledIn) && (play[i][j] != filledIn)) || (play[i][j] == filledIn && key[i][j] != filledIn))
			{
				return false;
			}

		}
	}

	return true;
}

//template<size_t rows, size_t cols>
void findHintsFromKey(std::vector<std::string>& hints, std::vector<std::vector<spaceState>> &key /*const spaceState (&key)[rows][cols]*/)
{

	int count = 0;
	std::stringstream temp;

	//hints.push_back

	//find coloums (last to first)
	for(int j = (key[0].size() /*cols*/) - 1; j > -1; j--)
	{
		for(int i = 0; i < key.size()/*rows*/; i++)
		{
			if(key[i][j] == filledIn)
			{
				count++;
			}
			else if(count > 0)
			{
				temp << count << " ";
				count = 0;
			}
		}

		if(count > 0)
		{
			temp << count << " ";
			count = 0;
		}

		hints.push_back(temp.str());
		temp.str(std::string());

	}

	//find rows (top to bottom)
	for(int i = 0; i < key.size() /*rows*/; i++)
	{
		for(int j = 0; j < key[0].size()/*cols*/; j++)
		{
			if(key[i][j] == filledIn)
			{
				count++;
			}
			else if(count > 0)
			{
				temp << count << " ";
				count = 0;
			}
		}

		if(count > 0)
		{
			temp << count << " ";
			count = 0;
		}

		hints.push_back(temp.str());
		temp.str(std::string());

	}

}

#endif