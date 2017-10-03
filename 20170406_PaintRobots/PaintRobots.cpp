// synchronisation of n paint robots and one pot fillup robot

#include <iostream>
#include <sstream>
#include <Windows.h>
#include <cassert>
#include "Lock.h"
#include "RandomGen.h"

using namespace std;

// number of robots
int const n = 3;

// name of mutex objects
string const gPot[] = { "Red_Pot", "Green_Pot", "Blue_Pot" };

// global termination flag
bool gTermination = false;

// CS for synchronized output
CritSec gCsAccess;

// synchronized print function
void PrintSync(string const & str, ostream & os) {
	CSLock Lock(gCsAccess);

	os << str << endl;
}

DWORD WINAPI PaintRobot(void * param) {

	//assert(param != 0);
	int idx = reinterpret_cast<int>(param);

	// open mutex object for desired pot
	HANDLE hMtx = OpenMutex(MUTEX_ALL_ACCESS, false, gPot[idx].c_str());

	while (!gTermination) {
		// wait for pot
		DWORD res = WaitForSingleObject(hMtx, INFINITE);
		try {

			ostringstream out;
			out << "Robot # " << idx << " paint with color " << gPot[idx];
			PrintSync(out.str(), cout);

			// simulate painting...
			Sleep(rgen::GetRandVal(1500 + 200 * idx, 3000));

			out.str(""); out.clear();
			out << "Robot # " << idx << " returns " << gPot[idx]
				<< " and cleans its nozzle";
			PrintSync(out.str(), cout);

			// release the ownership of the pot now
			ReleaseMutex(hMtx);

		}
		catch (...) {
			PrintSync("-> Exception caught: Releasing mutex object skipped.", cout);

		}

		// robot cleans nozzle now
		Sleep(rgen::GetRandVal(2000 + 400 * idx, 3000));

	}
	CloseHandle(hMtx);
	return 0;
}

DWORD WINAPI FillupRobot(void * param) {

	// retrieve the mutex handles of all pots
	HANDLE hMtx[n];
	for (int i = 0; i < n; ++i) {
		hMtx[i] = OpenMutex(MUTEX_ALL_ACCESS, false, gPot[i].c_str());
	}

	while (!gTermination) {

		// wait for all pots
		DWORD res = WaitForMultipleObjects(n, hMtx, true, INFINITE);

		if (res == WAIT_ABANDONED_0) {
			PrintSync("Error: A mutex was not released by the other thread.", cout);
			// Error handling goes here...
		}
		else {
			try {

				// fillup the pots now
				PrintSync("Filling up the pots now...", cout);

				// simulate fillup procedure...
				Sleep(rgen::GetRandVal(2000, 3000));

				PrintSync("Fillup done.", cout);

				for (auto h : hMtx) {
					ReleaseMutex(h);
				}

			}
			catch (...) {
				PrintSync("->Exception caught. Releasing mutex object skipped.", cout);
			}

			// wait somw time between refills
			Sleep(rgen::GetRandVal(4000, 6000));
		}
	}
	for (auto h : hMtx) {
		CloseHandle(h);
	}
	return 0;
}

int main()
{
	HANDLE hMutex[n];
	HANDLE hThread[n];

	srand(0);

	// a mutex for each pot
	for (int i = 0; i < n; ++i) {
		hMutex[i] = CreateMutex(nullptr, false, gPot[i].c_str());
	}

	// 3 paint robots
	for (int i = 0; i < n; ++i) {
		hThread[i] = CreateThread(nullptr, 0, PaintRobot, (void *)i, 0, 0);
	}

	// 1 fillup robot
	HANDLE hFillup = CreateThread(nullptr, 0, FillupRobot, nullptr, 0, 0);

	Sleep(20000);
	gTermination = true;

	WaitForMultipleObjects(n, hThread, true, INFINITE);
	WaitForSingleObject(hFillup, INFINITE);

	cout << "All done." << endl;

	for (int i = 0; i < n; ++i) {
		CloseHandle(hThread[i]);
		CloseHandle(hMutex[i]);
	}
	CloseHandle(hFillup);


	return 0;
}