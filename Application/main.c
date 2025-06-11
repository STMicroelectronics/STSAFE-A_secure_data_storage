/**
 ******************************************************************************
 * \file    		main.c
 * \author  		CS application team
 ******************************************************************************
 *           			COPYRIGHT 2022 STMicroelectronics
 *
 * This software is licensed under terms that can be found in the LICENSE file in
 * the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "Drivers/uart/uart.h"
#include "stselib.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

/* Defines -------------------------------------------------------------------*/
#define PRINT_RESET "\x1B[0m"
#define PRINT_CLEAR_SCREEN "\x1B[1;1H\x1B[2J"
#define PRINT_RED "\x1B[31m"   /* Red */
#define PRINT_GREEN "\x1B[32m" /* Green */

#define READ_BUFFER_SIZE 64
#define RANDOM_SIZE READ_BUFFER_SIZE

/* STDIO redirect */
#if defined(__GNUC__) && !defined(__ARMCC_VERSION)
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#define GETCHAR_PROTOTYPE int __io_getchar()
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#define GETCHAR_PROTOTYPE int fgetc(FILE *f)
#endif /* __GNUC__ */
PUTCHAR_PROTOTYPE {
    uart_putc(ch);
    return ch;
}
GETCHAR_PROTOTYPE {
    return uart_getc();
}

void apps_terminal_init(uint32_t baudrate) {
    (void)baudrate;
    /* - Initialize UART for example output log (baud 115200)  */
    uart_init(115200);
    /* Disable I/O buffering for STDOUT stream*/
    setvbuf(stdout, NULL, _IONBF, 0);
    /* - Clear terminal */
    printf(PRINT_RESET PRINT_CLEAR_SCREEN);
}

void apps_print_data_partition_record_table(stse_Handler_t *pSTSE) {
    uint8_t i;
    stse_ReturnCode_t ret;
    uint16_t data_partition_record_table_length;
    uint8_t total_zone_count;

    /*- Get total partition count */
    ret = stse_data_storage_get_total_partition_count(
        pSTSE,
        &total_zone_count);
    if (ret != STSE_OK) {
        printf("\n\n\r ### stsafe_get_total_partition_count : ERROR 0x%04X", ret);
        while (1)
            ;
    }

    /*- Allocate data partition table  */
    stsafea_data_partition_record_t data_partition_record_table[total_zone_count];

    /*- Get data partition table  */
    data_partition_record_table_length = sizeof(data_partition_record_table);
    ret = stse_data_storage_get_partitioning_table(
        pSTSE,
        total_zone_count,
        data_partition_record_table,
        data_partition_record_table_length);
    if (ret != STSE_OK) {
        printf("\n\r ### stse_get_data_partitions_configuration : ERROR 0x%04X", ret);
        while (1)
            ;
    } else {
        printf("\n\n\r - stse_get_data_partitions_configuration");
    }

    printf("\n\r  ID | COUNTER | DATA SEGMENT SIZE | READ AC CR |  READ AC | UPDATE AC CR | UPDATE AC |  COUNTER VAL \r\n");
    for (i = 0; i < total_zone_count; i++) {
        /*- print id (col 1)*/
        printf(" %03d | ", data_partition_record_table[i].index);

        /*- print counter presence (col 2)*/
        printf("   %c    |", (data_partition_record_table[i].zone_type) == 0 ? '.' : 'x');

        /*- print data segment size (col 3) */
        printf("       %04u        | ", data_partition_record_table[i].data_segment_length);

        /*- read ac change right (col 4) */
        printf(" %s | ", (data_partition_record_table[i].read_ac_cr) == 1 ? " ALLOWED " : " DENIED  ");

        /*- read ac right (col 5) */
        switch (data_partition_record_table[i].read_ac) {
        case STSE_AC_ALWAYS:
            printf(" ALWAYS  |");
            break;

        case STSE_AC_HOST:
            printf("   HOST  |");
            break;

        case STSE_AC_AUTH:
            printf("   AUTH  |");
            break;

        default:
            printf("  NEVER  |");
            break;
        }

        /*- update ac change right (col 4) */
        printf(" %s | ", (data_partition_record_table[i].update_ac_cr) == 1 ? "   ALLOWED  " : "   DENIED   ");

        /*- update ac right (col 5) */
        switch (data_partition_record_table[i].update_ac) {
        case STSE_AC_ALWAYS:
            printf("  ALWAYS  |");
            break;

        case STSE_AC_HOST:
            printf("   HOST   |");
            break;

        case STSE_AC_AUTH:
            printf("  AUTH    |");
            break;

        default:
            printf("  NEVER   |");
            break;
        }

        printf(" %06" PRIu32 "\r\n", data_partition_record_table[i].counter_value);
    }
}

void apps_print_hex_buffer(uint8_t *buffer, uint16_t buffer_size) {
    uint16_t i;
    for (i = 0; i < buffer_size; i++) {
        if (i % 16 == 0) {
            printf(" \n\r ");
        }
        printf(" 0x%02X", buffer[i]);
    }
}

