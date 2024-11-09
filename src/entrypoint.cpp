#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

#include "constants.h"
#include "hook_typedefs.h"
#include "bifrost.h"
#include <iostream>
#include <spdlog/spdlog.h>

std::shared_ptr<bifrost> bifrost_instance;

void setup_logger()
{
    auto stderr_sink = std::make_shared<spdlog::sinks::ansicolor_stderr_sink_mt>();
    auto logger = std::make_shared<spdlog::logger>("stderr_logger", stderr_sink);
    logger->set_pattern("<bifrost> [%H:%M:%S] [%^%l%$] %v");
    set_default_logger(logger);
    if (std::getenv(ENV_DEBUG)) {
        spdlog::set_level(spdlog::level::debug);
    }
}

__attribute__((constructor)) void library_entrypoint()
{
    setup_logger();

    spdlog::info("Bifrost loading...");
    bifrost_instance = std::make_shared<bifrost>();
}

extern "C" {
typedef int (*orig_libc_start_main_type)(int (*main)(int, char**, char**),
    int argc, char** argv,
    void (*init)(void),
    void (*fini)(void),
    void (*rtld_fini)(void),
    void(*stack_end));

int __libc_start_main(int (*main)(int, char**, char**),
    int argc, char** argv,
    void (*init)(void),
    void (*fini)(void),
    void (*rtld_fini)(void),
    void(*stack_end))
{
    const auto orig_libc_start_main = reinterpret_cast<orig_libc_start_main_type>(dlsym(RTLD_NEXT, "__libc_start_main"));
    setenv("LD_PRELOAD", "", 1);
    return orig_libc_start_main(main, argc, argv, init, fini, rtld_fini, stack_end);
}
}
