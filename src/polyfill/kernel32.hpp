﻿

#if (WP_SUPPORT_VERSION < NTDDI_WINXP)

// Minimum supported client	Windows Vista
// Minimum supported server	Windows Server 2008
// Windows XP有这个API啊……微软文档抽风了？
__DEFINE_THUNK(kernel32, 0, DWORD, WINAPI, WTSGetActiveConsoleSessionId, VOID)
{
    if (const auto pWTSGetActiveConsoleSessionId = wp_get_WTSGetActiveConsoleSessionId())
    {
        return pWTSGetActiveConsoleSessionId();
    }

    // 因为Windows 2000没有会话隔离，所有的进程始终在 0 会话中运行，因此直接返回 0 即可。
    return 0;
}
#endif

#if (WP_SUPPORT_VERSION < NTDDI_WINXPSP2)

// Minimum supported client	Windows Vista, Windows XP Professional x64 Edition, Windows XP with
// SP2[desktop apps only] Minimum supported server	Windows Server 2003 [desktop apps only]
__DEFINE_THUNK(
    kernel32,
    8,
    BOOL,
    WINAPI,
    GetNumaProcessorNode,
    _In_ UCHAR Processor,
    _Out_ PUCHAR NodeNumber)
{
    if (const auto pGetNumaProcessorNode = wp_get_GetNumaProcessorNode())
    {
        return pGetNumaProcessorNode(Processor, NodeNumber);
    }

    // 对于没有 Node 概念的系统，我们统一认为只有一个 Node。

    SYSTEM_INFO SystemInfo;
    GetSystemInfo(&SystemInfo);

    if (SystemInfo.dwNumberOfProcessors < Processor)
    {
        *NodeNumber = '\0';
        return TRUE;
    }
    else
    {
        *NodeNumber = 0xffu;

        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
}
#endif

#if (WP_SUPPORT_VERSION < NTDDI_WIN7)

// Minimum supported client	Windows 7 [desktop apps only]
// Minimum supported server	Windows Server 2008 R2 [desktop apps only]
__DEFINE_THUNK(
    kernel32,
    8,
    BOOL,
    WINAPI,
    GetNumaNodeNumberFromHandle,
    _In_ HANDLE hFile,
    _Out_ PUSHORT NodeNumber)
{
    if (const auto pGetNumaNodeNumberFromHandle = wp_get_GetNumaNodeNumberFromHandle())
    {
        return pGetNumaNodeNumberFromHandle(hFile, NodeNumber);
    }

    // 始终认为来自 Node 0
    *NodeNumber = 0;

    return TRUE;
}
#endif

#if (WP_SUPPORT_VERSION < NTDDI_WIN7)

// Minimum supported client	Windows 7 [desktop apps only]
// Minimum supported server	Windows Server 2008 R2 [desktop apps only]
__DEFINE_THUNK(
    kernel32,
    8,
    BOOL,
    WINAPI,
    GetNumaProcessorNodeEx,
    _In_ PPROCESSOR_NUMBER Processor,
    _Out_ PUSHORT NodeNumber)
{
    if (const auto pGetNumaProcessorNodeEx = wp_get_GetNumaProcessorNodeEx())
    {
        return pGetNumaProcessorNodeEx(Processor, NodeNumber);
    }

    // 老版本系统假定只有一组处理器
    if (Processor->Group == 0)
    {
        UCHAR NodeNumberTmp;
        auto bRet = GetNumaProcessorNode(Processor->Number, &NodeNumberTmp);

        if (bRet)
        {
            *NodeNumber = NodeNumberTmp;
        }
        else
        {
            *NodeNumber = 0xffffu;
        }

        return bRet;
    }

    *NodeNumber = 0xffffu;

    SetLastError(ERROR_INVALID_PARAMETER);
    return FALSE;
}
#endif

#if (WP_SUPPORT_VERSION < NTDDI_WINXPSP2)

// Minimum supported client	Windows Vista, Windows XP Professional x64 Edition, Windows XP with SP2
// [desktop apps only] Minimum supported server	Windows Server 2003 [desktop apps only]
__DEFINE_THUNK(
    kernel32,
    8,
    _Success_(return) BOOL,
    WINAPI,
    GetNumaAvailableMemoryNode,
    _In_ UCHAR Node,
    _Out_ PULONGLONG AvailableBytes)
{
    if (const auto pGetNumaAvailableMemoryNode = wp_get_GetNumaAvailableMemoryNode())
    {
        return pGetNumaAvailableMemoryNode(Node, AvailableBytes);
    }

    if (Node == 0)
    {
        // 统一的假定，Node数量为 1，所以该值必然为 0
        // 我们把所有可用内存都认为是该节点0的可用内存。
        MEMORYSTATUSEX statex = {sizeof(statex)};

        if (!GlobalMemoryStatusEx(&statex))
        {
            return FALSE;
        }

        *AvailableBytes = statex.ullAvailPhys;
        return TRUE;
    }

    SetLastError(ERROR_INVALID_PARAMETER);
    return FALSE;
}
#endif

#if (WP_SUPPORT_VERSION < NTDDI_WIN7)

// Minimum supported client	Windows 7 [desktop apps only]
// Minimum supported server	Windows Server 2008 R2 [desktop apps only]
__DEFINE_THUNK(
    kernel32,
    8,
    _Success_(return) BOOL,
    WINAPI,
    GetNumaAvailableMemoryNodeEx,
    _In_ USHORT Node,
    _Out_ PULONGLONG AvailableBytes)
{
    if (const auto pGetNumaAvailableMemoryNodeEx = wp_get_GetNumaAvailableMemoryNodeEx())
    {
        return pGetNumaAvailableMemoryNodeEx(Node, AvailableBytes);
    }

    // GetNumaAvailableMemoryNode 最大只接受 0xFF
    if (Node >= 0x100u)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    return GetNumaAvailableMemoryNode(UCHAR(Node), AvailableBytes);
}
#endif

#if (WP_SUPPORT_VERSION < NTDDI_WIN6)

// Minimum supported client	Windows Vista [desktop apps only]
// Minimum supported server	Windows Server 2008 [desktop apps only]
__DEFINE_THUNK(
    kernel32,
    8,
    BOOL,
    WINAPI,
    GetNumaProximityNode,
    _In_ ULONG ProximityId,
    _Out_ PUCHAR NodeNumber)
{
    if (const auto pGetNumaProximityNode = wp_get_GetNumaProximityNode())
    {
        return pGetNumaProximityNode(ProximityId, NodeNumber);
    }

    // 我们不知道CPU的组成情况，但是我们可以假定最接近的分组就是 Node 0。
    // 对于电脑来说，Node数量始终等于 1，因此问题不是特别的大。
    *NodeNumber = 0;
    return TRUE;
}
#endif

#if (WP_SUPPORT_VERSION < NTDDI_WIN6)

// Minimum supported client	Windows Vista [desktop apps only]
// Minimum supported server	Windows Server 2008 [desktop apps only]
__DEFINE_THUNK(
    kernel32,
    28,
    LPVOID,
    WINAPI,
    MapViewOfFileExNuma,
    _In_ HANDLE hFileMappingObject,
    _In_ DWORD dwDesiredAccess,
    _In_ DWORD dwFileOffsetHigh,
    _In_ DWORD dwFileOffsetLow,
    _In_ SIZE_T dwNumberOfBytesToMap,
    _In_opt_ LPVOID lpBaseAddress,
    _In_ DWORD nndPreferred)
{
    if (const auto pMapViewOfFileExNuma = wp_get_MapViewOfFileExNuma())
    {
        return pMapViewOfFileExNuma(
            hFileMappingObject,
            dwDesiredAccess,
            dwFileOffsetHigh,
            dwFileOffsetLow,
            dwNumberOfBytesToMap,
            lpBaseAddress,
            nndPreferred);
    }

    return MapViewOfFileEx(
        hFileMappingObject,
        dwDesiredAccess,
        dwFileOffsetHigh,
        dwFileOffsetLow,
        dwNumberOfBytesToMap,
        lpBaseAddress);
}
#endif
