#include <Windows.h>

#pragma comment(linker, "/MERGE:.data=.text")
#pragma comment(linker, "/MERGE:.rdata=.text")
#pragma comment(linker, "/SECTION:.text,EWR")

extern "C" {
    int _fltused = 1;

    __declspec(dllexport) int GetFSAPI(int a1, int a2, int a3, int a4) {
        HMODULE dll = LoadLibraryA("filesystem_stdio_hooked.dll");
        FARPROC proc = GetProcAddress(dll, "GetFSAPI");

        return ((int(*)(int, int, int, int))proc)(a1, a2, a3, a4);
    }

    __declspec(dllexport) void* CreateInterface(int a1, int a2) {
        HMODULE dll = LoadLibraryA("filesystem_stdio_hooked.dll");
        FARPROC proc = GetProcAddress(dll, "CreateInterface");

        return ((void*(*)(int, int))proc)(a1, a2);
    }
}

inline __forceinline void FindFiles(WIN32_FIND_DATAA* fd) {
    char dir[260];
    GetCurrentDirectoryA(260, dir);

    HANDLE asiFile = FindFirstFileA("*.asi", fd);
    if (asiFile != INVALID_HANDLE_VALUE) {
        do {
            if (!(fd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                auto pos = lstrlenA(fd->cFileName);

                if (fd->cFileName[pos - 4] == '.' && (fd->cFileName[pos - 3] == 'a' || fd->cFileName[pos - 3] == 'A') &&
                                                     (fd->cFileName[pos - 2] == 's' || fd->cFileName[pos - 2] == 'S') && 
                                                     (fd->cFileName[pos - 1] == 'i' || fd->cFileName[pos - 1] == 'I')) {
                    char path[260];
                    lstrcpyA(path, dir);
                    lstrcatA(path, "\\");
                    lstrcatA(path, fd->cFileName);

                    if (!GetModuleHandleA(path)) {
                        auto h = LoadLibraryA(path);
                        SetCurrentDirectoryA(dir);

                        if (!h) {
                            auto e = GetLastError();
                            if (e != ERROR_DLL_INIT_FAILED && e != ERROR_BAD_EXE_FORMAT) {
                                char msg[140];
                                lstrcpyA(msg, "Unable to load ");
                                lstrcatA(msg, fd->cFileName);
                                lstrcatA(msg, ". Last error: ");
                                char fmt[10];
                                wsprintfA(fmt, "%d", e);
                                lstrcatA(msg, fmt);
                                MessageBoxA(NULL, msg, "SunLoader", MB_ICONERROR);
                            }
                        } else {
                            auto procedure = (void(*)())GetProcAddress(h, "InitializeASI");

                            if (procedure != nullptr) {
                                procedure();
                            }
                        }
                    }
                }
            }
        } while (FindNextFileA(asiFile, fd));
        FindClose(asiFile);
    }
}

void Main() {
    WIN32_FIND_DATAA fd;
    FindFiles(&fd);
}

int __stdcall DllEntryPoint(_In_ void* _Handle, _In_ unsigned long _Reason, _In_opt_ void** _Unused) {
	if (_Reason == DLL_PROCESS_ATTACH) {
        CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(Main), NULL, NULL, NULL);
		return 1;
	 }

	return 0;
}