/* See LICENSE file for copyright and license details. */

#include "push.c"

/* appearance */
#include "dwmcolors.h"
static const unsigned int borderpx  = 2;        /* border pixel of windows */
static unsigned int gappx           = 15;        /* gap pixel between windows */
static const unsigned int snap      = 32;       /* snap pixel */
static const int showbar            = 1;        /* 0 means no bar */
static const char *colors[][3]      = {
	/*               fg         bg         border   */
	[SchemeNorm] = { normfgcolor, normbgcolor, normbordercolor },
	[SchemeSel]  = { selfgcolor, selbgcolor,  selbordercolor },
};

/* tagging */
static const char *tags[] = { "1", "2", "3", "4", "5", "♫" };

static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class      instance    title       tags mask     isfloating   monitor */
	{ "Gimp",     NULL,       NULL,       0,            1,           0,         0,        -1 },
	{ "Firefox",  NULL,       NULL,       1 << 8,       0,           0,         0,        -1 },
	{ "st",       NULL,       NULL,       0,            0,           1,         1,        -1 },
};

/* layout(s) */
static const float mfact     = 0.5; /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 0;    /* 1 means respect size hints in tiled resizals */

static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "T ",      tile },    /* first entry is default */
	{ "F ",      NULL },    /* no layout function means floating behavior */
	{ "M ",      monocle },
};

/* key definitions */
#define MODKEY Mod1Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      tag,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tagfollow,            {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

static char dmenumon[2] = "0"; /* component of dmenu, manipulated in spawn() */

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* custom functions declarations */
void selectclient(const Arg *arg);
void incgap(const Arg *arg);
void setgapzero(const Arg *arg);
void moveresize(const Arg *arg);
void runorraise(const Arg *arg);

enum { WIN_NW, WIN_N, WIN_NE, WIN_W, WIN_C, WIN_E, WIN_SW, WIN_S, WIN_SE }; /* coordinates for moveplace */
void moveplace(const Arg *arg);

/* commands */
static const char *term[]  = { "st", NULL };
static const char *scratchpad[]  = { "scratchpad", NULL };
static const char *qute[] = { "qutebrowser", NULL, NULL, NULL, NULL, "qutebrowser" };
static const char *news[] = { "st", "-c", "newsboat", "newsboat", NULL, "newsboat" };
static const char *mutt[] = { "st", "-c", "neomutt", "neomutt", NULL, "neomutt" };
static const char *lf[] = { "st", "lf", NULL};
static const char *hot[] = { "hot.sh", NULL };
static const char *spotify[] = { "spotify", NULL, NULL, NULL, NULL, "Spotify" };
static const char *dmenucmd[] = { "dmenu_run", "-m", dmenumon, "-fn", dmenufont, "-nb", normbgcolor, "-nf", normfgcolor, "-sb", selbgcolor, "-sf", selfgcolor, NULL };
static const char *dmenu_res[] = { "dmenu_run_res", "-m", dmenumon, "-fn", dmenufont, "-nb", normbgcolor, "-nf", normfgcolor, "-sb", selbgcolor, "-sf", selfgcolor, NULL }; 
static const char *fzf[] = { "st", "-e", "fzf.sh", NULL };
static const char *exitprompt[] = { "exitprompt", "-m", dmenumon, "-fn", dmenufont, "-nb", normbgcolor, "-nf", normfgcolor, "-sb", selbgcolor, "-sf", selfgcolor, NULL }; 
static const char *volumeup[] = { "pamixer", "-i", "5", NULL };
static const char *volumedown[] = { "pamixer", "-d", "5", NULL };
static const char *volumeUP[] = { "pamixer", "-i", "25", NULL };
static const char *volumeDOWN[] = { "pamixer", "-d", "25", NULL };
static const char *volumemute[] = { "pamixer", "-t", NULL };
static const char *caststart[] = { "st", "-e", "screencast", NULL };
static const char *caststop[] = { "killall", "screencast", NULL };
static const char *printscreen[] = { "screenshot", NULL};
static const char *sptrestart[] = {"sptrestart", NULL};
static const char *sptshow[] = {"killall", "-SIGUSR1", "spt", NULL};
static const char *sptpause[] = {"killall", "-SIGUSR2", "spt", NULL};

