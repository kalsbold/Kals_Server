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

 스택을 사용해서 생성자-소멸자 에 들어있는 EnterLock-LeaveLock 를 호출
 일종의 더미 렌더러 같은 역할
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