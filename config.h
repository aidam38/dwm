/* See LICENSE file for copyright and license details. */

#include "push.c"
#include "moveresize.c"

/* appearance */
#include "dwmcolors.h"
static const unsigned int borderpx  = 4;        /* border pixel of windows */
static unsigned int gappx           = 20;        /* gap pixel between windows */
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

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* custom functions declarations */
void selectclient(const Arg *arg);
void incgap(const Arg *arg);
void setgapzero(const Arg *arg);

/* commands */
static const char *termcmd[]  = { "st", NULL };
static const char *scratchpad[]  = { "scratchpad", NULL };
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *dmenucmd[] = { "dmenu_run", "-m", dmenumon, "-fn", dmenufont, "-nb", normbgcolor, "-nf", normfgcolor, "-sb", selbgcolor, "-sf", selfgcolor, NULL };
static const char *dmenucmd_res[] = { "dmenu_run_res", "-m", dmenumon, "-fn", dmenufont, "-nb", normbgcolor, "-nf", normfgcolor, "-sb", selbgcolor, "-sf", selfgcolor, NULL }; 
static const char *qutecmd[] = { "qutebrowser", NULL };
static const char *newscmd[] = { "st", "newsboat", NULL };
static const char *muttcmd[] = { "st", "neomutt", NULL };
static const char *lfcmd[] = { "st", "lf", NULL };
static const char *hotcmd[] = { "hot.sh", NULL };
static const char *spotifycmd[] = { "spotify", NULL };
static const char *exitprompt[] = { "exitprompt", "-m", dmenumon, "-fn", dmenufont, "-nb", normbgcolor, "-nf", normfgcolor, "-sb", selbgcolor, "-sf", selfgcolor, NULL }; 
static const char *volumeup[] = { "pamixer", "-i", "5", NULL };
static const char *volumedown[] = { "pamixer", "-d", "5", NULL };
static const char *volumeUP[] = { "pamixer", "-i", "25", NULL };
static const char *volumeDOWN[] = { "pamixer", "-d", "25", NULL };
static const char *volumemute[] = { "pamixer", "-t", NULL };
static const char *printscreen[] = { "screenshot", NULL};
static const char *sptrestart[] = {"sptrestart", NULL};
static const char *sptshow[] = {"killall", "-SIGUSR1", "spt", NULL};
static const char *sptpause[] = {"killall", "-SIGUSR2", "spt", NULL};

