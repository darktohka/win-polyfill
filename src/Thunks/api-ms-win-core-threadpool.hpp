﻿
#include <threadpoolapiset.h>

#ifdef YY_Thunks_Implemented

struct _TP_CLEANUP_GROUP_FUNCS
{
	//0
	void (__fastcall* pTppWorkpFree)(_TP_WORK* Work);
	//4
	void* pTppWorkCallbackEpilog;
	//8
	SRWLOCK srwLock;
	//C
	void* pTppWorkCancelPendingCallbacks;
	//0x10
};

struct _TP_CLEANUP_GROUP
{
	//0
	DWORD p0;
	//4
	DWORD p4;
	//8
	SRWLOCK srwLock;
	//0xC
	void* pC;
	//0x10
	void* p10;
	//0x14
	DWORD p14;
	//0x18
	DWORD p18;
	//0x1C
	DWORD p1C;
	//0x20
	SRWLOCK srwLock2;
	//0x24
	DWORD p24;
	//0x28
	SRWLOCK srwLock3;
	//0x2C
	void* p2C;
	//0x30
	void* p30;
	//0x34
	DWORD p34;
	//0x38
};

//此结构从微软Vista的ntdll 内部扒出来的，我们目前只实现到Vista
struct _TP_WORK
{
	//0 该结构的引用计数
	long nRef;
	//4
	const _TP_CLEANUP_GROUP_FUNCS* VFuncs;
	//8
	_TP_CLEANUP_GROUP* CleanupGroup;
	//0xC
	PTP_CLEANUP_GROUP_CANCEL_CALLBACK  CleanupGroupCancelCallback;
	//0x10
	PTP_SIMPLE_CALLBACK FinalizationCallback;
	//0x14
	void* p14;
	//0x18
	void* p18;
	//0x1C
	DWORD un1C;
	//0x20
	DWORD p20;
	//0x24
	DWORD p24;
	//0x28
	SRWLOCK srwLock;
	//0x2C
	DWORD p2C;
	//0x30
	union 
	{
		//0x30
		PTP_WORK_CALLBACK pfnwk;
		//0x30
		PTP_TIMER_CALLBACK pTimerCallback;
	};
	//0x34 上下文
	PVOID Context;
	//0x38
	_ACTIVATION_CONTEXT* ActivationContext;
	//0x3C
	void* SubProcessTag;
	//0x40
	PVOID RaceDll;
	//0x44
	PTP_POOL Pool;
	//0x48
	DWORD un48;
	//0x4C
	DWORD un4C;
	//0x50  TP_CALLBACK_ENVIRON 中的 Flags
	// 0x10000 已经调用Release
	long uFlags1;
	//0x54 似乎保存了 TpAllocWork 函数的返回地址意义暂时不明
	void* retaddr;
	//0x58 保存 TpReleaseWork 函数的返回地址
	void* un58;
	//0x5C
	DWORD un5C;
	//0x60
	void* pTaskVFuncs;
	//0x64
	DWORD uFlags;
	//0x68
	//附加字段，微软原版没有
	//当没有任务时，改句柄将处于激活状态
	HANDLE hEvent;
};

//0xD8
struct _TP_TIMER : public _TP_WORK
{

};



//不完善的定义，我们用多少，处理多少。
struct _TP_CALLBACK_INSTANCE
{
	//0x40
	PCRITICAL_SECTION CriticalSection;
	//0x44
	HMODULE DllHandle;
	//0x48
	HANDLE Event;
	//0x4C
	HANDLE Mutex;
	//0x50
	HANDLE Semaphore;
	//0x54
	DWORD ReleaseCount;
};

#endif


namespace YY
{
	namespace Thunks
	{
#if defined(YY_Thunks_Implemented) && (YY_Thunks_Support_Version < NTDDI_WIN6)

