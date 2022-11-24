// 
// 
// 

#include "eepromValues.h"
#include <EEPROM.h>

sEEValue m_sEEValues;

void EERead()
{
	EEPROM.get(0, m_sEEValues);
}

void EEWrite()
{
	EEPROM.put(0, m_sEEValues);
}

void EERead(void* pVoid, unsigned int uiSize)
{
	byte* pbPtr = (byte*)pVoid;
	for (unsigned int ui = 0; ui < uiSize; ui++)
	{
		*pbPtr = EEPROM.read(sizeof(m_sEEValues) + ui);
		pbPtr++;
	}
}

void EEWrite(void* pVoid, unsigned int uiSize)
{
	byte* pbPtr = (byte*)pVoid;
	for (unsigned int ui = 0; ui < uiSize; ui++)
	{
		EEPROM.write(sizeof(m_sEEValues) + ui, *pbPtr);
		pbPtr++;
	}
}
