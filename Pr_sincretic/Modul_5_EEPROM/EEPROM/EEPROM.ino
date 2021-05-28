#define FCPU 16000000
#define FOSC 16000000 // Clock Speed
#define BAUD 9600
#define MYUBRR FOSC/16/BAUD-1

#define MSG_MAX_LEN 30
#define FLOOD_COUNT_DETECTION_CYCLES 10

#include <LiquidCrystal.h>
#include <EEPROM.h>
#include <avr/io.h>

int volatile time_count = 0;

const int rs = 12, en = 11, d4 = 4, d5 = 3, d6 = 8, d7 = 5;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);//Adresa, coloane, linii

int pwm_val[3][2];
bool pwm_parsing = false;
int pwm_pos = 0;

char msg_val[MSG_MAX_LEN + 1];
char buf[] = "!INUNDATIE!";
bool msg_parsing = false;
int msg_pos = 0;
volatile bool msg_rec_completed = false;
volatile bool refresh_needed = false;

static char row_up[15] = "PS 2";
static char row_down[17] = "Semestrul II";

volatile bool flood_detected = false;
volatile int flood_count = 0;

typedef struct mesaje_nvm {
  char mesaj[MSG_MAX_LEN + 1];
  bool stare_citire;
  bool valid;
} mesaje_nvm_t;

mesaje_nvm_t mesaje_ram[10];

typedef struct inundatii_nvm {
  char mesaj[MSG_MAX_LEN + 1];
  bool valid;
} inundatii_nvm_t;

inundatii_nvm_t inundatii[10];

int j = 0;

void setup() {
  DDRB |= 1 << PB5;

  OCR2A = 65535; // 100 ms
  TCCR2A |= 1 << WGM21;
  TCCR2B |= (1 << CS22) | (1 << CS20) | (1 << CS21);
  TIMSK2 = (1 << OCIE2A);

  USART_Init(MYUBRR);
  PWM_Init();
  adc_init();
  Disp_Init();
  EINT_Init();
  //Clear_EEPROM_Mesaje();
  Read_EEPROM_Inundatii ();
  Read_EEPROM_Mesaje();
  Afiseaza_Mesaje();

  sei();
}

void loop() {
  if (true == flood_detected)
  {
    //cli();
    Salveaza_inundatie(buf, sizeof(buf));
    flood_detected = false;
  }
  else if (true == msg_rec_completed)
  {
    
    memset(row_up, 0, sizeof(row_up));
    memset(row_down, 0, sizeof(row_down));
    memcpy(row_up, &msg_val[0], 14);
    memcpy(row_down, &msg_val[14], 16);

    msg_rec_completed = false;
    Salveaza_Mesaj(msg_val, sizeof(msg_val));
    memset(msg_val, 0, sizeof(msg_val));

  }
  /*if (true == refresh_needed)
  {
    Refresh_Display();
  }
  */
}

void USART_Init(unsigned int ubrr)
{
  /* Set baud rate */
  UBRR0H = (unsigned char)(ubrr >> 8);
  UBRR0L = (unsigned char)ubrr;
  /* Enable receiver and transmitter */
  UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
  /* Set frame format: 8data, 2stop bit */
  UCSR0C = (1 << USBS0) | (3 << UCSZ00);
}

void PWM_Init(void)
{
  // 1. LED ca iesire.
  DDRB |= 1 << PB2; //red
  DDRB |= 1 << PB1; //green
  DDRD |= 1 << PD6; //blue

  // 2. Alegem Timer PWM CS
  TCCR0B = (1 << CS02);
  TCCR1B = (1 << CS12);

  // 3. Mod FAST PWM
  TCCR0A |= (1 << WGM00) | (1 << WGM01);
  TCCR1A |= 1 << WGM10;
  TCCR1B |= 1 << WGM12;

  // 4. Setam OCR output mode
  TCCR0A |= (1 << COM0A1);
  TCCR1A |= (1 << COM1A1);
  TCCR1A |= (1 << COM1B1);


  // 5. Test valori OCR   // P 22 44 98 W
  OCR1B = 0;    // Red
  OCR1A = 0;    // Green
  OCR0A = 0;    // Blue
}