		namespace Fallback
		{
			static void __fastcall DoWhenCallbackReturns(_TP_CALLBACK_INSTANCE* Instance)
			{
				if (auto CriticalSection = Instance->CriticalSection)
				{
					LeaveCriticalSection(CriticalSection);
				}

				if (auto DllHandle = Instance->DllHandle)
				{
					FreeLibrary(DllHandle);
				}

				if (auto Event = Instance->Event)
				{
					SetEvent(Event);
				}

				if (auto Mutex = Instance->Mutex)
				{
					ReleaseMutex(Mutex);
				}

				if (auto Semaphore = Instance->Semaphore)
				{
					ReleaseSemaphore(Semaphore, Instance->ReleaseCount, nullptr);
				}
			}

			static void __fastcall TppWorkpFree(_TP_WORK* Work)
			{
				if (Work)
				{
					const auto ProcessHeap = ((TEB*)NtCurrentTeb())->ProcessEnvironmentBlock->ProcessHeap;
					HeapFree(ProcessHeap, 0, Work);
				}
			}

			static void __fastcall TppWorkCallbackEpilog(void* p)
			{

			}

			static void __fastcall TppWorkCancelPendingCallbacks(void* p)
			{

			}

			static const _TP_CLEANUP_GROUP_FUNCS TppWorkpCleanupGroupMemberVFuncs = { &TppWorkpFree, &TppWorkCallbackEpilog, 0, &TppWorkCancelPendingCallbacks };
			static void* TppWorkpTaskVFuncs;

			static
			NTSTATUS
			__fastcall
			TppCleanupGroupMemberInitialize(
				_Inout_ _TP_WORK* pWork,
				_Inout_opt_ PVOID pv,
				_In_opt_ PTP_CALLBACK_ENVIRON pcbe,
				_In_ DWORD uFlags,
				const _TP_CLEANUP_GROUP_FUNCS* CleanupGroupVFuncs
				)
			{
				pWork->nRef = 1;
				pWork->VFuncs = CleanupGroupVFuncs;
				pWork->p20 = 0;
				pWork->p24 = 0;
				pWork->p2C = 0;

				::InitializeSRWLock(&pWork->srwLock);

				pWork->Context = pv;

				if (pcbe)
				{
					pWork->Pool = pcbe->Pool;
					pWork->CleanupGroup = pcbe->CleanupGroup;
					pWork->CleanupGroupCancelCallback = pcbe->CleanupGroupCancelCallback;
					pWork->FinalizationCallback = NULL;
					pWork->ActivationContext = pcbe->ActivationContext;
					pWork->RaceDll = pcbe->RaceDll;
				}
				else
				{
					pWork->Pool = NULL;
					pWork->CleanupGroup = NULL;
					pWork->CleanupGroupCancelCallback = NULL;
					pWork->FinalizationCallback = NULL;
					pWork->ActivationContext = NULL;
					pWork->RaceDll = NULL;
				}

				pWork->SubProcessTag = NtCurrentTeb()->SubProcessTag;
				pWork->uFlags1 = uFlags;
				pWork->p18 = pWork->p14 = &pWork->p14;


				if (pWork->ActivationContext != NULL)
				{
					if (pWork->ActivationContext != (_ACTIVATION_CONTEXT*)-1)
					{

					}
				}

				return STATUS_SUCCESS;
			}

			static
			NTSTATUS
			__fastcall
			TppWorkInitialize(
				_Inout_ _TP_WORK* pWork,
				_Inout_opt_ PVOID pv,
				_In_opt_ PTP_CALLBACK_ENVIRON pcbe,
				_In_ DWORD uFlags,
				_In_ const _TP_CLEANUP_GROUP_FUNCS* CleanupGroupVFuncs,
				_In_ void* TaskVFuncs
				)
			{
				auto Status = TppCleanupGroupMemberInitialize(pWork, pv, pcbe, uFlags, CleanupGroupVFuncs);

				if (Status >= 0)
				{
					//if(pWork->)
					pWork->pTaskVFuncs = TaskVFuncs;
					pWork->uFlags = 1;
				}

				return Status;
			}

