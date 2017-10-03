#ifndef LOCK_INCLUDED
#define LOCK_INCLUDED

// mutual exclusion for synchronization

#include <windows.h>

// initialisierung of the critical section
class CritSec {

	friend class CSLock;

public:
	CritSec() {
		InitializeCriticalSection(&mCS);
	}
	~CritSec() {
		DeleteCriticalSection(&mCS);
	}

private:
	void Acquire() {
		EnterCriticalSection(&mCS);
	}

	void Release() {
		LeaveCriticalSection(&mCS);
	}

	CritSec(CritSec const &) = delete;								// do not allow copying
	CritSec & operator = (CritSec const &) = delete;	// do not allow assignment

	CRITICAL_SECTION mCS;		// THE critical section variable
};

// apply and release the lock
class CSLock {
public:
	// lock for exclusive access
	CSLock(CritSec & CSObj) : mCSAccess(CSObj) {
		mCSAccess.Acquire();
	}

	// release lcok
	~CSLock() {
		mCSAccess.Release();
	}

private:
	CSLock();
	CSLock(CSLock const &) = delete;
	CSLock & operator = (CSLock const &) = delete;

	CritSec &mCSAccess;
};
#endif
