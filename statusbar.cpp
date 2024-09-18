#include "statusbar.hpp"

#include <signal.h>
#include <unistd.h>
#include <sstream>

namespace {

    Statusbar *sb = 0;

    void signalhandler_terminate(int) {
        if (sb) {
            sb->cancel();
        }
    }

    void signalhandler_trigger(int signum) {
        if (sb) {
            sb->trigger(signum);
        }
    }

    void modify_signalhandlers(const Section *cs, void (*sh_term)(int), void (*sh_trg)(int)) {
        struct sigaction sa;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;

        sa.sa_handler = sh_term;
        sigaction(SIGTERM, &sa, 0);
        sigaction(SIGINT, &sa, 0);

        sa.sa_handler = sh_trg;
        while (cs->fn) {
            if (cs->signum) {
                sigaction(cs->signum, &sa, 0);
            }
            ++cs;
        }
    }

    void install_signalhandlers(Statusbar& s) {
        sb = &s;
        modify_signalhandlers(sb->get_sections(), signalhandler_terminate, signalhandler_trigger);
    }

    void uninstall_signalhandlers() {
        modify_signalhandlers(sb->get_sections(), SIG_DFL, SIG_DFL);
        sb = 0;
    }

}

/* --------------------------------------------------------------------------- */

Exception::Exception(const char *msg) : msg(msg) { }

const char *Exception::what() const throw () {
    return msg.c_str();
}

/* --------------------------------------------------------------------------- */

Statusbar::Statusbar(const Section *sections) :
    sections(sections), running(false), dirty(false), dpy(0), scr(0), root(0)
{
    dpy = XOpenDisplay(0);
    if (!dpy) {
        throw Exception("Failed to open display");
    }
    scr = DefaultScreen(dpy);
    root = RootWindow(dpy, scr);
}

Statusbar::~Statusbar() {
    XCloseDisplay(dpy);
}

void Statusbar::run() {
    unsigned int max_update = 0;

    set_running(true);
    const Section *cs = sections;
    while (cs->fn) {
        update(*cs);
        if (cs->update > max_update) {
            max_update = cs->update;
        }
        ++cs;
    }
    set_dirty(true);
    draw();

    ::install_signalhandlers(*this);
    unsigned int counter = 0;
    while (is_running()) {
        sleep(1);
        {
            ++counter;
            const Section *cs = sections;
            while (cs->fn) {
                if (cs->update && counter % cs->update == 0) {
                    update(*cs);
                }
                ++cs;
            }
            if (counter >= max_update) {
                counter = 0;
            }
            draw();
        }
    }
    ::uninstall_signalhandlers();
}

void Statusbar::cancel() {
    set_running(false);
}

void Statusbar::trigger(int signum) {
    const Section *cs = sections;
    while (cs->fn) {
        if (cs->signum == signum) {
            update(*cs);
        }
        ++cs;
    }
    draw();
}

const Section *Statusbar::get_sections() const {
    return sections;
}

/**** PRIVATE ****/

void Statusbar::set_running(bool v) {
    running = v;
}

bool Statusbar::is_running() {
    return running;
}

void Statusbar::set_dirty(bool v) {
    dirty = v;
}

bool Statusbar::is_dirty() {
    return dirty;
}

void Statusbar::update(const Section& section) {
    elements[&section] = section.fn(section.args);
    set_dirty(true);
}

void Statusbar::draw() {
    if (is_dirty()) {
        std::stringstream ss;
        for (const auto& v : elements) {
            ss << v.second;
        }
        XStoreName(dpy, root, ss.str().c_str());
        XFlush(dpy);
        set_dirty(false);
    }
}
