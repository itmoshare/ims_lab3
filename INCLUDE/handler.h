#ifndef __HANDLERI__H
#define __HANDLERI__H

#include "common.h"

extern u8 mode;

static const u8 MODE_POLL=0x00;
static u8 MODE_COVERTER=0xFF;

void initialize_handler();
void doTriple();
void doConvert();
void convert();
void modes_handle();

#endif