			static
			void
			__fastcall
			TppCleanupGroupAddMember(
				_Outptr_ _TP_WORK* pWork
				)
			{
				::AcquireSRWLockExclusive(&pWork->CleanupGroup->srwLock);

				pWork->p14 = &pWork->CleanupGroup->pC;
				pWork->p18 = pWork->CleanupGroup->p10;

				*(void**)pWork->CleanupGroup->p10 = pWork->p14;
				pWork->CleanupGroup->p10 = &pWork->p14;

				::ReleaseSRWLockExclusive(&pWork->CleanupGroup->srwLock);
			}

			static
			NTSTATUS
			__fastcall
			TpAllocWork(
				_Outptr_ _TP_WORK** ppWork,
				_In_ PTP_WORK_CALLBACK pfnwk,
				_Inout_opt_ PVOID pv,
				_In_opt_ PTP_CALLBACK_ENVIRON pcbe
				)
			{
				if (ppWork == nullptr || pfnwk == nullptr || (pcbe && (pcbe->u.Flags & 0xFFFFFFFE)))
				{
					return STATUS_INVALID_PARAMETER;
				}

				const auto ProcessHeap = ((TEB*)NtCurrentTeb())->ProcessEnvironmentBlock->ProcessHeap;
				auto pWork = (_TP_WORK*)HeapAlloc(ProcessHeap, HEAP_ZERO_MEMORY, sizeof(_TP_WORK));

				if (!pWork)
					return STATUS_NO_MEMORY;

				pWork->retaddr = _ReturnAddress();

				auto Status = TppWorkInitialize(pWork, pv, pcbe, pcbe ? pcbe->u.Flags : 0, &TppWorkpCleanupGroupMemberVFuncs, &TppWorkpTaskVFuncs);

				do
				{
					if (Status < 0)
						break;

					pWork->pfnwk = pfnwk;

					if (pcbe)
						pWork->FinalizationCallback = pcbe->FinalizationCallback;

					if (pWork->CleanupGroup)
						TppCleanupGroupAddMember(pWork);

					auto hEvent = CreateEventW(nullptr, TRUE, TRUE, nullptr);

					if (hEvent == nullptr)
					{
						Status = STATUS_NO_MEMORY;
						break;
					}

					pWork->hEvent = hEvent;

					*ppWork = pWork;

					return STATUS_SUCCESS;
				} while (false);

				
				if(pWork)
					HeapFree(ProcessHeap, 0, pWork);
				

				return Status;
			}

			static
			bool
			__fastcall
			TppCleanupGroupMemberRelease(
				_TP_WORK* CleanupGroupMember,
				bool RaiseIfAlreadyRaised
				)
			{
				auto orgFlags1 = InterlockedOr(&CleanupGroupMember->uFlags1, 0x10000);

				if (RaiseIfAlreadyRaised && (orgFlags1 & 0x10000))
				{
					internal::RaiseStatus(STATUS_INVALID_PARAMETER);
				}

				return (orgFlags1 & 0x30000) == 0;
			}

			static
			void
			WINAPI
			TpReleaseWork(
				_TP_WORK* Work)
			{
				if (Work == nullptr || (Work->uFlags1 & 0x10000) || Work->VFuncs != &Fallback::TppWorkpCleanupGroupMemberVFuncs)
				{
					internal::RaiseStatus(STATUS_INVALID_PARAMETER);
					return;
				}

				if (TppCleanupGroupMemberRelease(Work, true))
				{
					Work->un58 = _ReturnAddress();

					if (InterlockedExchangeAdd(&Work->nRef, -1) == 0)
					{
						Work->VFuncs->pTppWorkpFree(Work);
					}
				}
			}

			static void __fastcall TppTimerpFree(_TP_WORK* Work)
			{
				if (Work)
				{
					const auto ProcessHeap = ((TEB*)NtCurrentTeb())->ProcessEnvironmentBlock->ProcessHeap;
					HeapFree(ProcessHeap, 0, (_TP_TIMER*)Work);
				}
			}

			static const _TP_CLEANUP_GROUP_FUNCS TppTimerpCleanupGroupMemberVFuncs = { &TppTimerpFree };
			static void* TppTimerpTaskVFuncs;

