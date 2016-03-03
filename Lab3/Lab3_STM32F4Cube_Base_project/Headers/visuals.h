#ifndef VISUALS_H
#define VISUALS_H

void Visuals_Init(void);
void Visuals_ToggleLEDs(void);

void Visuals_SetDirection(int dir);
void Visuals_TurnOn(void);
void Visuals_TurnOff(void);

#define COUNTERCLOCKWISE	0
#define CLOCKWISE 				1

#endif
