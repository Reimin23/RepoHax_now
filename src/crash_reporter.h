#pragma once

// Relies on src/pch.h (Windows.h) being included first, like the rest of the project.

namespace Cheat
{
    // Records why the game crashed while the mod was loaded.
    //
    // A report is written to "<game folder>\haxsdk_crashes\crash_<date>_<time>.txt" when:
    //   - a fatal CPU exception (access violation, illegal instruction, ...) happens inside
    //     the mod, or in another module while mod code is on the call stack;
    //   - nobody handles an exception (including C++ exceptions that escaped a hook);
    //   - std::terminate() or abort() is called;
    //   - Hax::Panic() is called (e.g. game classes/methods were not found after a game update).
    //
    // The report contains the reason, what the mod was doing at that moment (current hook and
    // initialization step), the game version, registers, the call stack (with function names
    // and source lines when dwmapi.pdb lies next to the dll) and the last lines of haxsdk_logs.txt.
    class CrashReporter
    {
    public:
        // Call as early as possible, before anything else is hooked.
        static void         Install(HMODULE hModule);

        // Call once Unity and Mono are running: they install their own exception handlers
        // after us, so ours are moved back in front of them.
        static void         OnUnityLoaded();

        // Call when the process is going away (TerminateProcess hook, DLL_PROCESS_DETACH).
        // Writes a report if a fatal exception with mod code on the stack happened just before.
        // Pass (UINT)-1 when the exit code is unknown.
        static void         OnProcessExit(UINT exitCode, bool insideLoaderLock = false);

        // Remembers the current step of the mod initialization ("Hooking DirectX 11", ...).
        // The string must stay alive (use literals).
        static void         SetInitStage(const char* stage);

        // Path of the last report written in this session, or an empty string.
        static const wchar_t* LastReportPath();

        // Marks the code running inside a scope (usually a hook body) for crash reports.
        // Use the CRASH_SCOPE("...") macro. The string must stay alive (use literals).
        class Scope
        {
        public:
            explicit        Scope(const char* name) noexcept;
                            ~Scope() noexcept;

                            Scope(const Scope&) = delete;
            Scope&          operator=(const Scope&) = delete;

        private:
            const char*     m_Prev;
        };
    };
}

#define CRASH_SCOPE_CONCAT_INNER(a, b) a##b
#define CRASH_SCOPE_CONCAT(a, b) CRASH_SCOPE_CONCAT_INNER(a, b)
#define CRASH_SCOPE(name) ::Cheat::CrashReporter::Scope CRASH_SCOPE_CONCAT(crashScope_, __LINE__)(name)
