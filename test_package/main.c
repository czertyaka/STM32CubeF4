#include "stm32f411xe.h"
#include <stdint.h>
#include <stddef.h>

void __libc_init_array(void) {}

#define ASSERT(X)       \
    if (!(X)) {     \
        *(int*)(0x00) = 0;    \
    }

int GPIO_init();
int SysTick_init();
int toggle_LED();
int update_SysTick_period(uint32_t msec);

int main(void) {
    ASSERT(GPIO_init() == 0);
    ASSERT(SysTick_init() == 0);
    while (1) {}
}

int GPIO_init() {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
    GPIOC->MODER |= GPIO_MODER_MODER13_0;
    GPIOC->OTYPER = 0;
    GPIOC->OSPEEDR = 0;
    return 0;
}

int SysTick_init() {
    SystemCoreClockUpdate();

    // enable systick exception, enable systick counter
    uint32_t systick_ctrl = SysTick->CTRL;
    systick_ctrl |= SysTick_CTRL_ENABLE_Msk;
    systick_ctrl |= SysTick_CTRL_TICKINT_Msk;
    SysTick->CTRL = systick_ctrl;

    // set reload value to 500 ms
    return update_SysTick_period(500);
}

int toggle_LED() {
    // toggle 13th bit of GPIOC_OD (PC13)
    GPIOC->ODR ^= GPIO_ODR_OD13;
    return 0;
}

int update_SysTick_period(uint32_t msec) {
    const uint32_t systick_freq = SystemCoreClock / 8; // Hz
    const uint32_t load = msec * systick_freq / 1000;
    if (load & ~SysTick_LOAD_RELOAD_Msk) {
        return -1;
    }
    SysTick->LOAD = load;
    return 0;
}

void SysTick_Handler(void) {
    ASSERT(toggle_LED() == 0);
    static const uint32_t periods[] = {
        400, 300, 200, 100, 200, 300, 400, 500
    };
    static size_t i = 0;
    ASSERT(i < sizeof(periods)/sizeof(periods[0]));
    ASSERT(update_SysTick_period(periods[i]) == 0);
    i = (i + 1) % (sizeof(periods)/sizeof(periods[0]));
}
