# Projeto final (Harmonichip) 🚀

Projeto envolve o desenvolvimento de um sistema embarcado com Raspberry Pi Pico para análise de som. Ele utiliza um microfone para captar o áudio, aplica FFT (Transformada Rápida de Fourier) para processar o sinal e exibir os resultados em um display OLED. Além disso, uma matriz de LEDs é usada para indicar a intensidade sonora em tempo real.

## Hardware 🛠️

- Microcontrolador RP2040 (Raspberry Pi Pico).
- pushbotton.
- SSD1306 OLED DISPLAY.
- Display NeoPixel RGB 5x5.
- Microfone
- LED RGB.

## Software 💻

* **SDK do Raspberry Pi Pico:** O SDK (Software Development Kit) do Pico, que inclui as bibliotecas e ferramentas necessárias para desenvolver e compilar o código. [Instruções de instalação](https://www.raspberrypi.com/documentation/pico/getting-started/)
* **CMake:** Um sistema de construção multiplataforma usado para gerar os arquivos de construção do projeto.
* **Compilador C/C++:**  Um compilador C/C++ como o GCC (GNU Compiler Collection).
* **Git:** (Opcional) Para clonar o repositório do projeto.


### O código está dividido em vários arquivos para melhor organização:

- **`microphone_dma.c`**:nálise de som, aplicando FFT para processar o áudio captado por um microfone e exibindo os resultados em um display OLED e uma matriz de LEDs e ao apertar no botão transmiti a musica do parabéns.
- **`neopixel.c/.h`:** Funções para matriz de L.
- **`ws2818b.pio`:** Configuração da matriz de LED.
- - **`ssd1306.c/.h`:** possibilitar o uso de caracteres no SSD1306 OLED DISPLAY.
- **`CMakeLists.txt`:** Define a estrutura do projeto para o CMake.



## Como Compilar e Executar ⚙️

1. **Instale o SDK do Raspberry Pi Pico:** Siga as instruções no site oficial do Raspberry Pi.
2. **Clone este repositório:** https://github.com/LuizEduardo-cyber/Projeto-final.git`
3. **Navegue até o diretório do projeto:** `cd microphone_dma.c`
4. **Compile o projeto:** `cmake -B build && cmake --build build`
5. **Copie para o Pico:** Copie o conteúdo da pasta `build` (gerada após a compilação) para o Raspberry Pi Pico. O programa iniciará automaticamente.


## Funcionamento do Loop Principal 🔄 
```
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
   
  ```
## Funcionamento do segundo Loop 🔄
```
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
   
  ```
## Funcionamento da interrupção.
```
void gpio_irq_handler(uint gpio, uint32_t events){ //definido manualmente para tratar eventos de interrupção.
   

  uint32_t current_time = to_us_since_boot(get_absolute_time());

  if (gpio == BUTTON_A) {
    play_song = true;
}
 
    

    
  }

  ```

## Diagrama de Conexões 💡:
https://drive.google.com/file/d/1gbfpPmT3eV1kxhkyUBORLbByO54jyoJ4/view?usp=drive_link
## Próximos Passos ➡️

- incluir notas originais no projeto.
- gerar uma animação com mais fluidez na matriz de LED.
- Gerar mais musicas no buzzer.
  
 ## 🔗 Link do Vídeo de Funcionamento:
https://drive.google.com/file/d/163ae8SIDxSiBRcRlbWKiaSp3NkjzKJjl/view?usp=drive_link
 ## Contribuições 🤝

Contribuições são bem-vindas! Sinta-se à vontade para abrir issues ou enviar pull requests.

## 📞 Contato

- 👤 **Autor**: Luiz Eduardo Soares Ferreira.
 
- 📧 **E-mail**: luizeduardosoaresferreira942@gmail.com 

--- 
