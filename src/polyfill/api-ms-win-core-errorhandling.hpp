﻿

#if (WP_SUPPORT_VERSION < NTDDI_WIN7)

// Windows 7 [desktop apps | UWP apps]
// Windows Server 2008 R2 [desktop apps | UWP apps]
__DEFINE_THUNK(
    kernel32,
    12,
    VOID,
    WINAPI,
    RaiseFailFastException,
    _In_opt_ PEXCEPTION_RECORD pExceptionRecord,
    _In_opt_ PCONTEXT pContextRecord,
    _In_ DWORD dwFlags)
{
    if (auto pRaiseFailFastException = wp_get_RaiseFailFastException())
    {
        return pRaiseFailFastException(pExceptionRecord, pContextRecord, dwFlags);
    }

    // 直接结束进程
    TerminateProcess(
        NtGetCurrentProcess(),
        pExceptionRecord ? pExceptionRecord->ExceptionCode : STATUS_FAIL_FAST_EXCEPTION);
}
#endif

#if (WP_SUPPORT_VERSION < NTDDI_WIN7)

// Windows 7 [desktop apps | UWP apps]
// Windows Server 2008 R2 [desktop apps | UWP apps]
__DEFINE_THUNK(
    kernel32,
    8,
    BOOL,
    WINAPI,
    SetThreadErrorMode,
    _In_ DWORD dwNewMode,
    _In_opt_ LPDWORD lpOldMode)
{
    if (auto pSetThreadErrorMode = wp_get_SetThreadErrorMode())
    {
        return pSetThreadErrorMode(dwNewMode, lpOldMode);
    }

    auto dwOldMode = SetErrorMode(dwNewMode);
    if (lpOldMode)
        *lpOldMode = dwOldMode;

    return TRUE;
}
#endif

#if (WP_SUPPORT_VERSION < NTDDI_WIN7)

// Windows 7 [desktop apps | UWP apps]
// Windows Server 2008 R2 [desktop apps | UWP apps]
__DEFINE_THUNK(kernel32, 0, DWORD, WINAPI, GetThreadErrorMode, VOID)
{
    if (auto pGetThreadErrorMode = wp_get_GetThreadErrorMode())
    {
        return pGetThreadErrorMode();
    }

    return wp_GetErrorMode();
}
#endif

#if (WP_SUPPORT_VERSION < NTDDI_WIN6)

// Windows Vista [desktop apps only]
// Windows Server 2008 [desktop apps only]
__DEFINE_THUNK(kernel32, 0, UINT, WINAPI, GetErrorMode, VOID)
{
    if (auto pGetErrorMode = wp_get_GetErrorMode())
    {
        return pGetErrorMode();
    }
    else if (auto pNtQueryInformationProcess = wp_get_NtQueryInformationProcess())
    {
        DWORD dwDefaultHardErrorMode;

        auto Status = pNtQueryInformationProcess(
            NtCurrentProcess(),
            ProcessDefaultHardErrorMode,
            &dwDefaultHardErrorMode,
            sizeof(dwDefaultHardErrorMode),
            nullptr);

        if (Status >= 0)
        {
            if (dwDefaultHardErrorMode & 0x00000001)
            {
                return dwDefaultHardErrorMode & 0xFFFFFFFE;
            }
            else
            {
                return dwDefaultHardErrorMode | 0x00000001;
            }
        }

        internal::BaseSetLastNTError(Status);

        return 0;
    }
    else
    {
        SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
        return 0;
    }
}
#endif
