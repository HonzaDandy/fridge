//CC @2019

#include <MFRC522Extended.h>
#include <require_cpp11.h>
#include <deprecated.h>
#include <L6470.h>
#include <LiquidMenu.h>
#include <TimerOne.h>
#include <ClickEncoder.h>
#include <LiquidCrystal.h>
#include <SPI.h>
#include <MFRC522.h>
#include "Structs.h"
#include "eepromValues.h" 
#include <SPI.h>
#include "Structs.h"


#define RST_PIN         11           // Configurable, see typical pin layout above
#define SS_PIN          10          // Configurable, see typical pin layout above

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance


MFRC522::MIFARE_Key m_key;


SData m_RfidData;

///////DEBUG///////
#define DEBUG 0
///////////////////

///////ID///////

#define AKTIVNI_BLOK_RFID 4
#define GENERATOR_CISEL A7 // experimentalne zjisteno :-)
#define BTN_EN1 72
#define BTN_EN2 14
#define BTN_ENC 9
#define STEPS_PER_NOTCH 4
#define KLAKSON 84


///////TYPY///////

//typy FRIDGE:

#define BANKOMAT 0
#define BANKOMAT_PLUS 1

#define POUST1 2
#define POUST2 3
#define POUST3 4

#define LED1 5
#define LED2 6
#define LED3 7

#define TOXIK1 8
#define TOXIK2 9
#define TOXIK3 10

#define POSTAPO1 11
#define POSTAPO2 12
#define POSTAPO3 13


byte m_bTypFridge = BANKOMAT;


//typy cipu:

#define ZLATY_KLIC 1
#define RUZOVY_KLIC 2
#define TYRKYSOVY_KLIC 3
		// 0 - penezenka	- Zustatek na karte
		//					- pocet modifikatoru 2x
		//					- pocet modifikatoru 3x
		//
		// 1 - Zlaty klic - autentifikace odemceni 
		// 2 - Ruzovy klic - autentifikace odemceni 
		// 3 - Tyrkysovy klic - autentifikace odemceni 



///////ODDILY///////

#define MODRI 1
#define ZELENI 2
#define CERVENI 3
#define ZLUTI 4
#define BILI 5

char m_strTypyCipu[1][21] = {

		"Penezenka"
};

char m_strTypyFridge[14][21] = {  

		"Bankomat",
		"Bankomat +",
		"Poust1",
		"Poust2",
		"Poust3",
		"Led1",
		"Led2",
		"Led3",
		"Toxik1",
		"Toxik2",
		"Toxik3",
		"Postapo1",
		"Postapo2",
		"Postapo3"

};

char m_strBarvyOddilu[6][8] = {

		"N/A",
		"Modri",
		"Zeleni",
		"Cerveni",
		"Zluti",
		"Bili"

};

char m_strPoust1[2][21] = {

		"> Ano",
		"> Ne"
};

char m_strPoust2[2][21] = {

		"> Dokoncime totem.",
		"> Cokoli jineho."
};

char m_strLed[2][21] = {

		"> Doleva.",
		"> Doprava."
};

char m_strToxik1[2][21] = {

		"> Modre.",
		"> Cervene"
};

char m_strToxik2[2][21] = {

		"> Bereme drahokamy.",
		"> Pokracujeme dal."
};

const int rs = 82, en = 18, d4 = 19, d5 = 70, d6 = 85, d7 = 71;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
ClickEncoder *encoder;
int16_t last, value;
int m_iMenuValue, m_iLastMenuValue;

