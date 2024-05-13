#include <stdio.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

SemaphoreHandle_t semaphore;
SemaphoreHandle_t mutex_e_o; // Mutex para fila leste para oeste
SemaphoreHandle_t mutex_o_e; // Mutex para fila oeste para leste
SemaphoreHandle_t cancela;

int fila_e_o = 0; // Fila leste para oeste
int fila_o_e = 0; // Fila oeste para leste

void trem_o_e(void *pvParameters)
{
    int id = *((int *)pvParameters);

    while (1)
    {
        // Determina a mensagem de direção baseada na direção do trem
        char *mensagem_direcao = "oeste para leste";

        printf("Trem %d se aproximando do cruzamento (%s).\n", id, mensagem_direcao);

        xSemaphoreTake(mutex_o_e, portMAX_DELAY);
        fila_o_e++;
        if (fila_o_e == 1 && fila_e_o == 0)
        {
            printf("Semaforo fica vermelho -- Cancela fechada\n");
            xSemaphoreTake(semaphore, portMAX_DELAY);
        }
        xSemaphoreGive(mutex_o_e);

        printf("Trem %d cruzando o cruzamento (%s).\n", id, mensagem_direcao);
        vTaskDelay(7); // Simula o tempo de cruzamento
        printf("Trem %d passou pelo cruzamento (%s).\n", id, mensagem_direcao);

        xSemaphoreTake(mutex_o_e, portMAX_DELAY);
        fila_o_e--;
        if (fila_e_o == 0 && fila_o_e == 0)
        {
            printf("Semaforo fica verde -- Cancela aberta\n");
            xSemaphoreGive(semaphore);
        }
        xSemaphoreGive(mutex_o_e);

        id = id + 1;
        vTaskDelay((rand() % 125 + 200) / portTICK_PERIOD_MS); // Simula o tempo entre os trens
    }
}

void trem_e_o(void *pvParameters)
{
    int id = *((int *)pvParameters);

    while (1)
    {
        // Determina a mensagem de direção baseada na direção do trem
        char *mensagem_direcao = "leste para oeste";

        printf("Trem %d se aproximando do cruzamento (%s).\n", id, mensagem_direcao);

        xSemaphoreTake(mutex_e_o, portMAX_DELAY);
        fila_e_o++;
        if (fila_e_o == 1 && fila_o_e == 0)
        {
            printf("Semaforo fica vermelho -- Cancela fechada\n");
            xSemaphoreTake(semaphore, portMAX_DELAY);
        }
        xSemaphoreGive(mutex_e_o);

        printf("Trem %d cruzando o cruzamento (%s).\n", id, mensagem_direcao);
        vTaskDelay(7); // Simula o tempo de cruzamento
        printf("Trem %d passou pelo cruzamento (%s).\n", id, mensagem_direcao);

        xSemaphoreTake(mutex_e_o, portMAX_DELAY);
        fila_e_o--;
        if (fila_e_o == 0 && fila_o_e == 0)
        {
            printf("Semaforo fica verde -- Cancela aberta\n");
            xSemaphoreGive(semaphore);
        }
        xSemaphoreGive(mutex_e_o);

        id = id + 1;
        vTaskDelay((rand() % 250 + 200) / portTICK_PERIOD_MS); // Simula o tempo entre os trens
    }
}

void carro(void *pvParameters)
{
    int id = *((int *)pvParameters);

    while (1)
    {
        // Verifica se a cancela está aberta
        printf("Carro %d chegou na cancela.\n", id);
        if (xSemaphoreTake(semaphore, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            printf("Carro %d passando.\n", id);
            vTaskDelay(30); // Simula o tempo de travessia
            printf("Carro %d passou .\n", id);
            xSemaphoreGive(semaphore); // Libera o semáforo da cancela após o carro passar
        }
        else
        {
            printf("Carro %d esperando na cancela.\n", id);
        }

        id = id + 1;
        vTaskDelay((rand() % 180 + 200) / portTICK_PERIOD_MS); // Simula o tempo entre os carros
    }
}

int main()
{

    srand(time(NULL));

    semaphore = xSemaphoreCreateMutex();
    mutex_e_o = xSemaphoreCreateMutex();
    mutex_o_e = xSemaphoreCreateMutex();

    // Criando threads para os trens
    // for (int i = 0; i < 2; ++i) {
    int *id = (int *)malloc(sizeof(int));
    *id = 1;
    xTaskCreate(trem_o_e, "Trem", 2048, (void *)id, 1, NULL);
    xTaskCreate(trem_e_o, "Trem", 2048, (void *)id, 1, NULL);
    // }

    // Criando threads para os carros
    //  for (int i = 0; i < 2; ++i) {
    //  int *id = (int *)malloc(sizeof(int));
    //*id = i + 1;
    xTaskCreate(carro, "Carro", 2048, (void *)id, 1, NULL);
    //}

    vTaskStartScheduler();

    return 0;
}