			static
			NTSTATUS
			__fastcall
			TppTimerAlloc(
				_TP_TIMER** ppTimer,
				_In_ PTP_TIMER_CALLBACK Callback,
				_Inout_opt_ PVOID Context,
				_In_opt_ PTP_CALLBACK_ENVIRON CallbackEnviron,
				DWORD Flags
				)
			{
				
				*ppTimer = nullptr;


				const auto ProcessHeap = ((TEB*)NtCurrentTeb())->ProcessEnvironmentBlock->ProcessHeap;
				auto pTimer = (_TP_TIMER*)HeapAlloc(ProcessHeap, HEAP_ZERO_MEMORY, sizeof(_TP_TIMER));

				if (!pTimer)
					return STATUS_NO_MEMORY;
				
				pTimer->retaddr = _ReturnAddress();

				auto Status = TppWorkInitialize(pTimer, Context, CallbackEnviron, Flags | 0x1040000, &TppTimerpCleanupGroupMemberVFuncs, TppTimerpTaskVFuncs);

				if (Status >= 0)
				{
					pTimer->pTimerCallback = Callback;

					if(CallbackEnviron)
						pTimer->FinalizationCallback = CallbackEnviron->FinalizationCallback;

					if (pTimer->CleanupGroup)
						TppCleanupGroupAddMember(pTimer);

					*ppTimer = pTimer;

					return Status;
				}

				HeapFree(ProcessHeap, 0, pTimer);
				return Status;

			}


			static
			NTSTATUS
			__fastcall
			TpAllocTimer(
				_TP_TIMER** ppTimer,
				_In_ PTP_TIMER_CALLBACK Callback,
				_Inout_opt_ PVOID Context,
				_In_opt_ PTP_CALLBACK_ENVIRON CallbackEnviron
				)
			{
				auto Flags = CallbackEnviron ? CallbackEnviron->u.Flags : 0;

				if (ppTimer == nullptr || Callback == nullptr || (Flags & 0xFFFFFFFE))
				{
					return STATUS_INVALID_PARAMETER;
				}

				auto Status = TppTimerAlloc(ppTimer, Callback, Context, CallbackEnviron, Flags);

				if (Status >= 0)
				{
					(*ppTimer)->retaddr = _ReturnAddress();
				}

				return Status;
			}

		}
#endif


#if (YY_Thunks_Support_Version < NTDDI_WIN6)

		//Minimum supported client	Windows Vista [desktop apps | UWP apps]
		//Minimum supported server	Windows Server 2008 [desktop apps | UWP apps]
		__DEFINE_THUNK(
		kernel32,
		12,
		PTP_WORK,
		WINAPI,
		CreateThreadpoolWork,
			_In_ PTP_WORK_CALLBACK pfnwk,
			_Inout_opt_ PVOID pv,
			_In_opt_ PTP_CALLBACK_ENVIRON pcbe
			)
		{
			if (auto const pCreateThreadpoolWork = try_get_CreateThreadpoolWork())
			{
				return pCreateThreadpoolWork(pfnwk, pv, pcbe);
			}

			//Fallback到我自己实现的 Tp
			_TP_WORK* pWork;

			auto Status = Fallback::TpAllocWork(&pWork, pfnwk, pv, pcbe);

			if (Status >= 0)
			{
				return pWork;
			}

			internal::BaseSetLastNTError(Status);
			return nullptr;
		}
#endif


#if (YY_Thunks_Support_Version < NTDDI_WIN6)

		//Minimum supported client	Windows Vista [desktop apps | UWP apps]
		//Minimum supported server	Windows Server 2008 [desktop apps | UWP apps]
		__DEFINE_THUNK(
		kernel32,
		4,
		VOID,
		WINAPI,
		CloseThreadpoolWork,
			_Inout_ PTP_WORK pwk
			)
		{
			if (auto const pCloseThreadpoolWork = try_get_CloseThreadpoolWork())
			{
				return pCloseThreadpoolWork(pwk);
			}

			Fallback::TpReleaseWork(pwk);
		}

#endif


#if (YY_Thunks_Support_Version < NTDDI_WIN6)

