#include <LiquidCrystal.h>
#include <EEPROM.h>

const int rs = 12, en = 11, d4 = 4, d5 = 3, d6 = 8, d7 = 5;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);//Adresa, coloane, linii

int ora0=0,ora1=14,min0=5,min1=9,sec0=5,sec1=0;

volatile bool blocat = true;
volatile bool submenu = false;
volatile bool necitit_sub_menu = false;
volatile bool citit_sub_menu = false;
volatile bool inundatii_sub_menu = false;
volatile double temperatura;

char hidden[10] = "";
char parola[10] = "";
char parola_introdusa[10];

//parola: "down down up down"
void initializare_char(void){
  strcpy(parola_introdusa, "");
  strcpy(parola, "++-+");
}

void timer1()
{
  DDRB |= 1 << PB5; 
  TCCR1A = 0;
  TCCR1B = 0;
  TCCR1B |= (1 << WGM12) | (1 << CS12) | _BV(CS10);
  TIMSK1 |= (1 << OCIE1A);
  OCR1A = 15624;

  sei();
}

enum Buttons {
  EV_OK,
  EV_CANCEL,
  EV_NEXT,
  EV_PREV,
  EV_NONE,
  EV_MAX_NUM
};

enum Menus {
  MENU_MAIN = 0,
  MENU_MESAJE,
  MENU_CONTROL,
  MENU_TEMPERATURA,
  MENU_INUNDATII,
  MENU_MAX_NUM
};

enum Sub_Menus {
  SUB_MENU_MAIN = 0,
  SUB_MENU_NECITITE,
  SUB_MENU_CITITE,
  SUB_MENU_STERGERE,
  SUB_MENU_MAX_NUM
};

enum Necitit_Sub_Menus {
  NECITIT_SUB_MENU_MAIN = 0,
  NECITIT_SUB_MENU_1,
  NECITIT_SUB_MENU_2,
  NECITIT_SUB_MENU_3,
  NECITIT_SUB_MENU_4,
  NECITIT_SUB_MENU_5,
  NECITIT_SUB_MENU_6,
  NECITIT_SUB_MENU_7,
  NECITIT_SUB_MENU_8,
  NECITIT_SUB_MENU_9,
  NECITIT_SUB_MENU_10,
  NECITIT_SUB_MENU_MAX_NUM
};

enum Citit_Sub_Menus {
  CITIT_SUB_MENU_MAIN = 0,
  CITIT_SUB_MENU_1,
  CITIT_SUB_MENU_2,
  CITIT_SUB_MENU_3,
  CITIT_SUB_MENU_4,
  CITIT_SUB_MENU_5,
  CITIT_SUB_MENU_6,
  CITIT_SUB_MENU_7,
  CITIT_SUB_MENU_8,
  CITIT_SUB_MENU_9,
  CITIT_SUB_MENU_10,
  CITIT_SUB_MENU_MAX_NUM
};

enum Inundatii_Sub_Menus {
  INUNDATII_SUB_MENU_MAIN = 0,
  INUNDATII_SUB_MENU_1,
  INUNDATII_SUB_MENU_2,
  INUNDATII_SUB_MENU_3,
  INUNDATII_SUB_MENU_4,
  INUNDATII_SUB_MENU_5,
  INUNDATII_SUB_MENU_6,
  INUNDATII_SUB_MENU_7,
  INUNDATII_SUB_MENU_8,
  INUNDATII_SUB_MENU_9,
  INUNDATII_SUB_MENU_10,
  INUNDATII_SUB_MENU_MAX_NUM
};

enum Menus_b {
  MENU_BLOCAT = 0,
  MENU_DEBLOCAT,
  MENU_MAX_NUM_b
};

int change = 0;

Menus_b scroll_menu_b = MENU_BLOCAT;
Menus_b current_menu_b =  MENU_BLOCAT;

Menus scroll_menu = MENU_MAIN;
Menus current_menu =  MENU_MAIN;

Sub_Menus scroll_menu_sub = SUB_MENU_MAIN;
Sub_Menus current_menu_sub =  SUB_MENU_MAIN;

Necitit_Sub_Menus scroll_menu_nec = NECITIT_SUB_MENU_MAIN;
Necitit_Sub_Menus current_menu_nec =  NECITIT_SUB_MENU_MAIN;

Citit_Sub_Menus scroll_menu_cit = CITIT_SUB_MENU_MAIN;
Citit_Sub_Menus current_menu_cit =  CITIT_SUB_MENU_MAIN;

Inundatii_Sub_Menus scroll_menu_in = INUNDATII_SUB_MENU_MAIN;
Inundatii_Sub_Menus current_menu_in =  INUNDATII_SUB_MENU_MAIN;

void state_machine(enum Menus menu, enum Buttons button);
void state_machine_b(enum Menus_b menu, enum Buttons button);
void state_machine_sub(enum Sub_Menus menu, enum Buttons button);
void state_machine_nec(enum Necitit_Sub_Menus menu, enum Buttons button);
void state_machine_cit(enum Citit_Sub_Menus menu, enum Buttons button);
void state_machine_in(enum Inundatii_Sub_Menus menu, enum Buttons button);

Buttons GetButtons(void);

