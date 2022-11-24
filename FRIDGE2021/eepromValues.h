// eepromValues.h

#ifndef _EEPROMVALUES_h
#define _EEPROMVALUES_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

typedef  struct S_EEPROM
{

  byte bTypFridge;

} sEEValue;

extern sEEValue m_sEEValues;

void EERead();
void EEWrite();

void EERead(void* pVoid, unsigned int uiSize);
void EEWrite(void* pVoid, unsigned int uiSize);

#endif