		__DEFINE_THUNK(
		kernel32,
		4,
		VOID,
		WINAPI,
		SubmitThreadpoolWork,
			_Inout_ PTP_WORK pwk
			)
		{
			if (auto const pSubmitThreadpoolWork = try_get_SubmitThreadpoolWork())
			{
				return pSubmitThreadpoolWork(pwk);
			}

			//参数验证
			if (pwk == nullptr
				|| (pwk->uFlags1 & 0x30000)
				|| pwk->VFuncs != &Fallback::TppWorkpCleanupGroupMemberVFuncs)
			{
				//0xC000000D 参数错误
				return;
			}

			//增加一次引用。
			auto uFlags = pwk->uFlags;

			for (;;)
			{
				auto uNewFlags = InterlockedCompareExchange(&pwk->uFlags, (uFlags & 0xFFFFFFFEu) + 2, uFlags);

				if (uNewFlags == uFlags)
					break;

				uFlags = uNewFlags;
			}

			InterlockedExchangeAdd(&pwk->nRef, 1);

			ResetEvent(pwk->hEvent);

			auto bRet = QueueUserWorkItem([](LPVOID lpThreadParameter) -> DWORD
				{
					auto pwk = (PTP_WORK)lpThreadParameter;

					if ((pwk->uFlags & 0x1) == 0)
					{
						TP_CALLBACK_INSTANCE Instance = {};

						pwk->pfnwk(&Instance, pwk->Context, pwk);

						Fallback::DoWhenCallbackReturns(&Instance);
					}


					auto uFlags = pwk->uFlags;

					for (;;)
					{
						auto uNewFlags = InterlockedCompareExchange(&pwk->uFlags, uFlags - 2, uFlags);

						if (uNewFlags == uFlags)
						{
							uFlags = uFlags - 2;
							break;
						}

						uFlags = uNewFlags;
					}

					if ((uFlags & 0xFFFFFFFEu) == 0)
					{
						//计数为 0，唤醒等待
						SetEvent(pwk->hEvent);
					}

					if (InterlockedExchangeAdd(&pwk->nRef, -1) == 0)
					{
						pwk->VFuncs->pTppWorkpFree(pwk);
					}


					return 0;

				}, pwk, 0);

			if (!bRet)
			{
				//QueueUserWorkItem失败，重新减少引用计数

				for (;;)
				{
					auto uNewFlags = InterlockedCompareExchange(&pwk->uFlags, uFlags - 2, uFlags);

					if (uNewFlags == uFlags)
					{
						uFlags = uFlags - 2;
						break;
					}
					
					uFlags = uNewFlags;
				}

				if ((uFlags & 0xFFFFFFFEu) == 0)
				{
					//计数为 0，唤醒等待
					SetEvent(pwk->hEvent);
				}

				if (InterlockedExchangeAdd(&pwk->nRef, -1) == 0)
				{
					pwk->VFuncs->pTppWorkpFree(pwk);
				}
			}




		}

#endif


#if (YY_Thunks_Support_Version < NTDDI_WIN6)

		//Minimum supported client	Windows Vista [desktop apps | UWP apps]
		//Minimum supported server	Windows Server 2008 [desktop apps | UWP apps]
		__DEFINE_THUNK(
		kernel32,
		8,
		VOID,
		WINAPI,
		WaitForThreadpoolWorkCallbacks,
			_Inout_ PTP_WORK pwk,
			_In_ BOOL fCancelPendingCallbacks
			)
		{
			if (auto const pWaitForThreadpoolWorkCallbacks = try_get_WaitForThreadpoolWorkCallbacks())
			{
				return pWaitForThreadpoolWorkCallbacks(pwk, fCancelPendingCallbacks);
			}

			if (pwk == nullptr
				|| (pwk->uFlags1 & 0x30000)
				|| (pwk->VFuncs != &Fallback::TppWorkpCleanupGroupMemberVFuncs))
			{
				//0xC000000D 参数错误
				return;
			}

			DWORD Count = 0;

			if (fCancelPendingCallbacks)
			{
				for (auto uFlags = pwk->uFlags; Count = uFlags >> 1;)
				{
					//添加取消任务标志
					auto uNewFlags = InterlockedCompareExchange(&pwk->uFlags, uFlags | 0x1, uFlags);

					if (uNewFlags == uFlags)
						break;

					uFlags = uNewFlags;
				}
			}


			if ((InterlockedCompareExchange(&pwk->uFlags, 0, 0) & 0xFFFFFFFEu) == 0)
			{
				//等待计数为 0，直接返回即可
				return;
			}

			WaitForSingleObject(pwk->hEvent, INFINITE);
		}

#endif


#if (YY_Thunks_Support_Version < NTDDI_WIN6)

