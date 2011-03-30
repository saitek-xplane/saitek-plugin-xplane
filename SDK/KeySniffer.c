/*
 * KeySniffer.c
 *
 * KeySniffer shows the use of key sniffers to intercept and process raw
 * keystrokes.  This one creates a window where all data about the keystroke
 * is displayed.
 *
 * Key strokes have two sets of character data.  The ASCII key code is a valid
 * ASCII value.  This value discriminates between the A key with and without shift
 * (e.g. 'A' and 'a') but does not discriminate between numbers on the main
 * keyboard vs. numeric keypad.  Virtual key codes tell exactly what physical key
 * was pressed (e.g. top-row-0 vs. num-pad-0) but do not change by modifier keys.
 * Modifier keys are returned separately.
 *
 * ASCII codes are good for handling text entry; virtual key codes are good for
 * setting up key commands (since they allow for separate binding of the numeric
 * key pad).
 *
 */
#include <stdio.h>
#include <string.h>

#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "XPLMUtilities.h"


/* We will store the information we got from our last key press globally
 * so the window can show the most recent key press. */
XPLMWindowID	gWindow = NULL;
XPLMKeyFlags	gFlags = 0;
char			gVirtualKey = 0;
char			gChar = 0;

void MyDrawWindowCallback(
                                   XPLMWindowID         inWindowID,
                                   void *               inRefcon);

void MyHandleKeyCallback(
                                   XPLMWindowID         inWindowID,
                                   char                 inKey,
                                   XPLMKeyFlags         inFlags,
                                   char                 inVirtualKey,
                                   void *               inRefcon,
                                   int                  losingFocus);

int MyHandleMouseClickCallback(
                                   XPLMWindowID         inWindowID,
                                   int                  x,
                                   int                  y,
                                   XPLMMouseStatus      inMouse,
                                   void *               inRefcon);

int MyKeySniffer(
                                   char                 inChar,
                                   XPLMKeyFlags         inFlags,
                                   char                 inVirtualKey,
                                   void *               inRefcon);



PLUGIN_API int XPluginStart(
						char *		outName,
						char *		outSig,
						char *		outDesc)
{
	/* First set up our plugin info. */
	strcpy(outName, "KeySniffer");
	strcpy(outSig, "xplanesdk.examples.keysniffer");
	strcpy(outDesc, "A plugin that makes a window.");

	/* Now create a new window.  Pass in our three callbacks. */
	gWindow = XPLMCreateWindow(
					50, 750, 350, 700,			/* Area of the window. */
					1,							/* Start visible. */
					MyDrawWindowCallback,		/* Callbacks */
					MyHandleKeyCallback,
					MyHandleMouseClickCallback,
					NULL);						/* Refcon - not used. */
					
	/* Finally register our key sniffer. */
	XPLMRegisterKeySniffer(
					MyKeySniffer, 				/* Our callback. */
					1, 							/* Receive input before plugin windows. */
					0);							/* Refcon - not used. */
	
	return 1;
}

PLUGIN_API void	XPluginStop(void)
{
	XPLMDestroyWindow(gWindow);
}

PLUGIN_API void XPluginDisable(void)
{
}

PLUGIN_API int XPluginEnable(void)
{
	return 1;
}

PLUGIN_API void XPluginReceiveMessage(
					XPLMPluginID	inFromWho,
					long			inMessage,
					void *			inParam)
{
}

/*
 * MyDrawWindowCallback
 *
 * This routine draws the window, showing the last keyboard stroke to be
 * recorded by our sniffer.
 *
 */
void MyDrawWindowCallback(
                                   XPLMWindowID         inWindowID,
                                   void *               inRefcon)
{
		char	str[50];
		int		left, top, right, bottom;
		float	color[] = { 1.0, 1.0, 1.0 };

	/* First get our window's location. */
	XPLMGetWindowGeometry(inWindowID, &left, &top, &right, &bottom);

	/* Draw a translucent dark box as our window outline. */
	XPLMDrawTranslucentDarkBox(left, top, right, bottom);

	/* Take the last key stroke and form a descriptive string.
	 * Note that ASCII values may be printed directly.  Virtual key
	 * codes are not ASCII and cannot be, but the utility function
	 * XPLMGetVirtualKeyDescription provides a human-readable string
	 * for each key.  These strings may be multicharacter, e.g. 'ENTER'
	 * or 'NUMPAD-0'. */
	sprintf(str,"%d '%c' | %d '%s' (%c %c %c %c %c)",
		gChar,
		(gChar) ? gChar : '0',
		(long) (unsigned char) gVirtualKey,
		XPLMGetVirtualKeyDescription(gVirtualKey),
		(gFlags & xplm_ShiftFlag) ? 'S' : ' ',
		(gFlags & xplm_OptionAltFlag) ? 'A' : ' ',
		(gFlags & xplm_ControlFlag) ? 'C' : ' ',
		(gFlags & xplm_DownFlag) ? 'D' : ' ',
		(gFlags & xplm_UpFlag) ? 'U' : ' ');

	/* Draw the string into the window. */
	XPLMDrawString(color, left + 5, top - 20, str, NULL, xplmFont_Basic);
}

void MyHandleKeyCallback(
                                   XPLMWindowID         inWindowID,
                                   char                 inKey,
                                   XPLMKeyFlags         inFlags,
                                   char                 inVirtualKey,
                                   void *               inRefcon,
                                   int                  losingFocus)
{
}

int MyHandleMouseClickCallback(
                                   XPLMWindowID         inWindowID,
                                   int                  x,
                                   int                  y,
                                   XPLMMouseStatus      inMouse,
                                   void *               inRefcon)
{
	return 0;
}

/*
 * MyKeySniffer
 *
 * This routnine receives keystrokes from the simulator as they are pressed.
 * A separate message is received for each key press and release as well as
 * keys being held down.
 *
 */
int MyKeySniffer(
                                   char                 inChar,
                                   XPLMKeyFlags         inFlags,
                                   char                 inVirtualKey,
                                   void *               inRefcon)
{
	/* First record the key data. */
	gVirtualKey = inVirtualKey;
	gFlags = inFlags;
	gChar = inChar;

	/* Return 1 to pass the keystroke to plugin windows and X-Plane.
	 * Returning 0 would consume the keystroke. */
	return 1;
}
