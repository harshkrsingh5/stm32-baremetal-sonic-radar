#include <stdint.h>
uint32_t volatile *pClkctrlreg   = (uint32_t *)0x40021018;
uint32_t volatile *pPortAModeReg = (uint32_t *)0x40010800; 
uint32_t volatile *pPortACRH     = (uint32_t *)0x40010804;
uint32_t volatile *pPortAOutReg  = (uint32_t *)0x4001080C; 
uint32_t volatile *pPortAInReg   = (uint32_t *)0x40010808; 
uint32_t volatile *USART1_SR     = (uint32_t *)0x40013800;
uint32_t volatile *USART1_DR     = (uint32_t *)0x40013804;
uint32_t volatile *USART1_BRR    = (uint32_t *)0x40013808;
uint32_t volatile *USART1_CR1    = (uint32_t *)0x4001380C;
uint16_t volatile *SPI_CR1       = (uint16_t *)0x40013000;
uint16_t volatile *SPI_SR        = (uint16_t *)0x40013008;
uint16_t volatile *SPI_DR        = (uint16_t *)0x4001300C;
void delay_us(uint32_t us) {
    while(us--) {
        __asm("nop"); __asm("nop"); __asm("nop");
    }
}
void UART1_Init(void) {
    *pClkctrlreg |= (1 << 2) | (1 << 14);
    *pPortACRH &= ~(0xF << 4);
    *pPortACRH |=  (0xB << 4);
    *USART1_BRR = 0x341;
    *USART1_CR1 |= (1 << 3) | (1 << 13);
}
void UART1_WriteChar(char ch) {
    while(!(*USART1_SR & (1 << 7))); 
    *USART1_DR = ch;                 
}
void UART1_PrintString(char *str) {
    while(*str) {
        UART1_WriteChar(*str++);     
    }
}
void SPI1_Init(void) {
    *pClkctrlreg |= (1 << 2) | (1 << 12);
    *pPortAModeReg &= ~((0xF << 16) | (0xF << 20) | (0xF << 24) | (0xF << 28) | (0xF << 12));
    *pPortAModeReg |=  ((0x3 << 16) | (0xB << 20) | (0x4 << 24) | (0xB << 28) | (0x3 << 12));
    *SPI_CR1 |= (1 << 2) | (7 << 3) | (1 << 9) | (1 << 8);
    *SPI_CR1 |= (1 << 6);
    *pPortAOutReg |= (1 << 4);
}
uint8_t SPI1_TransmitReceive(uint8_t data) {
     while(!(*SPI_SR & (1 << 1))); 
     *SPI_DR = data;               
     while(!(*SPI_SR & (1 << 0))); 
     return (uint8_t)(*SPI_DR);    
}
void HCSR04_Init(void) {
    *pPortAModeReg &= ~((0xF << 4) | (0xF << 8));
    *pPortAModeReg |=  ((0x3 << 4) | (0x4 << 8));
    *pPortAOutReg &= ~(1 << 1);
}

uint32_t HCSR04_GetDistance(void) {
    uint32_t echo_timer = 0;
    *pPortAOutReg |= (1 << 1); 
    delay_us(10);
    *pPortAOutReg &= ~(1 << 1);
    while(!(*pPortAInReg & (1 << 2)));
    while(*pPortAInReg & (1 << 2)) {
        echo_timer++;
        delay_us(1);
        if(echo_timer > 30000) break;
    }
    return (echo_timer / 44);
}
void UART1_PrintDistance(uint32_t distance) {
    char buffer[16];
    int i = 0;
    if (distance == 0) {
        buffer[i++] = '0';
    } else {
        uint32_t temp = distance;
        char reverse_buf[10];
        int j = 0;
        while (temp > 0) {
            reverse_buf[j++] = (temp % 10) + '0';
            temp /= 10;
        }
        while (j > 0) {
            buffer[i++] = reverse_buf[--j];
        }
    }
    buffer[i++] = ' '; buffer[i++] = 'c'; buffer[i++] = 'm';
    buffer[i++] = '\r'; buffer[i++] = '\n'; buffer[i++] = '\0'; 
    UART1_PrintString(buffer);
}
int main(void) {
    UART1_Init();
    SPI1_Init();
    HCSR04_Init();

    UART1_PrintString("--- Bare-Metal Sonic Radar Online ---\r\n");

    uint32_t current_distance = 0;
    uint8_t led_pattern = 0;

    while(1) {
        current_distance = HCSR04_GetDistance();
        UART1_PrintString("Distance: ");
        UART1_PrintDistance(current_distance);
        if(current_distance > 40) {
            led_pattern = 0x01; 
        }
        else if(current_distance <= 40 && current_distance > 25) {
            led_pattern = 0x0F;
        }
        else if(current_distance <= 25 && current_distance > 10) {
            led_pattern = 0x7F;
        }
        else if(current_distance <= 10) {
            led_pattern = 0xFF;
        }
        *pPortAOutReg &= ~(1 << 4);         
        SPI1_TransmitReceive(led_pattern);  
        *pPortAOutReg |= (1 << 4);         
        for(volatile int i = 0; i < 120000; i++);
    }
}
