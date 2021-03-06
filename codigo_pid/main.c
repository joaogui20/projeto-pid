#include <16F877A.h>
#device adc=10

#fuses NOWDT      // No Watch Dog Timer
#fuses NOBROWNOUT // No brownout reset
#fuses NOLVP      // No low voltage prgming, B3(PIC16) or B5(PIC18) used for I/O

#use delay(crystal=20000000)

#use rs232(baud=9600,parity=N,xmit=PIN_C6,rcv=PIN_C7,bits=8,stream=Serial)

#include <mod_lcd.c>

#ifndef lcd_enable                                                             
   #define lcd_enable     PIN_E1      // pino enable do LCD
   #define lcd_rs         PIN_E2      // pino rs do LCD
   #define lcd_d4         PIN_D4      // pino de dados d4 do LCD
   #define lcd_d5         PIN_D5      // pino de dados d5 do LCD
   #define lcd_d6         PIN_D6      // pino de dados d6 do LCD
   #define lcd_d7         PIN_D7      // pino de dados d7 do LCD
#endif

int cont = 0;

int16 pwm = 0, tempRef = 0, tempUser = 0, tempLast = 0;
int16 PIDcontrol = 0;
int16 e = 0;
int16 proporcional = 0, integrar = 0, derivar = 0, kp = 1, ki = 1, kd = 1;
   
#INT_RTCC
void RTCC_isr(void){
   // 5000ms/0,0542ms = 92 ciclos
   // (92*valor)/1020 => valor*(92/1020) => valor*0.0901960784313
   pwm*0.090196;
   if(cont <= pwm){
      output_high(PIN_C5);
   } else {
      output_low(PIN_C5);
   }
   
   cont++;
   
   if(cont == 92){
      cont = 0;
   }
}

void main()
{
      setup_adc_ports(AN0_AN1_AN2_AN3_AN4);
      set_adc_channel(1);
      delay_us(10);
      
      lcd_ini();
      delay_ms(10);
      
      setup_adc(ADC_CLOCK_DIV_16);
      setup_timer_0(RTCC_INTERNAL|RTCC_DIV_1|RTCC_8_bit); // 51,2us overflow
      setup_timer_2(T2_DIV_BY_16,255,1); // 819us overflow, 819us interrupt
      
      setup_ccp1(CCP_PWM);
      set_pwm1_duty((int16)510);
      
      enable_interrupts(INT_RTCC);
      enable_interrupts(GLOBAL);
      
   while(TRUE)
   {
      // Entradas 0 - 1024
      set_adc_channel(2);
      delay_ms(10);
      tempRef = read_adc()/2;
      
      set_adc_channel(0);
      delay_ms(10);
      tempUser = read_adc();
      
      set_adc_channel(1);
      delay_ms(10);
      set_pwm1_duty(read_adc());
      
      tempUser = (tempUser*0.0469)+32;
      
      // Calculo do erro
      e = tempUser - tempRef;
      
      // Incremento do PID
      proporcional = e * kp;
      
      integrar += e * ki;
      
      derivar = (tempLast - tempRef) * kd;
      tempLast = tempRef;
      
      // Calculo PID
      PIDcontrol = proporcional + integrar + derivar;
      
      pwm = PIDcontrol;
      
      printf(lcd_escreve, "\f R: %Ld Temp: %Ld \n", tempUser, tempRef);
      printf(lcd_escreve, "\r Erro: %Ld \n", e);
      fprintf(Serial, "%Ld %Ld %Ld \n", tempUser, tempRef, e);
      delay_ms(10);
   }

}