		//Minimum supported client	Windows Vista [desktop apps | UWP apps]
		//Minimum supported server	Windows Server 2008 [desktop apps | UWP apps]
		__DEFINE_THUNK(
		kernel32,
		12,
		PTP_TIMER,
		WINAPI,
		CreateThreadpoolTimer,
			_In_ PTP_TIMER_CALLBACK pfnti,
			_Inout_opt_ PVOID pv,
			_In_opt_ PTP_CALLBACK_ENVIRON pcbe
			)
		{
			if (auto const pCreateThreadpoolTimer = try_get_CreateThreadpoolTimer())
			{
				return pCreateThreadpoolTimer(pfnti, pv, pcbe);
			}

			PTP_TIMER pTimer;

			auto Status = Fallback::TpAllocTimer(&pTimer, pfnti, pv, pcbe);

			if (Status >= 0)
			{
				return pTimer;
			}

			internal::BaseSetLastNTError(Status);

			return nullptr;
		}
#endif


#if (YY_Thunks_Support_Version < NTDDI_WIN6)

		//Minimum supported client	Windows Vista [desktop apps | UWP apps]
		//Minimum supported server	Windows Server 2008 [desktop apps | UWP apps]
		__DEFINE_THUNK(
		kernel32,
		16,
		VOID,
		WINAPI,
		SetThreadpoolTimer,
			_Inout_ PTP_TIMER pti,
			_In_opt_ PFILETIME pftDueTime,
			_In_ DWORD msPeriod,
			_In_opt_ DWORD msWindowLength
			)
		{
			if (auto const pSetThreadpoolTimer = try_get_SetThreadpoolTimer())
			{
				return pSetThreadpoolTimer(pti, pftDueTime, msPeriod, msWindowLength);
			}

			if (pti == nullptr
				|| (pti->uFlags1 & 0x30000)
				|| pti->VFuncs != &Fallback::TppTimerpCleanupGroupMemberVFuncs)
			{
				internal::RaiseStatus(STATUS_INVALID_PARAMETER);
				return;
			}

			if (pftDueTime == nullptr)
			{
				if (auto hEvent = pti->hEvent)
				{
					pti->hEvent = nullptr;
					//销毁计数器
					DeleteTimerQueueTimer(nullptr, hEvent, nullptr);

					if (InterlockedExchangeAdd(&pti->nRef, -1) == 0)
					{
						pti->VFuncs->pTppWorkpFree(pti);
					}
				}

				return;
			}

			LARGE_INTEGER lDueTime;
			lDueTime.LowPart = pftDueTime->dwLowDateTime;
			lDueTime.HighPart = pftDueTime->dwHighDateTime;

			if (lDueTime.QuadPart < 0)
			{
				//相对时间
				lDueTime.QuadPart /= -1 * 10 * 1000;
			}
			else if (lDueTime.QuadPart > 0)
			{
				//绝对时间
				FILETIME CurrentTime;
				GetSystemTimeAsFileTime(&CurrentTime);
				LARGE_INTEGER lCurrentTime;
				lCurrentTime.LowPart = CurrentTime.dwLowDateTime;
				lCurrentTime.HighPart = CurrentTime.dwHighDateTime;

				if (lDueTime.QuadPart > lCurrentTime.QuadPart)
				{
					lDueTime.QuadPart = (lDueTime.QuadPart - lCurrentTime.QuadPart) / 10'000;
				}
				else
				{
					lDueTime.QuadPart = 0;
				}
			}


			if (pti->hEvent)
			{
				ChangeTimerQueueTimer(pti->hEvent, nullptr, lDueTime.QuadPart, msPeriod);
			}
			else
			{
				InterlockedExchangeAdd(&pti->nRef, 1);

				auto bRet = CreateTimerQueueTimer(&pti->hEvent, nullptr, [](PVOID Parameter, BOOLEAN)
					{
						auto Timer = (PTP_TIMER)Parameter;
						TP_CALLBACK_INSTANCE Instance = {};

						Timer->pTimerCallback(&Instance, Timer->Context, Timer);


						Fallback::DoWhenCallbackReturns(&Instance);

					}, pti, lDueTime.QuadPart, msPeriod, 0);

				if (!bRet)
				{
					if (InterlockedExchangeAdd(&pti->nRef, -1) == 0)
					{
						pti->VFuncs->pTppWorkpFree(pti);
					}
				}

			}

		}
#endif


#if (YY_Thunks_Support_Version < NTDDI_WIN6)

