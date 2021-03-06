// task4.cpp : Defines the entry point for the console application.
//


#include "stdafx.h"
#include "../include/tinyxml2.h"
#include <iostream>
#include <cmath>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <ctime>

//#pragma execution_character_set("utf-8")

using namespace tinyxml2;

#ifndef XMLCheckResult
#define XMLCheckResult(a_eResult) if (a_eResult != XML_SUCCESS) { std::cout << "Error: " << a_eResult << "\n"; exit(0); }
#endif

double rnd() //Generate random real number in range [0; 1)
{
	return double(rand() % RAND_MAX) / RAND_MAX;
}

int rnd(int maxvalue, int minvalue = 0) // Generate random number in range [minvalue; maxvalue)
{
	return int(rnd() * maxvalue) + minvalue;
}

struct Job
{
	int number;
	int time;
	int cpu_num; //Just a little optimization, so I wouldn't have to search for it between all CPUs.
	Job(int n, int t) : number(n), time(t), cpu_num(-1) {};
};

class CPU
{
public:
	int number;
	std::vector<int> job;
	void add_job(int j, int pos) //Add job into CPU queue. 
	{
		job.insert(job.begin() + pos, j);
	}
	void release_job(int num) //Delete job with number num from the CPU queue. 
	{
		for (auto i = job.begin(); i < job.end(); i++) //вечный цикл (или очень длинный [или нет])
		{
			if (*i == num)
			{
				job.erase(i);
				return;
			}
		}
		throw 1;
	}
	CPU(int n) : number(n) {};
	CPU(int n, std::vector<int> &j) : number(n), job(j) {};
};

class State
{
public:
	int M, N;
	std::vector<Job> job;
	std::vector<CPU> cpu;
	int energy() //Energy for annealing algorithm. Basically, time of all jobs. 
	{
		int maxtime = 0;
		for (auto i = cpu.begin(); i < cpu.end(); i++)
		{
			int time = 0;
			for (int j = 0; j < (*i).job.size(); j++)
			{
				time += job[(*i).job[j]].time;
			}
			maxtime = (time > maxtime) ? time : maxtime;
		}
		return maxtime;
	}
	State(int m, int n, std::vector<Job> &j) : M(m), N(n), job(j)
	{
		for (int i = 0; i < m; i++)
		{
			CPU c = CPU(i);
			cpu.push_back(c);
			if (i == 0)
			{
				for (int j = 0; j < n; j++)
				{
					cpu[i].add_job(j, j);
					job[j].cpu_num = 0;
				}
			}
		}
	}
	State newstate()
	{
		State s_new = *this;
		int j = rnd(N);//random from 0 to N-1
					   //std::cout << "j=" << j << " ";
		s_new.cpu[job[j].cpu_num].release_job(j);
		int p = rnd(M);//random from 0 to M-1
					   //std::cout << "p=" << p << " ";
		int k = rnd(int(s_new.cpu[p].job.size() + 1));//random from 0 to cpu[p].job.size();
													  //std::cout << "size=" << s_new.cpu[p].job.size() << " k=" << k << " ";
		s_new.cpu[p].add_job(j, k);
		s_new.job[j].cpu_num = p;
		return s_new;
	}
};

class Annealing
{
	State s, smin;
	const double t0, tmin;
	int iteration;
	bool log;
	double temperature()
	{
		return (iteration == 0) ? t0 : t0 * 0.1 / iteration;
	}
public:
	Annealing(int m, int n, std::vector<Job> &j, double tstart, double tend, bool l = false) : s(m, n, j), smin(s), t0(tstart), tmin(tend), iteration(0), log(l) {};
	State anneal()
	{
		double t = temperature();
		while (t > tmin)
		{
			//std::cout << iteration << ": ";
			State s_new = s.newstate();
			//std::cout << t << ' ' << tmin << ' ' << (t > tmin) << std::endl;
			int E = s_new.energy() - s.energy();
			if (E <= 0 || rnd() < std::exp(-double(E) / t)) // || random_value < exp(-E/t)
				s = s_new;
			iteration++;
			t = temperature();
			if (s.energy() < smin.energy())
			{
				smin = s;
			}
		}
		if (log) std::cout << "Number of iterations: " << iteration << '\n';
		return smin;
	}
};

void help()
{
	std::cout << "Hey! This program uses annealing method to create best schedule.\n\n";
	std::cout << "If you have correct .xml file with data use of this program can go like this:\n";
	std::cout << "task4 <FileName.xml> [-l]\n";
	std::cout << "Where -l option will show you some additional data, not only calculated min time.\n\n";
	std::cout << "If you wanna put your own problem do it this way:\n";
	std::cout << "task4 -p <M> <N> <time1> <time2> ... <timeN> [-l]\n";
	std::cout << "Where -l option will show you some additional data, not only calculated min time.\n\n";
	std::cout << "Thank you, come again!\n";
	exit(0);
}

