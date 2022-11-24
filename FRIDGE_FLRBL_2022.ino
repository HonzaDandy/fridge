//CC @2019


#include <OneButton.h>
#include <CRC32.h>
#include <SPI.h>
#include <MFRC522Extended.h>
#include <require_cpp11.h>
#include <deprecated.h>
#include <L6470.h>
#include <TimerOne.h>
#include <ClickEncoder.h>
#include <LiquidCrystal.h>
#include "Structs.h"
#include "eepromValues.h" 

#define RST_PIN         11          
#define SS_PIN          10        
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance


MFRC522::MIFARE_Key m_key;


SData m_RfidData;

///////DEBUG///////
#define DEBUG false
///////////////////



///////ID///////

#define AKTIVNI_BLOK_RFID 4
#define GENERATOR_CISEL A7 // experimentalne zjisteno A7 :-)
#define BTN_EN1 72
#define BTN_EN2 14
#define BTN_ENC 9
#define STEPS_PER_NOTCH 4
#define KLAKSON 84

OneButton btn(BTN_ENC, true);


///////Kategorie///////

#define ODPOCIVKA 0
#define VUPM 1
#define STK 2
#define KOPA_ODPADU 3
#define GARAZ 4
#define SROTAK 5
#define PRODEJNA_DILU 6
#define LAKOVNA 7
#define PNEUSERVIS 8
#define NEDOJEL_A_SYN 9

///////Nastaveni///////

#define POCET_HLASEK 2
#define NACITACICH_KECU 34
#define TIMEOUT 80000
#define RESET_EEPROM false

///////ODDILY///////

#define MODRI 1
#define ZELENI 2
#define CERVENI 3
#define ZLUTI 4
#define BILI 5

char m_strBarvyOddilu[6][8] = {

		"N/A",
		"Modri",
		"Zeleni",
		"Cerveni",
		"Zluti",
		"Bili"

};

byte m_bStage = 0;

byte m_bTypFridge = 0;

byte m_bTechChoosen = 0;

byte m_bSelectedTechLevel;


unsigned long m_ulChecksum32;

unsigned long previousMillis = 0;

const char m_strNacitaciKecy[NACITACICH_KECU][200] PROGMEM = {
		
		"Ohybam lzici . . . ",
		"Nacitam dlouhodobou pamet . . . . . . . . . .",
		"Generuji uzivatelsky ucet . . . . ",
		"Teraformuji Mars . . . . . ",
		"Lubrikuji kluzne spoje CPU . . . . . . ",
		"Prekonavam opotrebeni obvodu . . . . . .",
		"Kontroluji obsazenost jidelny . . . . ",
		"Kontaktuji servery V-T",
		"Prenasim nastaveni z predchozi verze. . . . . ",
		"Gratulujeme, uspesne jste zaktualizovali WIN XP na novejsi verzi.",
		"Krystalizuji neony - - - - hotovo",
		"Rafaktoruji hypermodifikace. . . .",
		"Hledam les . . . . ",
		"Rekalibruji linie trasy . . .",
		"Nacitam adaptace protokolu . . . ",
		"Kvantuji bitovy prostor",
		"Updatuji bezpecnostni protokoly . . . ",
		"Trasuji optimalni neuronovou drahu . . . .",
		"Cistim mezipamet . . . . . ",
		"Ukladam nastaveni pro skenovani ... ... ... ...",
		"Parametrizuji stav strukturalni integrity . . . ",
		"Objektivizuji dvojkove nebinarni pametove medium . . . . ",
		"Ukoncuji sezeni predchoziho uzivatele - - - - -",
		"Trva to dlouho, co?",
		"Strukturalizuji uzivatelske rozhrani . . .",
		"Instaluji bezpecnostni zaplaty . . . .",
		"Pripojuji se k satelitnimu navadeni - - - - - - - - - - satelity nenalezeny",
		"Stratuji procesy jadra systemu . . . .",
		"Nacitam ..... ..... ..... ..... ..... ..... ..... ..... ..... ..... ..... Nacteno.",
		"Uvolnuji systemove kapacity pro uzivatelskou interakci . . . .",
		"Nacitam system . . . ",
		"Zpristupnuji soubory . . . .",
		"Opravuji tepelny stit . . .",
		"Regeneruji holografickou navigaci . . . ."
		
	
};



