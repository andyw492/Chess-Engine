#define NOMINMAX
#define _CRTDBG_MAP_ALLOC

#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cinttypes>
#include <random>

#include <stdlib.h>
#include <crtdbg.h>

#include "parameters.h"
#include "engine.h"
#include "worker.h"
#include "window.h"
#include "position.h"

#pragma comment(lib, "ws2_32.lib")

using std::cout;
using std::endl;
using std::string;
using std::mt19937;
using std::uniform_int_distribution;
using std::copy;

UINT engineThread(LPVOID pParam)
{
	Parameters *p = ((Parameters*)pParam);

	bool enginePrint = false;
	U64 zobristTable[13][64];

	WaitForSingleObject(p->mutex, INFINITE);
	enginePrint = p->enginePrint;
	memcpy(zobristTable, p->zobristTable, 13 * 64 * sizeof(U64));
	ReleaseMutex(p->mutex);

	Engine e(enginePrint, zobristTable);
	return e.start(pParam);
}

UINT workerThreads(LPVOID pParam)
{
	Parameters *p = ((Parameters*)pParam);

	bool workerPrint = false;
	U64 zobristTable[13][64];
	bool randomize = false;

	WaitForSingleObject(p->mutex, INFINITE);
	workerPrint = p->workerPrint;
	memcpy(zobristTable, p->zobristTable, 13 * 64 * sizeof(U64));
	randomize = p->workerRandomize;
	ReleaseMutex(p->mutex);

	Worker w(workerPrint, zobristTable, randomize);
	return w.start(pParam);
}

UINT windowThread(LPVOID pParam)
{
	Parameters *p = ((Parameters*)pParam);

	bool windowPrint = false;
	WaitForSingleObject(p->mutex, INFINITE);
	windowPrint = p->windowPrint;
	ReleaseMutex(p->mutex);

	Window w(windowPrint);
	return w.start(pParam);
}

UINT quitThread(LPVOID pParam)
{
	Parameters *p = ((Parameters*)pParam);

	WaitForSingleObject(p->finished, INFINITE);
	WaitForSingleObject(p->finished, INFINITE);

	SetEvent(p->eventQuit);

	return 0;
}

vector<U64> fenToBoard(string fen)
{
	// replace numbers with spaces, e.g. "4" -> "    " (" " * 4)
	vector<string> parts = helper::splitToVector(fen, '/');
	vector<string> processedParts;
	for (int i = 0; i < parts.size(); i++)
	{
		string partsString = parts[i];
		string processedPartsString = "";
		for (int j = 0; j < partsString.length(); j++)
		{
			if (isdigit(partsString[j]))
			{
				int spaceCount = partsString[j] - '0';
				string spaceString = "        ";
				processedPartsString += spaceString.substr(0, spaceCount);
			}
			else
			{
				processedPartsString += partsString[j];
			}
		}

		processedParts.push_back(processedPartsString);
	}

	vector<U64> board;

	for (int i = 0; i < 12; i++)
	{
		U64 bitBoard = 0ULL;
		char toMatch = helper::pieces[i];
		int square = 0;

		for (string s : processedParts)
		{
			for (int j = 0; j < s.length(); j++)
			{
				if (s.at(j) == toMatch)
				{
					setBit(bitBoard, square);
				}
				square++;
			}
		}

		board.push_back(bitBoard);
	}

	// extra info: legal castling in all four ways, no last castle, no enpassant square
	board.push_back(30ULL);

	return board;
}

int main(void)
{
	int workerCount = 10;
	int threadCount = workerCount + 2;

	HANDLE *handles = new HANDLE[threadCount + 1];
	Parameters p;

	p.mutex = CreateMutex(NULL, 0, NULL);
	p.finished = CreateSemaphore(NULL, 0, threadCount, NULL);
	p.eventQuit = CreateEvent(NULL, true, false, NULL);

	string fen = "";

	p.maxDepth = 4;
	p.workerCount = workerCount;
	p.workerRandomize = true;

	vector<U64> board = fenToBoard(helper::splitToVector(fen, ' ')[0]);
	board.push_back(0);

	// initialize zobrist table
	mt19937 mt(01234567);
	for (int i = 0; i < 13; i++)
	{
		for (int j = 0; j < 64; j++)
		{
			uniform_int_distribution<unsigned long long int> dist(0, UINT64_MAX);
			p.zobristTable[i][j] = dist(mt);
		}
	}

	//auto begin = std::chrono::high_resolution_clock::now();
	//vector<vector<U64>> nextPositions = helper::getNextPositions(board, true, p.zobristTable);
	//auto end = std::chrono::high_resolution_clock::now();

	//for (auto position : nextPositions)
	//{
	//	printf("-----------------------\n");
	//	helper::printBoard(position);
	//}

	//printf("%d legal moves found in %.3f ms\n", nextPositions.size(), float(std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count()) / 1e6);
	//return 0;

	// starting position
	fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

	// debug
	if (0)
	{
		fen = "r1bqkb1r/ppp2ppp/2np1n2/4p3/4P3/2NPB3/PPP2PPP/R2QKB1R w KQkq - 0 1";
	}
	p.initialFen = fen;

	// debug
	vector<int> debugPrint
	{
		0
		,0
		,0
		,0
		,0
	};

	/*
		0 - window board pieces
		1 - node creation/deletion
		2 - bad moves
	*/

	p.debugPrint = debugPrint;

	srand(time(0));

	handles[0] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)engineThread, &p, 0, NULL);
	handles[1] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)windowThread, &p, 0, NULL);
	handles[2] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)quitThread, &p, 0, NULL);

	for (int i = 3; i < 3 + workerCount; i++)
	{
		handles[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)workerThreads, &p, 0, NULL);
	}

	for (int i = 0; i < threadCount + 1; i++)
	{
		WaitForSingleObject(handles[i], INFINITE);
		CloseHandle(handles[i]);
	}

	delete[] handles;

	//_CrtDumpMemoryLeaks();

	return 0;
}
