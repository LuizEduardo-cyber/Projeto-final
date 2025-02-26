#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "inc/ssd1306.h"
#include "neopixel.c"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "pico/multicore.h"




#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define OLED_ADDR 0x3C
#define MIC_CHANNEL 2
#define MIC_PIN (26 + MIC_CHANNEL)
#define SAMPLE_RATE 4000
#define FFT_SIZE 256
#define LED_PIN 7
#define LED_COUNT 25
#define ADC_MAX 4095.0
#define ADC_VREF 3.3
const uint BUTTON_A = 5;
#define BUZZER 10
static volatile uint32_t last_time = 0; //para fazer o debounce
volatile bool play_song = false;




static ssd1306_t ssd;
uint16_t adc_buffer[FFT_SIZE];

// Estrutura de notas musicais
typedef struct {
    char *note;
    float freq;
} Note;

Note notes[] = {
     {"RE", 609.00}, {"DO", 1156.00}, {"RE", 921.00},{"RE", 1531.00}, {"MI", 687.00},{"MI", 1718.00},{"MI", 1375.00}, {"FA", 750.00},  {"FA", 1296.00}, {"MI", 937.00},
     {"FA", 718.00},{"SOL", 812.00}, {"LA", 453.00}, {"SI", 1546.00}, 
     {"DO", 546.00},{"SI", 1031.00}, {"SI", 515.00}, {"DO", 1640.00}, 
     {"DO", 1093.00}
};

// Função para encontrar a nota mais próxima
const char* find_closest_note(float frequency) {
    float minDiff = 1000.0;
    const char* bestNote = "??";
    for (int i = 0; i < sizeof(notes)/sizeof(notes[0]); i++) {
        float diff = fabs(notes[i].freq - frequency);
        if (diff < minDiff) {
            minDiff = diff;
            bestNote = notes[i].note;
        }
    }
    return bestNote;
}

// FFT simplificada
void fft(float *real, float *imag) {
    int n = FFT_SIZE, half = n / 2, j = 0;
    for (int i = 1; i < n - 1; i++) {
        int bit = half;
        while (j >= bit) { j -= bit; bit /= 2; }
        j += bit;
        if (i < j) {
            float temp = real[i]; real[i] = real[j]; real[j] = temp;
            temp = imag[i]; imag[i] = imag[j]; imag[j] = temp;
        }
    }
    for (int len = 2; len <= n; len *= 2) {
        float angle = -2.0 * M_PI / len;
        float wReal = cos(angle), wImag = sin(angle);
        for (int i = 0; i < n; i += len) {
            float uReal = 1.0, uImag = 0.0;
            for (int j = 0; j < len / 2; j++) {
                int evenIndex = i + j, oddIndex = i + j + len / 2;
                float tReal = uReal * real[oddIndex] - uImag * imag[oddIndex];
                float tImag = uReal * imag[oddIndex] + uImag * real[oddIndex];

                real[oddIndex] = real[evenIndex] - tReal;
                imag[oddIndex] = imag[evenIndex] - tImag;
                real[evenIndex] += tReal;
                imag[evenIndex] += tImag;

                float tempReal = uReal;
                uReal = tempReal * wReal - uImag * wImag;
                uImag = tempReal * wImag + uImag * wReal;
            }
        }
    }
}

// Captura o sinal do microfone e aplica normalização
void sample_mic(float *real) {
    float sum = 0.0;
    for (int i = 0; i < FFT_SIZE; i++) {
        adc_select_input(MIC_CHANNEL);
        adc_buffer[i] = adc_read();
        real[i] = (adc_buffer[i] * ADC_VREF / ADC_MAX) - (ADC_VREF / 2);
        sum += real[i];
        sleep_us(1000000 / SAMPLE_RATE);
    }
    float avg = sum / FFT_SIZE;
    for (int i = 0; i < FFT_SIZE; i++) {
        real[i] -= avg; // Remove offset DC
        real[i] *= 0.5 * (1 - cos(2 * M_PI * i / (FFT_SIZE - 1))); // Aplica janela de Hann
    }
}

// Exibe informações no OLED
void display_on_oled(float freq, const char* note) {
    char buffer[20];
    snprintf(buffer, sizeof(buffer), "Freq: %.2f Hz", freq);
    ssd1306_fill(&ssd, false);
    ssd1306_draw_string(&ssd, buffer, 5, 5);
    ssd1306_draw_string(&ssd, note, 55, 35);
    ssd1306_send_data(&ssd);
}

