/*
 * @(#)register.h
 *
 * Copyright 2000-2001, Aaron Ardiri (mailto:aaron@ardiri.com)
 * All rights reserved.
 *
 * This file was generated as part of the "dkung" program developed for 
 * the Palm Computing Platform designed by Palm: 
 *
 *   http://www.palm.com/ 
 *
 * The contents of this file is confidential and proprietrary in nature 
 * ("Confidential Information"). Redistribution or modification without 
 * prior consent of the original author is prohibited. 
 */

#ifndef _REGISTER_H
#define _REGISTER_H

#include "palm.h"

#define MAX_IDLENGTH 8

enum gameAdjustMode
{
  gameKeyReset  = 0,
  gameScoreCode,
  gameHighScore
};

typedef struct
{
  UInt8 adjustMode;                        // what type of adjustment?
  union {

    // generic
    struct {
      UInt8  unused[16];
    } generic; 

    // key reset
    struct { } keyReset;

    // score code
    struct { 
      Char   *key;
      UInt16 code;
    } scoreCode;

    // high score
    struct { } highScore;

  } data;
} GameAdjustmentType;

extern void    RegisterInitialize(PreferencesType *)                       __REGISTER__;
extern void    RegisterShowMessage(PreferencesType *)                      __REGISTER__;
extern Boolean RegisterAdjustGame(PreferencesType *, GameAdjustmentType *) __REGISTER__;
extern UInt8   RegisterChecksum(UInt8*, Int16)                             __REGISTER__;
extern void    RegisterDecryptChunk(UInt8*, Int16, Int16, UInt8)           __REGISTER__;
extern void    RegisterTerminate()                                         __REGISTER__;

extern void    _regiLoader(PreferencesType *)                              __SAFE0001__;
extern Boolean _adjustGame(PreferencesType *, GameAdjustmentType *)        __SAFE0002__;

#endif
