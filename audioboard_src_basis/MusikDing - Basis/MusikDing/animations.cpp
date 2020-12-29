/*
Animationen für Matrix
*/

#include <avr/io.h>
#include <string.h>

#include "animation.h"
#include "global.h"

/*
TODO: Animationen einfügen

Einfacher Weg:
	anim_frame() in animation.cpp bearbeiten, so dass dort alles berechnet wird

Schwerer Weg:
	Beliebieg viele Animationen hier einfügen. Je eine Funktion für Init und einer für die Berechnung jedes Frames.
	Anschliessend in animation.cpp in die anim_list eintragen und ANIM_LIST_NUM in animation.h ändern.
	Dieses vorgehen bietet die Möglichkeit viele Animationen zu erstellen und die über z.B. ein LCD auszuwählen. Ihr solltet wissen was ihr macht!
*/