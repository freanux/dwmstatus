#ifndef _FUNCTIONS_HPP_
#define _FUNCTIONS_HPP_

#include <string>

std::string fn_execute(const char *args);
std::string fn_execute_home(const char *args);
std::string fn_datetime(const char *args);
std::string fn_kernel_release(const char *args);
std::string fn_print(const char *args);

#endif