static Key keys[] = {
	/* modifier                     key        function        argument */
/* spawning applications */
	{ MODKEY,                       XK_Return, spawn,          {.v = term } },
	{ MODKEY|ShiftMask,             XK_Return, spawn,          {.v = scratchpad } },
	{ MODKEY,                       XK_q,      runorraise,     {.v = qute } },
	{ MODKEY,                       XK_e,      runorraise,     {.v = mutt } },
	{ MODKEY,                       XK_r,      spawn,          {.v = lf } },
	{ MODKEY,                       XK_a,      spawn,          {.v = hot } },
	{ MODKEY,                       XK_s,      runorraise,     {.v = spotify } },
	{ MODKEY|ShiftMask,             XK_d,      spawn,          {.v = dmenucmd } },
	{ MODKEY,                       XK_d,      spawn,          {.v = dmenu_res } },
	{ MODKEY,                       XK_f,      spawn,          {.v = fzf } },
	{ MODKEY,                       XK_c,      runorraise,     {.v = news } },
/* function keys bindings (volume control, screencasting) */
	{ 0,                            XK_F3,     spawn,          {.v = volumeup } },
	{ 0,                            XK_F2,     spawn,          {.v = volumedown } },
	{ MODKEY,                       XK_F3,     spawn,          {.v = volumeUP } },
	{ MODKEY,                       XK_F2,     spawn,          {.v = volumeDOWN } },
	{ 0,                            XK_F1,     spawn,          {.v = volumemute } },
	{ 0,                            XK_F9,     spawn,          {.v = caststart } },
	{ 0,                            XK_F10,    spawn,          {.v = caststop } },
/* application control */
	{ 0,                            XK_Print,  spawn,          {.v = printscreen } },
	{ 0,                            XK_Home,   spawn,          {.v = sptrestart } },
	{ 0,                            XK_End,    spawn,          {.v = sptshow } },
	{ 0,                            XK_Pause,  spawn,          {.v = sptpause } },
/* basic movement */
	{ MODKEY,                       XK_j,      selectclient,   {.i = 0} },
	{ MODKEY,                       XK_m,      selectclient,   {.i = 1} },
	{ MODKEY|ShiftMask,             XK_m,      selectclient,   {.i = 2} },
	{ MODKEY,                       XK_k,      selectclient,   {.i = 10} },
	{ MODKEY,                       XK_comma,  selectclient,   {.i = 11} },
	{ MODKEY|ShiftMask,             XK_comma,  selectclient,   {.i = 12} },
	{ MODKEY|ControlMask|ShiftMask, XK_j,      focusstack,     {.i = +1} },
	{ MODKEY|ControlMask|ShiftMask, XK_k,      focusstack,     {.i = -1} },
/* setting layouts */
	{ MODKEY,                       XK_t,      setlayout,      {.v = &layouts[0]} },
	{ MODKEY,                       XK_z,      togglefloating, {0} },
	{ MODKEY|ShiftMask,             XK_z,      setlayout,      {.v = &layouts[1]} },
	{ MODKEY,                       XK_u,      setlayout,      {.v = &layouts[2]} },
/* moving windows */
        { MODKEY|ControlMask,           XK_j,      pushdown,       {0} },
        { MODKEY|ControlMask,           XK_k,      pushup,         {0} },
	{ MODKEY|ControlMask,           XK_h,      incnmaster,     {.i = +1 } },
	{ MODKEY|ControlMask,           XK_l,      incnmaster,     {.i = -1 } },
	{ MODKEY,                       XK_space,  zoom,           {0} },
/* resizing windows */
	{ MODKEY|ShiftMask,             XK_h,      setmfact,       {.f = -0.05} },
	{ MODKEY|ShiftMask,             XK_l,      setmfact,       {.f = +0.05} },
	{ MODKEY|ShiftMask,             XK_j,      setcfact,       {.f = +0.25} },
	{ MODKEY|ShiftMask,             XK_k,      setcfact,       {.f = -0.25} },
	{ MODKEY|ShiftMask,             XK_o,      setcfact,       {.f =  0.00} },
/* controling the gap size */
	{ MODKEY,                       XK_i,      incgap,         {.i = +2 } },
	{ MODKEY,                       XK_o,      incgap,         {.i = -2 } },
	{ MODKEY,                       XK_p,      setgapzero,     {0}        },
/* navigating across tags */
	{ MODKEY,                       XK_Tab,    view,           {0} },
	TAGKEYS(                        XK_semicolon,                   5)
	TAGKEYS(                        XK_plus,                        0)
	TAGKEYS(                        XK_ecaron,                      1)
	TAGKEYS(                        XK_scaron,                      2)
	TAGKEYS(                        XK_ccaron,                      3)
	TAGKEYS(                        XK_rcaron,                      4)
	TAGKEYS(                        XK_zcaron,                      6)
	TAGKEYS(                        XK_yacute,                      7)
	TAGKEYS(                        XK_aacute,                      8)
	TAGKEYS(                        XK_iacute,                      9)
	{ MODKEY|ShiftMask,             XK_0,      tag,            {.ui = ~0 } },
/* moving floating windows */
	{ MODKEY,                       XK_Down,    moveresize,     {.v = (int []){ 0, 100, 0, 0 }}},
	{ MODKEY,                       XK_Up,      moveresize,     {.v = (int []){ 0, -100, 0, 0 }}},
	{ MODKEY,                       XK_Right,   moveresize,     {.v = (int []){ 100, 0, 0, 0 }}},
	{ MODKEY,                       XK_Left,    moveresize,     {.v = (int []){ -100, 0, 0, 0 }}},
	{ MODKEY|ShiftMask,             XK_Down,    moveresize,     {.v = (int []){ 0, 0, 0, 100 }}},
	{ MODKEY|ShiftMask,             XK_Up,      moveresize,     {.v = (int []){ 0, 0, 0, -100 }}},
	{ MODKEY|ShiftMask,             XK_Right,   moveresize,     {.v = (int []){ 0, 0, 100, 0 }}},
	{ MODKEY|ShiftMask,             XK_Left,    moveresize,     {.v = (int []){ 0, 0, -100, 0 }}},
/* miscellaneous */
	{ MODKEY|ShiftMask,	        XK_x,      quit,           {0} },
	{ MODKEY|ShiftMask,             XK_y,      spawn,          {.v = exitprompt } },
	{ MODKEY,                       XK_w,      killclient,     {0} },
	{ MODKEY,                       XK_b,      togglebar,      {0} },
/* multiple monitor shit I don't use */
	{ MODKEY,                       XK_parenleft,  focusmon,       {.i = -1 } },
	{ MODKEY,                       XK_parenright, focusmon,       {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_parenleft,  tagmon,         {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_parenright, tagmon,         {.i = +1 } },
};

/* button definitions */
/* click can be ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
	{ ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
	{ ClkWinTitle,          0,              Button2,        zoom,           {0} },
	{ ClkStatusText,        0,              Button2,        spawn,          {.v = term } },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};

/* select a specific client by count on a tag */
void selectclient(const Arg *arg)
{
	// input in arg->i should be < 10 for clients in the master and >= 10 for stack are, e.g. 1 -- second client in master, whereas 10 -- first client in stack
	int i;
	Client *c;
	if (arg->i < 10) {
		for(i = 0, c = nexttiled(selmon->clients); c && i < arg->i; c = nexttiled(c->next), i++);
		if (i != arg->i || i > selmon->nmaster-1) return;
	} else {
		for(i = 0, c = nexttiled(selmon->clients); c && i < arg->i-10+selmon->nmaster; c = nexttiled(c->next), i++);
		if (i != arg->i-10+selmon->nmaster) return;
	}
	focus(c);
	restack(selmon);
}

/* increase the gap between windows (by ME) */
void incgap(const Arg *arg)
{
	gappx = MAX(gappx + arg->i, 0);
	if (gappx <= 0)
		gappx = 0;
	arrange(selmon);
}

/* set the gap between windows to zero (by ME) */
void setgapzero(const Arg *arg)
{
	gappx = 0;
	arrange(selmon);
}

/* move floating clients using keyboard (patch moveresize) */
void moveresize(const Arg *arg)
{
    XEvent ev;
    Monitor *m = selmon;

    if(!(m->sel && arg && arg->v && m->sel->isfloating))
        return;

    resize(m->sel, m->sel->x + ((int *)arg->v)[0],
        m->sel->y + ((int *)arg->v)[1],
        m->sel->w + ((int *)arg->v)[2],
        m->sel->h + ((int *)arg->v)[3],
        True);

    while(XCheckMaskEvent(dpy, EnterWindowMask, &ev));
}

/* move floating clients to one thirds of screen (patch moveplace) */
void moveplace(const Arg *arg)
{
	Client *c;
	int nh, nw, nx, ny;
	c = selmon->sel;
	if (!c || (arg->ui >= 9))
		 return;
	if (selmon->lt[selmon->sellt]->arrange && !c->isfloating)
		togglefloating(NULL);
	nh = (selmon->wh / 3) - (c->bw * 2);
	nw = (selmon->ww / 3) - (c->bw * 2);
	nx = (arg->ui % 3) -1;
	ny = (arg->ui / 3) -1;
	if (nx < 0)
		nx = selmon->wx;
	else if(nx > 0)
		nx = selmon->wx + selmon->ww - nw - c->bw*2;
	else
		nx = selmon->wx + selmon->ww/2 - nw/2 - c->bw;
	if (ny <0)
		ny = selmon->wy;
	else if(ny > 0)
		ny = selmon->wy + selmon->wh - nh - c->bw*2;
	else
		ny = selmon->wy + selmon->wh/2 - nh/2 - c->bw;
	resize(c, nx, ny, nw, nh, True);
	XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, nw/2, nh/2);
}

/* run or raise windows (patch runorraise) */
void runorraise(const Arg *arg) {
    char *app = ((char **)arg->v)[5];
    Arg a = { .ui = ~0 };
    Monitor *mon;
    Client *c;
    XClassHint hint = { NULL, NULL };
    /* Tries to find the client */
    for (mon = mons; mon; mon = mon->next) {
        for (c = mon->clients; c; c = c->next) {
            XGetClassHint(dpy, c->win, &hint);
            if (hint.res_class && strcmp(app, hint.res_class) == 0) {
                a.ui = c->tags;
                view(&a);
                focus(c);
                XRaiseWindow(dpy, c->win);
                return;
            }
        }
    }
    /* Client not found: spawn it */
    spawn(arg);
}
