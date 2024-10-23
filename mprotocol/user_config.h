#ifndef USER_CONFIG_H
#define USER_CONFIG_H

// Define a flag to indicate whether the protocol is on host or user side.
#define IS_HOST

#ifdef IS_HOST
// Define the platform
#if defined(_WIN32) || defined(_WIN64)
    #define PLATFORM_WINDOWS
    #define MAX_FIELDS_COUNT 100

#elif defined(__linux__)
    #define PLATFORM_LINUX
    #define MAX_FIELDS_COUNT 100

#elif defined(__APPLE__) && defined(__MACH__)
    #define PLATFORM_MAC
    //#define MAX_FIELDS_COUNT 100

#elif defined(STM32F4xx) || defined(STM32F7xx) || defined(STM32H7xx)
    #define PLATFORM_STM32
    #define MAX_FIELDS_COUNT 50

#else
    #error "Unknown platform"
#endif

#endif // IS_HOST

#endif // USER_CONFIG_H