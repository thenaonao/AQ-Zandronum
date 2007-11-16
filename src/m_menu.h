// Emacs style mode select	 -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// DESCRIPTION:
//	 Menu widget stuff, episode selection and such.
//	  
//-----------------------------------------------------------------------------


#ifndef __M_MENU_H__
#define __M_MENU_H__

#include "d_event.h"
#include "c_cvars.h"

//
// MENUS
//
// Called by main loop,
// saves config file and calls I_Quit when user exits.
// Even when the menu is not displayed,
// this can resize the view and change game parameters.
// Does all the real work of the menu interaction.
bool M_Responder (event_t *ev);

// Called by main loop,
// only used for menu (skull cursor) animation.
void M_Ticker (void);

// Called by main loop,
// draws the menus directly into the screen buffer.
void M_Drawer (void);

// Called by D_DoomMain, loads the config file.
void M_Init (void);

void M_Deinit ();

// Called by intro code to force menu up upon a keypress,
// does nothing if menu is already up.
void M_StartControlPanel (bool makeSound);

// Setup multiplayer menu
bool M_StartMultiplayerMenu (void);

// Turns off the menu
void M_ClearMenus ();

// [RH] Setup options menu
bool M_StartOptionsMenu (void);

//  Setup join team menu
bool M_StartJoinTeamMenu (void);

// [RC] Join, but no teams
bool M_StartJoinMenu (void);

// [RH] Handle keys for options menu
void M_OptResponder (event_t *ev);

// [RH] Draw options menu
void M_OptDrawer (void);

// [RH] Initialize options menu
void M_OptInit (void);

// [RH] Initialize the video modes menu
void M_InitVideoModesMenu (void);

struct menu_s;
void M_SwitchMenu (struct menu_s *menu);

void M_PopMenuStack (void);

// [RH] Called whenever the display mode changes
void M_RefreshModesList ();

void M_ActivateMenuInput ();
void M_DeactivateMenuInput ();

void M_NotifyNewSave (const char *file, const char *title, bool okForQuicksave);

// Menu flag definitions.
#define MNF_ALIGNLEFT			1	// Align menu to the left
#define MNF_CENTERED			2	// Menu text is centered

//
// MENU TYPEDEFS
//
typedef enum {
	whitetext,
	redtext,
	more,
	rightmore,
	safemore,
	rsafemore,
	slider,
	absslider,
	inverter,
	discrete,
	cdiscrete,
	discrete_guid,
	control,
	screenres,
	bitflag,
	bitmask,
	listelement,
	nochoice,
	numberedmore,
	colorpicker,
	intslider,
	palettegrid,
	string,
	pwstring,
	skintype,
	classtype,
	number,
	botslot,
	announcer,
	levelslot,
	browserheader,
	browserslot,
	weaponslot,
	txslider,
	mnnumber,

} itemtype;

struct GUIDName;

typedef struct menuitem_s {
	itemtype		  type;
	const char			 *label;
	union {
		FBaseCVar		 *cvar;
		FIntCVar		 *intcvar;
		FStringCVar		 *stringcvar;
		FGUIDCVar		 *guidcvar;
		FColorCVar		 *colorcvar;
		int				  selmode;
		float			  fval;
	} a;
	union {
		float			  min;		/* aka numvalues aka invflag */
		float			  numvalues;
		float			  invflag;
		int				  key1;
		char			 *res1;
		int				  position;
	} b;
	union {
		float			  max;
		int				  key2;
		char			 *res2;
		void			 *extra;
	} c;
	union {
		float			  step;
		char			 *res3;
	} d;
	union {
		struct value_s	 *values;
		GUIDName		 *guidvalues;
		char			 *command;
		void			(*cfunc)(FBaseCVar *cvar, float newval);
		void			(*mfunc)(void);
		void			(*lfunc)(int);
		int				  highlight;
		int				  flagmask;
	} e;
	union {

		// Server in the server browser list this slot indexes.
		LONG			lServer;

	} f;
} menuitem_t;

typedef struct menu_s {
	const char		   *texttitle;
	int				lastOn;
	int				numitems;
	int				indent;
	menuitem_t	   *items;
	int				scrolltop;
	int				scrollpos;
	int				y;
	void		  (*PreDraw)(void);
	bool			DontDim;
	void		  (*EscapeHandler)(void);

	// Special flags for this menu.
	int				iFlags;
} menu_t;

typedef struct value_s {
	float		value;
	const char	*name;
} value_t;

typedef struct
{
	// -1 = no cursor here, 1 = ok, 2 = arrows ok
	SBYTE		status;
	BYTE		fulltext;	// [RH] Menu name is text, not a graphic
	
	// hotkey in menu
	char		alphaKey;						
	
	const char	*name;
	
	// Use botskill settings for this episode?
	bool		bBotSkill;

	// Is the title of this botskill menu a text version?
	bool		bBotSkillFullText;

	// choice = menu item #.
	// if status = 2,
	//	 choice=0:leftarrow,1:rightarrow
	void		(*routine)(int choice);
	int			textcolor;
} oldmenuitem_t;

typedef struct oldmenu_s
{
	short				numitems;		// # of menu items
	oldmenuitem_t		*menuitems;		// menu items
	void				(*routine)(void);	// draw routine
	short				x;
	short				y;				// x,y of menu
	short				lastOn; 		// last item user was on in menu
} oldmenu_t;

typedef struct
{
	union {
		menu_t *newmenu;
		oldmenu_t *old;
	} menu;
	bool isNewStyle;
	bool drawSkull;
} menustack_t;

extern value_t YesNo[2];
extern value_t NoYes[2];
extern value_t OnOff[2];

extern menustack_t MenuStack[16];
extern int MenuStackDepth;

extern bool	OptionsActive;

extern menu_t  *CurrentMenu;
extern int		CurrentItem;

#define MAX_EPISODES	8

extern oldmenuitem_t EpisodeMenu[MAX_EPISODES];
extern bool EpisodeNoSkill[MAX_EPISODES];
extern char EpisodeMaps[MAX_EPISODES][8];
extern char EpisodeSkillHeaders[MAX_EPISODES][64];
extern oldmenu_t EpiDef;

#endif
