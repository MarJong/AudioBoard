/*
Animationen f�r Matrix
*/

#include <avr/io.h>
#include <string.h>

#include "animation.h"
#include "global.h"

/*
TODO: Animationen einf�gen

Einfacher Weg:
	anim_frame() in animation.cpp bearbeiten, so dass dort alles berechnet wird

Schwerer Weg:
	Beliebieg viele Animationen hier einf�gen. Je eine Funktion f�r Init und einer f�r die Berechnung jedes Frames.
	Anschliessend in animation.cpp in die anim_list eintragen und ANIM_LIST_NUM in animation.h �ndern.
	Dieses vorgehen bietet die M�glichkeit viele Animationen zu erstellen und die �ber z.B. ein LCD auszuw�hlen. Ihr solltet wissen was ihr macht!
*/