		//Minimum supported client	Windows Vista [desktop apps | UWP apps]
		//Minimum supported server	Windows Server 2008 [desktop apps | UWP apps]
		__DEFINE_THUNK(
		kernel32,
		4,
		VOID,
		WINAPI,
		CloseThreadpoolTimer,
			_Inout_ PTP_TIMER Timer
			)
		{
			if (auto const pCloseThreadpoolTimer = try_get_CloseThreadpoolTimer())
			{
				return pCloseThreadpoolTimer(Timer);
			}

			if (Timer == nullptr
				|| (Timer->uFlags1 & 0x10000)
				|| Timer->VFuncs != &Fallback::TppTimerpCleanupGroupMemberVFuncs)
			{
				internal::RaiseStatus(STATUS_INVALID_PARAMETER);
				return;
			}

			if (Fallback::TppCleanupGroupMemberRelease(Timer, true))
			{
				Timer->un58 = _ReturnAddress();

				if (auto hEvent = Timer->hEvent)
				{
					Timer->hEvent = nullptr;
					//销毁计数器
					DeleteTimerQueueTimer(nullptr, hEvent, INVALID_HANDLE_VALUE);

					InterlockedExchangeAdd(&Timer->nRef, -1);
				}

				if(InterlockedExchangeAdd(&Timer->nRef, -1) == 0)
				{
					Timer->VFuncs->pTppWorkpFree(Timer);
				}
			}

		}
#endif

		
#if (YY_Thunks_Support_Version < NTDDI_WIN6)

		//Minimum supported client	Windows Vista [desktop apps | UWP apps]
		//Minimum supported server	Windows Server 2008 [desktop apps | UWP apps]
		__DEFINE_THUNK(
		kernel32,
		8,
		VOID,
		WINAPI,
		SetEventWhenCallbackReturns,
			_Inout_ PTP_CALLBACK_INSTANCE Instance,
			_In_ HANDLE Event
			)
		{
			if (auto const pSetEventWhenCallbackReturns = try_get_SetEventWhenCallbackReturns())
			{
				return pSetEventWhenCallbackReturns(Instance, Event);
			}


			//只允许被设置一次
			if (Instance == nullptr
				|| Event == nullptr
				|| Event == INVALID_HANDLE_VALUE
				|| Instance->Event != nullptr)
			{
				internal::RaiseStatus(STATUS_INVALID_PARAMETER);
				return;
			}

			Instance->Event = Event;
		}
#endif


#if (YY_Thunks_Support_Version < NTDDI_WIN6)