char m_strKategorieBlueprintu[10][21] = {

		"Odpocivka",
		"VUPM",
		"STK",
		"Kopa odpadu",
		"Garaz",
		"Srotak",
		"Prodejna dilu",
		"Lakovna",
		"Pneuservis",
		"Nedojel a syn"
};

char m_strSecrets[30][11] = {

		"Dirge",
		"for",
		"the",
		"planet",
		"Miraculous",
		"crystal",
		"given",
		"by",
		"stars",
		"I",
		"can",
		"foresee",
		"a",
		"future",
		"in",
		"fabulous",
		"glass",
		"Lie",
		"spit",
		"on",
		"caves",
		"mock",
		"scribbled",
		"to",
		"us",
		"Earths",
		"doom",
		"day",
		"is",
		"close"
};


char m_strAnoNe[2][21] = {

		"Ne",
		"Ano"
};

char m_strNastaveni[2][21] = {

		"cip",
		"Fridge"
};


unsigned int m_uiIterators[30];

const int rs = 82, en = 18, d4 = 19, d5 = 70, d6 = 85, d7 = 71;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
ClickEncoder *encoder;
int16_t last, value;
int m_iMenuValue, m_iLastMenuValue;
bool m_bBtnClicked = false;

struct buffStruct
{
	unsigned int uiIterator = 0;
	char secret[32];
	buffStruct()
	{
		memset(secret, 0, sizeof(secret));
	}
};

void text(char stringIn[], bool bEndPause = true)
{

	int iRadek = 0;
	int iZnakuNaRadku = 0;
	char* pch;
	pch = strtok(stringIn, " ");
	lcd.clear();

	while (pch != NULL)
	{
		delay(random(100, 300));
		char* pCh = strrchr(pch, '\n');
		if (pCh)
		{
			*pCh = 0;
		}
		if (iZnakuNaRadku + strlen(pch) > 19)
		{
			//Serial.print("\t\t");
			//Serial.print(iZnakuNaRadku);
			iRadek++;
			iZnakuNaRadku = 0;
			//Serial.println();
			lcd.setCursor(0, iRadek);
			if (iRadek > 3)
			{
				delay(random(1000, 2000));
				iRadek = 0;
				lcd.clear();
				//Serial.println();
			}
		}
		iZnakuNaRadku += strlen(pch);

		//Serial.print(pch);
		lcd.print(pch);
		if (iZnakuNaRadku + strlen(pch) >= 19)
		{
			//Serial.print("\t\t");
			//Serial.print(iZnakuNaRadku);
			iRadek++;
			iZnakuNaRadku = 0;
			//Serial.println();
			lcd.setCursor(0, iRadek);
			if (iRadek > 3)
			{
				delay(random(3000, 5000));
				iRadek = 0;
				lcd.clear();
				Serial.println();
			}
		}
		else
		{
			//Serial.print(" ");
			lcd.print(" ");
			iZnakuNaRadku++;
		}
		if (pCh)
		{
			//PotvrdENC();
			iRadek = 0;
			lcd.clear();
			iZnakuNaRadku = 0;
			//Serial.println();
		}
		pch = strtok(NULL, " ");
	}

	if (bEndPause) delay(random(2000, 2200));

}

//---------------------------------------------------------------------------//
void setup()
{

	Serial.begin(115200);
	Serial.println("---------------------------------");
	pinMode(BTN_ENC, INPUT_PULLUP);
	btn.attachClick(click);
	rfidSetup();
	lcdSetup();
	encSetup();



	for (int i = 0; i < 30; i++)
	{
		m_uiIterators[i] = 1;
	}
	if (RESET_EEPROM)
	{
		SaveToEEPROM();
	}

	LoadFromEEPROM();




		buffStruct bstruct;

		char byteBuffer[34];
		size_t numBytes = sizeof(byteBuffer) - 1;
		memcpy(byteBuffer, &bstruct, sizeof(byteBuffer));

		CRC32 crc1;

		for (size_t i = 0; i < numBytes; i++)
		{
			Serial.println(byteBuffer[i], HEX);
			crc1.update(byteBuffer[i]);
		}

		uint32_t checksum = crc1.finalize();
		m_ulChecksum32 = checksum;
		Serial.println(checksum, HEX);


		for (int i = 0; i < 30; i++)
		{

			if (!(i % 3))
			{
				Serial.print(m_strKategorieBlueprintu[i / 3]);
				Serial.println(":");

			}
			Serial.print("I");
			Serial.print(i);
			Serial.print(": ");
			Serial.print(m_uiIterators[i]);
			Serial.print(" kod: ");
			Serial.print(i / 3);
			Serial.print(i % 3);
			Serial.print(" ");
			Serial.println(m_strSecrets[i]);


		}



	int s = 0;
	for (int ii = 0; ii < 30; ii++)
	{
		s += m_uiIterators[ii];
	}

	randomSeed(s+analogRead(GENERATOR_CISEL));


	for (int i = 0; i < 20; i++)
	{
		Serial.println(random(0, 10));
	}


	if (digitalRead(BTN_ENC) == LOW) showSettings();


}

