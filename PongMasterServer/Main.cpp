#include "PongMasterServerIncludes.h"
#include "PongMasterServer.h"
#include "Entities/GameWorldFactory.h"

#include "Application/CrashHandler.h"

#include "resource.h"

#include "Define.h"
#include "Logger.h"
#include "GlobalCEFHandler.h"

#ifdef _WIN32
#include "include/cef_sandbox_win.h"
#endif

#include <string>
#include <iostream>



#define PROGRAMUSE_CUSTOMJS 0
#if PROGRAMUSE_CUSTOMJS == 1
// Define the actual macro //
#define LEVIATHAN_USES_CUSTOMJS
// Include required files //
#include "GlobalCEFHandler.h"
#include "GUI/GuiCEFApplication.h"
#endif

#ifdef LEVIATHAN_USES_CUSTOMJS
// Include the header //
#include ""
#include ""
#endif


// Breakpad is used to detect and report crashes
#ifdef MAKE_RELEASE
#ifdef __linux
#include "client/linux/handler/exception_handler.h"
#elif defined(_WIN32)
#include "breakpad/client/windows/handler/exception_handler.h"
#else
#error no breakpad on platform
#endif
#endif //MAKE_RELEASE

using namespace Pong;
// This is for easier logic with using standard classes instead of application specific
using namespace Leviathan;

// Don't look at the mess ahead, just set the variables in your cmake file //

#if MAKE_RELEASE
#ifdef _WIN32
#ifndef _DEBUG
bool DumpCallback(const wchar_t* dump_path,
    const wchar_t* minidump_id, void* context, EXCEPTION_POINTERS* exinfo,
    MDRawAssertionInfo* assertion, bool succeeded)
{
    // This is probably better to use than the conversion function
    std::wcout << L"Created stack dump: " << dump_path << L"\n";
    
    // const std::string path = Convert::WstringToString(dump_path);
    // printf("Created stack dump: %s\n", path.c_str());

    Leviathan::CrashHandler::DoBSFCallStackAfterBreakpad();
    return succeeded;
}

void BreakpadTriggerHelper(google_breakpad::ExceptionHandler& handler)
{
    handler.WriteMinidump();
}

void BreakpadTriggerHelperSEH(google_breakpad::ExceptionHandler& handler, void* data)
{
    handler.WriteMinidumpForException((EXCEPTION_POINTERS*)data);
}

#endif //_DEBUG
#else
bool DumpCallback(const google_breakpad::MinidumpDescriptor& descriptor, void* context,
    bool succeeded)
{
    printf("Created stack dump: %s\n", descriptor.path());
    Leviathan::CrashHandler::DoBSFCallStackAfterBreakpad();
    return succeeded;
}

void BreakpadTriggerHelper(google_breakpad::ExceptionHandler& handler)
{
    handler.SimulateSignalDelivery(SIGABRT);
}

#endif
#endif //MAKE_RELEASE

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine,
    int nCmdShow)
{
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_DEBUG);
#endif

