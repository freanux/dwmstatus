#include "statusbar.hpp"

#include <signal.h>
#include <unistd.h>
#include <sstream>

namespace {

    struct Scope {
        Scope(std::mutex& mtx) : mtx(mtx) { mtx.lock();   }
        ~Scope()                          { mtx.unlock(); }
        std::mutex& mtx;
    };

    std::mutex smtx;
    Statusbar *sb = 0;

    void signalhandler_terminate(int) {
        Scope lock(smtx);
        if (sb) {
            sb->cancel();
        }
    }

    void signalhandler_trigger(int signum) {
        Scope lock(smtx);
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
        Scope lock(smtx);
        sb = &s;
        modify_signalhandlers(sb->get_sections(), signalhandler_terminate, signalhandler_trigger);
    }

    void uninstall_signalhandlers() {
        Scope lock(smtx);
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
            ::Scope lock(mtx);
            ++counter;
            const Section *cs = sections;
            while (cs->fn) {
                if (cs->update && counter % cs->update == 0) {
                    update_no_lock(*cs);
                }
                ++cs;
            }
            if (counter >= max_update) {
                counter = 0;
            }
            draw_no_lock();
        }
    }
    ::uninstall_signalhandlers();
}

void Statusbar::cancel() {
    set_running(false);
}

void Statusbar::trigger(int signum) {
    ::Scope lock(mtx);
    const Section *cs = sections;
    while (cs->fn) {
        if (cs->signum == signum) {
            update_no_lock(*cs);
        }
        ++cs;
    }
    draw_no_lock();
}

const Section *Statusbar::get_sections() const {
    return sections;
}

/**** PRIVATE ****/

void Statusbar::set_running(bool v) {
   ::Scope lock(mtx);
    running = v;
}

bool Statusbar::is_running() {
    bool rv = false;
    {
        ::Scope lock(mtx);
        rv = running;
    }

    return rv;
}

void Statusbar::set_dirty(bool v) {
    ::Scope lock(mtx);
    set_dirty_no_lock(v);
}

bool Statusbar::is_dirty() {
    ::Scope lock(mtx);
    return is_dirty_no_lock();
}

void Statusbar::update(const Section& section) {
    ::Scope lock(mtx);
    update_no_lock(section);
}

void Statusbar::draw() {
    ::Scope lock(mtx);
    draw_no_lock();
}

/**** CRITICAL SECTION ****/

void Statusbar::set_dirty_no_lock(bool v) {
    dirty = v;
}

bool Statusbar::is_dirty_no_lock() {
    return dirty;
}

void Statusbar::update_no_lock(const Section& section) {
    elements[&section] = section.fn(section.args);
    set_dirty_no_lock(true);
}

void Statusbar::draw_no_lock() {
    if (is_dirty_no_lock()) {
        std::stringstream ss;
        for (const auto& v : elements) {
            ss << v.second;
        }
        XStoreName(dpy, root, ss.str().c_str());
        XFlush(dpy);
        set_dirty_no_lock(false);
    }
}