// Atualiza a matriz de LEDs conforme a frequência
void update_led_matrix(float freq) {
    int level = (int)((freq - 400) / 65.0);
    if (level < 0) level = 0;
    if (level > LED_COUNT) level = LED_COUNT;
     printf("level : %d \n",level);
    npClear();
    for (float i = 0; i < level; i+=0.1) {
      if(i>=0 && i<=7){
        npSetLED(12, 0, 0, 80); // Acende apenas o centro.
      }
      if(i>=8 && i<=9){
        npSetLED(12, 0, 0, 120); // Acente o centro.

        // Primeiro anel.
        npSetLED(7, 0, 0, 80);
        npSetLED(11, 0, 0, 80);
        npSetLED(13, 0, 0, 80);
        npSetLED(17, 0, 0, 80);
      }
      if(i>=10 && i<=11){
        npSetLED(12, 60, 60, 0);

        // Primeiro anel.
        npSetLED(7, 0, 0, 120);
        npSetLED(11, 0, 0, 120);
        npSetLED(13, 0, 0, 120);
        npSetLED(17, 0, 0, 120);

        // Segundo anel.
        npSetLED(2, 0, 0, 80);
        npSetLED(6, 0, 0, 80);
        npSetLED(8, 0, 0, 80);
        npSetLED(10, 0, 0, 80);
        npSetLED(14, 0, 0, 80);
        npSetLED(16, 0, 0, 80);
        npSetLED(18, 0, 0, 80);
        npSetLED(22, 0, 0, 80);
      }
      if(i>=11){
           // Centro.
        npSetLED(12, 80, 0, 0);

        // Primeiro anel.
        npSetLED(7, 60, 60, 0);
        npSetLED(11, 60, 60, 0);
        npSetLED(13, 60, 60, 0);
        npSetLED(17, 60, 60, 0);

        // Segundo anel.
        npSetLED(2, 0, 0, 120);
        npSetLED(6, 0, 0, 120);
        npSetLED(8, 0, 0, 120);
        npSetLED(10, 0, 0, 120);
        npSetLED(14, 0, 0, 120);
        npSetLED(16, 0, 0, 120);
        npSetLED(18, 0, 0, 120);
        npSetLED(22, 0, 0, 120);

        // Terceiro anel.
        npSetLED(1, 0, 0, 80);
        npSetLED(3, 0, 0, 80);
        npSetLED(5, 0, 0, 80);
        npSetLED(9, 0, 0, 80);
        npSetLED(15, 0, 0, 80);
        npSetLED(19, 0, 0, 80);
        npSetLED(21, 0, 0, 80);
        npSetLED(23, 0, 0, 80);
      }
    }
    npWrite();
}
void gpio_irq_handler();

void play_music() {
  while (1) {
      if (play_song) {
          //Parabéns pra você
    play_tone(BUZZER,392,300); 
     sleep_ms(100);  
    play_tone(BUZZER,392,300); 
    sleep_ms(100);  
    play_tone(BUZZER,440,600); 
    sleep_ms(100);  
    play_tone(BUZZER,392,600); 
    sleep_ms(100);  
    play_tone(BUZZER,523,600); 
    sleep_ms(100);  
    play_tone(BUZZER,494,1200); 
    sleep_ms(200);  
    //Nesta data querida
    play_tone(BUZZER,392,300); 
    sleep_ms(100);  
    play_tone(BUZZER,392,300); 
    sleep_ms(100);  
    play_tone(BUZZER,440,600); 
    sleep_ms(100);  
    play_tone(BUZZER,392,600); 
    sleep_ms(100);   
    play_tone(BUZZER,587,600); 
    sleep_ms(100);  
    play_tone(BUZZER,523,1200); 
    sleep_ms(200);  
     //Muitas felicidades 
    play_tone(BUZZER,392,300); 
    sleep_ms(100);  
    play_tone(BUZZER,392,300); 
    sleep_ms(100);  
    play_tone(BUZZER,392,300); 
    sleep_ms(100);  
    play_tone(BUZZER,659,600); 
    sleep_ms(100);  
    play_tone(BUZZER,523,600); 
    sleep_ms(100);  
    play_tone(BUZZER,494,600); 
    sleep_ms(100);  
    play_tone(BUZZER,440,1200);
    sleep_ms(200);  
     //Muitos anos de vida
    play_tone(BUZZER,698,300);
    sleep_ms(100);  
    play_tone(BUZZER,698,300);
    sleep_ms(100);  
    play_tone(BUZZER,659,600);
    sleep_ms(100);  
    play_tone(BUZZER,523,600);
    sleep_ms(100);  
    play_tone(BUZZER,587,600);
    sleep_ms(100);  
    play_tone(BUZZER,523,1200);
    sleep_ms(200);  
    // Desativar o buzzer
          play_song = false; // Música terminada
      }
      sleep_ms(10);
  }
}

int main() {
    stdio_init_all();
    sleep_ms(3000);
    gpio_init(BUZZER);
    gpio_set_dir(BUZZER,GPIO_OUT);
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A,GPIO_IN);
    gpio_pull_up(BUTTON_A);

    npInit(LED_PIN, LED_COUNT);
    adc_init();
    adc_gpio_init(MIC_PIN);
    adc_select_input(MIC_CHANNEL);

    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_init(&ssd, 128, 64, false, OLED_ADDR, I2C_PORT);
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    float real[FFT_SIZE], imag[FFT_SIZE];
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler); // configura a interrupção e exuta a interrupção.

    multicore_launch_core1(play_music);  // Roda a música no segundo núcleo

    while (1) {
        sleep_ms(100);
        sample_mic(real);
        for (int i = 0; i < FFT_SIZE; i++) imag[i] = 0.0;
        fft(real, imag);
        
        int maxIndex = 0;
        float maxMagnitude = 0;
        for (int i = 5; i < FFT_SIZE / 2; i++) {
            float magnitude = sqrt(real[i] * real[i] + imag[i] * imag[i]);
            if (magnitude > maxMagnitude) {
                maxMagnitude = magnitude;
                maxIndex = i;
            }
        }

        float dominantFrequency = (maxIndex * SAMPLE_RATE) / FFT_SIZE;
        const char* detectedNote = find_closest_note(dominantFrequency);

        display_on_oled(dominantFrequency, detectedNote);
        update_led_matrix(dominantFrequency);

        printf("Freq: %.2f Hz | Nota: %s\n", dominantFrequency, detectedNote);
                

        sleep_ms(500);
    }
}
void gpio_irq_handler(uint gpio, uint32_t events){ //definido manualmente para tratar eventos de interrupção.
   

  uint32_t current_time = to_us_since_boot(get_absolute_time());

  if (gpio == BUTTON_A) {
    play_song = true;
}
 
    

    
  }








