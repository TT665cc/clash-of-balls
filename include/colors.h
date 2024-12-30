#ifndef COLORS_H

#define COLORS_H

#define NB_COLORS 13

struct rgb {
	int r, g, b;
} const colors[] = {
    {255, 255, 255},	/* white */
    {  0,   0,   0},    /* black */
	{230,  25,  75},	/* red */
	{ 60, 180,  75},	/* green */
	{255, 225,  25},	/* yellow */
	{  0, 130, 200},	/* blue */
	{245, 130,  48},	/* orange */
	{ 70, 240, 240},	/* cyan */
	{240,  50, 230},	/* magenta */
	{250, 190, 212},	/* pink */
	{  0, 128, 128},	/* teal */
	{170, 110,  40},	/* brown */
	{255, 250, 200},	/* beige */
};

#endif