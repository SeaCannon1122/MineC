#if defined(_WIN32)
#define DLL_EXPORT __declspec(dllexport)
#elif defined(__linux)
#define DLL_EXPORT __attribute__((visibility("default")))
#endif