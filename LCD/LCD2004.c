

#include<stdint.h>

#define RCC_APB2EN (*(volatile uint32_t *)0x40021018) //Address of the reg to enable PORTA
#define GPIOB_CRH  (*(volatile uint32_t *)0x40010C04) //address of the reg to set the pin 0 as input
#define GPIOB_CRL  (*(volatile uint32_t *)0x40010C00)  //Address of the reg to enable Pin 5 of Port B
#define GPIOB_ODR  (*(volatile uint32_t *)0x40010C0C)  //address of the reg to set the pin 5
#define GPIOA_CRL  (*(volatile uint32_t *)0x40010800) //address of GPIOA_CRL reg, to set I/0 direction
#define GPIOA_IDR  (*(volatile uint32_t *)0x40010808) //address of the input reg of port A
#define GPIOA_ODR  (*(volatile uint32_t *)0x4001080C) //address of the outut reg of port A

#define AFIO_MAPR (*(volatile uint32_t *)0x40010004)

#define Function_Set          0x38
#define DisplayON             0x0F
#define Clear                 0x01
#define EntryMode             0x06
#define _2ndLine              0xc0

void int_to_str(uint32_t num, char *str);
void delay(volatile uint32_t time);
void Init_LCD(void);
void send_cmd(uint8_t cmd);
void send_data(uint8_t data);
void send_string(char *Str);
void delay_Short(volatile uint32_t time);
void Trigger(void);
uint32_t Pulse();
uint32_t measureDist();

// To set an bit A,    |= (1<<A)
// To reset an bit A, &= ~(1<<A)
int main(void)
{	delay(500);
	 // Disable JTAG, keep SWD 
	RCC_APB2EN |= (1 << 0);  // Bit 0 = AFIOEN
	AFIO_MAPR |= (1 << 26);

	// Enable GPIOA and GPIOB clocks
	RCC_APB2EN |= (1<<2)|(1<<3);

	//set lower 8 bits as outputs and pulled up For the LCD
	GPIOB_CRL = 0x11111111;

	GPIOA_CRL &=~(0xF<< 0);//Clear
	GPIOA_CRL |= (0x01<<0);//PA0 is output (Trig)
	GPIOA_CRL &=~(0xF<< 4);//clear
	GPIOA_CRL |= (0x04<<4);//PA1 is floating input (Echo)


	//set PB8 and PB9 as outputs;
	GPIOB_CRH &= ~(0xFF << 0);//clear
	GPIOB_CRH |=  (0x11 << 0);//set

	char dist[10];

	Init_LCD();
	send_string("measuring distance");
	delay(1000);
	send_cmd(Clear);


  while (1)
  {
	  uint32_t distance = measureDist();
	  int_to_str(distance,dist);
	  send_cmd(Clear);
	  send_string("Distance: ");
	  send_string(dist);
	  send_string(" cm");
	  delay(1000);
  }
}

void delay_Short(volatile uint32_t time)
{
    while (time--)
    {
        for (volatile int i = 0; i < 10; i++);
    }
}


void delay(volatile uint32_t time)
{
    while (time--)
    {
        for (volatile int i = 0; i < 1000; i++);
    }
}

void send_cmd(uint8_t cmd)
{
	GPIOB_ODR &= ~(1<<8); //RS = 0

	GPIOB_ODR = (GPIOB_ODR & 0xFF00)|cmd; // Send the cmd

	GPIOB_ODR |= (1<<9); //set Enable
	delay(2);
	GPIOB_ODR &= ~(1<<9); //Reset Enable
	delay(20);

}

void send_data(uint8_t data)
{
	GPIOB_ODR |= (1<<8); //RS = 1

	GPIOB_ODR = (GPIOB_ODR & 0xFF00)|data; // Send the data

	GPIOB_ODR |= (1<<9); //set Enable
	delay(2);
	GPIOB_ODR &= ~(1<<9); //Reset Enable
	delay(20);

}

void Init_LCD()
{
	delay(500);
	send_cmd(Function_Set);
	delay(1000);
	send_cmd(DisplayON);
	delay(1000);
	send_cmd(Clear);
	delay(1000);
	send_cmd(EntryMode);

}

void send_string(char *str)
{
	while(*str)
	{
		send_data(*str++);
	}
}

void Trigger(void)
{
	GPIOA_ODR |= (1<<0);
	delay_Short(1);
	GPIOA_ODR&=~(1<<0);
}

uint32_t Pulse()
{
	uint32_t time = 0;

	while(!(GPIOA_IDR & (1<<1)));

	while(GPIOA_IDR & (1<<1))
	{
		time++;
		delay_Short(1);
	}
	return time;
}
uint32_t measureDist()
{
    uint32_t time, distance;
    Trigger();

    // Wait for Echo to go high
    while (!(GPIOA_IDR & (1<<1)));

    // Measure how long Echo stays high
    for (time = 0; (GPIOA_IDR & (1<<1)) && time < 30000; time++);

    distance = (time * 34) / 2000;  // Convert to cm
    return distance;
}


void int_to_str(uint32_t num, char *str) {
    int i = 0;
    if (num == 0) {
        str[i++] = '0';
    } else {
        while (num != 0) {
            str[i++] = (num % 10) + '0';
            num /= 10;
        }
    }
    str[i] = '\0';

    // Reverse the string
    for (int j = 0; j < i / 2; j++) {
        char temp = str[j];
        str[j] = str[i - j - 1];
        str[i - j - 1] = temp;
    }
}