void adc_init()
{
  ADCSRA |= ((1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0));
  ADMUX  |= (1 << REFS0);
  ADCSRA |= (1 << ADEN);
  ADCSRA |= (1 << ADSC);
}

void Disp_Init(void)
{
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("Sincretic 2021");
}

void EINT_Init(void)
{
  PORTD |= (1 << PORTD2); // Rezistor de tip pull-up activat
  EICRA = 0;
  EIMSK |= 1 << INT0;
}

ISR(INT0_vect)
{
  flood_count++;
  if (flood_count > FLOOD_COUNT_DETECTION_CYCLES)
  {
    flood_detected = true;
    flood_count = 0;
    // Avem cotinuintate intre PD2 si GND, posibil eveniment inundatie
  }
}

ISR(USART_RX_vect)
{
  char c = USART_Receive();
  if (c == '`')
  {
    PORTB |= 1 << PB5;
  }
  else if (c == '~')
  {
    PORTB &= ~(1 << PB5);
  }
  else if (c == '$')
  {
    // incepem parsare PWM
    pwm_parsing = true;
    pwm_pos = 0;
  }
  else if (c == '&')
  {
    // incheiem parsarea PWM
    pwm_parsing = false;

    OCR1B = pwm_val[0][0] * 10 + pwm_val[0][1];    // Red
    OCR1A = pwm_val[1][0] * 10 + pwm_val[1][1];    // Green
    OCR0A = pwm_val[2][0] * 10 + pwm_val[2][1];    // Blue
  }
  else if (c == '#')
  {
    msg_parsing = true;
    msg_pos = 0;
  }
  else if (c == '^')
  {
    msg_parsing = false;
    msg_rec_completed = true;
  }
  else
  {
    if (pwm_parsing == true)
    {
      // salvam valorile pwm...
      pwm_val[pwm_pos / 2][pwm_pos % 2] = c - '0';
      pwm_pos++;
    }
    if (msg_parsing == true)
    {
      if (('\n' != c) && (msg_pos < MSG_MAX_LEN))
      {
        msg_val[msg_pos++] = c;
      }
    }
  }

}

ISR(TIMER2_COMPA_vect)
{
  time_count++;
  if (!(time_count % 150))
  {
    refresh_needed = true;
  }

  if (!(time_count % 500))
  {
    Transmitere_Temperatura();
  }

  if (!(time_count % 50)) {
    Transmitere_water();
  }
}

void Read_EEPROM_Inundatii ()
{
  EEPROM.get(1, inundatii);
}

void Write_EEPROM_Inundatii()
{
  EEPROM.put(1, inundatii);
}

void Read_EEPROM_Mesaje ()
{
  EEPROM.get(0, mesaje_ram);
}

void Write_EEPROM_Mesaje()
{
  EEPROM.put(0, mesaje_ram);
}

void Clear_EEPROM_Mesaje()
{
  for (int i = 0 ; i < EEPROM.length() ; i++) {
    EEPROM.write(i, 0);
  }
}

int Salveaza_inundatie(char* msg, int len)
{
  int i = 0;
  for (i = 0; i < 10; i++)
  {
    if (inundatii[i].valid != true)
    {
      break;
    }
  }

  if (i==10)
 {
  i=0;
 }

  //Am gasit un loc pentru salvarea mesajului.
  memcpy(inundatii[i].mesaj, msg, MSG_MAX_LEN + 1);
  inundatii[i].valid = true;
  Write_EEPROM_Inundatii();
  Afiseaza_Inundatii();
  return 0;
}

int Salveaza_Mesaj(char* msg, int len)
{
  int i = 0;
  for (i = 0; i < 10; i++)
  {
    if (mesaje_ram[i].valid != true)
    {
      break;
    }
  }

  if (i == 10)
  {
    mesaje_ram[j].valid = false;
    j++;
    if (j > 10)
      j = 0;
    return Salveaza_Mesaj(msg_val, sizeof(msg_val));
  }

  //Am gasit un loc pentru salvarea mesajului.
  memcpy(mesaje_ram[i].mesaj, msg, MSG_MAX_LEN + 1);
  mesaje_ram[i].valid = true;
  mesaje_ram[i].stare_citire = false;
  Write_EEPROM_Mesaje();
  Afiseaza_Mesaje();
  return 0;
}