int main(void) {
    stse_ReturnCode_t stse_ret = STSE_API_INVALID_PARAMETER;
    stse_Handler_t stse_handler;
    uint8_t readBuffer[READ_BUFFER_SIZE];
    uint8_t random[RANDOM_SIZE];
    uint32_t counter_value;

    /* - Initialize Terminal */
    apps_terminal_init(115200);

    /* - Print Example instruction on terminal */
    printf(PRINT_CLEAR_SCREEN);
    printf("----------------------------------------------------------------------------------------------------------------");
    printf("\n\r-                            STSAFE-A120 secure data storage counter zone access example                               -");
    printf("\n\r----------------------------------------------------------------------------------------------------------------");
    printf("\n\r-                                                                                                              -");
    printf("\n\r- description :                                                                                                -");
    printf("\n\r- This examples illustrates how to makes use of the STSAFE-A data storage APIs by performing following         -");
    printf("\n\r- accesses/commands to the target STSAFE device                                                                -");
    printf("\n\r-          o Query STSAFE-A total partition count                                                              -");
    printf("\n\r-          o Query STSAFE-A partitions information                                                             -");
    printf("\n\r-          o Read STSAFE-A zone 5 data & associated counter value                                              -");
    printf("\n\r-          o Decrement STSAFE-A zone 5                                                                         -");
    printf("\n\r-          o Read STSAFE-A zone 5 with new data & new associated counter value                                 -");
    printf("\n\r-                                                                                                              -");
    printf("\n\r- Note : zone IDs used in this example are aligned with STSAFE-A120 SPL05 personalization                      -");
    printf("\n\r-        Accesses parameters must be adapted for other device personalization                                  -");
    printf("\n\r-                                                                                                              -");
    printf("\n\r----------------------------------------------------------------------------------------------------------------");

    /* ## Initialize STSAFE-A120 device handler */
    stse_ret = stse_set_default_handler_value(&stse_handler);
    if (stse_ret != STSE_OK) {
        printf(PRINT_RED "\n\r ## stse_set_default_handler_value ERROR : 0x%04X\n\r", stse_ret);
        while (1)
            ; // infinite loop
    }

    stse_handler.device_type = STSAFE_A120;
    stse_handler.io.busID = 1; // Needed to use expansion board I2C

    printf("\n\r - Initialize target STSAFE-A120");
    stse_ret = stse_init(&stse_handler);
    if (stse_ret != STSE_OK) {
        printf(PRINT_RED "\n\r ## stse_init ERROR : 0x%04X\n\r", stse_ret);
        while (1)
            ; // infinite loop
    }

    /* ## Print User NVM data partitioning information */
    apps_print_data_partition_record_table(&stse_handler);

    /* ## Read Zone 5 */
    stse_ret = stse_data_storage_read_counter_zone(
        &stse_handler,      /* SE handler		*/
        5,                  /* Zone index		*/
        0x0000,             /* Read Offset		*/
        readBuffer,         /* Read buffer		*/
        sizeof(readBuffer), /* Read length		*/
        04,                 /* Read chunk size	*/
        &counter_value,     /* Counter Value	*/
        STSE_NO_PROT);
    if (stse_ret != STSE_OK) {
        printf(PRINT_RED "\n\n\r ### stse_data_storage_read_data_zone : ERROR 0x%04X", stse_ret);
        while (1)
            ; // infinite loop
    } else {
        printf("\n\n\r - stse_data_storage_read_data_zone (zone : 05 - length : %d - counter : %lu)", sizeof(readBuffer) / sizeof(readBuffer[0]), counter_value);
        apps_print_hex_buffer(readBuffer, sizeof(readBuffer));
    }

    /*## Generate random number */
    stse_ret = stse_generate_random(&stse_handler, random, READ_BUFFER_SIZE);
    if (stse_ret != STSE_OK) {
        printf(PRINT_RED "\n\n\r ### STSAFE-A generate random : ERROR 0x%04X", stse_ret);
        while (1)
            ; // infinite loop
    } else {
        printf("\n\n\r - stse_generate_random (length : %d)", sizeof(random));
        apps_print_hex_buffer(random, sizeof(random));
    }

    /* ## Decrement zone 5 counter and store Randomized Associated data */
    stse_ret = stse_data_storage_decrement_counter_zone(
        &stse_handler,  /* SE handler 			*/
        5,              /* Zone index 			*/
        1,              /* Decrement amount		*/
        0x0000,         /* Update Offset 		*/
        random,         /* Update input buffer 	*/
        sizeof(random), /* Update Length 		*/
        &counter_value, /* Counter value		*/
        STSE_NO_PROT);
    if (stse_ret != STSE_OK) {
        printf(PRINT_RED "\n\n\r ### stse_data_storage_decrement_counter_zone : ERROR 0x%04X", stse_ret);
        while (1)
            ; // infinite loop
    } else {
        printf("\n\n\r - stse_data_storage_decrement_counter_zone (zone = 05 - length = %d - New counter : %lu)", sizeof(random) / sizeof(random[0]), counter_value);
        apps_print_hex_buffer(random, sizeof(random));
    }

    /* ## Read Zone 5 (counter zone) */
    stse_ret = stse_data_storage_read_counter_zone(
        &stse_handler,      /* SE handler		*/
        5,                  /* Zone index		*/
        0x0000,             /* Read Offset		*/
        readBuffer,         /* Read buffer		*/
        sizeof(readBuffer), /* Read length		*/
        04,                 /* Read chunk size	*/
        &counter_value,     /* Counter Value	*/
        STSE_NO_PROT);
    if (stse_ret != STSE_OK) {
        printf(PRINT_RED "\n\n\r ### stse_data_storage_read_data_zone : ERROR 0x%04X", stse_ret);
        while (1)
            ; // infinite loop
    } else {
        printf(PRINT_GREEN "\n\n\r - stse_data_storage_read_data_zone (zone : 05 - length : %d - counter : %lu)", sizeof(readBuffer) / sizeof(readBuffer[0]), counter_value);
        apps_print_hex_buffer(readBuffer, sizeof(readBuffer));
    }

    while (1)
        ; // infinite loop

    return 0;
}