static Key keys[] = {
	/* modifier                     key        function        argument */
/* spawning aplications */
	{ MODKEY,                       XK_Return, spawn,          {.v = termcmd } },
	{ MODKEY|ShiftMask,             XK_Return, spawn,          {.v = scratchpad } },
	{ MODKEY|ShiftMask,             XK_d,      spawn,          {.v = dmenucmd } },
	{ MODKEY,                       XK_d,      spawn,          {.v = dmenucmd_res } },
	{ MODKEY,                       XK_q,      spawn,          {.v = qutecmd } },
	{ MODKEY,                       XK_w,      spawn,          {.v = newscmd } },
	{ MODKEY,                       XK_e,      spawn,          {.v = muttcmd } },
	{ MODKEY,                       XK_r,      spawn,          {.v = lfcmd } },
	{ MODKEY,                       XK_a,      spawn,          {.v = hotcmd } },
	{ MODKEY,                       XK_s,      spawn,          {.v = spotifycmd } },
	{ MODKEY|ShiftMask,             XK_q,      spawn,          {.v = exitprompt } },
	{ MODKEY,                       XK_s,      spawn,          {.v = volumeup } },
	{ MODKEY,                       XK_x,      spawn,          {.v = volumedown } },
	{ MODKEY|ShiftMask,             XK_s,      spawn,          {.v = volumeUP } },
	{ MODKEY|ShiftMask,             XK_x,      spawn,          {.v = volumeDOWN } },
	{ MODKEY,                       XK_y,      spawn,          {.v = volumemute } },
	{ MODKEY,                       XK_Print,  spawn,          {.v = printscreen } },
	{ MODKEY,                       XK_Home,   spawn,          {.v = sptrestart } },
	{ MODKEY,                       XK_End,    spawn,          {.v = sptshow } },
	{ MODKEY,                       XK_Pause,  spawn,          {.v = sptpause } },
	{ MODKEY,                       XK_w,      killclient,     {0} },
/* basic movement */
	{ MODKEY,                       XK_j,      selectclient,   {.i = 0} },
	{ MODKEY,                       XK_k,      selectclient,   {.i = 1} },
	{ MODKEY,                       XK_l,      selectclient,   {.i = 2} },
	{ MODKEY,                       XK_uring,  selectclient,   {.i = 3} },
	{ MODKEY|ControlMask|ShiftMask, XK_j,      focusstack,     {.i = +1} },
	{ MODKEY|ControlMask|ShiftMask, XK_k,      focusstack,     {.i = -1} },
/* setting layouts */
	{ MODKEY,                       XK_t,      setlayout,      {.v = &layouts[0]} },
	{ MODKEY|ShiftMask,             XK_f,      setlayout,      {.v = &layouts[1]} },
	{ MODKEY,                       XK_m,      setlayout,      {.v = &layouts[2]} },
/* moving windows */
        { MODKEY|ControlMask,           XK_j,      pushdown,       {0} },
        { MODKEY|ControlMask,           XK_k,      pushup,         {0} },
	{ MODKEY|ControlMask,           XK_h,      incnmaster,     {.i = +1 } },
	{ MODKEY|ControlMask,           XK_l,      incnmaster,     {.i = -1 } },
	{ MODKEY,                       XK_space,  zoom,           {0} },
	{ MODKEY,                       XK_f,      togglefloating, {0} },
//	{ MODKEY|ShiftMask,             XK_0,      tag,            {.ui = ~0 } },
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
/* moving floating windows */
	{ MODKEY,                   XK_Down,    moveresize,     {.v = (int []){ 0, 25, 0, 0 }}},
	{ MODKEY,                   XK_Up,      moveresize,     {.v = (int []){ 0, -25, 0, 0 }}},
	{ MODKEY,                   XK_Right,   moveresize,     {.v = (int []){ 25, 0, 0, 0 }}},
	{ MODKEY,                   XK_Left,    moveresize,     {.v = (int []){ -25, 0, 0, 0 }}},
	{ MODKEY|ShiftMask,         XK_Down,    moveresize,     {.v = (int []){ 0, 0, 0, 25 }}},
	{ MODKEY|ShiftMask,         XK_Up,      moveresize,     {.v = (int []){ 0, 0, 0, -25 }}},
	{ MODKEY|ShiftMask,         XK_Right,   moveresize,     {.v = (int []){ 0, 0, 25, 0 }}},
	{ MODKEY|ShiftMask,         XK_Left,    moveresize,     {.v = (int []){ 0, 0, -25, 0 }}},
/* miscleaneous */
	{ MODKEY,                       XK_b,      togglebar,      {0} },
	{ MODKEY|ShiftMask,     	XK_e,      quit,           {0} },
/* multiple monitor shit I don't use */
	{ MODKEY,                       XK_comma,  focusmon,       {.i = -1 } },
	{ MODKEY,                       XK_period, focusmon,       {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_comma,  tagmon,         {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_period, tagmon,         {.i = +1 } },
};

/* button definitions */
/* click can be ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
	{ ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
	{ ClkWinTitle,          0,              Button2,        zoom,           {0} },
	{ ClkStatusText,        0,              Button2,        spawn,          {.v = termcmd } },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};

/* select a specific client by count on a tag */
void
selectclient(const Arg *arg)
{
	int i;
	Client *c;
	for(i = 0, c = nexttiled(selmon->clients); c && i < arg->i; c = nexttiled(c->next), i++);
	focus(c);
	restack(selmon);
}

/* increase the gap between windwos */
void incgap(const Arg *arg)
{
gappx = MAX(gappx + arg->i, 0);
if (gappx <= 0)
	gappx = 0;
arrange(selmon);
}

/* set the gap between windows to zeor */
void setgapzero(const Arg *arg)
{
gappx = 0;
arrange(selmon);
}
