﻿

#if (WP_SUPPORT_VERSION < NTDDI_WIN6)

// Windows Vista,  Windows Server 2008
__DEFINE_THUNK(
    kernel32,
    28,
    int,
    WINAPI,
    GetDateFormatEx,
    _In_opt_ LPCWSTR lpLocaleName,
    _In_ DWORD dwFlags,
    _In_opt_ CONST SYSTEMTIME *lpDate,
    _In_opt_ LPCWSTR lpFormat,
    _Out_writes_opt_(cchDate) LPWSTR lpDateStr,
    _In_ int cchDate,
    _In_opt_ LPCWSTR lpCalendar)
{
    if (auto pGetDateFormatEx = wp_get_GetDateFormatEx())
    {
        return pGetDateFormatEx(
            lpLocaleName, dwFlags, lpDate, lpFormat, lpDateStr, cchDate, lpCalendar);
    }

    auto Locale = wp_LocaleNameToLCID(lpLocaleName, 0);

    if (Locale == 0)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }

    return GetDateFormatW(Locale, dwFlags, lpDate, lpFormat, lpDateStr, cchDate);
}
#endif

#if (WP_SUPPORT_VERSION < NTDDI_WIN6)

// Windows Vista,  Windows Server 2008
__DEFINE_THUNK(
    kernel32,
    24,
    int,
    WINAPI,
    GetTimeFormatEx,
    _In_opt_ LPCWSTR lpLocaleName,
    _In_ DWORD dwFlags,
    _In_opt_ CONST SYSTEMTIME *lpTime,
    _In_opt_ LPCWSTR lpFormat,
    _Out_writes_opt_(cchTime) LPWSTR lpTimeStr,
    _In_ int cchTime)
{
    if (auto pGetTimeFormatEx = wp_get_GetTimeFormatEx())
    {
        return pGetTimeFormatEx(lpLocaleName, dwFlags, lpTime, lpFormat, lpTimeStr, cchTime);
    }

    auto Locale = wp_LocaleNameToLCID(lpLocaleName, 0);

    if (Locale == 0)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }

    return GetTimeFormatW(Locale, dwFlags, lpTime, lpFormat, lpTimeStr, cchTime);
}
#endif
