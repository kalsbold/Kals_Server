#include "stdafx.h"
#include "FastSpinLock.h"


FastSpinLock::FastSpinLock() : m_LockFlag(0)
{
}


FastSpinLock::~FastSpinLock()
{
}

void FastSpinLock::EnterLock()
{
	for (int nloops = 0; ; nloops++)
	{
		//m_LockFlag가 0이면 1로 바꾸고 리턴.
		if (InterlockedExchange( &m_LockFlag, 1) == 0)
		{
			return;
		}

		//m_LockFlag가 0이 아니면 락이 걸려있으므로 한타임 쉬고 재시도.
		//최소 타이머 resolution을 1로 설정.
		UINT u_TimerRes = 1;
		timeBeginPeriod(u_TimerRes);

		//sleep 시간을 0ms ~ 10ms 까지 설정함.
		Sleep((DWORD)min(10, nloops));

		timeEndPeriod(u_TimerRes);
	}
}

void FastSpinLock::LeaveLock()
{
	InterlockedExchange(&m_LockFlag, 0);
}
