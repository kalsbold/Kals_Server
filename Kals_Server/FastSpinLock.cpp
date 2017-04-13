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
		//m_LockFlag�� 0�̸� 1�� �ٲٰ� ����.
		if (InterlockedExchange( &m_LockFlag, 1) == 0)
		{
			return;
		}

		//m_LockFlag�� 0�� �ƴϸ� ���� �ɷ������Ƿ� ��Ÿ�� ���� ��õ�.
		//�ּ� Ÿ�̸� resolution�� 1�� ����.
		UINT u_TimerRes = 1;
		timeBeginPeriod(u_TimerRes);

		//sleep �ð��� 0ms ~ 10ms ���� ������.
		Sleep((DWORD)min(10, nloops));

		timeEndPeriod(u_TimerRes);
	}
}

void FastSpinLock::LeaveLock()
{
	InterlockedExchange(&m_LockFlag, 0);
}
