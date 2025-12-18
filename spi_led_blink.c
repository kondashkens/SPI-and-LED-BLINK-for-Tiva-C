#include "TM4C123GH6PM.h"
#define LED_RED (1U << 1)
#define LED_BLUE (1U << 2)
#define LED_GREEN (1U << 3)
#define TOGGLE_RED_LED (GPIOF->DATA ^= LED_RED)

void Timer0_Init(void);
void redLED_init(void);
void Timer1_Init(void);
void SPI1_init(void);
void SPI1_Write(unsigned int data);
int GLOBAL_DATA_SPI_WRITING = 0;
int BOOLEAN_GLOBAL_TIMER0_TRIGGER_FLAG = 1;
int BOOLEAN_GLOBAL_TIMER1_TRIGGER_FLAG = 1;

int main() {
  redLED_init();
  SPI1_init();
  Timer0_Init();
  Timer1_Init();

  unsigned int counter = 0;

  while (1) {
    if (BOOLEAN_GLOBAL_TIMER0_TRIGGER_FLAG == 0) {
      TOGGLE_RED_LED;
      BOOLEAN_GLOBAL_TIMER0_TRIGGER_FLAG = 1;
    }

    if (BOOLEAN_GLOBAL_TIMER1_TRIGGER_FLAG == 1) {
      SPI1_Write(counter);
      counter++;
      if (counter > 255)
        counter = 0;
      BOOLEAN_GLOBAL_TIMER1_TRIGGER_FLAG = 0;
    }
  }
}

void redLED_init() {
  SYSCTL->RCGCGPIO |= (1U << 5);                // enable clock for GPIOF
  GPIOF->DEN |= LED_RED | LED_BLUE | LED_GREEN; // enable digital function
  GPIOF->DIR |= LED_RED | LED_BLUE | LED_GREEN; // set as output
}

void Timer0_Init(void) {
  SYSCTL->RCGCTIMER |= (1U << 0); // general registry
  TIMER0->CTL &= ~(1U << 0);
  TIMER0->CFG = 0x00000000;
  TIMER0->TAMR = 0x00000002;
  TIMER0->TAILR = 16000000 / 2 - 1;
  TIMER0->IMR |= (1U << 0);
  NVIC->ISER[0] |= (1 << 19);
  TIMER0->CTL |= (1U << 0);
}

void TIMER0A_Handler(void) {
  TOGGLE_RED_LED;
  BOOLEAN_GLOBAL_TIMER0_TRIGGER_FLAG = 1;
  TIMER0->ICR |= (1U << 0);
}

void SPI1_Write(unsigned int data) {
  while ((SSI1->SR & 2) == 0)
    ;
  SSI1->DR = data;
  while (SSI1->SR & 0x10)
    ;
}

void SPI1_init(void) {
  SYSCTL->RCGCSSI |= (1 << 1);
  SYSCTL->RCGCGPIO |= (1 << 3);
  SYSCTL->RCGCGPIO |= (1 << 5);

  GPIOD->AMSEL &= ~0x0B;
  GPIOD->DEN |= 0x0B;
  GPIOD->AFSEL |= 0x0B;
  GPIOD->PCTL &= ~0x0000F0FF;
  GPIOD->PCTL |= 0x00002022;

  SSI1->CR1 = 0;
  SSI1->CC = 0;
  SSI1->CPSR = 4;
  SSI1->CR0 = 0x00007;
  SSI1->CR1 |= 2;
}

void Timer1_Init(void) {
  SYSCTL->RCGCTIMER |= (1U << 1);
  TIMER1->CTL = 0;
  TIMER1->CFG = 0x0;
  TIMER1->TAMR = 0x02;
  TIMER1->TAILR = 8000 - 1;
  TIMER1->IMR |= (1U << 0);
  NVIC->ISER[0] |= (1 << 21);
  TIMER1->CTL |= (1 << 0);
}

void TIMER1A_Handler(void) {
  BOOLEAN_GLOBAL_TIMER1_TRIGGER_FLAG = 1;
  TIMER1->ICR |= (1U << 0);
}