//---------------------------------------------------------------------------//
void rfidSetup()
{
	SPI.begin();                                                  // Init SPI bus
	mfrc522.PCD_Init();                                              // Init MFRC522 card
	for (byte i = 0; i < 6; i++)
		m_key.keyByte[i] = 0xFF;
}

//---------------------------------------------------------------------------//
void lcdSetup()
{
	pinMode(BTN_EN1, INPUT_PULLUP);
	pinMode(BTN_EN2, INPUT_PULLUP);
	pinMode(BTN_ENC, INPUT_PULLUP);
	pinMode(KLAKSON, OUTPUT);
	lcd.begin(20, 4);
	lcd.clear();
}

void click()
{
	m_bBtnClicked = true;
}

void resetClick()
{
	m_bBtnClicked = false;
}

//---------------------------------------------------------------------------//
void loop()
{
	//timer();
	btn.tick();


	switch (m_bStage)
	{
	case 0:
		if (start()) m_bStage++;
		break;
	case 1:
		if (waitOnStart()) m_bStage++;
		break;
	case 2:
		if (DEBUG)
		{
		m_bStage++;
		break;
		}
		if (loadingTexts()) m_bStage++;
		break;
	case 3:
		if (confirmAfterLoad()) m_bStage++;
		break;
	case 4:
		if (chooseBlueprint()) m_bStage++;
		break;
	case 5:
		if (computeCode()) m_bStage++;
		break;
	case 6:
		if (showCode()) m_bStage++;
		break;
	case 7:
		if (goodBye()) m_bStage++;
		break;
	default:
		m_bStage = 0;
		break;
	}

}
bool start()
{
	text("Zahajit hledani?", false);
	lcd.setCursor(0, 2);
	lcd.print(m_strKategorieBlueprintu[m_bTypFridge]);
	return true;
}

bool waitOnStart()
{
	if (m_bBtnClicked)
	{
		resetClick();

		ReadRFID(AKTIVNI_BLOK_RFID, &m_RfidData, "cip.");

		if (!m_RfidData.bKlice[m_bTypFridge])
		{
			lcd.clear();
			lcd.print("Pristup zamezen! ");
			digitalWrite(KLAKSON, HIGH);
			while (true);
		}
		return true;
	}
	return false;
}

bool loadingTexts()
{



	for (byte b = 0; b < POCET_HLASEK; b++)
	{
		char ch[200];
		strncpy_P(ch, m_strNacitaciKecy[random(0, NACITACICH_KECU)], 200);
		text(ch);
	//	text(m_strNacitaciKecy[random(0, NACITACICH_KECU)]);

	}
	text("Nacteno. Pokracovat?", false);
	return true;
}

bool confirmAfterLoad()
{
	/*if (m_bBtnClicked)
	{
		resetClick();
		m_iMenuValue = 0;
		lcd.clear();
		lcd.setCursor(0, 0);
		lcd.print("Zvolte blueprint:");
		lcd.setCursor(9, 1);
		lcd.print(m_strUrovneTechu[m_iMenuValue]);
		return true;
	}
	return false;*/
	return true;
}

