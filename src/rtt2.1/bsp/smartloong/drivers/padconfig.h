/*
 * File      : padcofig.h
 * http://www.openloongson.org/forum.php
 * description:
 * pad multiplex for smart loong(ls1c)
 * Change Logs:
 * Date                Author             Notes
 * 2015-07-05     chinesebear    first version
 */


#ifndef __PAD_H__
#define __PAD_H__
 
#include "ls1c.h"

#define PAD_MUTI_FIRST			1
#define PAD_MUTI_SECOND			2
#define PAD_MUTI_THIRD			3
#define PAD_MUTI_FOURTH			4
#define PAD_MUTI_FIFTH			5
#define PAD_MUTI_GPIO			6
#define PAD_MUTI_DEFAULT		7
#define PAD_MUTI_MAX			PAD_MUTI_DEFAULT

void rt_pad_multiplex_init(void);
void rt_pad_show(void);

#endif



