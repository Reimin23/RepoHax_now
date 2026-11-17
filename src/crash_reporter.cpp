#include "pch.h"

#include "crash_reporter.h"

#include <atomic>
#include <csignal>
#include <exception>
#include <strsafe.h>
#include <DbgHelp.h>

// Everything that runs while the game is crashing avoids the heap and the loader lock where it can:
// reports are built in static buffers and written with plain Win32 calls. Code that touches memory
// which may be broken (stack walking, reading the exception object) runs under __try/__except.
// dbghelp.dll is loaded at runtime (no import), it is only used to put function names into reports.

namespace Cheat
{
    namespace
    {
        constexpr wchar_t   kLogFileName[]      = L"haxsdk_logs.txt";
        constexpr wchar_t   kCrashDirName[]     = L"haxsdk_crashes";
        constexpr int       kMaxFrames          = 48;
        constexpr int       kMaxScanHits        = 24;
        constexpr size_t    kScanBytes          = 32 * 1024;
        constexpr int       kLogTailLines       = 60;
        constexpr int       kMaxReports         = 10;
        constexpr ULONGLONG kPendingMaxAgeMs    = 15000;
        constexpr DWORD     kCppExceptionCode   = 0xE06D7363; // 'msc' | 0xE0000000

        // ------------------------------------------------------------------ state

        std::atomic<bool>               g_Installed;

        uintptr_t                       g_ModuleBase;
        uintptr_t                       g_ModuleEnd;
        uintptr_t                       g_CodeBegin;
        uintptr_t                       g_CodeEnd;
        wchar_t                         g_ModulePath[MAX_PATH];
        wchar_t                         g_ExePath[MAX_PATH];
        wchar_t                         g_LogPath[MAX_PATH];
        wchar_t                         g_CrashDir[MAX_PATH];
        wchar_t                         g_UnityLogPath[MAX_PATH];
        wchar_t                         g_LastReportPath[MAX_PATH];
        char                            g_GameVersion[128];
        char                            g_WindowsVersion[64];

        PVOID                           g_VehHandle;
        LPTOP_LEVEL_EXCEPTION_FILTER    g_PrevFilter;
        SafetyHookInline                g_SetFilterHook;

        using SignalHandler = void(__cdecl*)(int);
        SignalHandler                   g_PrevAbortHandler;
        std::atomic<bool>               g_TerminateReported;

        const char* volatile            g_InitStage = "not started";
        thread_local const char*        t_Stage;
        thread_local bool               t_ThreadReady;

        std::atomic<DWORD>              g_WriterThread;
        int                             g_ReportCount;

        // Mono, resolved in OnUnityLoaded() (used to name managed exceptions).
        using MonoObjectGetClassFn = void* (*)(void*);
        using MonoClassGetNameFn   = const char* (*)(void*);
        MonoObjectGetClassFn            g_MonoObjectGetClass;
        MonoClassGetNameFn              g_MonoClassGetName;
        MonoClassGetNameFn              g_MonoClassGetNamespace;

        // dbghelp, loaded in Install().
        using SymGetOptionsFn       = DWORD   (WINAPI*)();
        using SymSetOptionsFn       = DWORD   (WINAPI*)(DWORD);
        using SymInitializeWFn      = BOOL    (WINAPI*)(HANDLE, PCWSTR, BOOL);
        using SymCleanupFn          = BOOL    (WINAPI*)(HANDLE);
        using SymLoadModuleExWFn    = DWORD64 (WINAPI*)(HANDLE, HANDLE, PCWSTR, PCWSTR, DWORD64, DWORD, PMODLOAD_DATA, DWORD);
        using SymGetModuleInfoW64Fn = BOOL    (WINAPI*)(HANDLE, DWORD64, PIMAGEHLP_MODULEW64);
        using SymFromAddrWFn        = BOOL    (WINAPI*)(HANDLE, DWORD64, PDWORD64, PSYMBOL_INFOW);
        using SymGetLineFromAddrW64Fn = BOOL  (WINAPI*)(HANDLE, DWORD64, PDWORD, PIMAGEHLP_LINEW64);

        SymGetOptionsFn                 g_SymGetOptions;
        SymSetOptionsFn                 g_SymSetOptions;
        SymInitializeWFn                g_SymInitializeW;
        SymCleanupFn                    g_SymCleanup;
        SymLoadModuleExWFn              g_SymLoadModuleExW;
        SymGetModuleInfoW64Fn           g_SymGetModuleInfoW64;
        SymFromAddrWFn                  g_SymFromAddrW;
        SymGetLineFromAddrW64Fn         g_SymGetLineFromAddrW64;

        // Own dbghelp session id: not a real process handle on purpose, so that Unity's crash
        // handler can still initialize dbghelp for the real process afterwards.
        const HANDLE                    kSymSession = (HANDLE)(ULONG_PTR)0x52484158;

        // ------------------------------------------------------------------ report input

        struct ReportInput
        {
            const char*                 Title;          // one line: what happened
            const wchar_t*              Message;        // optional, e.g. the panic message
            const char*                 MessageA;       // optional, e.g. std::terminate details
            const EXCEPTION_RECORD*     Record;         // optional
            const CONTEXT*              Context;        // optional
            const char*                 Stage;          // what the crashed thread was doing
            DWORD                       ThreadId;
            ULONG_PTR                   StackLow;       // stack bounds of the crashed thread
            ULONG_PTR                   StackHigh;
            bool                        AllowSymbols;   // false inside the loader lock
        };

        // A fatal exception outside the mod but with mod code on the stack. It may still be handled
        // by someone, so it is only written if the process dies shortly after.
        struct PendingException
        {
            bool                        Valid;
            ULONGLONG                   Tick;
            DWORD                       ThreadId;
            const char*                 Stage;
            ULONG_PTR                   StackLow;
            ULONG_PTR                   StackHigh;
            EXCEPTION_RECORD            Record;
            CONTEXT                     Context;
        };

        PendingException                g_Pending;
        ReportInput                     g_HelperInput;
        CONTEXT                         g_HelperContext;
        CONTEXT                         g_CaptureContext;
        CONTEXT                         g_WalkContext;

        // ------------------------------------------------------------------ output buffer

        HANDLE                          g_File = INVALID_HANDLE_VALUE;
        char                            g_Out[64 * 1024];
        size_t                          g_OutLen;
        char                            g_Line[2048];
        wchar_t                         g_WideTail[16 * 1024];
        uintptr_t                       g_Frames[kMaxFrames];
        int                             g_FrameCount;
        bool                            g_FramesTruncated;
        alignas(SYMBOL_INFOW) char      g_SymbolBuffer[sizeof(SYMBOL_INFOW) + 512 * sizeof(wchar_t)];

        void Flush()
        {
            if (g_File != INVALID_HANDLE_VALUE && g_OutLen > 0)
            {
                DWORD written = 0;
                ::WriteFile(g_File, g_Out, (DWORD)g_OutLen, &written, nullptr);
                ::FlushFileBuffers(g_File);
            }
            g_OutLen = 0;
        }