bool chooseBlueprint()
{
	/*encService();
	if (chooseBetween(0, 2, false))
	{
		lcd.setCursor(9, 1);
		lcd.print(m_strUrovneTechu[m_iMenuValue]);
		m_bSelectedTechLevel = m_iMenuValue;
	}
	if (m_bBtnClicked)
	{

		resetClick();
		
		return true;
	}

	return false;*/

	m_bSelectedTechLevel = random(0, 2);
	return true;
}
bool computeCode()
{

	buffStruct bstruct;

	bstruct.uiIterator = m_uiIterators[(m_bTypFridge * 3) + m_bSelectedTechLevel];
	strncpy(bstruct.secret, m_strSecrets[(m_bTypFridge * 3) + m_bSelectedTechLevel], 11);
	char byteBuffer[34];
	size_t numBytes = sizeof(byteBuffer) - 1;
	memcpy(byteBuffer, &bstruct, sizeof(byteBuffer));

	m_uiIterators[(m_bTypFridge*3) + m_bSelectedTechLevel]++;

	CRC32 crc;

	for (size_t i = 0; i < numBytes; i++)
	{
		crc.update(byteBuffer[i]);
	}

	uint32_t checksum = crc.finalize();
	m_ulChecksum32 = checksum;


	SaveToEEPROM();
	lcd.clear();
	lcd.setCursor(0, 0);
	lcd.print("Kod dilu:");
	lcd.setCursor(0, 1);
	lcd.print(m_bTypFridge);
	lcd.print(m_bSelectedTechLevel);
	lcd.print(m_ulChecksum32, HEX);

	lcd.setCursor(0, 3);
	lcd.print("Cekam na vypnuti.");

	Serial.print(m_bTypFridge);
	Serial.print(m_bSelectedTechLevel);
	Serial.println(m_ulChecksum32, HEX);

	return true;
}

bool showCode()
{



}

bool goodBye()
{

}


//---------------------------------------------------------------------------//
bool chooseBetween(long bottomValue, long topValue, boolean accOn) {
	encoder->setAccelerationEnabled(accOn);
	m_iMenuValue = value;
	if (value >= topValue)
	{
		value = topValue;
	}
	else if (value <= bottomValue)
	{
		value = bottomValue;
	}
	if (m_iMenuValue != m_iLastMenuValue) {
		m_iLastMenuValue = m_iMenuValue;
		if (m_iMenuValue >= topValue)
		{
			m_iMenuValue = topValue;
		}
		else if (m_iMenuValue <= bottomValue)
		{
			m_iMenuValue = bottomValue;
		}
		return true;
	}
	return false;
}

//---------------------------------------------------------------------------//

void PotvrdENC()
{
	while (digitalRead(BTN_ENC) == LOW);
	while (digitalRead(BTN_ENC) == HIGH);
	delay(100);
	lcd.clear();
	while (digitalRead(BTN_ENC) == LOW);
}