void print_menu_b(enum Menus_b menu);
void print_menu_sub(enum Sub_Menus menu);
void print_menu(enum Menus menu);
void print_menu_nec(enum Necitit_Sub_Menus menu);
void print_menu_cit(enum Citit_Sub_Menus menu);
void print_menu_in(enum Inundatii_Sub_Menus menu);

typedef void (state_machine_handler_t)(void);
typedef void (state_machine_handler_t_b)(void);
typedef void (state_machine_handler_t_sub)(void);
typedef void (state_machine_handler_t_nec)(void);
typedef void (state_machine_handler_t_cit)(void);
typedef void (state_machine_handler_t_in)(void);


/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
//----------------------------AFISARE NECITITE----------------------------------------//
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

void print_menu_nec(enum Necitit_Sub_Menus menu)
{
  lcd.clear();
  switch (menu)
  {
    case NECITIT_SUB_MENU_1:
      lcd.setCursor(0, 0);
      /*if((mesaje_ram[0].stare_citire == false) && (mesaje_ram[0].valid == true))
       * {
       *  lcd.print("1.");
       *  lcd.print(mesaje_ram[0].mesaj);
       *  mesaje_ram[0].stare_citire = true;
       *  }
       *  else  lcd.print("1.GOL");
       */
      lcd.print("1.MESAJ");
      break;
    case NECITIT_SUB_MENU_2:
    /*if((mesaje_ram[1].stare_citire == false) && (mesaje_ram[1].valid == true))
       * {
       *  lcd.print("2.");
       *  lcd.print(mesaje_ram[1].mesaj);
       *  mesaje_ram[1].stare_citire = true;
       *  }
       *  else  lcd.print("2.GOL");
       */
      lcd.setCursor(0, 0);
      lcd.print("2.MESAJ");
      break;
    case NECITIT_SUB_MENU_3:
      /*if((mesaje_ram[2].stare_citire == false) && (mesaje_ram[2].valid == true))
       * {
       *  lcd.print("3.");
       *  lcd.print(mesaje_ram[2].mesaj);
       *  mesaje_ram[2].stare_citire = true;
       *  }
       *  else  lcd.print("3.GOL");
       */
      lcd.setCursor(0, 0);
      lcd.print("3.MESAJ");
      break;
      case NECITIT_SUB_MENU_4:
      /*if((mesaje_ram[3].stare_citire == false) && (mesaje_ram[3].valid == true))
       * {
       *  lcd.print("4.");
       *  lcd.print(mesaje_ram[3].mesaj);
       *  mesaje_ram[3].stare_citire = true;
       *  }
       *  else  lcd.print("4.GOL");
       */
      lcd.setCursor(0, 0);
      lcd.print("4.MESAJ");
      break;
      case NECITIT_SUB_MENU_5:
      /*if((mesaje_ram[4].stare_citire == false) && (mesaje_ram[4].valid == true))
       * {
       *  lcd.print("5.");
       *  lcd.print(mesaje_ram[4].mesaj);
       *  mesaje_ram[4].stare_citire = true;
       *  }
       *  else  lcd.print("5.GOL");
       */
      lcd.setCursor(0, 0);
      lcd.print("5.MESAJ");
      break;
      case NECITIT_SUB_MENU_6:
     /*if((mesaje_ram[5].stare_citire == false) && (mesaje_ram[5].valid == true))
       * {
       *  lcd.print("6.");
       *  lcd.print(mesaje_ram[5].mesaj);
       *  mesaje_ram[5].stare_citire = true;
       *  }
       *  else  lcd.print("6.GOL");
       */
      lcd.setCursor(0, 0);
      lcd.print("6.MESAJ");
      break;
      case NECITIT_SUB_MENU_7:
      /*if((mesaje_ram[6].stare_citire == false) && (mesaje_ram[6].valid == true))
       * {
       *  lcd.print("7.");
       *  lcd.print(mesaje_ram[6].mesaj);
       *  mesaje_ram[6].stare_citire = true;
       *  }
       *  else  lcd.print("7.GOL");
       */
      lcd.setCursor(0, 0);
      lcd.print("7.MESAJ");
      break;
      case NECITIT_SUB_MENU_8:
      /*if((mesaje_ram[7].stare_citire == false) && (mesaje_ram[7].valid == true))
       * {
       *  lcd.print("8.");
       *  lcd.print(mesaje_ram[7].mesaj);
       *  mesaje_ram[7].stare_citire = true;
       *  }
       *  else  lcd.print("8.GOL");
       */
      lcd.setCursor(0, 0);
      lcd.print("8.MESAJ");
      break;
      case NECITIT_SUB_MENU_9:
      /*if((mesaje_ram[8].stare_citire == false) && (mesaje_ram[8].valid == true))
       * {
       *  lcd.print("9.");
       *  lcd.print(mesaje_ram[8].mesaj);
       *  mesaje_ram[8].stare_citire = true;
       *  }
       *  else  lcd.print("9.GOL");
       */
      lcd.setCursor(0, 0);
      lcd.print("9.MESAJ");
      break;
      case NECITIT_SUB_MENU_10:
      /*if((mesaje_ram[9].stare_citire == false) && (mesaje_ram[9].valid == true))
       * {
       *  lcd.print("10.");
       *  lcd.print(mesaje_ram[9].mesaj);
       *  mesaje_ram[9].stare_citire = true;
       *  }
       *  else  lcd.print("10.GOL");
       */
      lcd.setCursor(0, 0);
      lcd.print("10.MESAJ");
      break;
      
    case NECITIT_SUB_MENU_MAIN:
    default:
      lcd.setCursor(0, 0);
      lcd.print("X NECITITE");
      //lcd.print(Mesaje_necitite());
      break;
  }
  
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
//----------------------------AFISARE CITITE----------------------------------------//
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

void print_menu_cit(enum Citit_Sub_Menus menu)
{
  lcd.clear();
  switch (menu)
  {
    case CITIT_SUB_MENU_1:
      lcd.setCursor(0, 0);
      /*if((mesaje_ram[0].stare_citire == true) && (mesaje_ram[0].valid == true))
       * {
       *  lcd.print(mesaje_ram[0].mesaj);
       *  }
       *  else  lcd.print("1.GOL");
       */
      lcd.print("1.MESAJ");
      break;
    case CITIT_SUB_MENU_2:
    /*if((mesaje_ram[1].stare_citire == true) && (mesaje_ram[1].valid == true))
       * {
       *  lcd.print(mesaje_ram[1].mesaj);
       *  }
       *  else  lcd.print("2.GOL");
       */
      lcd.setCursor(0, 0);
      lcd.print("2.MESAJ");
      break;
    case CITIT_SUB_MENU_3:
      /*if((mesaje_ram[2].stare_citire == true) && (mesaje_ram[2].valid == true))
       * {
       *  lcd.print(mesaje_ram[2].mesaj);
       *  }
       *  else  lcd.print("3.GOL");
       */
      lcd.setCursor(0, 0);
      lcd.print("3.MESAJ");
      break;
      case CITIT_SUB_MENU_4:
      /*if((mesaje_ram[3].stare_citire == true) && (mesaje_ram[3].valid == true))
       * {
       *  lcd.print(mesaje_ram[3].mesaj);
       *  }
       *  else  lcd.print("4.GOL");
       */
      lcd.setCursor(0, 0);
      lcd.print("4.MESAJ");
      break;
      case CITIT_SUB_MENU_5:
      /*if((mesaje_ram[4].stare_citire == true) && (mesaje_ram[4].valid == true))
       * {
       *  lcd.print(mesaje_ram[4].mesaj);
       *  }
       *  else  lcd.print("5.GOL");
       */
      lcd.setCursor(0, 0);
      lcd.print("5.MESAJ");
      break;
      case CITIT_SUB_MENU_6:
     /*if((mesaje_ram[5].stare_citire == true) && (mesaje_ram[5].valid == true))
       * {
       *  lcd.print(mesaje_ram[5].mesaj);
       *  }
       *  else  lcd.print("6.GOL");
       */
      lcd.setCursor(0, 0);
      lcd.print("6.MESAJ");
      break;
      case CITIT_SUB_MENU_7:
      /*if((mesaje_ram[6].stare_citire == true) && (mesaje_ram[6].valid == true))
       * {
       *  lcd.print(mesaje_ram[6].mesaj);
       *  }
       *  else  lcd.print("7.GOL");
       */
      lcd.setCursor(0, 0);
      lcd.print("7.MESAJ");
      break;
      case CITIT_SUB_MENU_8:
      /*if((mesaje_ram[7].stare_citire == true) && (mesaje_ram[7].valid == true))
       * {
       *  lcd.print(mesaje_ram[7].mesaj);
       *  }
       *  else  lcd.print("8.GOL");
       */
      lcd.setCursor(0, 0);
      lcd.print("8.MESAJ");
      break;
      case CITIT_SUB_MENU_9:
      /*if((mesaje_ram[8].stare_citire == true) && (mesaje_ram[8].valid == true))
       * {
       *  lcd.print(mesaje_ram[8].mesaj);
       *  }
       *  else  lcd.print("9.GOL");
       */
      lcd.setCursor(0, 0);
      lcd.print("9.MESAJ");
      break;
      case CITIT_SUB_MENU_10:
      /*if((mesaje_ram[9].stare_citire == true) && (mesaje_ram[9].valid == true))
       * {
       *  lcd.print(mesaje_ram[9].mesaj);
       *  }
       *  else  lcd.print("10.GOL");
       */
      lcd.setCursor(0, 0);
      lcd.print("10.MESAJ");
      break;
      
    case CITIT_SUB_MENU_MAIN:
    default:
      lcd.setCursor(0, 0);
      lcd.print("X CITITE");
      //lcd.print(Mesaje_citite());
      break;
  }
  
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
//----------------------------AFISARE INUNDATII----------------------------------------//
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

void print_menu_in(enum Inundatii_Sub_Menus menu)
{
  lcd.clear();
  switch (menu)
  {
    case INUNDATII_SUB_MENU_1:
      lcd.setCursor(0, 0);
      /*if((mesaje_ram[0].stare_citire == true) && (mesaje_ram[0].valid == true))
       * {
       *  lcd.print(mesaje_ram[0].mesaj);
       *  }
       *  else  lcd.print("1.GOL");
       */
      lcd.print("1.IN");
      break;
    case INUNDATII_SUB_MENU_2:
    /*if((mesaje_ram[1].stare_citire == true) && (mesaje_ram[1].valid == true))
       * {
       *  lcd.print(mesaje_ram[1].mesaj);
       *  }
       *  else  lcd.print("2.GOL");
       */
      lcd.setCursor(0, 0);
      lcd.print("2.in");
      break;
    case INUNDATII_SUB_MENU_3:
      /*if((mesaje_ram[2].stare_citire == true) && (mesaje_ram[2].valid == true))
       * {
       *  lcd.print(mesaje_ram[2].mesaj);
       *  }
       *  else  lcd.print("3.GOL");
       */
      lcd.setCursor(0, 0);
      lcd.print("3.in");
      break;
      case INUNDATII_SUB_MENU_4:
      /*if((mesaje_ram[3].stare_citire == true) && (mesaje_ram[3].valid == true))
       * {
       *  lcd.print(mesaje_ram[3].mesaj);
       *  }
       *  else  lcd.print("4.GOL");
       */
      lcd.setCursor(0, 0);
      lcd.print("4.in");
      break;
      case INUNDATII_SUB_MENU_5:
      /*if((mesaje_ram[4].stare_citire == true) && (mesaje_ram[4].valid == true))
       * {
       *  lcd.print(mesaje_ram[4].mesaj);
       *  }
       *  else  lcd.print("5.GOL");
       */
      lcd.setCursor(0, 0);
      lcd.print("5.in");
      break;
      case INUNDATII_SUB_MENU_6:
     /*if((mesaje_ram[5].stare_citire == true) && (mesaje_ram[5].valid == true))
       * {
       *  lcd.print(mesaje_ram[5].mesaj);
       *  }
       *  else  lcd.print("6.GOL");
       */
      lcd.setCursor(0, 0);
      lcd.print("6.in");
      break;
      case INUNDATII_SUB_MENU_7:
      /*if((mesaje_ram[6].stare_citire == true) && (mesaje_ram[6].valid == true))
       * {
       *  lcd.print(mesaje_ram[6].mesaj);
       *  }
       *  else  lcd.print("7.GOL");
       */
      lcd.setCursor(0, 0);
      lcd.print("7.in");
      break;
      case INUNDATII_SUB_MENU_8:
      /*if((mesaje_ram[7].stare_citire == true) && (mesaje_ram[7].valid == true))
       * {
       *  lcd.print(mesaje_ram[7].mesaj);
       *  }
       *  else  lcd.print("8.GOL");
       */
      lcd.setCursor(0, 0);
      lcd.print("8.in");
      break;
      case INUNDATII_SUB_MENU_9:
      /*if((mesaje_ram[8].stare_citire == true) && (mesaje_ram[8].valid == true))
       * {
       *  lcd.print(mesaje_ram[8].mesaj);
       *  }
       *  else  lcd.print("9.GOL");
       */
      lcd.setCursor(0, 0);
      lcd.print("9.in");
      break;
      case INUNDATII_SUB_MENU_10:
      /*if((mesaje_ram[9].stare_citire == true) && (mesaje_ram[9].valid == true))
       * {
       *  lcd.print(mesaje_ram[9].mesaj);
       *  }
       *  else  lcd.print("10.GOL");
       */
      lcd.setCursor(0, 0);
      lcd.print("10.in");
      break;
      
    case INUNDATII_SUB_MENU_MAIN:
    default:
      lcd.setCursor(0, 0);
      lcd.print("X INUNDATII");
      //lcd.print(Inundatii_citite());
      break;
  }
  
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
//----------------------------AFISARE BLOCAT----------------------------------------//
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

void print_menu_b(enum Menus_b menu)
{
  lcd.clear();
  switch (menu)
  {
    case MENU_BLOCAT:
      sec1++; 
      if(sec1>9)
  {
    sec1=0;
    sec0++;
    if(sec0>5)
    {
      sec0=0;
      min1++;
    }
  }
  if(min1>9)
  {
    min1=0;
    min0++;
    if(min0>5)
    {
      min0=0;
      ora1++;
    }
  }
  if(ora1==25)
    ora1=0;
  
  if(ora1<10)
  {  
    lcd.print(ora0);
  }  
  lcd.print(ora1);
  lcd.print(":");
  lcd.print(min0);
  lcd.print(min1);
  lcd.print(":");
  lcd.print(sec0);
  lcd.print(sec1);
      lcd.print("  ");
      lcd.print(temperatura);
      lcd.print("C");
      lcd.setCursor(0, 1);
      lcd.print("Mesaje noi:2");
      break;
  }
  
  if (current_menu_b != MENU_BLOCAT)
  {
    switch(current_menu_b)
    {
      case MENU_DEBLOCAT:
      //prelucrare parola
      lcd.clear(); 
      lcd.setCursor(0, 0);
      lcd.print("INTRODU PAROLA:");  
      lcd.setCursor(0, 1);
      lcd.print(hidden);  
      break;
      
    }
  }
  
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
//----------------------------AFISARE SUBMENIU----------------------------------------//
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

void print_menu_sub(enum Sub_Menus menu)
{
  lcd.clear();
  switch (menu)
  {
    case SUB_MENU_NECITITE:
      lcd.setCursor(0, 0);
      lcd.print("SUB MENIU");
      lcd.setCursor(0, 1);
      lcd.print("NECITITE");
      break;
    case SUB_MENU_CITITE:
      lcd.setCursor(0, 0);
      lcd.print("SUB MENIU");
      lcd.setCursor(0, 1);
      lcd.print("CITITE");
      break;
    case SUB_MENU_STERGERE:
      lcd.setCursor(0, 0);
      lcd.print("SUB MENIU");
      lcd.setCursor(0, 1);
      lcd.print("STERGERE");
      lcd.setCursor(5, 1);
      break;
    case SUB_MENU_MAIN:
    default:
      lcd.setCursor(0, 0);
      lcd.print("SUB MENIU");
      lcd.setCursor(0, 1);
      lcd.print("SCROLL: UP/DOWN ");
      break;
  }
  
  if (current_menu_sub != SUB_MENU_MAIN)
  {
    switch(current_menu_sub)
    {
      case SUB_MENU_NECITITE:
      lcd.setCursor(0, 0);
      lcd.print("SUB MENIU MESAJE");
      lcd.setCursor(0, 1);
      lcd.print("->NECITITE");
      break;
      case SUB_MENU_CITITE:
      lcd.setCursor(0, 0);
      lcd.print("SUB MENIU MESAJE");
      lcd.setCursor(0, 1);
      lcd.print("->CITITE");
      break;
      case SUB_MENU_STERGERE:
      lcd.clear(); 
      lcd.setCursor(0, 0);
      lcd.print("CONFIRMATI?");  
      lcd.setCursor(0, 1);
      lcd.print("CANCEL/YES");  
      break;
    }
  } 
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
//----------------------------AFISARE PRINCIPAL----------------------------------------//
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

void print_menu(enum Menus menu)
{
  lcd.clear();
  switch (menu)
  {
    case MENU_MESAJE:
      lcd.setCursor(0, 0);
      lcd.print("MENIU PRINCIPAL");
      lcd.setCursor(0, 1);
      lcd.print("    MESAJE");
      break;
    case MENU_CONTROL:
      lcd.setCursor(0, 0);
      lcd.print("MENIU PRINCIPAL");
      lcd.setCursor(0, 1);
      lcd.print("    CONTROL");
      break;
    case MENU_TEMPERATURA:
      lcd.setCursor(0, 0);
      lcd.print("MENIU PRINCIPAL");
      lcd.setCursor(0, 1);
      lcd.print("TEMPERATURA");
      lcd.setCursor(5, 1);
      break;
    case MENU_INUNDATII:
      lcd.setCursor(0, 0);
      lcd.print("MENIU PRINCIPAL");
      lcd.setCursor(0, 1);
      lcd.print("   INUNDATII");
      break;
    case MENU_MAIN:
    default:
      lcd.setCursor(0, 0);
      lcd.print("MENIU PRINCIPAL");
      lcd.setCursor(0, 1);
      lcd.print("SCROLL: UP/DOWN ");
      break;
  }
  
  if (current_menu != MENU_MAIN)
  {
    lcd.clear();
    switch(current_menu)
    {
      case MENU_MESAJE:
      lcd.setCursor(0, 0);
      lcd.print("CATRE SUB MENIU");
      lcd.setCursor(0, 1);
      lcd.print("  ->MESAJE");
      break;
      case MENU_CONTROL:
      lcd.clear(); 
      lcd.setCursor(0, 0);
      lcd.print("MANUAL");  
      lcd.setCursor(0, 1);
      lcd.print("AUTOMAT");  
      break;
      case MENU_TEMPERATURA:
      lcd.clear(); 
      lcd.setCursor(0, 0);
      lcd.print("TEMPERATURA");  
      lcd.setCursor(0, 1);
      lcd.print("T=");  
      lcd.print(temperatura); 
      lcd.print(" C");
      break;
      case MENU_INUNDATII:
      lcd.setCursor(0, 0);
      lcd.print("AFISARE MESAJE");
      lcd.setCursor(0, 1);
      lcd.print(" ->INUNDATII");
      
      break;
    }
  } 
}

//no action function
void no_action(void)
{
  
}


/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
//----------------------------MENIU PRINCIPAL----------------------------------------//
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

void enter_menu(void)
{
  current_menu = scroll_menu;
}

void enter_menu_sub(void)
{
  necitit_sub_menu = false;
  citit_sub_menu = false;
  inundatii_sub_menu = false;
  submenu = true;
  scroll_menu_sub = SUB_MENU_MAIN;
  current_menu_sub = scroll_menu_sub;
}

void enter_menu_subin(void)
{
  necitit_sub_menu = false;
  citit_sub_menu = false;
  submenu = false;
  inundatii_sub_menu = true;
  scroll_menu_in = INUNDATII_SUB_MENU_MAIN;
  current_menu_in = scroll_menu_in;
}

void go_home(void)
{
  scroll_menu = MENU_MAIN;
  current_menu = scroll_menu;
}

void go_next(void)
{
  scroll_menu = (Menus) ((int)scroll_menu + 1);
  scroll_menu = (Menus) ((int)scroll_menu % MENU_MAX_NUM);
}

void go_prev(void)
{
  scroll_menu = (Menus) ((int)scroll_menu - 1);
  scroll_menu = (Menus) ((int)scroll_menu % MENU_MAX_NUM);
}


/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
//----------------------------SUB MENIU ----------------------------------------//
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

void enter_menu_subb(void)
{
  current_menu_sub = scroll_menu_sub;
}

void enter_menu_subnec(void)
{
  submenu = false;
  citit_sub_menu = false;
  inundatii_sub_menu = false;
  necitit_sub_menu = true;
  scroll_menu_nec = NECITIT_SUB_MENU_MAIN;
  current_menu_nec = scroll_menu_nec;
}

void enter_menu_subcit(void)
{
  submenu = false;
  necitit_sub_menu = false;
  inundatii_sub_menu = false;
  citit_sub_menu = true;
  scroll_menu_cit = CITIT_SUB_MENU_MAIN;
  current_menu_cit = scroll_menu_cit;
}

void go_home_sub(void)
{
  scroll_menu_sub = SUB_MENU_MAIN;
  current_menu_sub = scroll_menu_sub;
}

void go_back_menu_sub(void)
{
  submenu = false;
  necitit_sub_menu = false;
  inundatii_sub_menu = false;
  citit_sub_menu = false;
  scroll_menu = MENU_MAIN;
  current_menu = scroll_menu;
}

void go_next_sub(void)
{
  scroll_menu_sub = (Sub_Menus) ((int)scroll_menu_sub + 1);
  scroll_menu_sub = (Sub_Menus) ((int)scroll_menu_sub % SUB_MENU_MAX_NUM);
}

void go_prev_sub(void)
{
  scroll_menu_sub = (Sub_Menus) ((int)scroll_menu_sub - 1);
  scroll_menu_sub = (Sub_Menus) ((int)scroll_menu_sub % SUB_MENU_MAX_NUM);
}

void confirm(void)
{
  //sterge Clear_EEPROM_Mesaje();
  go_home_sub();
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
//----------------------------MENIU NECITITE----------------------------------------//
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

void go_back_menu_nec(void)
{
  citit_sub_menu = false;
  necitit_sub_menu = false;
  inundatii_sub_menu = false;
  submenu = true;
  scroll_menu_sub = SUB_MENU_MAIN;
  current_menu_sub = scroll_menu_sub;
}

void go_next_sub_nec(void)
{
  scroll_menu_nec = (Necitit_Sub_Menus) ((int)scroll_menu_nec + 1);
  scroll_menu_nec = (Necitit_Sub_Menus) ((int)scroll_menu_nec % NECITIT_SUB_MENU_MAX_NUM);
}

void go_prev_sub_nec(void)
{
  scroll_menu_nec = (Necitit_Sub_Menus) ((int)scroll_menu_nec - 1);
  scroll_menu_nec = (Necitit_Sub_Menus) ((int)scroll_menu_nec % NECITIT_SUB_MENU_MAX_NUM);
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
//----------------------------MENIU CITITE----------------------------------------//
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

void go_back_menu_cit(void)
{
  necitit_sub_menu = false;
  citit_sub_menu = false;
  inundatii_sub_menu = false;
  submenu = true;
  scroll_menu_sub = SUB_MENU_MAIN;
  current_menu_sub = scroll_menu_sub;
}

void go_next_sub_cit(void)
{
  scroll_menu_cit = (Citit_Sub_Menus) ((int)scroll_menu_cit + 1);
  scroll_menu_cit = (Citit_Sub_Menus) ((int)scroll_menu_cit % CITIT_SUB_MENU_MAX_NUM);
}

void go_prev_sub_cit(void)
{
  scroll_menu_cit = (Citit_Sub_Menus) ((int)scroll_menu_cit - 1);
  scroll_menu_cit = (Citit_Sub_Menus) ((int)scroll_menu_cit % CITIT_SUB_MENU_MAX_NUM);
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
//----------------------------MENIU INUNDATII----------------------------------------//
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

void go_back_menu_in(void)
{
  necitit_sub_menu = false;
  citit_sub_menu = false;
  submenu = false;
  inundatii_sub_menu = false;
  scroll_menu = MENU_MAIN;
  current_menu = scroll_menu;
}

void go_next_sub_in(void)
{
  scroll_menu_in = (Inundatii_Sub_Menus) ((int)scroll_menu_in + 1);
  scroll_menu_in = (Inundatii_Sub_Menus) ((int)scroll_menu_in % INUNDATII_SUB_MENU_MAX_NUM);
}

void go_prev_sub_in(void)
{
  scroll_menu_in = (Inundatii_Sub_Menus) ((int)scroll_menu_in - 1);
  scroll_menu_in = (Inundatii_Sub_Menus) ((int)scroll_menu_in % INUNDATII_SUB_MENU_MAX_NUM);
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
//----------------------------MENIU BLOCAT----------------------------------------//
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

void bloc(void)
{
  current_menu_b = MENU_DEBLOCAT;
}

void plus(void)
{
  //adaugam cate un caracter +
  strcat(hidden,"*");
  strcat(parola_introdusa, "+");
}

void minus(void)
{
  //adaugam cate un caracter -
  strcat(hidden,"*");
  strcat(parola_introdusa, "-");
}

void ok(void)
{
  check_pass();
}

void sterge(void)
{
  //stergem cate un caracter
  char aux1[5];
  strcpy(aux1,hidden);
  int i = strlen(aux1);
  strncpy(hidden, aux1, i-1);
  hidden[i - 1] = '\0';

  //stergem cate un caracter
  char aux2[5];
  strcpy(aux2,parola_introdusa);
  int j = strlen(aux2);
  strncpy(parola_introdusa, aux2, j-1);
  parola_introdusa[j - 1] = '\0';
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
//----------------------------Verificare parola----------------------------------------//
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

void check_pass()
{
  if (strcmp(parola, parola_introdusa) == 0) //daca parola buna
  {
  blocat = false;
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("PAROLA CORECTA!");
  lcd.setCursor(0,1);
  lcd.print("    WELCOME");
  delay(2000);
  
  }
  else //daca nu
  {
    //golim cei 2 char arrays
    memset(hidden, 0, sizeof hidden);
    memset(parola_introdusa, 0, sizeof parola_introdusa);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("PAROLA GRESITA! INCEARCA DIN NOU");
    lcd.setCursor(0,1);
    lcd.print("INCEARCA DIN NOU");
    delay(2000);
    scroll_menu_b = MENU_BLOCAT;
    current_menu_b = MENU_BLOCAT;
  }
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
//----------------------------STATE MACHINE----------------------------------------//
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

state_machine_handler_t* sm[MENU_MAX_NUM][EV_MAX_NUM] =
{ //events: OK , CANCEL , NEXT, PREV
  {enter_menu, go_home, go_next, go_prev},                          // MENU_MAIN
  {enter_menu_sub, go_home, no_action, no_action},                  // MENU_MESAJE
  {go_home, go_home, no_action, no_action},                         // MENU_CONTROL
  {go_home, go_home, no_action, no_action},                         // MENU_TEMPERATURA
  {enter_menu_subin, go_home, no_action, no_action},                // MENU_INUNDATII
}; 

state_machine_handler_t_sub* sm_sub[SUB_MENU_MAX_NUM][EV_MAX_NUM] =
{ //events: OK , CANCEL , NEXT, PREV
  {enter_menu_subb, go_back_menu_sub, go_next_sub, go_prev_sub},                // SUB_MENU_MAIN
  {enter_menu_subnec, go_home_sub, go_home_sub, go_home_sub},                   // MENU_NECITITE
  {enter_menu_subcit, go_home_sub, go_home_sub, go_home_sub},                   // MENU_CITITE
  {confirm, go_home_sub, go_home_sub, go_home_sub},                             // MENU_STERGERE
}; 

state_machine_handler_t_nec* sm_nec[NECITIT_SUB_MENU_MAX_NUM][EV_MAX_NUM] =
{ //events: OK , CANCEL , NEXT, PREV
  {no_action, go_back_menu_nec, go_next_sub_nec, go_prev_sub_nec},                // NECITITE_SUB_MENU_MAIN
  {no_action, no_action, no_action, no_action},                    
  {no_action, no_action, no_action, no_action},
  {no_action, no_action, no_action, no_action},                     
  {no_action, no_action, no_action, no_action},                     
  {no_action, no_action, no_action, no_action},                  
  {no_action, no_action, no_action, no_action},                   
  {no_action, no_action, no_action, no_action},                   
  {no_action, no_action, no_action, no_action},         
  {no_action, no_action, no_action, no_action},               
  {no_action, no_action, no_action, no_action},               
};

state_machine_handler_t_cit* sm_cit[CITIT_SUB_MENU_MAX_NUM][EV_MAX_NUM] =
{ //events: OK , CANCEL , NEXT, PREV
  {no_action, go_back_menu_cit, go_next_sub_cit, go_prev_sub_cit},                // CITITE_SUB_MENU_MAIN
  {no_action, no_action, no_action, no_action},                    
  {no_action, no_action, no_action, no_action},
  {no_action, no_action, no_action, no_action},                     
  {no_action, no_action, no_action, no_action},                     
  {no_action, no_action, no_action, no_action},                  
  {no_action, no_action, no_action, no_action},                   
  {no_action, no_action, no_action, no_action},                   
  {no_action, no_action, no_action, no_action},         
  {no_action, no_action, no_action, no_action},               
  {no_action, no_action, no_action, no_action}, 
};


state_machine_handler_t_in* sm_in[INUNDATII_SUB_MENU_MAX_NUM][EV_MAX_NUM] =
{ //events: OK , CANCEL , NEXT, PREV
  {no_action, go_back_menu_in, go_next_sub_in, go_prev_sub_in},                // INUNDATII_SUB_MENU_MAIN
  {no_action, no_action, no_action, no_action},                    
  {no_action, no_action, no_action, no_action},
  {no_action, no_action, no_action, no_action},                     
  {no_action, no_action, no_action, no_action},                     
  {no_action, no_action, no_action, no_action},                  
  {no_action, no_action, no_action, no_action},                   
  {no_action, no_action, no_action, no_action},                   
  {no_action, no_action, no_action, no_action},         
  {no_action, no_action, no_action, no_action},               
  {no_action, no_action, no_action, no_action}, 
};


state_machine_handler_t_b* sm_b[MENU_MAX_NUM_b][EV_MAX_NUM] =
{ //events: OK , CANCEL , NEXT, PREV
  {bloc, bloc, bloc, bloc},                       // MENU_BLOCAT
  {ok, sterge, plus, minus},                      // MENU_DEBLOCAT
};

void state_machine_b(enum Menus_b menu, enum Buttons button)
{
  sm_b[menu][button]();
}

void state_machine_sub(enum Sub_Menus menu, enum Buttons button)
{
  sm_sub[menu][button]();
}

void state_machine_nec(enum Necitit_Sub_Menus menu, enum Buttons button)
{
  sm_nec[menu][button]();
}

void state_machine_cit(enum Citit_Sub_Menus menu, enum Buttons button)
{
  sm_cit[menu][button]();
}

void state_machine_in(enum Inundatii_Sub_Menus menu, enum Buttons button)
{
  sm_in[menu][button]();
}

void state_machine(enum Menus menu, enum Buttons button)
{
  sm[menu][button]();
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
//----------------------------BUTTONS----------------------------------------//
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

int evaluateButton(int x) {
  int result = 0;
  if (x < 50) {
    result = 1; // right
  } else if (x < 195) {
    result = 2; // up
  } else if (x < 380) {
    result = 3; // down
  } else if (x < 790) {
    result = 4; // left
  }
  return result;
}

Buttons GetButtons(void)
{
  enum Buttons ret_val = EV_NONE;
  int readKey = analogRead(0);
  if (readKey < 790) {
      delay(100);
      readKey = analogRead(0);
    }
 int button = evaluateButton(readKey);
 
  if (button == 1)
  {
    ret_val = EV_OK;
  }
  else if (button == 4)
  {
    ret_val = EV_CANCEL;
  }
  else if (button == 3)
  {
    ret_val = EV_NEXT;
  }
  else if (button == 2)
  {
    ret_val = EV_PREV;
  }
  return ret_val;
}

void adc_init()
{
  ADCSRA |= ((1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0));
  ADMUX  |= (1<<REFS0);
  ADCSRA |= (1<<ADEN);
  ADCSRA |= (1<<ADSC);
}

uint16_t read_adc(uint8_t channel)
{
  ADMUX &= 0xf0;
  ADMUX |= channel;
  ADCSRA |= (1<<ADSC);
  while(ADCSRA & (1<<ADSC));
  return ADC;
}

double read_temp() //read and return temperature
{                                                                                                               
 double voltage=0;                      
 double Temperature=0;                     
 double temperatureCelsius=0; int reading;
 reading=read_adc(1);
 voltage=reading*5.0; voltage/=1024.0; 
 temperatureCelsius=(voltage-0.5)*100;
 return temperatureCelsius;
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
//----------------------------SETUP AND LOOP----------------------------------------//
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

void setup() {
  adc_init();
  lcd.begin(16,2);
  initializare_char();
}

void loop() {
      temperatura = read_temp(); //citim temperatura de pe senzor
      lcd.clear();
    volatile Buttons event = GetButtons();
    if((blocat == true) && (event != EV_NONE))
    {
      state_machine_b(current_menu_b, event);
    }
    else if ((submenu == false) && (necitit_sub_menu == false) && (citit_sub_menu == false) && (inundatii_sub_menu == false) && ((event != EV_NONE)))
    {
      state_machine(current_menu, event);
    }
    else if ((submenu == true) && (necitit_sub_menu == false) && (citit_sub_menu == false) && (inundatii_sub_menu == false) && ((event != EV_NONE)))
    {
      state_machine_sub(current_menu_sub, event);
    }
    else if ((necitit_sub_menu == true) && (submenu == false) && (citit_sub_menu == false) && (inundatii_sub_menu == false) && ((event != EV_NONE)))
    {
      state_machine_nec(current_menu_nec, event);
    }
    else if ((citit_sub_menu == true) && (submenu == false) && (necitit_sub_menu == false) && (inundatii_sub_menu == false) && ((event != EV_NONE)))
    {
      state_machine_cit(current_menu_cit, event);
    }
    else if ((inundatii_sub_menu == true) && (submenu == false) && (necitit_sub_menu == false) && (citit_sub_menu == false) && ((event != EV_NONE)))
    {
      state_machine_in(current_menu_in, event);
    }
    if(blocat)
    {
    print_menu_b(scroll_menu_b);
    }
    else if ((submenu == true) && (necitit_sub_menu == false) && (citit_sub_menu == false) && (inundatii_sub_menu == false))
    {
    print_menu_sub(scroll_menu_sub);
    }
    else if ((necitit_sub_menu == true) && (submenu == false) && (citit_sub_menu == false) && (inundatii_sub_menu == false))
    {
    print_menu_nec(scroll_menu_nec);
    }
    else if ((citit_sub_menu == true) && (submenu == false) && (necitit_sub_menu == false) && (inundatii_sub_menu == false))
    {
    print_menu_cit(scroll_menu_cit);
    }
    else if((inundatii_sub_menu == true) && (submenu == false) && (necitit_sub_menu == false) && (citit_sub_menu == false))
    {
      print_menu_in(scroll_menu_in);
    }
    else
    print_menu(scroll_menu);
    delay(1000);
}
 