//---------------------------------------------------------------------------//
void setup()
{
	Serial.begin(115200);
	Serial.println("---------------------------------");
	randomSeed(analogRead(GENERATOR_CISEL));
	rfidSetup();
	lcdSetup();
	encSetup();
	LoadFromEEPROM();
	for (int i = 0; i < 20; i++)
	{
		Serial.println(random(0, 10));
	}
	if (digitalRead(BTN_ENC) == LOW) showSettings();
	Serial.println(m_strTypyFridge[m_bTypFridge]);
	TypyFridge();
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



//---------------------------------------------------------------------------//
void loop()
{
	
	nashle();
	/*
	ReadRFID(AKTIVNI_BLOK_RFID, &m_RfidData, "cip."); // vycte do struktury RfidData
	PrintRFID(AKTIVNI_BLOK_RFID, &m_RfidData);
	m_RfidData.uiZustatek += 1000;
	m_RfidData.bBarvaOddilu++;
	m_RfidData.bTypCipu++;
	m_RfidData.bZdroje++;
	m_RfidData.bModTrikrat++;


	WriteRFID(AKTIVNI_BLOK_RFID, &m_RfidData, "cip."); // zapise na kartu strukturu RfidData


	delay(1000);*/

}

//---------------------------------------------------------------------------//
void TypyFridge()
{
	int lastmenu1;
	int vydelek;
	byte cislo;
	byte slovo;


	switch (m_bTypFridge)
	{
	case BANKOMAT:
		text("Vitejte v intranetu. (Pomaly rezim)");
		text("Zamestnanecky kreditni system se nacita. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .");
		Serial.println("DALSI KECY BANKOMAT");
		ReadRFID(AKTIVNI_BLOK_RFID, &m_RfidData, "kartu.");
		if (m_RfidData.bTypCipu)
		{
			lcd.clear();
			lcd.print("Naruseni! ");
			digitalWrite(KLAKSON, HIGH);
			while (true);
		}
		lcd.clear();
		lcd.setCursor(0, 0);
		lcd.print(m_strBarvyOddilu[m_RfidData.bBarvaOddilu]);
		lcd.setCursor(0, 1);
		lcd.print("Zustatek AP: ");
		lcd.print(m_RfidData.uiZustatek);
		lcd.setCursor(0, 2);
		lcd.print("Zdroje: ");
		lcd.print(m_RfidData.bZdroje);
		PotvrdENC();

		lcd.clear();
		lcd.setCursor(0, 0);
		lcd.print("Vytisknout");
		lcd.setCursor(10, 1);
		lcd.print("stvrzenku?");
		lcd.setCursor(3, 3);
		lcd.print("> Ne");
		PotvrdENC();

		lcd.clear();
		lcd.setCursor(0, 0);
		lcd.print("Kvantita prace");
		lcd.setCursor(4, 1);
		lcd.print("je jedinou");
		lcd.setCursor(8, 2);
		lcd.print("kvalitou.");
		lcd.setCursor(4, 3);
		lcd.print("Pracujte tvrde!");
		PotvrdENC();
		while (true);
		break;
	case BANKOMAT_PLUS:
		text("BankomatPlus");
		while (true)
		{
			ReadRFID(AKTIVNI_BLOK_RFID, &m_RfidData, "kartu.");
			if (m_RfidData.bTypCipu)
			{
				lcd.clear();
				lcd.print(m_strTypyCipu[m_RfidData.bTypCipu]);
				PotvrdENC();
				continue;
			}
			lcd.clear();
			unsigned int uiToSetZustatek = m_RfidData.uiZustatek;
			byte bToSetZdroje = m_RfidData.bZdroje;
			value = uiToSetZustatek;
			int lastmenu1 = -1;

			while (true) {
				encService();
				if (m_iMenuValue != lastmenu1) lcd.clear();
				lcd.setCursor(0, 0);
				lcd.print(m_strBarvyOddilu[m_RfidData.bBarvaOddilu]);
				lcd.setCursor(0, 1);
				lcd.print("Nastavit zustatek");
				lcd.setCursor(0, 2);
				lcd.print(m_iMenuValue);
				lcd.setCursor(0, 3);
				lcd.print("(");
				if (int(m_iMenuValue - m_RfidData.uiZustatek) > 0) lcd.print("+");
				lcd.print(int(m_iMenuValue - m_RfidData.uiZustatek));
				lcd.print(")");
				lastmenu1 = m_iMenuValue;
				uiToSetZustatek = value;
				if (chooseBetween(0, 0xFFFF, true));
				if (digitalRead(BTN_ENC) == LOW) break;
			}

			while (digitalRead(BTN_ENC) == HIGH);
			delay(200);
			lcd.clear();

			lcd.clear();
			while (digitalRead(BTN_ENC) == LOW);
			value = bToSetZdroje;
			lastmenu1 = -1;

			while (true) {
				encService();
				if (m_iMenuValue != lastmenu1) lcd.clear();
				lcd.setCursor(0, 0);
				lcd.print(m_strBarvyOddilu[m_RfidData.bBarvaOddilu]);
				lcd.setCursor(0, 1);
				lcd.print("Nastavit zdroje");
				lcd.setCursor(0, 2);
				lcd.print(m_iMenuValue);
				lcd.setCursor(0, 3);
				lcd.print("(");
				if (int(m_iMenuValue - m_RfidData.bZdroje) > 0) lcd.print("+");
				lcd.print(int(m_iMenuValue - m_RfidData.bZdroje));
				lcd.print(")");
				lastmenu1 = m_iMenuValue;
				bToSetZdroje = value;
				if (chooseBetween(0, 0xFFFF, true));
				if (digitalRead(BTN_ENC) == LOW) break;
			}
			while (digitalRead(BTN_ENC) == HIGH);
			delay(200);
			lcd.clear();
			while (digitalRead(BTN_ENC) == LOW);

			m_RfidData.uiZustatek = uiToSetZustatek;
			m_RfidData.bZdroje = bToSetZdroje;

			WriteRFID(AKTIVNI_BLOK_RFID, &m_RfidData, "kartu.");

			text("BankomatPlus");
		}
		break;

	case POUST1:
		text("Nacitani systemu...");
		text("Zpristupneni souboru...");
		text("Zobrazeni posledniho zaznamu...");
		text("Data:");
		text("Po letech hledani jsem konecne nalezl cestu k posvatne relikvii mocnemu Aj, neboli AjaJAjaJAj. V tomto archivu jsem ukryl mapu k nalezeni Aj(e) a doufam, ze stihnu...");
		text("Konec zaznamu.");
		lastmenu1 = -1;
		m_iMenuValue = 0;
		value = 0;

		while (true)
		{
			encService();
			if (m_iMenuValue != lastmenu1) lcd.clear();
			lcd.setCursor(0, 0);
			lcd.print("Zobrazit poznamku?");
			lcd.setCursor(0, 1);
			lastmenu1 = m_iMenuValue;
			lcd.print(m_strPoust1[m_iMenuValue]);
			if (chooseBetween(0, 1, false));
			if (digitalRead(BTN_ENC) == LOW) break;
		}
		while (digitalRead(BTN_ENC) == HIGH);
		delay(200);
		lcd.clear();
		while (digitalRead(BTN_ENC) == LOW);

		if (!m_iMenuValue)
		{
			//Ano
			text("V okruhu 10 m hledejte mapu.");
		}
		else
		{
			//Ne
			text("Soubor smazan. Terminal explodoval. Vsichni se dojdete uzdravit.");
		}
		break;


	case POUST2:
		text("Prisli jste k oaze s jedinym vzrostlym stromem uprostred. Je tu podivny klid, az mozna tisnive ticho, ani vitr nefouka. Prichazite ke stromu a vidite, ze je to ve skutecnosti totem, ktery ale neni dokonceny.");
		text("Co udelate? Domluvte se a kliknete.");
		lastmenu1 = -1;
		m_iMenuValue = 0;
		value = 0;

		while (true)
		{
			encService();
			if (m_iMenuValue != lastmenu1) lcd.clear();
			lcd.setCursor(0, 0);
			lcd.print("Co udelate?");
			lcd.setCursor(0, 1);
			lastmenu1 = m_iMenuValue;
			lcd.print(m_strPoust2[m_iMenuValue]);
			if (chooseBetween(0, 1, false));
			if (digitalRead(BTN_ENC) == LOW) break;
		}
		while (digitalRead(BTN_ENC) == HIGH);
		delay(200);
		lcd.clear();
		while (digitalRead(BTN_ENC) == LOW);

		if (!m_iMenuValue)
		{
			//Dokoncime.
			text("Vyborne. Vzdali jste uctu velkemu Aj. Nasledujte stopy zelene.");
		}
		else
		{
			//Neco jineho
			text("Nejste hodni. Mocna sila Vas zdvihne do vzduchu a prasti s Vami o zem. Dojdete se vsichni uzdravit do tabora.");
		}
		break;

	case POUST3:

		text("Velky Aj nyni zada basen. Slozte basen, kde se obejvi alespon 5x slovo Aj a odmena je Vase.");
		text("Hotovo? Tak ji nahlas zarecitujte.");
		text("Velky Aj je potesen. Hledejte v okruhu 5 m.");
		break;

	case LED1:
		text("Toto je osobni denik technika 2. tridy Arnolda J. Trimmera, zaznam c. 17654372.");
		text("Opet jsem dostal mizernej ukol, ktery neni hoden meho intelektu, meho odhodlani, meho potencialu a me snahy. Brzy, az slozim dustojnicke zkousky, ukazu vsem, zac stoji pan Trimmer!");
		text("Ted je potreba doplnit palivo do tepelneho stitu, ktery zajistuje, ze teplo nasi planety neunika do vesmiru. Prosty jak bulharska prodavacka.");
		text("Postupujte do sekce 3 podle planku.");
		break;
	
	case LED2:
		text("Toto je osobni denik technika 2. tridy Arnolda J. Trimmera, zaznam c. 17654373.");
		text("V sekci 3 zase nikdo neni, klasika. Abych otevrel prislusnou nadrz, je potreba presunout paku ovladajici polohu stitu do spravne polohy. Muj mozek sice pochybuje, jestli si to spravne pamatuje, ale ja dobre vim, ze je to doleva. Nebo doprava? Sakra.");

		lastmenu1 = -1;
		m_iMenuValue = 0;
		value = 0;

		while (true)
		{
			encService();
			if (m_iMenuValue != lastmenu1) lcd.clear();
			lcd.setCursor(0, 0);
			lcd.print("Posunte paku.");
			lcd.setCursor(0, 1);
			lastmenu1 = m_iMenuValue;
			lcd.print(m_strLed[m_iMenuValue]);
			if (chooseBetween(0, 1, false));
			if (digitalRead(BTN_ENC) == LOW) break;
		}
		while (digitalRead(BTN_ENC) == HIGH);
		delay(200);
		lcd.clear();
		while (digitalRead(BTN_ENC) == LOW);

		if (!m_iMenuValue)
		{
			//Doleva.
			text("Zaznel priserny zvuk skripajiciho kovu. Ted radsi rychle najdete na planku sekci 8 a vydejte se doplnit palivo.");
		}
		else
		{
			//Doprava.
			text("Zaznel priserny zvuk skripajiciho kovu. Ted radsi rychle najdete na planku sekci 8 a vydejte se doplnit palivo.");
		}
		break;

	case LED3:
		text("Toto je osobni denik technika 2. tridy Arnolda J. Trimmera, zaznam c. 17654374.");
		text("Dolit palivo do nadrze. Takova potupa. To by zvladl...");
		text("Zapis prerusen.");
		text("Kam jste posunuli paku?");
		lastmenu1 = -1;
		m_iMenuValue = 0;
		value = 0;

		while (true)
		{
			encService();
			if (m_iMenuValue != lastmenu1) lcd.clear();
			lcd.setCursor(0, 0);
			lcd.print("Kam?");
			lcd.setCursor(0, 1);
			lastmenu1 = m_iMenuValue;
			lcd.print(m_strLed[m_iMenuValue]);
			if (chooseBetween(0, 1, false));
			if (digitalRead(BTN_ENC) == LOW) break;
		}
		while (digitalRead(BTN_ENC) == HIGH);
		delay(200);
		lcd.clear();
		while (digitalRead(BTN_ENC) == LOW);

		if (!m_iMenuValue)
		{
			//Doleva.
			text("Stejne jako technik Trimmer, posunuli jste paku do chybne polohy. Spravne bylo nechat ji rovne. Tepelny stit se zhroutil a planeta se tak postupne ponorila do vecne doby ledove. Palivo do nadrze nikdy nebylo doplneno. Tak si ho vemte, ve Vasi barve.");
		}
		else
		{
			//Doprava.
			text("Stejne jako technik Trimmer, posunuli jste paku do chybne polohy. Spravne bylo nechat ji rovne. Tepelny stit se zhroutil a planeta se tak postupne ponorila do vecne doby ledove. Palivo do nadrze nikdy nebylo doplneno. Tak si ho vemte, ve Vasi barve.");
		}
		break;

	case TOXIK1:
		text("Prochazite neuveritelnou krajinou. Barvy, tvary, nic, co by bylo jen vzdalene podobne Zemi. Presto, kdyz se rozhlizite, nakonec vidite neco povedomeho. V cemsi, co vypada jako houba s oknem, je videt cosi, co vypada jako letak. Sice podivny, zivy, ale presto letak. Slova, jakoby se menila pred Vasima ocima a nakonec opravdu v cestine ctete:");
		text("Vyfotte bajneho Safirimixe, tvora, ktereho dosud nikdo nespatril!");
		text("Tuhle prilezitost si nemuzete nechat ujit. Po jakych stopach se vydate?");
		lastmenu1 = -1;
		m_iMenuValue = 0;
		value = 0;

		while (true)
		{
			encService();
			if (m_iMenuValue != lastmenu1) lcd.clear();
			lcd.setCursor(0, 0);
			lcd.print("Kam?");
			lcd.setCursor(0, 1);
			lastmenu1 = m_iMenuValue;
			lcd.print(m_strToxik1[m_iMenuValue]);
			if (chooseBetween(0, 1, false));
			if (digitalRead(BTN_ENC) == LOW) break;
		}
		while (digitalRead(BTN_ENC) == HIGH);
		delay(200);
		lcd.clear();
		while (digitalRead(BTN_ENC) == LOW);

		if (!m_iMenuValue)
		{
			//Modra.
			text("Nasledujte modre znacky.");
		}
		else
		{
			//Cervena.
			text("Nasledujte cervene znacky.");
		}
		break;

	case TOXIK2:
		text("Jste u hnizda.Tedy, vypada to jako hnizdo.");
		text("Akorat se hybe, kvika a rostou z neho podivna chapadylka. Jste blizko. Presto Vas prepada podivna nervozita. Jako by Vam neco naseptavalo, abyste patrani zanechali. Kolem hnizda lezi drahokamy.");
		text("Co udelate?");
		lastmenu1 = -1;
		m_iMenuValue = 0;
		value = 0;

		while (true)
		{
			encService();
			if (m_iMenuValue != lastmenu1) lcd.clear();
			lcd.setCursor(0, 0);
			lcd.print("Co?");
			lcd.setCursor(0, 1);
			lastmenu1 = m_iMenuValue;
			lcd.print(m_strToxik2[m_iMenuValue]);
			if (chooseBetween(0, 1, false));
			if (digitalRead(BTN_ENC) == LOW) break;
		}
		while (digitalRead(BTN_ENC) == HIGH);
		delay(200);
		lcd.clear();
		while (digitalRead(BTN_ENC) == LOW);

		if (!m_iMenuValue)
		{
			//Bereme drahokamy.
			text("Vemte si sacek ve sve barve a odejdete.");
		}
		else
		{
			//Pokracujeme dal.
			text("Pokracujte po modrych stopach.");
		}
		break;

	case TOXIK3:
		text("Dosli jste na kraj srazu. U neho stoji postava, ktera je ale proti svetlu nerozeznatelna. Je to Safirimix? Mrak na chvili zahali slunce a...");
		text("Nakreslete, jak si takoveho Safirimixe predstavujete. Obrazek poveste zezadu na strom.");
		text("Hotovo?");
		text("Vemte si odmenu za fotografii ve sve barve. Gratulujeme.");	
		break;


	case POSTAPO1:

		digitalWrite(KLAKSON, HIGH);
		text("NEBEZPECI...");
		text("Potlacit poplach?");
		digitalWrite(KLAKSON, LOW);
		text("ZKONTROLUJTE teplotu polarniho senzoru...");
		text("NEBEZPECI");
		text("Zpristupnit historii?");
		text("Teplota planety nebezpecne stoupa. Polarni ledove cepicky taji. Vzdusne i morske proudy meni svou rychlost, smer i vlastnosti. Smerujeme ke katastrofe.");
		text("Pokracujte k restrukturalizaci polarnich cepicek pomoci zarizeni Klimatron 2000.");
		text("Prilozte ridici vyrez k mape (cervena sipka smeruje dolu) a pokracujte k terminalu Klimatronu. Vyrez vemte s sebou.");
		break;


	case POSTAPO2:

		text("Terminal Klimatronu. Vlozte vstupni kartu do zlabku, instrukce najdete na blizke nastence. Pote kliknete.");
		text("Vstupni karta akceptovana.");
		text("Otocte vyrez modrou sipkou dolu a pokracujte do kontrolni mistnosti.");
		break;


	case POSTAPO3:

		text("STIKNETE TLACITKO");
		text("Opravdu si prejete provest rekonstrukci polarnich ledovcu? Pro pokracovani kliknete.");
		text("Zahajuji proces . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . Pokracujte kliknutim.");
		text("Proces zahajen.");
		text("Automaticke odhlaseni aktivovano.");
		text("System uzamcen.");
		break;

	default:
		break;
	}
	//PotvrdENC();
}

//---------------------------------------------------------------------------//
/*
void text(char stringIn[])
{
	int lineCount = 0;
	int lineNumber = 0;
	boolean stillProcessing = true;
	long charCount = 1;
	lcd.clear();
	lcd.setCursor(0, 0);

	while (stillProcessing) {



		if (++lineCount > 20) {
			lineNumber++;
			lcd.setCursor(0, lineNumber);
			lineCount = 1;
			if (lineNumber == 4)
			{
				while (digitalRead(BTN_ENC) == HIGH);
				delay(200);
				while (digitalRead(BTN_ENC) == LOW);
				lineNumber = 0;
				lineCount = 1;
				lcd.clear();
				lcd.setCursor(0, 0);
			}
		}

		lcd.print(stringIn[charCount - 1]);

		if (!stringIn[charCount]) {   // no more chars to process?
			stillProcessing = false;
		}
		charCount++;

	}
	while (digitalRead(BTN_ENC) == HIGH);
	delay(200);
	while (digitalRead(BTN_ENC) == LOW);
	lcd.clear();
}
*/
//---------------------------------------------------------------------------//
void PotvrdENC()
{
	while (digitalRead(BTN_ENC) == LOW);
	while (digitalRead(BTN_ENC) == HIGH);
	delay(100);
	lcd.clear();
	while (digitalRead(BTN_ENC) == LOW);
}

//---------------------------------------------------------------------------//
void nahodnyNalez(int min, int max)
{

	lcd.setCursor(0, 2);
	lcd.print("Ziskavate");
	lcd.setCursor(5, 3);
	lcd.print(random(min, max));
	lcd.print("   AP   ");
	while (digitalRead(BTN_ENC) == HIGH);
	delay(200);
	value = 0;
	while (digitalRead(BTN_ENC) == LOW);
	lcd.clear();
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
	byte toSetTypFridge = m_bTypFridge;

	lcd.clear();



	int lastmenu1;

	value = m_bTypFridge;

	while (true) {
		encService();
		if (m_iMenuValue != lastmenu1) lcd.clear();
		lcd.setCursor(0, 0);
		lcd.print("Nastavit");
		lcd.setCursor(0, 1);
		lcd.print("Typ Fridge");
		lcd.setCursor(2, 3);
		lcd.print(m_strTypyFridge[m_iMenuValue]);
		lastmenu1 = m_iMenuValue;
		m_bTypFridge = value;
		if (chooseBetween(0, 13, false));
		if (digitalRead(BTN_ENC) == LOW) break;
	}
	while (digitalRead(BTN_ENC) == HIGH);
	delay(200);
	lcd.clear();
	while (digitalRead(BTN_ENC) == LOW);
	SaveToEEPROM();

	ReadRFID(AKTIVNI_BLOK_RFID, &m_RfidData, "cip.");
	byte toSetTypKarty = m_RfidData.bTypCipu;
	unsigned int toSetZustatek = m_RfidData.uiZustatek;
	byte toSetBarva = m_RfidData.bBarvaOddilu;
	byte toSetZdroje = m_RfidData.bZdroje;
	value = toSetBarva;
	lastmenu1 = -1;

	while (true) {
		encService();
		if (m_iMenuValue != lastmenu1) lcd.clear();
		lcd.setCursor(0, 0);
		lcd.print("Nastavit");
		lcd.setCursor(0, 1);
		lcd.print("Barva oddilu");
		lcd.setCursor(5, 3);
		lcd.print(m_strBarvyOddilu[m_iMenuValue]);
		lastmenu1 = m_iMenuValue;
		toSetBarva = value;
		if (chooseBetween(0, 5, false));
		if (digitalRead(BTN_ENC) == LOW) break;
	}
	while (digitalRead(BTN_ENC) == HIGH);
	delay(200);
	lcd.clear();
	while (digitalRead(BTN_ENC) == LOW);
	value = toSetTypKarty;
	lastmenu1 = -1;

	while (true) {
		encService();
		if (m_iMenuValue != lastmenu1) lcd.clear();
		lcd.setCursor(0, 0);
		lcd.print("Nastavit");
		lcd.setCursor(0, 1);
		lcd.print("Typ cipu");
		lcd.setCursor(5, 3);
		lcd.print(m_strTypyCipu[m_iMenuValue]);
		lastmenu1 = m_iMenuValue;
		toSetTypKarty = value;
		if (chooseBetween(0, 0, false));
		if (digitalRead(BTN_ENC) == LOW) break;
	}
	while (digitalRead(BTN_ENC) == HIGH);
	delay(200);
	lcd.clear();
	while (digitalRead(BTN_ENC) == LOW);
	value = toSetZustatek;
	lastmenu1 = -1;

	while (true) {
		encService();
		if(m_iMenuValue!= lastmenu1) lcd.clear();
		lcd.setCursor(0, 0);
		lcd.print("Nastavit");
		lcd.setCursor(0, 1);
		lcd.print("Zustatek AP");
		lcd.setCursor(5, 3);
		lcd.print(m_iMenuValue);
		lastmenu1 = m_iMenuValue;
		toSetZustatek = value;
		if (chooseBetween(0, 0xFFFF, true));
		if (digitalRead(BTN_ENC) == LOW) break;
	}
	while (digitalRead(BTN_ENC) == HIGH);
	delay(200);
	lcd.clear();
	while (digitalRead(BTN_ENC) == LOW);
	value = toSetZdroje;
	lastmenu1 = -1;

	while (true) {
		encService();
		if (m_iMenuValue != lastmenu1) lcd.clear();
		lcd.setCursor(0, 0);
		lcd.print("Nastavit");
		lcd.setCursor(0, 1);
		lcd.print("Zustatek Zdroje");
		lcd.setCursor(5, 3);
		lcd.print(m_iMenuValue);
		lastmenu1 = m_iMenuValue;
		toSetZdroje = value;
		if (chooseBetween(0, 0xFFFF, true));
		if (digitalRead(BTN_ENC) == LOW) break;
	}
	while (digitalRead(BTN_ENC) == HIGH);
	delay(200);
	lcd.clear();
	while (digitalRead(BTN_ENC) == LOW);


	m_RfidData.bTypCipu = toSetTypKarty;
	m_RfidData.uiZustatek = toSetZustatek;
	m_RfidData.bBarvaOddilu = toSetBarva;
	m_RfidData.bZdroje = toSetZdroje;

	WriteRFID(AKTIVNI_BLOK_RFID, &m_RfidData, "cip");
	lcd.clear();
	while (digitalRead(BTN_ENC) == LOW);
	nashle();
}

//---------------------------------------------------------------------------//
void ReadUID()
{
	/*Serial.print(F("Card UID:"));    //Dump UID
	for (byte i = 0; i < mfrc522.uid.size; i++)
	{
		Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
		Serial.print(mfrc522.uid.uidByte[i], HEX);
	}*/
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
	Serial.print("Zustatek AP: ");
	Serial.println(psData->uiZustatek);
	Serial.print("Barva oddilu: ");
	Serial.println(m_strBarvyOddilu[psData->bBarvaOddilu]);
	Serial.print("Typ: ");
	Serial.println(m_strTypyCipu[psData->bTypCipu]);
	Serial.print("Zustatek ZD: ");
	Serial.println(psData->bZdroje);

}

//---------------------------------------------------------------------------//
void SaveToEEPROM()
{
	m_sEEValues.bTypFridge = m_bTypFridge;
	EEWrite();
}

//---------------------------------------------------------------------------//
void LoadFromEEPROM()
{
	EERead();
	m_bTypFridge = m_sEEValues.bTypFridge;

}

//---------------------------------------------------------------------------//
void nashle() {
	lcd.clear();
	lcd.setCursor(0, 0);
	lcd.print("To je vse!");
	//digitalWrite(KLAKSON, HIGH);
	while (true) {};
}

//---------------------------------------------------------------------------//
void text(char stringIn[])
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
				PotvrdENC();
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
				PotvrdENC();
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
			PotvrdENC();
			iRadek = 0;
			lcd.clear();
			iZnakuNaRadku = 0;
			//Serial.println();
		}
		pch = strtok(NULL, " ");
	}
	PotvrdENC();

}