		//Minimum supported client	Windows Vista [desktop apps | UWP apps]
		//Minimum supported server	Windows Server 2008 [desktop apps | UWP apps]
		__DEFINE_THUNK(
		kernel32,
		12,
		VOID,
		WINAPI,
		ReleaseSemaphoreWhenCallbackReturns,
			_Inout_ PTP_CALLBACK_INSTANCE Instance,
			_In_ HANDLE Semaphore,
			_In_ DWORD ReleaseCount
			)
		{
			if (auto const pReleaseSemaphoreWhenCallbackReturns = try_get_ReleaseSemaphoreWhenCallbackReturns())
			{
				return pReleaseSemaphoreWhenCallbackReturns(Instance, Semaphore, ReleaseCount);
			}

			//只允许被设置一次
			if (Instance == nullptr
				|| Semaphore == nullptr
				|| Semaphore == INVALID_HANDLE_VALUE
				|| ReleaseCount == 0
				|| Instance->Semaphore != nullptr)
			{
				internal::RaiseStatus(STATUS_INVALID_PARAMETER);
				return;
			}

			Instance->Semaphore = Semaphore;
			Instance->ReleaseCount = ReleaseCount;
		}
#endif


#if (YY_Thunks_Support_Version < NTDDI_WIN6)

		//Minimum supported client	Windows Vista [desktop apps | UWP apps]
		//Minimum supported server	Windows Server 2008 [desktop apps | UWP apps]
		__DEFINE_THUNK(
		kernel32,
		8,
		VOID,
		WINAPI,
		ReleaseMutexWhenCallbackReturns,
			_Inout_ PTP_CALLBACK_INSTANCE Instance,
			_In_ HANDLE Mutex
			)
		{
			if (auto const pReleaseMutexWhenCallbackReturns = try_get_ReleaseMutexWhenCallbackReturns())
			{
				return pReleaseMutexWhenCallbackReturns(Instance, Mutex);
			}

			//只允许被设置一次
			if (Instance == nullptr
				|| Mutex == nullptr
				|| Mutex == INVALID_HANDLE_VALUE
				|| Instance->Mutex != nullptr)
			{
				internal::RaiseStatus(STATUS_INVALID_PARAMETER);
				return;
			}

			Instance->Mutex = Mutex;
		}
#endif


#if (YY_Thunks_Support_Version < NTDDI_WIN6)

		//Minimum supported client	Windows Vista [desktop apps | UWP apps]
		//Minimum supported server	Windows Server 2008 [desktop apps | UWP apps]
		__DEFINE_THUNK(
		kernel32,
		8,
		VOID,
		WINAPI,
		LeaveCriticalSectionWhenCallbackReturns,
			_Inout_ PTP_CALLBACK_INSTANCE Instance,
			_Inout_ PCRITICAL_SECTION CriticalSection
			)
		{
			if (auto const pLeaveCriticalSectionWhenCallbackReturns = try_get_LeaveCriticalSectionWhenCallbackReturns())
			{
				return pLeaveCriticalSectionWhenCallbackReturns(Instance, CriticalSection);
			}

			//只允许被设置一次
			if (Instance == nullptr
				|| CriticalSection == nullptr
				|| Instance->CriticalSection != nullptr)
			{
				internal::RaiseStatus(STATUS_INVALID_PARAMETER);
				return;
			}

			Instance->CriticalSection = CriticalSection;
		}
#endif


#if (YY_Thunks_Support_Version < NTDDI_WIN6)

		//Minimum supported client	Windows Vista [desktop apps | UWP apps]
		//Minimum supported server	Windows Server 2008 [desktop apps | UWP apps]
		__DEFINE_THUNK(
		kernel32,
		8,
		VOID,
		WINAPI,
		FreeLibraryWhenCallbackReturns,
			_Inout_ PTP_CALLBACK_INSTANCE Instance,
			_In_ HMODULE DllHandle
			)
		{
			if (auto const pFreeLibraryWhenCallbackReturns = try_get_FreeLibraryWhenCallbackReturns())
			{
				return pFreeLibraryWhenCallbackReturns(Instance, DllHandle);
			}

			//只允许被设置一次
			if (Instance == nullptr
				|| DllHandle == nullptr
				|| DllHandle == INVALID_HANDLE_VALUE
				|| Instance->DllHandle != nullptr)
			{
				internal::RaiseStatus(STATUS_INVALID_PARAMETER);
				return;
			}

			Instance->DllHandle = DllHandle;
		}
#endif




	}
}