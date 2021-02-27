#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <chrono>
#include <windows.h>
#include <algorithm>
#include <map> 

class Pizza
{
public:
	int ID;
	std::vector<int> hashedToppingList;
	bool isDelivered;

	Pizza() : ID(-1), isDelivered(false){	}

	//< operator is implemented for 
	//sort operation of the pizzas
	bool operator < (const Pizza& str) const
	{
		return (hashedToppingList.size() > str.hashedToppingList.size());
	}
};

void ReadInputs(std::string path, std::vector<int> & vectTeamSize, std::vector<Pizza> & pizzaList)
{
	//fills vectTeamSize and pizzaList vectors
	std::cout << "ReadInputs Operation Starts!" << std::endl;

	//uses a hash to translate from topping names
	//to integers to make calculations 
	//faster
	std::map<std::string, int> hash;
	unsigned int hashValue = 0;

	std::ifstream ReadFile(path);
	std::string text;
	std::string line;
	while (ReadFile.good())
	{
		std::getline(ReadFile, line);
		text += line;
		text += "\n";
	}

	std::stringstream stream(text);

	int nPizzaCount = 0;
	stream >> nPizzaCount >> vectTeamSize[0] >> vectTeamSize[1] >> vectTeamSize[2];

	//to make calculations faster 
	//allocate whole memory at first 
	pizzaList.resize(nPizzaCount);

	for (int pizzaIndex = 0; pizzaIndex < nPizzaCount; pizzaIndex++)
	{
		pizzaList[pizzaIndex].ID = pizzaIndex;

		int toppingCount = 0;
		stream >> toppingCount;

		//to make calculations faster 
		//allocate whole memory at first 
		pizzaList[pizzaIndex].hashedToppingList.resize(toppingCount, -1);

		for (int toppingIndex = 0; toppingIndex < toppingCount; toppingIndex++)
		{
			std::string topping;
			stream >> topping;

			auto itr = hash.find(topping);
			if (itr != hash.end())
			{
				//if it exists in the hash table
				//find hash value of the given topping(string)
				//use it as the hash value
				pizzaList[pizzaIndex].hashedToppingList[toppingIndex] = itr->second;
			}
			else
			{
				//if it does not exist in the hash table
				//use "hashValue variable" as the new hash value
				hash.insert({ topping, hashValue });
				pizzaList[pizzaIndex].hashedToppingList[toppingIndex] = hashValue++;
			}
		}
		//sort all the toppings in the list according to their integer equivalence
		std::sort(pizzaList[pizzaIndex].hashedToppingList.begin(), pizzaList[pizzaIndex].hashedToppingList.end());
	}

	ReadFile.close();
}

void sortPizzaList(std::vector<Pizza> & pizzaList)
{
	std::sort(pizzaList.begin(), pizzaList.end());
}

void SelectTeamsToBeServed(int par_nPizzaCount, std::vector<int> par_vectTeamSize, std::vector<int> & vectServedTeamCounts)
{
	//this function first serves for teamsof4
	//then serves for teamsof3
	//then serves for teamsof2
	//and fills the vectServedTeamCounts
	for (int nTeamSize = 4; nTeamSize > 1; nTeamSize--)
	{
		if (par_nPizzaCount < nTeamSize)
			break;

		int nVectorIterator = nTeamSize - 2;

		if (par_nPizzaCount / nTeamSize > par_vectTeamSize[nVectorIterator])
		{
			vectServedTeamCounts[nVectorIterator] = par_vectTeamSize[nVectorIterator];
			par_nPizzaCount -= par_vectTeamSize[nVectorIterator] * nTeamSize;
		}
		else
		{
			vectServedTeamCounts[nVectorIterator] = par_nPizzaCount / nTeamSize;
			par_nPizzaCount -= (par_nPizzaCount / nTeamSize) * nTeamSize;
		}
	}
}

int GetFirstAvailablePizzaIndex(std::vector<Pizza> & pizzaList)
{
	//finds first available pizza in the list
	for (int i = 0; i < (int)pizzaList.size(); i++)
	{
		if (false == pizzaList[i].isDelivered)
		{
			return i;
		}
	}

	return -1;
}

void GetUnionOfToppings(std::vector<int> & unionVector, std::vector<int> & firstToppingList, std::vector<int> & secondToppingList)
{
	auto it = std::set_union(firstToppingList.begin(), firstToppingList.end(),
		secondToppingList.begin(), secondToppingList.end(),
		unionVector.begin());

	unionVector.resize(it - unionVector.begin());
}

int CalculatePizzaScore(std::vector<int> & firstToppingList, std::vector<int> & secondToppingList)
{
	std::vector<int> unionVector(firstToppingList.size() + secondToppingList.size());

	GetUnionOfToppings(unionVector, firstToppingList, secondToppingList);

	//since there is not any negative topping number, 
	//did not bother to get the square of the size
	//merely the size is enough for calc
	return unionVector.size();
}

int FindNextPizzaIndexThatGeneratesMaxScore(std::vector<Pizza> & pizzaList, std::vector<int> & toppingListSoFar, int lastPizzaIndex)
{
	int score = -1;
	int maxScore = -1;
	int maxScoreIndex = -1;
	for (int i = lastPizzaIndex + 1; i < (int)pizzaList.size(); i++)
	{
		if (true == pizzaList[i].isDelivered)
			continue;

		//mathSumOfToppings => sum of all topics even if they are same
		int mathSumOfToppings = (int)toppingListSoFar.size() + (int)pizzaList[i].hashedToppingList.size();

		//since pizza list is sorted according to pizza topping count
		//when sum of toppings starts to get equal to to maxscore
		//it is sure that it cannot be more than maxscore
		//so no need to visit all!
		if (maxScore >= mathSumOfToppings)
			break;

		score = CalculatePizzaScore(toppingListSoFar, pizzaList[i].hashedToppingList);

		if (score > maxScore)
		{
			maxScore = score;
			maxScoreIndex = i;
		}
	}

	return maxScoreIndex;
}

