#include "functions.hpp"

#include <unistd.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <pwd.h>
#include <cstdio>
#include <ctime>
#include <cstring>

std::string fn_execute(const char *args) {
    std::string rv;
    char buf[256];
    buf[sizeof(buf) - 1] = 0;
    FILE *f = popen(args, "r");
    if (f) {
        std::fgets(buf, sizeof(buf) - 1, f);
        pclose(f);
        std::size_t len = std::strlen(buf);
        if (len) {
            buf[len - 1] = 0;
        }
        rv = buf;
    }
    return rv;
}

std::string fn_execute_home(const char *args) {
    passwd *pw = getpwuid(getuid());
    std::string filename = pw->pw_dir;
    filename += "/";
    filename += args;
    return fn_execute(filename.c_str());
}

std::string fn_datetime(const char *args) {
    std::time_t t = std::time(0);
    char buf[256];
    buf[sizeof(buf) - 1] = 0;
    std::strftime(buf, sizeof(buf) - 1, args, std::localtime(&t));
    return buf;
}

std::string fn_kernel_release(const char *args) {
    utsname ud;
    uname(&ud);
    return ud.release;
}

std::string fn_print(const char *args) {
    char buf[256];
    buf[sizeof(buf) - 1] = 0;
    std::snprintf(buf, sizeof(buf) - 1, "%s", args);
    return buf;
}