int Mesaje_necitite(void)
{
  int nr = 0;
  for ( int j = 0; j < 10; j++)
  {
    if (!(mesaje_ram[j].stare_citire))
    {
      nr++;
    }
  }
 return nr;
}

int Mesaje_citite(void)
{
  int nr = 0;
  for ( int j = 0; j < 10; j++)
  {
    if (mesaje_ram[j].stare_citire)
    {
      nr++;
    }
  }
 return nr;
}

void Afiseaza_Inundatii()
{
  char buff[MSG_MAX_LEN + 1];
  for ( int j = 0; j < 10; j++)
  {
    memset(buff, 0, sizeof(buff));
     if (inundatii[j].valid == true)
    {
      snprintf(buff, MSG_MAX_LEN + 1, "%d - %s\n", j, inundatii[j].mesaj);
    }
    else
    {
      snprintf(buff, MSG_MAX_LEN + 1, "%d - GOL\n", j);
    }
    for (int i = 0; i < strlen(buf); i++)
    {
      USART_Transmit(buff[i]);
    }
  }
}

void Afiseaza_Mesaje()
{
  char buf[MSG_MAX_LEN + 1];
  for ( int j = 0; j < 10; j++)
  {
    memset(buf, 0, sizeof(buf));
    snprintf(buf, MSG_MAX_LEN + 1, "%d - %s\n", j, inundatii[j]);
    for (int i = 0; i < strlen(buf); i++)
    {
      USART_Transmit(buf[i]);
    }
  }
}

unsigned char USART_Receive(void)
{
  /* Wait for data to be received */
  while (!(UCSR0A & (1 << RXC0)))
    ;
  /* Get and return received data from buffer */
  return UDR0;
}

void USART_Transmit(unsigned char data)
{
  /* Wait for empty transmit buffer */
  while (!(UCSR0A & (1 << UDRE0)))
    ;
  /* Put data into buffer, sends the data */
  UDR0 = data;
}

uint16_t read_adc(uint8_t channel)
{
  ADMUX &= 0xf0;
  ADMUX |= channel;
  ADCSRA |= (1 << ADSC);
  while (ADCSRA & (1 << ADSC));
  return ADC;
}

double read_water() //read and return temperature
{
  int reading;
  reading = read_adc(3);
  return reading;
}

void Transmitere_water() //read and return temperature
{
  int water = read_water();
  if (water < 100)
  {
    flood_detected = false;
  }
  /*else if (water > 100 && water < 450)
  {
    flood_detected = false;
  }
  */
  else if (water > 250) {
    flood_count++;
    if (flood_count > FLOOD_COUNT_DETECTION_CYCLES)
    {
      flood_detected = true;
      flood_count = 0;
      // Avem cotinuintate intre PD2 si GND, posibil eveniment inundatie
    }
  }
}

void Transmitere_Temperatura() //read and return temperature
{
  float voltage = 0;
  float temperatureCelsius = 0; int reading;
  reading = read_adc(1);
  voltage = reading * 5.0; voltage /= 1024.0;
  temperatureCelsius = (voltage - 0.5) * 100;

  int parte_intreaga, parte_zecimala;
  int int_temperatureCelsius = temperatureCelsius * 100;

  parte_intreaga = int_temperatureCelsius / 100;
  parte_zecimala = int_temperatureCelsius % 100;

  char buf[50];
  memset(buf, 0, sizeof(buf));
  sprintf(buf, "T=%d.%d\n", parte_intreaga, parte_zecimala);
  //sprintf(buf, "T=%f\n", temperatureCelsius); aparent nu mere %f pe serial nuj de ce

  for (int i = 0; i < strlen(buf); i++)
  {
    USART_Transmit(buf[i]);
  }
}

void Refresh_Display()
{
  cli();
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print(row_up);
  lcd.setCursor(0, 1);
  lcd.print(row_down);
  refresh_needed = false;
  sei();
}
