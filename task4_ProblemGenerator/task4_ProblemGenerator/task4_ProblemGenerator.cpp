// task4_ProblemGenerator.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../include/tinyxml2.h"
#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <sstream>
#include <ctime>
#include <windows.h>


using namespace tinyxml2;


#ifndef XMLCheckResult
#define XMLCheckResult(a_eResult) if (a_eResult != XML_SUCCESS) { printf("Error: %i\n", a_eResult); return a_eResult; }
#endif

double rnd() //Generate in range [0; 1)
{
	return double(rand() % RAND_MAX) / RAND_MAX;
}

int rnd(int maxvalue, int minvalue = 0) // Generate in range [minvalue; maxvalue)
{
	return int(rnd() * maxvalue) + minvalue;
}

int createTestData(int M, std::vector<int> &time)
{
	static int i = 0;
	tinyxml2::XMLDocument xmlDoc;
	XMLNode *pRoot = xmlDoc.NewElement("Root");
	xmlDoc.InsertFirstChild(pRoot);
	XMLElement * pElement = xmlDoc.NewElement("M");
	pElement->SetText(M);
	pRoot->InsertEndChild(pElement);

	pElement = xmlDoc.NewElement("Time");
	for (const auto & item : time)
	{
		XMLElement * pListElement = xmlDoc.NewElement("Item");
		pListElement->SetText(item);
		pElement->InsertEndChild(pListElement);
	}
	pElement->SetAttribute("N", int(time.size()));
	pRoot->InsertEndChild(pElement);

	std::string dir("Test");
	if (CreateDirectory(dir.c_str(), NULL) || ERROR_ALREADY_EXISTS == GetLastError())
	{
		std::stringstream s;
		s << dir << "\\TestData" << i++ << ".xml";
		XMLError eResult = xmlDoc.SaveFile(s.str().c_str());
		XMLCheckResult(eResult);
	}
	return 0;
}

#define MIN(a,b)  (a < b) ? a : b

int main()
{
	srand(time(NULL));
	const int opttime = 100, maxproc = 10, files = 100;

	for (int i = 0; i < files; i++)
	{
		int M = rnd(maxproc, 1);
		int space_left = opttime * M - rnd(M - 1);
		std::vector<int> time;
		while (space_left > 0)
		{
			int t = rnd(MIN(space_left, opttime / 2), 1);
			time.push_back(t);
			space_left -= t;
		}
		createTestData(M, time);
	}

	return 0;
}