void showSettings() {
	lcd.clear();
	lcd.setCursor(0, 0);
	lcd.print("UPOZORNENI!");
	lcd.setCursor(0, 1);
	lcd.print("Pokud jsi hrac,");
	lcd.setCursor(0, 2);
	lcd.print("ihned restartuj");
	lcd.setCursor(0, 3);
	lcd.print("zarizeni!");
	PotvrdENC();

	int lastmenu1;

	bool bFridge = 0;

	while (true) {
		encService();
		if (m_iMenuValue != lastmenu1) lcd.clear();
		lcd.setCursor(0, 0);
		lcd.print("Nastavit");
		lcd.setCursor(0, 1);
		lcd.print(m_strNastaveni[m_iMenuValue]);
		lastmenu1 = m_iMenuValue;
		bFridge = value;
		if (chooseBetween(0, 1, false));
		if (digitalRead(BTN_ENC) == LOW) break;
	}
	while (digitalRead(BTN_ENC) == HIGH);
	delay(200);
	lcd.clear();
	while (digitalRead(BTN_ENC) == LOW);
	lastmenu1 = -1;

	if (bFridge)
	{
		byte toSetTypFridge = m_bTypFridge;
		lcd.clear();
		value = 0;
		bool bReset = 0;
		while (true) {
			encService();
			if (m_iMenuValue != lastmenu1) lcd.clear();
			lcd.setCursor(0, 0);
			lcd.print("Resetovat");
			lcd.setCursor(0, 1);
			lcd.print("pamet?");
			lcd.setCursor(0, 2);
			lcd.print(m_strAnoNe[m_iMenuValue]);
			lastmenu1 = m_iMenuValue;
			bReset = value;
			if (chooseBetween(0, 1, false));
			if (digitalRead(BTN_ENC) == LOW) break;
		}
		while (digitalRead(BTN_ENC) == HIGH);
		delay(200);
		lcd.clear();
		while (digitalRead(BTN_ENC) == LOW);

		if (bReset)
		{
			for (int i = 0; i < 30; i++)
			{
				m_uiIterators[i] = 0;
			}
			m_bTypFridge = 0;
			SaveToEEPROM();
			LoadFromEEPROM();
		}

		value = m_bTypFridge;

		while (true) {
			encService();
			if (m_iMenuValue != lastmenu1) lcd.clear();
			lcd.setCursor(0, 0);
			lcd.print("Nastavit");
			lcd.setCursor(0, 1);
			lcd.print("Typ Fridge");
			lcd.setCursor(0, 3);
			lcd.print(m_strKategorieBlueprintu[m_iMenuValue]);
			lastmenu1 = m_iMenuValue;
			m_bTypFridge = value;
			if (chooseBetween(0, 9, false));
			if (digitalRead(BTN_ENC) == LOW) break;
		}
		while (digitalRead(BTN_ENC) == HIGH);
		delay(200);
		lcd.clear();
		while (digitalRead(BTN_ENC) == LOW);

		value = m_uiIterators[m_bTypFridge * 3];

		for (int i = 0; i < 3; i++)
		{
			while (true) {
				encService();
				if (m_iMenuValue != lastmenu1) lcd.clear();
				lcd.setCursor(0, 0);
				lcd.print("Nastavit");
				lcd.setCursor(0, 1);
				lcd.print("iterator ");
				lcd.print(i);
				lcd.setCursor(0, 2);
				lcd.print(m_strKategorieBlueprintu[m_bTypFridge]);
				lcd.setCursor(5, 3);
				lcd.print(m_iMenuValue);
				lastmenu1 = m_iMenuValue;
				m_uiIterators[i + (m_bTypFridge * 3)] = value;
				if (chooseBetween(0, 0xFFFF, true));
				if (digitalRead(BTN_ENC) == LOW) break;
			}

			while (digitalRead(BTN_ENC) == HIGH);
			delay(200);
			lcd.clear();
			while (digitalRead(BTN_ENC) == LOW);
			value = m_uiIterators[i + 1 + (m_bTypFridge * 3)];
			lastmenu1 = -1;
		}

		SaveToEEPROM();
	}

	else
	{
		ReadRFID(AKTIVNI_BLOK_RFID, &m_RfidData, "cip.");

		for (int i = 0; i < 10; i++)
		{


			value = m_RfidData.bKlice[i];

			bool bPlatny = 0;

			while (true) {
				encService();
				if (m_iMenuValue != lastmenu1) lcd.clear();
				lcd.setCursor(0, 0);
				lcd.print("Pristup do");
				lcd.setCursor(0, 1);
				lcd.print(m_strKategorieBlueprintu[i]);
				lcd.print("?");

				lcd.setCursor(0, 2);
				lcd.print(m_strAnoNe[m_iMenuValue]);
				lastmenu1 = m_iMenuValue;
				bPlatny = value;
				if (chooseBetween(0, 1, false));
				if (digitalRead(BTN_ENC) == LOW) break;
			}
			while (digitalRead(BTN_ENC) == HIGH);
			delay(200);
			lcd.clear();
			while (digitalRead(BTN_ENC) == LOW);
			m_RfidData.bKlice[i] = bPlatny;
		}

		WriteRFID(AKTIVNI_BLOK_RFID, &m_RfidData, "cip");
		lcd.clear();
		while (digitalRead(BTN_ENC) == LOW);
	}

	



	nashle();
}

//---------------------------------------------------------------------------//
void ReadUID()
{
	Serial.print(F("Card UID:"));    //Dump UID
	for (byte i = 0; i < mfrc522.uid.size; i++)
	{
		Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
		Serial.print(mfrc522.uid.uidByte[i], HEX);
	}
}

//---------------------------------------------------------------------------//
void WriteRFID(byte bBlock, SData* psData, char co[])
{
	lcd.setCursor(1, 0);
	lcd.print("");
	lcd.setCursor(0, 1);
	lcd.print("<<-----------");
	lcd.setCursor(1, 2);
	lcd.print("");
	lcd.setCursor(4, 2);
	lcd.print("Prilozte ");
	lcd.print(co);
	bool done = false;
	while (!done)
	{
		// Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
		if (!mfrc522.PICC_IsNewCardPresent())
			continue;

		// Select one of the cards
		if (!mfrc522.PICC_ReadCardSerial())
			continue;

		ReadUID();

		Serial.print(F(" PICC type: "));   // Dump PICC type
		MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
		Serial.println(mfrc522.PICC_GetTypeName(piccType));

		if (mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, bBlock, &m_key, &(mfrc522.uid)) != MFRC522::STATUS_OK)
		{
			Serial.print(F("PCD_Authenticate() failed: "));
			mfrc522.PICC_HaltA(); // Halt PICC
			mfrc522.PCD_StopCrypto1();  // Stop encryption on PCD
			delay(50);
			continue;
		}

		if (mfrc522.MIFARE_Write(bBlock, (byte*)psData, 16) != MFRC522::STATUS_OK)
		{
			Serial.print(F("MIFARE_Write() failed: "));
			mfrc522.PICC_HaltA(); // Halt PICC
			mfrc522.PCD_StopCrypto1();  // Stop encryption on PCD
			delay(50);
			continue;
		}
		mfrc522.PICC_HaltA(); // Halt PICC
		mfrc522.PCD_StopCrypto1();  // Stop encryption on PCD
		digitalWrite(KLAKSON, HIGH);
		delay(50);
		digitalWrite(KLAKSON, LOW);
		done = true;

	}
}