#else
int main(int argcount, char* args[]){
#endif
    
#ifdef _WIN32
    std::vector<char*> argPtrs;
    const auto parsedArgs = Leviathan::LeviathanApplication::CommandLineStringSplitter(lpCmdLine, argPtrs);
    int argcount = parsedArgs.size();
    char** args = argPtrs.data();
#endif
    
    int Return = 0;

    // We need to manage the lifetime of an object //
    std::shared_ptr<Leviathan::CEFApplicationKeeper> KeepThisSafe;

    // This needs to create the sandbox as otherwise this doesn't work
#ifdef CEF_ENABLE_SANDBOX
    // Manage the life span of the sandbox information object. This is necessary
    // for sandbox support on Windows. See cef_sandbox_win.h for complete details.
    CefScopedSandboxInfo sandbox = CefScopedSandboxInfo();
#endif
    
    
    // The default CEF handling needs to be the first thing in the program //
    if(Leviathan::GlobalCEFHandler::CEFFirstCheckChildProcess(argcount, args, Return,
            KeepThisSafe, "PongMaster"
        #ifdef _WIN32
        #ifdef CEF_ENABLE_SANDBOX
            , sandbox
        #endif
            , hInstance
        #endif
        ))
    {
        // This was a child process, end it now //
        return Return;
    }
    // This is the main process, continue normally //
    
#ifdef _WIN32
    HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

    if(SUCCEEDED(CoInitialize(NULL))){
#else

#endif

#if MAKE_RELEASE
    // Crash handler //
#ifdef _WIN32
#ifndef _DEBUG
    google_breakpad::ExceptionHandler ExceptionHandler(L"./", NULL, DumpCallback, NULL, 
        google_breakpad::ExceptionHandler::HANDLER_ALL,
        (MINIDUMP_TYPE)(MiniDumpNormal & MiniDumpWithThreadInfo),
        (const wchar_t*)nullptr, NULL);

    Leviathan::CrashHandler::RegisterBreakpadWindowsSEHHandler(std::bind(&BreakpadTriggerHelperSEH,
        std::ref(ExceptionHandler), std::placeholders::_1));

#endif //_DEBUG
#else
    google_breakpad::MinidumpDescriptor descriptor("./");

    google_breakpad::ExceptionHandler ExceptionHandler(descriptor, NULL, DumpCallback, NULL,
        true, -1);
#endif
    // Generic handler registration
    Leviathan::CrashHandler::RegisterBreakpadGenericHandler(std::bind(&BreakpadTriggerHelper, std::ref(ExceptionHandler)));
#endif //MAKE_RELEASE

    // Logger. This is here to make logging shutdown errors etc. possible //
    Logger mainLog("PongMaster" + std::string("Log.txt"));

    // World creator object //
    GameWorldFactory worldFactory;
    
    // Create program object //
    PongMasterServer app;

    std::unique_ptr<AppDef> ProgramDefinition(AppDef::GenerateAppdefine(
            "./EngineConf.conf", "./PongMaster.conf", "",
            &PongMasterServer::CheckGameConfigurationVariables, &PongMasterServer::CheckGameKeyConfigVariables));

    // Fail if no definition could be created //
    if(!ProgramDefinition){

        std::cout << "FATAL: failed to create AppDefine" << std::endl;
        return 2;
    }
    
    
    // customize values //
#ifdef _WIN32
    ProgramDefinition->SetHInstance(hInstance);
#endif
    ProgramDefinition->SetMasterServerParameters(MasterServerInformation(true, "Pong_" GAME_VERSIONS)).
        SetApplicationIdentification(
        "Pong master version " GAME_VERSIONS, "Pong",
        "GAME_VERSIONS");
    
    // Create window last //
    ProgramDefinition->StoreWindowDetails(PongMasterServer::GenerateWindowTitle(), true,
#ifdef _WIN32
        LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1)),
#endif
        &app);

    if(!app.PassCommandLine(argcount, args)){

        std::cout << "Error: Invalid Command Line arguments. Shutting down" << std::endl;
        return 3;
    }

    // Register our custom JS object if we are using one //
#ifdef LEVIATHAN_USES_CUSTOMJS
    Leviathan::GlobalCEFHandler::RegisterCustomJavaScriptQueryHandler(
        std::make_shared<>());
    
    // Register the custom file //
    Leviathan::GlobalCEFHandler::RegisterCustomExtension(
        std::make_shared<Leviathan::GUI::CustomExtension>(
            "", , ,
            ));
    
#endif //LEVIATHAN_USES_CUSTOMJS
        
    
    if(app.Initialize(ProgramDefinition.get())){

        // this is where the game should customize the engine //
        app.CustomizeEnginePostLoad();

        LOG_INFO("Engine successfully initialized");
        Return = app.RunMessageLoop();
    } else {
        LOG_ERROR("App init failed, closing");
        app.ForceRelease();
        Return = 5;
    }
#ifdef _WIN32
    }
    //_CrtDumpMemoryLeaks();
    CoUninitialize();
#endif

    // The CEF application will go out of scope after this //
    KeepThisSafe.reset();

    // This needs to be called before quitting //
    Leviathan::GlobalCEFHandler::CEFLastThingInProgram();

    // Sandbox will go out of scope after this

    return Return;
}
