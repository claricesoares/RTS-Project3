#include <stdio.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

SemaphoreHandle_t semaphore; // Semáforo para controlar a abertura e o fechamento da cancela
SemaphoreHandle_t mutex_e_o; // Mutex para fila de trens no sentido leste para oeste 
SemaphoreHandle_t mutex_o_e; // Mutex para fila de trens no sentido oeste para leste

int fila_e_o = 0; // Fila de trens no sentido leste para oeste 
int fila_o_e = 0; // Fila de trens no sentido oeste para leste

int trem_id = 1; // ID do trem

// Função para o comportamento da task trem - sentido oeste para leste
void trem_o_e(void *pvParameters)
{
    int id = *((int *)pvParameters);
    while (1)
    {
        // Determina a mensagem de direção baseada na direção do trem
        char *mensagem_direcao = "oeste para leste";

        printf("Trem %d se aproximando do cruzamento (%s).\n", trem_id, mensagem_direcao);

        xSemaphoreTake(mutex_o_e, portMAX_DELAY); // Adquire o mutex para acessar a fila
        fila_o_e++; 
        if (fila_o_e == 1 && fila_e_o == 0)
        {
            printf("Semaforo fica vermelho -- Cancela fechada\n");
            xSemaphoreTake(semaphore, portMAX_DELAY); // Fecha a cancela se um trem se aproxima enquanto não há outros trens
        }
        xSemaphoreGive(mutex_o_e); // Libera o mutex para a fila

        printf("Trem %d atravessando o cruzamento (%s).\n", trem_id, mensagem_direcao);
        vTaskDelay(7); // Simula o tempo de cruzamento
        printf("Trem %d passou pelo cruzamento (%s).\n", trem_id, mensagem_direcao);

        xSemaphoreTake(mutex_o_e, portMAX_DELAY); // Adquire o mutex para acessar a fila
        fila_o_e--; 
        if (fila_e_o == 0 && fila_o_e == 0)
        {
            printf("Semaforo fica verde -- Cancela aberta\n");
            xSemaphoreGive(semaphore); // Abre a cancela se não há trens
        }
        xSemaphoreGive(mutex_o_e); // Libera o mutex para a fila

        trem_id++;
        vTaskDelay((rand() % 125 + 200) / portTICK_PERIOD_MS); // Simula o tempo entre os trens
    }
}

// Função para o comportamento da task trem - sentido leste para oeste
void trem_e_o(void *pvParameters)
{
    int id = *((int *)pvParameters);
    while (1)
    {
        // Determina a mensagem de direção baseada na direção do trem
        char *mensagem_direcao = "leste para oeste";

        printf("Trem %d se aproximando do cruzamento (%s).\n", trem_id, mensagem_direcao);

        xSemaphoreTake(mutex_e_o, portMAX_DELAY); // Adquire o mutex para acessar a fila
        fila_e_o++; 
        if (fila_e_o == 1 && fila_o_e == 0)
        {
            printf("Semaforo fica vermelho -- Cancela fechada\n"); // Fecha a cancela se um trem se aproxima enquanto não há outros trens
            xSemaphoreTake(semaphore, portMAX_DELAY);
        }
        xSemaphoreGive(mutex_e_o); // Libera o mutex

        printf("Trem %d atravessando o cruzamento (%s).\n", trem_id, mensagem_direcao);
        vTaskDelay(7); // Simula o tempo de cruzamento
        printf("Trem %d passou pelo cruzamento (%s).\n", trem_id, mensagem_direcao);

        xSemaphoreTake(mutex_e_o, portMAX_DELAY); // Adquire o mutex para acessar a fila
        fila_e_o--; 
        if (fila_e_o == 0 && fila_o_e == 0)
        {
            printf("Semaforo fica verde -- Cancela aberta\n");
            xSemaphoreGive(semaphore); //Abre a cancela se não há trens
        }
        xSemaphoreGive(mutex_e_o); // Libera o mutex

        trem_id++;
        vTaskDelay((rand() % 250 + 200) / portTICK_PERIOD_MS); // Simula o tempo entre os trens
    }
}

// Função para o comportamento da task carro
void carro(void *pvParameters)
{
    int id = *((int *)pvParameters);

    while (1)
    {
        // Verifica se a cancela está aberta
        printf("Carro %d chegou na cancela.\n", id);
        if (xSemaphoreTake(semaphore, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            printf("Carro %d atravessando o cruzamento.\n", id);
            vTaskDelay(30); // Simula o tempo de cruzamento
            printf("Carro %d passou pelo cruzamento.\n", id);
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

    // Criação dos semáforos
    semaphore = xSemaphoreCreateMutex();
    mutex_e_o = xSemaphoreCreateMutex();
    mutex_o_e = xSemaphoreCreateMutex();

    // Criação de tasks para os trens e carros
    int *id = (int *)malloc(sizeof(int));
    *id = 1;
    xTaskCreate(trem_o_e, "Trem", 2048, (void *)id, 1, NULL);
    xTaskCreate(trem_e_o, "Trem", 2048, (void *)id, 1, NULL);
    xTaskCreate(carro, "Carro", 2048, (void *)id, 1, NULL);

    // Inicia o escalonador de tarefas
    vTaskStartScheduler();

    return 0;
}