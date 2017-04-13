#pragma once
class FastSpinLock
{
public:
	FastSpinLock();
	~FastSpinLock();

	void EnterLock();
	void LeaveLock();

private:
	FastSpinLock(const FastSpinLock & rhs);
	FastSpinLock& operator=(const FastSpinLock & rhs);

	volatile long m_LockFlag;
};

/*
Dummy Lock Class

 ������ ����ؼ� ������-�Ҹ��� �� ����ִ� EnterLock-LeaveLock �� ȣ��
 ������ ���� ������ ���� ����
*/
class FastSpinLockGuard
{
public:
	FastSpinLockGuard(FastSpinLock& lock) : m_Lock(lock)
	{
		m_Lock.EnterLock();
	}
	~FastSpinLockGuard()
	{
		m_Lock.LeaveLock();
	}

private:
	FastSpinLock& m_Lock;
};

template <class TargetClass>
class ClassTypeLock
{
public:
	struct LockGuard
	{
		LockGuard()
		{
			TargetClass::m_Lock.EnterLock();
		}

		~LockGuard()
		{
			TargetClass::m_Lock.LeaveLock();
		}

	};

private:
	static FastSpinLock m_Lock;

	//friend struct LockGuard;
};

template <class TargetClass>
FastSpinLock ClassTypeLock<TargetClass>::m_Lock;