void parse(int argc, char * argv[], int &M, int &N, std::vector<Job> &time, bool &log)
{
	if (argc == 1)
		help();

	if (!std::strcmp(argv[1], "-l")) //log
	{
		log = true;
		if (argc > 2 && !std::strcmp(argv[2], "-p")) //parameters
		{
			M = std::atoi(argv[3]);
			N = std::atoi(argv[4]);
			for (int i = 0; i < N; i++)
			{
				time.emplace_back(i, std::atoi(argv[5 + i]));
			}
		}
		else
		{
			if (argc == 2)
				help();

			tinyxml2::XMLDocument xmlDoc;
			XMLError eResult = xmlDoc.LoadFile(argv[2]);
			XMLCheckResult(eResult);

			XMLNode * pRoot = xmlDoc.FirstChild();
			if (pRoot == nullptr)
			{
				std::cout << "XML_ERROR_FILE_READ_ERROR\n";
				exit(0);
			}

			XMLElement *pElement = pRoot->FirstChildElement("M");
			if (pElement == nullptr)
			{
				std::cout << "XML_ERROR_PARSING_ELEMENT\n";
				exit(0);
			}
			eResult = pElement->QueryIntText(&M);

			pElement = pRoot->FirstChildElement("Time");
			if (pElement == nullptr)
			{
				std::cout << "XML_ERROR_PARSING_ELEMENT\n";
				exit(0);
			}
			XMLElement * pListElement = pElement->FirstChildElement("Item");
			int i = 0;
			while (pListElement != nullptr)
			{
				int tmp;
				eResult = pListElement->QueryIntText(&tmp);
				XMLCheckResult(eResult);
				time.emplace_back(i++, tmp);
				pListElement = pListElement->NextSiblingElement("Item");
			}
			N = time.size();
		}
	}
	else if (!std::strcmp(argv[1], "-p"))
	{
		M = std::atoi(argv[2]);
		N = std::atoi(argv[3]);
		for (int i = 0; i < N; i++)
		{
			time.emplace_back(i, std::atoi(argv[4 + i]));
		}
		if (argc > 4 + N && !std::strcmp(argv[4 + N], "-l"))
		{
			log = true;
		}
	}
	else
	{
		tinyxml2::XMLDocument xmlDoc;
		XMLError eResult = xmlDoc.LoadFile(argv[1]);
		XMLCheckResult(eResult);

		XMLNode * pRoot = xmlDoc.FirstChild();
		if (pRoot == nullptr)
		{
			std::cout << "XML_ERROR_FILE_READ_ERROR\n";
			exit(0);
		}

		XMLElement *pElement = pRoot->FirstChildElement("M");
		if (pElement == nullptr)
		{
			std::cout << "XML_ERROR_PARSING_ELEMENT\n";
			exit(0);
		}
		eResult = pElement->QueryIntText(&M);

		pElement = pRoot->FirstChildElement("Time");
		if (pElement == nullptr)
		{
			std::cout << "XML_ERROR_PARSING_ELEMENT\n";
			exit(0);
		}
		XMLElement * pListElement = pElement->FirstChildElement("Item");
		int i = 0;
		while (pListElement != nullptr)
		{
			int tmp;
			eResult = pListElement->QueryIntText(&tmp);
			XMLCheckResult(eResult);
			time.emplace_back(i++, tmp);
			pListElement = pListElement->NextSiblingElement("Item");
		}
		N = time.size();
		if (argc > 2 && !std::strcmp(argv[2], "-l"))
		{
			log = true;
		}
	}
}

int main(int argc, char *argv[])
{
	bool log = false;
	int M, N;
	std::vector<Job> time;
	parse(argc, argv, M, N, time, log);

	double t0 = 255.0*std::sqrt(N / 2), tend = 0.1;
	srand(std::time(NULL));
	Annealing a = Annealing(M, N, time, t0, tend, log);
	State s = a.anneal();
	if (log)
	{
		for (int i = 0; i < M; i++)
		{
			std::cout << i << ": ";
			if (s.cpu[i].job.size())
			{
				for (int j = 0; j < s.cpu[i].job.size() - 1; j++)
				{
					std::cout << s.cpu[i].job[j] << ", ";
				}
				std::cout << s.cpu[i].job[s.cpu[i].job.size() - 1] << "\n";
			}
			else std::cout << "\n";
		}
		std::cout << "Working time: ";
	}
	std::cout << s.energy() << '\n';

	/*std::cout << M << " " << N << "\n";
	for (auto i : time)
	{
	std::cout << i.time << ' ';
	}
	std::cout << std::endl;*/

	return 0;
}