//---------------------------------------------------------------------------//
void ReadRFID(byte bBlock, SData* psData, char co[])
{
	lcd.clear();
	lcd.setCursor(1, 0);
	lcd.print("");
	lcd.setCursor(0, 1);
	lcd.print("<<-----------");
	lcd.setCursor(1, 2);
	lcd.print("");
	lcd.setCursor(4, 2);
	lcd.print("Prilozte ");
	lcd.print(co);

	bool done = false;
	while (!done)
	{
		// Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
		if (!mfrc522.PICC_IsNewCardPresent())
			continue;

		// Select one of the cards
		if (!mfrc522.PICC_ReadCardSerial())
			continue;

		ReadUID();

		Serial.print(F(" PICC type: "));   // Dump PICC type
		MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
		Serial.println(mfrc522.PICC_GetTypeName(piccType));

		if (mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, bBlock, &m_key, &(mfrc522.uid)) != MFRC522::STATUS_OK)
		{
			Serial.print(F("PCD_Authenticate() failed: "));
			mfrc522.PICC_HaltA(); // Halt PICC
			mfrc522.PCD_StopCrypto1();  // Stop encryption on PCD
			delay(50);
			continue;
		}

		byte bBuffer[18];
		byte bSize = sizeof(bBuffer);

		if (mfrc522.MIFARE_Read(bBlock, bBuffer, &bSize) != MFRC522::STATUS_OK)
		{
			Serial.print(F("MIFARE_Read() failed: "));
			mfrc522.PICC_HaltA(); // Halt PICC
			mfrc522.PCD_StopCrypto1();  // Stop encryption on PCD
			delay(50);
			continue;
		}

		memcpy(psData, bBuffer, sizeof(m_RfidData));

		mfrc522.PICC_HaltA(); // Halt PICC
		mfrc522.PCD_StopCrypto1();  // Stop encryption on PCD
		digitalWrite(KLAKSON, HIGH);
		delay(50);
		digitalWrite(KLAKSON, LOW);
		done = true;
	}
	PrintRFID(AKTIVNI_BLOK_RFID, &m_RfidData);
}

//---------------------------------------------------------------------------//
void PrintRFID(int bData, SData* psData)
{
	Serial.println("");
	Serial.print("Blok: ");
	Serial.println(bData);
	Serial.println("Klice:");

	for (int i = 0; i < 10; i++)
	{
		Serial.print(m_strKategorieBlueprintu[i]);
		Serial.print(": ");
		Serial.println(psData->bKlice[i]);
	}


}

//---------------------------------------------------------------------------//
void SaveToEEPROM()
{
	m_sEEValues.bTypFridge = m_bTypFridge;
	for (int i = 0; i < 30; i++)
	{
		m_sEEValues.uiIterator[i] = m_uiIterators[i];
	}

	EEWrite();
}

//---------------------------------------------------------------------------//
void LoadFromEEPROM()
{
	EERead();

	for (int i = 0; i < 30; i++)
	{
		 m_uiIterators[i] = m_sEEValues.uiIterator[i];
	}
	m_bTypFridge = m_sEEValues.bTypFridge;
	

}

//---------------------------------------------------------------------------//
void nashle() {
	lcd.clear();
	lcd.setCursor(0, 0);
	lcd.print("Ulozeno.");
	//digitalWrite(KLAKSON, HIGH);
	while (true) {};
}

//---------------------------------------------------------------------------//

void timer()
{
	unsigned long currentMillis = millis();



	if (currentMillis - previousMillis >= TIMEOUT)
	{
		previousMillis = currentMillis;
		text("Vypinani . . . . . . . . . ");
		while (true);
	}

}