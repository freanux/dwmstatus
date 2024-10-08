#ifndef _STATUSBAR_HPP_
#define _STATUSBAR_HPP_

#include <X11/Xlib.h>
#include <string>
#include <map>
#include <exception>

typedef std::string (*function)(const char *args);

struct Section {
    function fn;
    const char *args;
    unsigned int update;
    int signum;
};

/* --------------------------------------------------------------------------- */

class Exception : public std::exception {
public:
    Exception(const char *msg);
    const char *what() const throw ();

private:
    std::string msg;
};

/* --------------------------------------------------------------------------- */

class Statusbar {
private:
    Statusbar(const Statusbar&);
    const Statusbar& operator=(const Statusbar&);

public:
    Statusbar(const Section *sections);
    ~Statusbar();

    void run();
    void cancel();
    void trigger(int signum);
    const Section *get_sections() const;

private:
    typedef std::map<const Section *, std::string> Elements;

    const Section *sections;
    bool running;
    bool dirty;
    Display *dpy;
    int scr;
    Window root;

    Elements elements;

    void set_running(bool v);
    bool is_running();
    void set_dirty(bool v);
    bool is_dirty();
    void update(const Section& section);
    void draw();
};

#endif