void SelectPizzas(std::vector<Pizza> & pizzaList, std::vector<int> & selectedPizzaIndices, int nTeamSize)
{
	//this function selects the best pizzas
	//to maximize the score of the order.
	//1- the first pizza selected from the ordered pizza list -> first not delivered pizza
	//2- rest of the pizzas are selected if their union is maximum
	// with the previous ones
	int firstSelectedPizzaIndex = GetFirstAvailablePizzaIndex(pizzaList);
	selectedPizzaIndices[0] = firstSelectedPizzaIndex;
	pizzaList[firstSelectedPizzaIndex].isDelivered = true;

	int secondSelectedPizzaIndex = FindNextPizzaIndexThatGeneratesMaxScore(pizzaList, pizzaList[firstSelectedPizzaIndex].hashedToppingList, firstSelectedPizzaIndex);
	selectedPizzaIndices[1] = secondSelectedPizzaIndex;
	pizzaList[secondSelectedPizzaIndex].isDelivered = true;

	if (nTeamSize < 3)
		return;

	static std::vector<int> unionVector(10000);
	unionVector.resize(pizzaList[firstSelectedPizzaIndex].hashedToppingList.size() + pizzaList[secondSelectedPizzaIndex].hashedToppingList.size());
	GetUnionOfToppings(unionVector, pizzaList[firstSelectedPizzaIndex].hashedToppingList, pizzaList[secondSelectedPizzaIndex].hashedToppingList);
	
	int thirdSelectedPizzaIndex = FindNextPizzaIndexThatGeneratesMaxScore(pizzaList, unionVector, secondSelectedPizzaIndex);
	selectedPizzaIndices[2] = thirdSelectedPizzaIndex;
	pizzaList[thirdSelectedPizzaIndex].isDelivered = true;

	if (nTeamSize < 4)
		return;

	static std::vector<int> unionVector2(10000);
	unionVector2.resize(unionVector.size() + pizzaList[thirdSelectedPizzaIndex].hashedToppingList.size());
	GetUnionOfToppings(unionVector2, unionVector, pizzaList[thirdSelectedPizzaIndex].hashedToppingList);

	int fourthSelectedPizzaIndex = FindNextPizzaIndexThatGeneratesMaxScore(pizzaList, unionVector2, thirdSelectedPizzaIndex);
	selectedPizzaIndices[3] = fourthSelectedPizzaIndex;
	pizzaList[fourthSelectedPizzaIndex].isDelivered = true;
}

void HandleOrders(std::vector<Pizza> & pizzaList, std::vector<int> vectServedTeamCounts, std::vector <std::pair<int, std::vector<int>>>  & TeamOrderList)
{
	std::cout << "HandleOrders Operation Starts!" << std::endl;

	int teamIndex = 0;
	for (int nTeamSize = 4; nTeamSize > 1; nTeamSize--)
	{
		int nVectorIterator = nTeamSize - 2;
		for (int i = 0; i < vectServedTeamCounts[nVectorIterator]; i++)
		{
			//TeamOrder[teamCounter].first  -> team size
			//TeamOrder[teamCounter].second -> orders -> pizza indices not ids!
			TeamOrderList[teamIndex].first = nTeamSize;
			TeamOrderList[teamIndex].second.resize(nTeamSize, -1);

			SelectPizzas(pizzaList, TeamOrderList[teamIndex].second, nTeamSize);

			teamIndex++;
		}
	}
}

void WriteOutput(std::string path, std::vector<Pizza> & pizzaList, std::vector <std::pair<int, std::vector<int>>> & TeamOrderList)
{
	std::ofstream WriteFile(path);
	std::streambuf *coutbuf = std::cout.rdbuf(); //save old buf
	std::cout.rdbuf(WriteFile.rdbuf()); //redirect std::cout to out.txt!

	std::cout << TeamOrderList.size() << std::endl;

	for (auto TeamOrder : TeamOrderList)
	{
		std::cout << TeamOrder.first;
		for (int i = 0; i < (int)TeamOrder.second.size(); i++)
		{
			std::cout << " " << pizzaList[TeamOrder.second[i]].ID;
		}
		std::cout << std::endl;
	}
	
	std::cout.rdbuf(coutbuf); //reset to standard output again
	WriteFile.close();
}

int main()
{
	std::vector<std::string> paths{
		"a_example",
		"b_little_bit_of_everything",
		"c_many_ingredients",
		"d_many_pizzas",
		"e_many_teams",
	};

	int fileCount = 1;
	for (std::string path : paths)
	{
		//vectTeamSize -> includes number of teamsof2,teamsof3,teamsof4 in order
		std::vector<int> vectTeamSize = { 0,0,0 };
		std::vector<Pizza> pizzaList;
		std::string outPath = path + "_out";

		ReadInputs(path, vectTeamSize, pizzaList);
		sortPizzaList(pizzaList);

		std::vector<int> vectServedTeamCounts = { 0,0,0 };
		SelectTeamsToBeServed(pizzaList.size(), vectTeamSize, vectServedTeamCounts);

		int totalPickedTeamCount = 0;
		for (int i = 0; i < 3; i++)
			totalPickedTeamCount += vectServedTeamCounts[i];

		std::vector <std::pair<int, std::vector<int>>> TeamOrderList(totalPickedTeamCount);
		HandleOrders(pizzaList, vectServedTeamCounts, TeamOrderList);

		WriteOutput(outPath, pizzaList, TeamOrderList);
	}	
}