        void Out(const char* s)
        {
            if (s == nullptr)
                return;

            size_t len = ::strlen(s);
            while (len > 0)
            {
                if (g_OutLen == sizeof(g_Out))
                    Flush();

                size_t chunk = Hax::Min(len, sizeof(g_Out) - g_OutLen);
                ::memcpy(g_Out + g_OutLen, s, chunk);
                g_OutLen += chunk;
                s += chunk;
                len -= chunk;
            }
        }

        void OutF(const char* fmt, ...)
        {
            va_list args;
            va_start(args, fmt);
            ::StringCchVPrintfA(g_Line, _countof(g_Line), fmt, args);
            va_end(args);
            Out(g_Line);
        }

        // Writes a UTF-16 string as UTF-8.
        void OutW(const wchar_t* s, size_t len = (size_t)-1)
        {
            if (s == nullptr)
                return;

            if (len == (size_t)-1)
                len = ::wcslen(s);

            // 3 bytes per UTF-16 unit at most, keep chunks well inside g_Line.
            constexpr size_t kChunk = 600;
            while (len > 0)
            {
                size_t chunk = Hax::Min(len, kChunk);
                int n = ::WideCharToMultiByte(CP_UTF8, 0, s, (int)chunk, g_Line, (int)sizeof(g_Line) - 1, nullptr, nullptr);
                g_Line[n > 0 ? n : 0] = 0;
                Out(g_Line);
                s += chunk;
                len -= chunk;
            }
        }

        // Writes a multi-line text, each line indented.
        void OutIndentedW(const wchar_t* s, const char* indent)
        {
            const wchar_t* lineStart = s;
            for (const wchar_t* p = s; ; ++p)
            {
                if (*p == L'\n' || *p == 0)
                {
                    size_t len = (size_t)(p - lineStart);
                    if (len > 0 && lineStart[len - 1] == L'\r')
                        --len;

                    Out(indent);
                    OutW(lineStart, len);
                    Out("\r\n");

                    if (*p == 0)
                        break;
                    lineStart = p + 1;
                }
            }
        }

        void OutIndentedA(const char* s, const char* indent)
        {
            const char* lineStart = s;
            for (const char* p = s; ; ++p)
            {
                if (*p == '\n' || *p == 0)
                {
                    Out(indent);
                    size_t len = (size_t)(p - lineStart);
                    ::StringCchCopyNA(g_Line, _countof(g_Line), lineStart, len);
                    Out(g_Line);
                    Out("\r\n");

                    if (*p == 0)
                        break;
                    lineStart = p + 1;
                }
            }
        }

        // ------------------------------------------------------------------ memory / modules

        bool IsReadable(const void* ptr, size_t size)
        {
            if (ptr == nullptr)
                return false;

            MEMORY_BASIC_INFORMATION mbi;
            if (::VirtualQuery(ptr, &mbi, sizeof(mbi)) == 0)
                return false;

            if (mbi.State != MEM_COMMIT || (mbi.Protect & (PAGE_NOACCESS | PAGE_GUARD)) != 0)
                return false;

            constexpr DWORD kReadable = PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY |
                PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY;
            if ((mbi.Protect & kReadable) == 0)
                return false;

            return (uintptr_t)ptr + size <= (uintptr_t)mbi.BaseAddress + mbi.RegionSize;
        }

        bool IsModAddress(uintptr_t addr)
        {
            return addr >= g_ModuleBase && addr < g_ModuleEnd;
        }

        bool IsModCode(uintptr_t addr)
        {
            return addr >= g_CodeBegin && addr < g_CodeEnd;
        }

        const IMAGE_NT_HEADERS64* GetNtHeaders(uintptr_t base)
        {
            const auto* dos = (const IMAGE_DOS_HEADER*)base;
            if (!IsReadable(dos, sizeof(*dos)) || dos->e_magic != IMAGE_DOS_SIGNATURE)
                return nullptr;

            const auto* nt = (const IMAGE_NT_HEADERS64*)(base + dos->e_lfanew);
            if (!IsReadable(nt, sizeof(*nt)) || nt->Signature != IMAGE_NT_SIGNATURE)
                return nullptr;

            return nt;
        }

        // Short module name without taking the loader lock: the name stored in the export
        // directory, else the file name (GetModuleFileNameW, may take the loader lock).
        void GetModuleShortName(uintptr_t base, char* out, size_t outSize)
        {
            out[0] = 0;

            if (const IMAGE_NT_HEADERS64* nt = GetNtHeaders(base))
            {
                const IMAGE_DATA_DIRECTORY& dir = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
                if (dir.VirtualAddress != 0 && dir.Size != 0)
                {
                    const auto* exports = (const IMAGE_EXPORT_DIRECTORY*)(base + dir.VirtualAddress);
                    if (IsReadable(exports, sizeof(*exports)) && exports->Name != 0 && IsReadable((const void*)(base + exports->Name), 1))
                    {
                        ::StringCchCopyA(out, outSize, (const char*)(base + exports->Name));
                        return;
                    }
                }
            }

            wchar_t path[MAX_PATH];
            if (::GetModuleFileNameW((HMODULE)base, path, MAX_PATH) != 0)
            {
                const wchar_t* name = ::wcsrchr(path, L'\\');
                name = name ? name + 1 : path;
                ::WideCharToMultiByte(CP_UTF8, 0, name, -1, out, (int)outSize, nullptr, nullptr);
                return;
            }

            ::StringCchCopyA(out, outSize, "?");
        }

        // "0x00007FF812345678  UnityPlayer.dll+0x1234" or "... (no module: ...)"
        void DescribeAddress(uintptr_t addr, char* out, size_t outSize)
        {
            PVOID base = nullptr;
            ::RtlPcToFileHeader((PVOID)addr, &base);

            if (base == nullptr)
            {
                ::StringCchPrintfA(out, outSize, "0x%016llX  (no module: JIT-compiled game code, generated code or bad address)", (unsigned long long)addr);
                return;
            }

            char name[96];
            GetModuleShortName((uintptr_t)base, name, sizeof(name));
            ::StringCchPrintfA(out, outSize, "0x%016llX  %s+0x%llX%s", (unsigned long long)addr, name,
                (unsigned long long)(addr - (uintptr_t)base), IsModAddress(addr) ? "   <-- RepoHax" : "");
        }

        // ------------------------------------------------------------------ stack walking

