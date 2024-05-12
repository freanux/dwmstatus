#include "functions.hpp"
#include "statusbar.hpp"

#include <signal.h>
#include <iostream>

namespace {

    const Section sections[] = {
        { fn_execute_home,      "scripts/dwm/microstatus.sh",                       0, SIGUSR2 },
        { fn_print,             "^f2^^c#444444^^r0,0,2,38^^f2^^d^^f2^ ",            0, 0       },
        { fn_execute_home,      "scripts/dwm/audiostatus.sh",                       0, SIGUSR1 },
        { fn_print,             " ^f2^^c#444444^^r0,0,2,38^^f2^^d^ ",               0, 0       },
        { fn_datetime,          "%a, %d %b %Y ^f2^^c#444444^^r0,0,2,38^^f2^^d^ %T", 1, 0       },
        { fn_print,             " ^f2^^c#444444^^r0,0,2,38^^f2^^d^ ",               0, 0       },
        { fn_kernel_release,    0,                                                  0, 0       },
        { fn_print,             "^f12^",                                            0, 0       },
        { 0, 0, 0, 0 }
    };

}

/* --------------------------------------------------------------------------- */

int main(int argc, const char *argv[]) {
    try {
        Statusbar sb(::sections);
        sb.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

	return 0;
}