        // Collects return addresses into g_Frames. Must not be inlined into a function with C++ objects
        // (it uses SEH).
        void WalkStack(const CONTEXT* start)
        {
            g_FrameCount = 0;
            g_FramesTruncated = false;

            __try
            {
                g_WalkContext = *start;
                CONTEXT& ctx = g_WalkContext;

                for (int i = 0; i < kMaxFrames; ++i)
                {
                    const DWORD64 pc = ctx.Rip;
                    if (pc == 0 && i > 0)
                        break;

                    g_Frames[g_FrameCount++] = (uintptr_t)pc;

                    DWORD64 imageBase = 0;
                    PRUNTIME_FUNCTION fn = ::RtlLookupFunctionEntry(pc, &imageBase, nullptr);
                    const DWORD64 prevRsp = ctx.Rsp;

                    if (fn != nullptr)
                    {
                        PVOID handlerData = nullptr;
                        DWORD64 establisher = 0;
                        ::RtlVirtualUnwind(UNW_FLAG_NHANDLER, imageBase, pc, fn, &ctx, &handlerData, &establisher, nullptr);
                    }
                    else if (i == 0)
                    {
                        // Leaf function or a call through a bad pointer: the return address is at [rsp].
                        if (!IsReadable((const void*)ctx.Rsp, sizeof(DWORD64)))
                            break;
                        ctx.Rip = *(const DWORD64*)ctx.Rsp;
                        ctx.Rsp += sizeof(DWORD64);
                    }
                    else
                    {
                        g_FramesTruncated = true; // no unwind info for this frame
                        break;
                    }

                    if (ctx.Rsp <= prevRsp && i > 0)
                    {
                        g_FramesTruncated = true;
                        break;
                    }
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                g_FramesTruncated = true;
            }
        }

        bool IsModOnStack(const CONTEXT* ctx)
        {
            WalkStack(ctx);
            for (int i = 0; i < g_FrameCount; ++i)
            {
                if (IsModCode(g_Frames[i]))
                    return true;
            }
            return false;
        }

        // Returns how many addresses inside mod code were found on the stack.
        int ScanStackForModCode(ULONG_PTR rsp, ULONG_PTR stackHigh, uintptr_t* hits, ULONG_PTR* offsets, int maxHits)
        {
            int count = 0;
            __try
            {
                ULONG_PTR end = rsp + kScanBytes;
                if (stackHigh != 0 && end > stackHigh)
                    end = stackHigh;

                for (ULONG_PTR p = rsp & ~(ULONG_PTR)7; p + sizeof(uintptr_t) <= end && count < maxHits; p += sizeof(uintptr_t))
                {
                    if ((p & 0xFFF) == 0 && !IsReadable((const void*)p, sizeof(uintptr_t)))
                        break;

                    uintptr_t value = *(const uintptr_t*)p;
                    if (IsModCode(value))
                    {
                        hits[count] = value;
                        offsets[count] = p - rsp;
                        ++count;
                    }
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
            }
            return count;
        }

        // ------------------------------------------------------------------ exception details

        const char* ExceptionCodeName(DWORD code)
        {
            switch (code)
            {
            case EXCEPTION_ACCESS_VIOLATION:        return "EXCEPTION_ACCESS_VIOLATION";
            case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:   return "EXCEPTION_ARRAY_BOUNDS_EXCEEDED";
            case EXCEPTION_DATATYPE_MISALIGNMENT:   return "EXCEPTION_DATATYPE_MISALIGNMENT";
            case EXCEPTION_FLT_DIVIDE_BY_ZERO:      return "EXCEPTION_FLT_DIVIDE_BY_ZERO";
            case EXCEPTION_FLT_INVALID_OPERATION:   return "EXCEPTION_FLT_INVALID_OPERATION";
            case EXCEPTION_ILLEGAL_INSTRUCTION:     return "EXCEPTION_ILLEGAL_INSTRUCTION";
            case EXCEPTION_IN_PAGE_ERROR:           return "EXCEPTION_IN_PAGE_ERROR";
            case EXCEPTION_INT_DIVIDE_BY_ZERO:      return "EXCEPTION_INT_DIVIDE_BY_ZERO";
            case EXCEPTION_INT_OVERFLOW:            return "EXCEPTION_INT_OVERFLOW";
            case EXCEPTION_NONCONTINUABLE_EXCEPTION:return "EXCEPTION_NONCONTINUABLE_EXCEPTION";
            case EXCEPTION_PRIV_INSTRUCTION:        return "EXCEPTION_PRIV_INSTRUCTION";
            case EXCEPTION_STACK_OVERFLOW:          return "EXCEPTION_STACK_OVERFLOW";
            case EXCEPTION_BREAKPOINT:              return "EXCEPTION_BREAKPOINT";
            case 0xC0000374:                        return "STATUS_HEAP_CORRUPTION";
            case 0xC0000409:                        return "STATUS_STACK_BUFFER_OVERRUN";
            case kCppExceptionCode:                 return "C++ exception";
            default:                                return "unknown exception";
            }
        }

        bool IsFatalCode(DWORD code)
        {
            switch (code)
            {
            case EXCEPTION_ACCESS_VIOLATION:
            case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
            case EXCEPTION_DATATYPE_MISALIGNMENT:
            case EXCEPTION_FLT_DIVIDE_BY_ZERO:
            case EXCEPTION_FLT_INVALID_OPERATION:
            case EXCEPTION_ILLEGAL_INSTRUCTION:
            case EXCEPTION_IN_PAGE_ERROR:
            case EXCEPTION_INT_DIVIDE_BY_ZERO:
            case EXCEPTION_INT_OVERFLOW:
            case EXCEPTION_NONCONTINUABLE_EXCEPTION:
            case EXCEPTION_PRIV_INSTRUCTION:
            case EXCEPTION_STACK_OVERFLOW:
            case 0xC0000374: // heap corruption
                return true;
            default:
                return false;
            }
        }

        // ".?AVException@System@@" -> "System::Exception"
        void UndecorateTypeName(const char* decorated, char* out, size_t outSize)
        {
            out[0] = 0;
            if (::strncmp(decorated, ".?A", 3) != 0 || (decorated[3] != 'V' && decorated[3] != 'U'))
            {
                ::StringCchCopyA(out, outSize, decorated);
                return;
            }

            const char* parts[8];
            size_t lengths[8];
            int count = 0;

            const char* p = decorated + 4;
            while (*p != 0 && *p != '@' && count < 8)
            {
                const char* start = p;
                while (*p != 0 && *p != '@')
                    ++p;
                parts[count] = start;
                lengths[count] = (size_t)(p - start);
                ++count;
                if (*p == '@')
                    ++p;
            }

            for (int i = count - 1; i >= 0; --i)
            {
                ::StringCchCatNA(out, outSize, parts[i], lengths[i]);
                if (i > 0)
                    ::StringCchCatA(out, outSize, "::");
            }
        }

        // Managed exception class and message ("System.NullReferenceException: Object reference ...").
        // Reads Mono's internal MonoException layout (message is the 2nd field after the object
        // header), every pointer is checked and the whole thing runs under SEH.
        bool DescribeManagedException(void* monoObject, char* out, size_t outSize)
        {
            out[0] = 0;
            if (g_MonoObjectGetClass == nullptr || g_MonoClassGetName == nullptr || !IsReadable(monoObject, 32))
                return false;

            __try
            {
                void* klass = g_MonoObjectGetClass(monoObject);
                if (klass == nullptr)
                    return false;

                const char* ns = g_MonoClassGetNamespace ? g_MonoClassGetNamespace(klass) : "";
                const char* name = g_MonoClassGetName(klass);
                ::StringCchPrintfA(out, outSize, "%s%s%s", ns && ns[0] ? ns : "", ns && ns[0] ? "." : "", name ? name : "?");

                const char* const* fields = (const char* const*)monoObject;
                const char* message = fields[3]; // vtable, sync, class_name, message
                if (message != nullptr && IsReadable(message, 24))
                {
                    int length = *(const int*)(message + 16);
                    const wchar_t* chars = (const wchar_t*)(message + 20);
                    if (length > 0 && length < 2048 && IsReadable(chars, (size_t)length * sizeof(wchar_t)))
                    {
                        size_t used = ::strlen(out);
                        if (used + 4 < outSize)
                        {
                            ::StringCchCatA(out, outSize, ": ");
                            used += 2;
                            int n = ::WideCharToMultiByte(CP_UTF8, 0, chars, length, out + used, (int)(outSize - used - 1), nullptr, nullptr);
                            out[used + (n > 0 ? n : 0)] = 0;
                        }
                    }
                }
                return true;
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                return false;
            }
        }

        // Type of a thrown C++ object, taken from the exception record (MSVC x64 ThrowInfo layout).
        bool DescribeCppException(const EXCEPTION_RECORD* rec, char* typeOut, size_t typeOutSize, void** object)
        {
            typeOut[0] = 0;
            *object = nullptr;
            if (rec->NumberParameters < 4)
                return false;

            __try
            {
                const uintptr_t imageBase = rec->ExceptionInformation[3];
                const uintptr_t throwInfo = rec->ExceptionInformation[2];
                *object = (void*)rec->ExceptionInformation[1];

                // ThrowInfo { attributes, pmfnUnwind, pForwardCompat, pCatchableTypeArray } (RVAs)
                const int ctaRva = *(const int*)(throwInfo + 12);
                const uintptr_t cta = imageBase + ctaRva;
                if (*(const int*)cta <= 0)
                    return false;

                // CatchableTypeArray { count, rva[] }, the first entry is the thrown type itself.
                const uintptr_t catchableType = imageBase + *(const int*)(cta + 4);
                // CatchableType { properties, pType, ... }
                const uintptr_t typeDescriptor = imageBase + *(const int*)(catchableType + 4);
                // TypeDescriptor { pVFTable, spare, name[] }
                const char* decorated = (const char*)(typeDescriptor + 16);
                UndecorateTypeName(decorated, typeOut, typeOutSize);
                return true;
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                return false;
            }
        }

        // ------------------------------------------------------------------ report sections

        HANDLE CreateReportFile()
        {
            ::CreateDirectoryW(g_CrashDir, nullptr);

            SYSTEMTIME st;
            ::GetLocalTime(&st);

            for (int i = 1; i <= 20; ++i)
            {
                wchar_t path[MAX_PATH];
                if (i == 1)
                    ::StringCchPrintfW(path, MAX_PATH, L"%ls\\crash_%04u-%02u-%02u_%02u-%02u-%02u.txt",
                        g_CrashDir, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
                else
                    ::StringCchPrintfW(path, MAX_PATH, L"%ls\\crash_%04u-%02u-%02u_%02u-%02u-%02u_%d.txt",
                        g_CrashDir, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, i);

                HANDLE file = ::CreateFileW(path, GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
                if (file != INVALID_HANDLE_VALUE)
                {
                    ::StringCchCopyW(g_LastReportPath, MAX_PATH, path);
                    return file;
                }

                if (::GetLastError() != ERROR_FILE_EXISTS)
                    break;
            }
            return INVALID_HANDLE_VALUE;
        }

        void WriteHeader(const ReportInput& in)
        {
            SYSTEMTIME st;
            ::GetLocalTime(&st);

            Out("==================== RepoHax crash report ====================\r\n");
            OutF("What happened : %s\r\n", in.Title);
            OutF("Time          : %04u-%02u-%02u %02u:%02u:%02u\r\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
            OutF("Mod was in    : %s\r\n", in.Stage ? in.Stage : "(no hook running on this thread)");
            OutF("Init step     : %s\r\n", g_InitStage);
            OutF("Thread id     : %lu\r\n", in.ThreadId);
            OutF("Game version  : %s\r\n", g_GameVersion[0] ? g_GameVersion : "unknown (game not fully loaded yet)");
            Out ("Game exe      : "); OutW(g_ExePath); Out("\r\n");
            Out ("Mod dll       : "); OutW(g_ModulePath); OutF("  (built %s %s)\r\n", __DATE__, __TIME__);
            OutF("Windows       : %s\r\n", g_WindowsVersion);
            if (g_UnityLogPath[0])
            {
                Out("Unity log     : "); OutW(g_UnityLogPath); Out("  (attach it too when reporting the crash)\r\n");
            }

            if (in.Message != nullptr)
            {
                Out("Message       :\r\n");
                OutIndentedW(in.Message, "    ");
            }
            if (in.MessageA != nullptr)
            {
                Out("Details       :\r\n");
                OutIndentedA(in.MessageA, "    ");
            }
            Out("\r\n");
        }

        void WriteException(const ReportInput& in)
        {
            if (in.Record == nullptr)
                return;

            const EXCEPTION_RECORD* rec = in.Record;
            char address[256];

            Out("-- Exception --\r\n");
            OutF("Code          : 0x%08lX %s\r\n", rec->ExceptionCode, ExceptionCodeName(rec->ExceptionCode));

            DescribeAddress((uintptr_t)rec->ExceptionAddress, address, sizeof(address));
            OutF("Address       : %s\r\n", address);

            if (rec->ExceptionCode == EXCEPTION_ACCESS_VIOLATION && rec->NumberParameters >= 2)
            {
                const ULONG_PTR kind = rec->ExceptionInformation[0];
                const ULONG_PTR target = rec->ExceptionInformation[1];
                const char* op = kind == 0 ? "read from" : kind == 1 ? "write to" : kind == 8 ? "execute at" : "access to";
                OutF("Details       : %s address 0x%016llX%s\r\n", op, (unsigned long long)target,
                    target < 0x10000 ? "  (a null object/pointer was used)" : "");
            }
            else if (rec->ExceptionCode == kCppExceptionCode)
            {
                char type[256];
                void* object = nullptr;
                if (DescribeCppException(rec, type, sizeof(type), &object))
                {
                    OutF("C++ type      : %s\r\n", type);

                    // System::* wrappers hold the managed exception object as their only member.
                    if (::strncmp(type, "System::", 8) == 0 && IsReadable(object, sizeof(void*)))
                    {
                        char managed[1024];
                        if (DescribeManagedException(*(void**)object, managed, sizeof(managed)))
                            OutF("Managed       : %s\r\n", managed);
                    }
                }
            }

            if (IsModAddress((uintptr_t)rec->ExceptionAddress))
                Out("Note          : the crash happened inside RepoHax code.\r\n");
            Out("\r\n");
        }

        void WriteRegisters(const CONTEXT* c)
        {
            Out("-- Registers --\r\n");
            OutF("RAX=%016llX RBX=%016llX RCX=%016llX RDX=%016llX\r\n", c->Rax, c->Rbx, c->Rcx, c->Rdx);
            OutF("RSI=%016llX RDI=%016llX RBP=%016llX RSP=%016llX\r\n", c->Rsi, c->Rdi, c->Rbp, c->Rsp);
            OutF("R8 =%016llX R9 =%016llX R10=%016llX R11=%016llX\r\n", c->R8, c->R9, c->R10, c->R11);
            OutF("R12=%016llX R13=%016llX R14=%016llX R15=%016llX\r\n", c->R12, c->R13, c->R14, c->R15);
            OutF("RIP=%016llX EFLAGS=%08lX\r\n\r\n", c->Rip, c->EFlags);
        }

        void WriteCallStack(const ReportInput& in, const CONTEXT* ctx)
        {
            WalkStack(ctx);

            Out("-- Call stack --\r\n");
            char address[256];
            for (int i = 0; i < g_FrameCount; ++i)
            {
                DescribeAddress(g_Frames[i], address, sizeof(address));
                OutF(" #%02d %s\r\n", i, address);
            }
            if (g_FramesTruncated)
                Out(" ... (the rest of the stack could not be walked, see the scan below)\r\n");
            Out("\r\n");

            uintptr_t hits[kMaxScanHits];
            ULONG_PTR offsets[kMaxScanHits];
            const int hitCount = ScanStackForModCode((ULONG_PTR)ctx->Rsp, in.StackHigh, hits, offsets, kMaxScanHits);

            Out("-- RepoHax code addresses found on the stack (return addresses of active mod functions) --\r\n");
            if (hitCount == 0)
                Out(" none: mod code was not running on this thread\r\n");
            for (int i = 0; i < hitCount; ++i)
            {
                DescribeAddress(hits[i], address, sizeof(address));
                OutF(" [rsp+0x%05llX] %s\r\n", (unsigned long long)offsets[i], address);
            }
            Out("\r\n");
        }

        void WriteLogTail()
        {
            Out("-- Last lines of haxsdk_logs.txt --\r\n");

            HANDLE file = ::CreateFileW(g_LogPath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
            if (file == INVALID_HANDLE_VALUE)
            {
                Out(" (log file not found)\r\n\r\n");
                return;
            }

            LARGE_INTEGER size{};
            ::GetFileSizeEx(file, &size);

            const LONGLONG maxBytes = (LONGLONG)(sizeof(g_WideTail) - sizeof(wchar_t));
            LONGLONG offset = size.QuadPart > maxBytes ? size.QuadPart - maxBytes : 0;
            offset &= ~(LONGLONG)1; // the log is UTF-16

            LARGE_INTEGER pos;
            pos.QuadPart = offset;
            ::SetFilePointerEx(file, pos, nullptr, FILE_BEGIN);

            DWORD read = 0;
            ::ReadFile(file, g_WideTail, (DWORD)maxBytes, &read, nullptr);
            ::CloseHandle(file);

            size_t count = read / sizeof(wchar_t);
            g_WideTail[count] = 0;

            const wchar_t* text = g_WideTail;
            if (count > 0 && text[0] == 0xFEFF)
            {
                ++text;
                --count;
            }

            // Find where the last kLogTailLines lines start.
            int lines = 0;
            const wchar_t* start = text;
            for (size_t i = count; i > 0; --i)
            {
                if (text[i - 1] == L'\n' && i != count && ++lines >= kLogTailLines)
                {
                    start = text + i;
                    break;
                }
            }
            if (start == text && offset > 0)
            {
                // Started in the middle of a line, skip it.
                const wchar_t* nl = ::wcschr(text, L'\n');
                if (nl != nullptr)
                    start = nl + 1;
            }

            OutW(start);
            Out("\r\n");
        }

        // Function names for mod frames. Uses dbghelp, so it goes last: if it fails, the rest of the
        // report is already on disk.
        void WriteSymbols()
        {
            int modFrames = 0;
            for (int i = 0; i < g_FrameCount; ++i)
                modFrames += IsModCode(g_Frames[i]) ? 1 : 0;
            if (modFrames == 0)
                return;

            Out("-- RepoHax function names --\r\n");
            if (g_SymInitializeW == nullptr || g_SymLoadModuleExW == nullptr || g_SymFromAddrW == nullptr)
            {
                Out(" dbghelp.dll is not available.\r\n\r\n");
                return;
            }

            const DWORD oldOptions = g_SymGetOptions ? g_SymGetOptions() : 0;
            if (g_SymSetOptions)
                g_SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES | SYMOPT_FAIL_CRITICAL_ERRORS | SYMOPT_NO_PROMPTS);

            wchar_t searchPath[MAX_PATH];
            ::StringCchCopyW(searchPath, MAX_PATH, g_ModulePath);
            if (wchar_t* slash = ::wcsrchr(searchPath, L'\\'))
                *slash = 0;

            if (g_SymInitializeW(kSymSession, searchPath, FALSE))
            {
                g_SymLoadModuleExW(kSymSession, nullptr, g_ModulePath, nullptr, (DWORD64)g_ModuleBase, (DWORD)(g_ModuleEnd - g_ModuleBase), nullptr, 0);

                IMAGEHLP_MODULEW64 info{};
                info.SizeOfStruct = sizeof(info);
                const bool hasPdb = g_SymGetModuleInfoW64 && g_SymGetModuleInfoW64(kSymSession, (DWORD64)g_ModuleBase, &info) && info.SymType == SymPdb;

                if (!hasPdb)
                {
                    Out(" dwmapi.pdb was not found next to the dll. Put it there to get function names;\r\n");
                    Out(" the module offsets above are enough for the developer.\r\n");
                }
                else
                {
                    for (int i = 0; i < g_FrameCount; ++i)
                    {
                        if (!IsModCode(g_Frames[i]))
                            continue;

                        // Return addresses point after the call instruction.
                        const DWORD64 addr = (DWORD64)g_Frames[i] - (i > 0 ? 1 : 0);

                        auto* symbol = (SYMBOL_INFOW*)g_SymbolBuffer;
                        ::memset(symbol, 0, sizeof(SYMBOL_INFOW));
                        symbol->SizeOfStruct = sizeof(SYMBOL_INFOW);
                        symbol->MaxNameLen = 512;

                        DWORD64 displacement = 0;
                        OutF(" #%02d ", i);
                        if (g_SymFromAddrW(kSymSession, addr, &displacement, symbol))
                        {
                            OutW(symbol->Name);
                            OutF("+0x%llX", (unsigned long long)displacement);
                        }
                        else
                        {
                            Out("?");
                        }

                        IMAGEHLP_LINEW64 line{};
                        line.SizeOfStruct = sizeof(line);
                        DWORD lineDisplacement = 0;
                        if (g_SymGetLineFromAddrW64 && g_SymGetLineFromAddrW64(kSymSession, addr, &lineDisplacement, &line) && line.FileName)
                        {
                            Out("   ");
                            OutW(line.FileName);
                            OutF(":%lu", line.LineNumber);
                        }
                        Out("\r\n");
                    }
                }

                g_SymCleanup(kSymSession);
            }
            else
            {
                Out(" dbghelp could not be initialized.\r\n");
            }

            if (g_SymSetOptions)
                g_SymSetOptions(oldOptions);
            Out("\r\n");
        }

        // ------------------------------------------------------------------ writer

        bool AcquireWriter()
        {
            const DWORD self = ::GetCurrentThreadId();
            for (int i = 0; i < 500; ++i)
            {
                DWORD expected = 0;
                if (g_WriterThread.compare_exchange_strong(expected, self))
                    return true;
                if (expected == self)
                    return false; // crashed while writing a report
                ::Sleep(10);
            }
            return false;
        }

        void ReleaseWriter()
        {
            g_WriterThread.store(0);
        }

        void WriteReport(const ReportInput& in)
        {
            if (!AcquireWriter())
                return;

            if (g_ReportCount >= kMaxReports)
            {
                ReleaseWriter();
                return;
            }
            ++g_ReportCount;

            g_File = CreateReportFile();
            if (g_File == INVALID_HANDLE_VALUE)
            {
                ReleaseWriter();
                return;
            }

            g_OutLen = 0;
            Out("\xEF\xBB\xBF");
            WriteHeader(in);
            Flush();

            WriteException(in);
            Flush();

            const CONTEXT* ctx = in.Context;
            if (ctx == nullptr)
            {
                ::RtlCaptureContext(&g_CaptureContext);
                ctx = &g_CaptureContext;
            }

            WriteRegisters(ctx);
            Flush();

            WriteCallStack(in, ctx);
            Flush();

            WriteLogTail();
            Flush();

            if (in.AllowSymbols)
            {
                WriteSymbols();
                Flush();
            }

            Out("==================== end of report ====================\r\n");
            Flush();

            ::CloseHandle(g_File);
            g_File = INVALID_HANDLE_VALUE;
            ReleaseWriter();
        }

        DWORD WINAPI ReportHelperThread(LPVOID)
        {
            WriteReport(g_HelperInput);
            return 0;
        }

        // For stack overflows: the crashed thread has almost no stack left.
        void WriteReportOnHelperThread(const ReportInput& in)
        {
            g_HelperInput = in;
            if (in.Context != nullptr)
            {
                g_HelperContext = *in.Context;
                g_HelperInput.Context = &g_HelperContext;
            }

            HANDLE thread = ::CreateThread(nullptr, 256 * 1024, ReportHelperThread, nullptr, 0, nullptr);
            if (thread != nullptr)
            {
                ::WaitForSingleObject(thread, 10000);
                ::CloseHandle(thread);
            }
        }

        void GetCurrentStackBounds(ULONG_PTR& low, ULONG_PTR& high)
        {
            const NT_TIB* tib = (const NT_TIB*)::NtCurrentTeb();
            low = (ULONG_PTR)tib->StackLimit;
            high = (ULONG_PTR)tib->StackBase;
        }

        ReportInput MakeCurrentThreadInput(const char* title)
        {
            ReportInput in{};
            in.Title = title;
            in.Stage = t_Stage;
            in.ThreadId = ::GetCurrentThreadId();
            in.AllowSymbols = true;
            GetCurrentStackBounds(in.StackLow, in.StackHigh);
            return in;
        }

        bool PendingIsFresh()
        {
            return g_Pending.Valid && ::GetTickCount64() - g_Pending.Tick <= kPendingMaxAgeMs;
        }

        void WritePendingReport(const char* title, bool allowSymbols)
        {
            if (!PendingIsFresh())
                return;

            g_Pending.Valid = false;

            ReportInput in{};
            in.Title = title;
            in.Record = &g_Pending.Record;
            in.Context = &g_Pending.Context;
            in.Stage = g_Pending.Stage;
            in.ThreadId = g_Pending.ThreadId;
            in.StackLow = g_Pending.StackLow;
            in.StackHigh = g_Pending.StackHigh;
            in.AllowSymbols = allowSymbols;
            WriteReport(in);
        }

        // ------------------------------------------------------------------ handlers

        LONG CALLBACK VectoredHandler(PEXCEPTION_POINTERS ep)
        {
            const EXCEPTION_RECORD* rec = ep->ExceptionRecord;
            if (!IsFatalCode(rec->ExceptionCode))
                return EXCEPTION_CONTINUE_SEARCH;

            if (g_WriterThread.load() == ::GetCurrentThreadId())
                return EXCEPTION_CONTINUE_SEARCH; // a crash inside the reporter itself

            const uintptr_t pc = (uintptr_t)rec->ExceptionAddress;

            // Faults in JIT-compiled code are normal: Mono turns them into managed exceptions
            // (NullReferenceException, ...) and the game goes on.
            PVOID base = nullptr;
            ::RtlPcToFileHeader((PVOID)pc, &base);
            if (base == nullptr)
                return EXCEPTION_CONTINUE_SEARCH;

            const bool inMod = IsModCode(pc);
            if (!inMod && !IsModOnStack(ep->ContextRecord))
                return EXCEPTION_CONTINUE_SEARCH;

            ULONG_PTR stackLow, stackHigh;
            GetCurrentStackBounds(stackLow, stackHigh);

            if (inMod)
            {
                // Nothing in the mod handles CPU exceptions, the game is going down.
                ReportInput in{};
                in.Title = "Fatal error inside RepoHax code";
                in.Record = rec;
                in.Context = ep->ContextRecord;
                in.Stage = t_Stage;
                in.ThreadId = ::GetCurrentThreadId();
                in.StackLow = stackLow;
                in.StackHigh = stackHigh;
                in.AllowSymbols = true;

                if (rec->ExceptionCode == EXCEPTION_STACK_OVERFLOW)
                    WriteReportOnHelperThread(in);
                else
                    WriteReport(in);
            }
            else if (AcquireWriter())
            {
                g_Pending.Valid = true;
                g_Pending.Tick = ::GetTickCount64();
                g_Pending.ThreadId = ::GetCurrentThreadId();
                g_Pending.Stage = t_Stage;
                g_Pending.StackLow = stackLow;
                g_Pending.StackHigh = stackHigh;
                g_Pending.Record = *rec;
                g_Pending.Record.ExceptionRecord = nullptr;
                g_Pending.Context = *ep->ContextRecord;
                ReleaseWriter();
            }

            return EXCEPTION_CONTINUE_SEARCH;
        }

        LONG WINAPI TopLevelFilter(PEXCEPTION_POINTERS ep)
        {
            // Nobody handled this exception: the game is crashing for sure.
            if (g_WriterThread.load() != ::GetCurrentThreadId())
            {
                const EXCEPTION_RECORD* rec = ep->ExceptionRecord;
                if (g_Pending.Valid && g_Pending.Record.ExceptionCode == rec->ExceptionCode && g_Pending.Record.ExceptionAddress == rec->ExceptionAddress)
                    g_Pending.Valid = false; // same exception, the report below covers it

                ReportInput in = MakeCurrentThreadInput(rec->ExceptionCode == kCppExceptionCode
                    ? "Unhandled C++ exception (an error escaped a hook)"
                    : "Unhandled exception, the game crashed");
                in.Record = rec;
                in.Context = ep->ContextRecord;

                if (rec->ExceptionCode == EXCEPTION_STACK_OVERFLOW)
                    WriteReportOnHelperThread(in);
                else
                    WriteReport(in);
            }

            // Let Unity's crash handler do its job too.
            LPTOP_LEVEL_EXCEPTION_FILTER prev = g_PrevFilter;
            if (prev != nullptr && prev != TopLevelFilter)
                return prev(ep);
            return EXCEPTION_CONTINUE_SEARCH;
        }

        // Keeps our filter installed: later callers (Unity) are chained behind it.
        LPTOP_LEVEL_EXCEPTION_FILTER WINAPI Hooked_SetUnhandledExceptionFilter(LPTOP_LEVEL_EXCEPTION_FILTER filter)
        {
            LPTOP_LEVEL_EXCEPTION_FILTER old = g_PrevFilter;
            if (filter != TopLevelFilter)
                g_PrevFilter = filter;
            return old;
        }

        void __cdecl OnAbortSignal(int sig)
        {
            if (!g_TerminateReported.load())
            {
                if (PendingIsFresh())
                {
                    WritePendingReport("abort() after a fatal error with RepoHax code on the call stack", true);
                }
                else
                {
                    ReportInput in = MakeCurrentThreadInput("abort() was called");
                    WriteReport(in);
                }
            }

            SignalHandler prev = g_PrevAbortHandler;
            if (prev != nullptr && prev != SIG_DFL && prev != SIG_IGN && prev != SIG_ERR && prev != OnAbortSignal)
                prev(sig);
        }

        char g_TerminateDetails[1200];

        void DescribeCurrentException()
        {
            ::StringCchCopyA(g_TerminateDetails, _countof(g_TerminateDetails), "no active exception");

            std::exception_ptr current = std::current_exception();
            if (!current)
                return;

            try
            {
                std::rethrow_exception(current);
            }
            catch (const System::Exception& ex)
            {
                char managed[1024];
                if (DescribeManagedException(std::bit_cast<void*>(ex), managed, sizeof(managed)))
                    ::StringCchPrintfA(g_TerminateDetails, _countof(g_TerminateDetails), "uncaught System::Exception -> managed %s", managed);
                else
                    ::StringCchCopyA(g_TerminateDetails, _countof(g_TerminateDetails), "uncaught System::Exception (managed exception from the game)");
            }
            catch (const std::exception& ex)
            {
                ::StringCchPrintfA(g_TerminateDetails, _countof(g_TerminateDetails), "uncaught std::exception: %s", ex.what());
            }
            catch (...)
            {
                ::StringCchCopyA(g_TerminateDetails, _countof(g_TerminateDetails), "uncaught exception of unknown type");
            }
        }

        [[noreturn]] void OnTerminate()
        {
            if (!g_TerminateReported.exchange(true))
            {
                DescribeCurrentException();

                ReportInput in = MakeCurrentThreadInput("std::terminate() was called");
                in.MessageA = g_TerminateDetails;
                WriteReport(in);
            }
            std::abort();
        }

        const wchar_t* OnPanic(const wchar_t* message)
        {
            ReportInput in = MakeCurrentThreadInput("The mod stopped the game with a critical error (Hax::Panic)");
            in.Message = message;
            WriteReport(in);
            return g_LastReportPath[0] ? g_LastReportPath : nullptr;
        }

        void PrepareThread()
        {
            t_ThreadReady = true;
            std::set_terminate(OnTerminate); // terminate handlers are per thread
        }

        void InstallAbortHandler()
        {
            SignalHandler prev = std::signal(SIGABRT, OnAbortSignal);
            if (prev != SIG_ERR && prev != OnAbortSignal)
                g_PrevAbortHandler = prev;
        }

        // ------------------------------------------------------------------ setup helpers

        void ReadModuleLayout(HMODULE hModule)
        {
            g_ModuleBase = (uintptr_t)hModule;
            g_ModuleEnd = g_ModuleBase;
            g_CodeBegin = g_CodeEnd = 0;

            const IMAGE_NT_HEADERS64* nt = GetNtHeaders(g_ModuleBase);
            if (nt == nullptr)
                return;

            g_ModuleEnd = g_ModuleBase + nt->OptionalHeader.SizeOfImage;

            const IMAGE_SECTION_HEADER* section = IMAGE_FIRST_SECTION(nt);
            for (WORD i = 0; i < nt->FileHeader.NumberOfSections; ++i, ++section)
            {
                if ((section->Characteristics & IMAGE_SCN_MEM_EXECUTE) == 0)
                    continue;

                const uintptr_t begin = g_ModuleBase + section->VirtualAddress;
                const uintptr_t end = begin + Hax::Max<DWORD>(section->Misc.VirtualSize, section->SizeOfRawData);
                if (g_CodeBegin == 0 || begin < g_CodeBegin)
                    g_CodeBegin = begin;
                if (end > g_CodeEnd)
                    g_CodeEnd = end;
            }

            if (g_CodeBegin == 0)
            {
                g_CodeBegin = g_ModuleBase;
                g_CodeEnd = g_ModuleEnd;
            }
        }

        void ReadWindowsVersion()
        {
            ::StringCchCopyA(g_WindowsVersion, _countof(g_WindowsVersion), "unknown");

            using RtlGetVersionFn = LONG(WINAPI*)(OSVERSIONINFOW*);
            HMODULE ntdll = ::GetModuleHandleW(L"ntdll.dll");
            auto rtlGetVersion = ntdll ? (RtlGetVersionFn)::GetProcAddress(ntdll, "RtlGetVersion") : nullptr;
            if (rtlGetVersion == nullptr)
                return;

            OSVERSIONINFOW info{};
            info.dwOSVersionInfoSize = sizeof(info);
            if (rtlGetVersion(&info) == 0)
                ::StringCchPrintfA(g_WindowsVersion, _countof(g_WindowsVersion), "%lu.%lu.%lu", info.dwMajorVersion, info.dwMinorVersion, info.dwBuildNumber);
        }

        void LoadDbgHelp()
        {
            // Only from System32: never pick up a dbghelp.dll planted in the game folder.
            HMODULE dbghelp = ::LoadLibraryExW(L"dbghelp.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
            if (dbghelp == nullptr)
                return;

            g_SymGetOptions         = (SymGetOptionsFn)::GetProcAddress(dbghelp, "SymGetOptions");
            g_SymSetOptions         = (SymSetOptionsFn)::GetProcAddress(dbghelp, "SymSetOptions");
            g_SymInitializeW        = (SymInitializeWFn)::GetProcAddress(dbghelp, "SymInitializeW");
            g_SymCleanup            = (SymCleanupFn)::GetProcAddress(dbghelp, "SymCleanup");
            g_SymLoadModuleExW      = (SymLoadModuleExWFn)::GetProcAddress(dbghelp, "SymLoadModuleExW");
            g_SymGetModuleInfoW64   = (SymGetModuleInfoW64Fn)::GetProcAddress(dbghelp, "SymGetModuleInfoW64");
            g_SymFromAddrW          = (SymFromAddrWFn)::GetProcAddress(dbghelp, "SymFromAddrW");
            g_SymGetLineFromAddrW64 = (SymGetLineFromAddrW64Fn)::GetProcAddress(dbghelp, "SymGetLineFromAddrW64");

            if (g_SymCleanup == nullptr)
                g_SymInitializeW = nullptr;
        }

        void HookSetUnhandledExceptionFilter()
        {
            // kernel32's export jumps into kernelbase, hooking kernelbase covers both.
            HMODULE module = ::GetModuleHandleW(L"kernelbase.dll");
            if (module == nullptr)
                module = ::GetModuleHandleW(L"kernel32.dll");
            if (module == nullptr)
                return;

            void* target = (void*)::GetProcAddress(module, "SetUnhandledExceptionFilter");
            if (target == nullptr)
                return;

            auto error = SafetyHookInline::create(target, (void*)&Hooked_SetUnhandledExceptionFilter, g_SetFilterHook);
            (void)error; // without the hook our filter still works until someone replaces it
        }

        // Reads a small UTF-8 text file into a wide buffer. Returns the number of characters.
        int ReadSmallTextFile(const wchar_t* path, wchar_t* out, int outCount)
        {
            out[0] = 0;
            HANDLE file = ::CreateFileW(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
            if (file == INVALID_HANDLE_VALUE)
                return 0;

            char bytes[1024];
            DWORD read = 0;
            ::ReadFile(file, bytes, sizeof(bytes) - 1, &read, nullptr);
            ::CloseHandle(file);

            int skip = (read >= 3 && (unsigned char)bytes[0] == 0xEF && (unsigned char)bytes[1] == 0xBB && (unsigned char)bytes[2] == 0xBF) ? 3 : 0;
            int n = ::MultiByteToWideChar(CP_UTF8, 0, bytes + skip, (int)read - skip, out, outCount - 1);
            n = n > 0 ? n : 0;
            out[n] = 0;
            return n;
        }

        // Unity keeps company/product names in <Game>_Data\app.info; the game writes its version to
        // %USERPROFILE%\AppData\LocalLow\<company>\<product>\Cache\version.txt (BuildManager).
        void ReadGameInfo()
        {
            wchar_t appInfoPath[MAX_PATH];
            ::StringCchCopyW(appInfoPath, MAX_PATH, g_ExePath);
            wchar_t* dot = ::wcsrchr(appInfoPath, L'.');
            if (dot == nullptr)
                return;
            *dot = 0;
            ::StringCchCatW(appInfoPath, MAX_PATH, L"_Data\\app.info");

            wchar_t appInfo[256];
            if (ReadSmallTextFile(appInfoPath, appInfo, _countof(appInfo)) == 0)
                return;

            // line 1: company, line 2: product
            wchar_t* company = appInfo;
            wchar_t* product = ::wcschr(appInfo, L'\n');
            if (product == nullptr)
                return;
            *product++ = 0;
            if (wchar_t* cr = ::wcschr(company, L'\r')) *cr = 0;
            if (wchar_t* end = ::wcspbrk(product, L"\r\n")) *end = 0;

            wchar_t userProfile[MAX_PATH];
            if (::GetEnvironmentVariableW(L"USERPROFILE", userProfile, MAX_PATH) == 0)
                return;

            wchar_t dataDir[MAX_PATH];
            ::StringCchPrintfW(dataDir, MAX_PATH, L"%ls\\AppData\\LocalLow\\%ls\\%ls", userProfile, company, product);
            ::StringCchPrintfW(g_UnityLogPath, MAX_PATH, L"%ls\\Player.log", dataDir);

            wchar_t versionPath[MAX_PATH];
            ::StringCchPrintfW(versionPath, MAX_PATH, L"%ls\\Cache\\version.txt", dataDir);

            wchar_t version[128];
            if (ReadSmallTextFile(versionPath, version, _countof(version)) > 0)
            {
                if (wchar_t* end = ::wcspbrk(version, L"\r\n")) *end = 0;
                ::WideCharToMultiByte(CP_UTF8, 0, version, -1, g_GameVersion, (int)sizeof(g_GameVersion), nullptr, nullptr);
            }
        }
    }

    // ---------------------------------------------------------------------- public API

    void CrashReporter::Install(HMODULE hModule)
    {
        if (g_Installed.exchange(true))
            return;

        ReadModuleLayout(hModule);
        ::GetModuleFileNameW(hModule, g_ModulePath, MAX_PATH);
        ::GetModuleFileNameW(nullptr, g_ExePath, MAX_PATH);

        // Same folder as haxsdk_logs.txt / haxsdk.ini (they use the current directory too).
        ::GetFullPathNameW(kLogFileName, MAX_PATH, g_LogPath, nullptr);
        ::GetFullPathNameW(kCrashDirName, MAX_PATH, g_CrashDir, nullptr);

        ReadWindowsVersion();
        LoadDbgHelp();

        g_VehHandle = ::AddVectoredExceptionHandler(1, VectoredHandler);

        // Hook first, then install our filter through the original function: Unity runs on another
        // thread and may set its own filter at any moment, it must end up chained behind ours.
        HookSetUnhandledExceptionFilter();
        LPTOP_LEVEL_EXCEPTION_FILTER prevFilter = g_SetFilterHook
            ? g_SetFilterHook.unsafe_stdcall<LPTOP_LEVEL_EXCEPTION_FILTER>(&TopLevelFilter)
            : ::SetUnhandledExceptionFilter(TopLevelFilter);
        if (g_PrevFilter == nullptr && prevFilter != TopLevelFilter)
            g_PrevFilter = prevFilter; // else Unity already went through the hook in between

        InstallAbortHandler();
        Hax::SetPanicHandler(OnPanic);
        PrepareThread();
    }

    void CrashReporter::OnUnityLoaded()
    {
        if (!g_Installed.load())
            return;

        // Mono and Unity added their vectored handlers after ours; a handler that ends the process
        // would run before we could record anything. Move ours back to the front.
        if (g_VehHandle != nullptr)
            ::RemoveVectoredExceptionHandler(g_VehHandle);
        g_VehHandle = ::AddVectoredExceptionHandler(1, VectoredHandler);

        InstallAbortHandler();

        if (HMODULE mono = (HMODULE)Hax::Unity::GetUvmHandle())
        {
            g_MonoObjectGetClass    = (MonoObjectGetClassFn)::GetProcAddress(mono, "mono_object_get_class");
            g_MonoClassGetName      = (MonoClassGetNameFn)::GetProcAddress(mono, "mono_class_get_name");
            g_MonoClassGetNamespace = (MonoClassGetNameFn)::GetProcAddress(mono, "mono_class_get_namespace");
        }

        ReadGameInfo();
    }

    void CrashReporter::OnProcessExit(UINT exitCode, bool insideLoaderLock)
    {
        if (!g_Installed.load() || !PendingIsFresh())
            return;

        static char title[160];
        if (exitCode == (UINT)-1)
            ::StringCchCopyA(title, _countof(title),
                "The game was closed right after a fatal error with RepoHax code on the call stack");
        else
            ::StringCchPrintfA(title, _countof(title),
                "The game was closed (exit code 0x%08X) right after a fatal error with RepoHax code on the call stack", exitCode);
        WritePendingReport(title, !insideLoaderLock);
    }

    void CrashReporter::SetInitStage(const char* stage)
    {
        g_InitStage = stage;
    }

    const wchar_t* CrashReporter::LastReportPath()
    {
        return g_LastReportPath;
    }

    CrashReporter::Scope::Scope(const char* name) noexcept : m_Prev(t_Stage)
    {
        if (!t_ThreadReady)
            PrepareThread();
        t_Stage = name;
    }

    CrashReporter::Scope::~Scope() noexcept
    {
        t_Stage = m_Prev;
    }
}
