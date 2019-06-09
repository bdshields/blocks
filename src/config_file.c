/*
 * config_file.c
 *
 *  Created on: 6 Jun. 2019
 *      Author: brandon
 */


#include "config_file.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char config_filename[500];
char *config_buffer=NULL;
uint16_t config_length=0;

#define CONFIG_BUF_SIZE 10000

void config_init(char *file_name)
{
    FILE *config;
    if(file_name == NULL)
    {
        file_name = FILE_NAME;
    }
    strcpy(config_filename, file_name);
    config_buffer = malloc(CONFIG_BUF_SIZE);

    config = fopen(FILE_NAME, "r");
    if(config == NULL)
    {
        goto no_file;
    }
    config_length = fread(config_buffer,1,CONFIG_BUF_SIZE, config);
    config_buffer[config_length]='\0';
    fclose(config);
no_file:
    return;
}

void config_save(void)
{
    FILE *config;
    config = fopen(FILE_NAME, "w");
    fwrite(config_buffer, config_length, 1, config);
    fclose(config);
}

void config_close(void)
{
    config_save();
    free(config_buffer);
    config_buffer = NULL;
}


char *config_get_line(char *key, char **end)
{
    char     *str;
    char     *end_ptr;
    uint16_t key_len;

    key_len = strlen(key);
    str = config_buffer;
    end_ptr = config_buffer+config_length;
    while(str < end_ptr)
    {
        str = strstr(str, key);
        if(str != NULL)
        {
            // found a posibility
            if((str+key_len) >= end_ptr)
            {
                // there's no space for the value
                break;
            }
            if((str == config_buffer)||(*(str-1)=='\n'))
            {
                if(str[key_len]==':')
                {
                    if(end != NULL)
                    {
                        end_ptr = strchrnul(str,'\n');
                        if(*end_ptr == '\n')
                        {
                            end_ptr ++;
                        }
                        *end = end_ptr;
                    }
                    return str;
                }
            }
            // failed to find it, just increment over first character
            str++;
        }
        else
        {
            break;
        }
    }
    return NULL;
}

uint16_t config_get_string(char *key, char *value)
{
    char     *str;
    char     *end_ptr;
    uint16_t key_len;
    uint16_t value_len;

    key_len = strlen(key);

    str = config_get_line(key, &end_ptr);
    if(str != NULL)
    {
        value_len = end_ptr - str - key_len - 1;
        memcpy(value, str + key_len + 1, value_len);
        value[value_len] = '\0';
        return 1;
    }
    return 0;
}


uint16_t config_get_int(char *key, int32_t *value)
{
    uint16_t result = 0;
    int32_t data;
    char *tailptr;
    char string[500];
    if(config_get_string(key, string))
    {
        data = strtol(string, &tailptr, 10);
        if(tailptr != (string))
        {
            // success
            *value = data;
            result = 1;
        }
    }
    return result;
}


void config_put_string(char *key, char *value)
{
    char new_line[500];
    uint16_t length;
    int16_t  size_increase;
    char *existing;
    char *end_ptr;

    length = snprintf(new_line, 500, "%s:%s\n",key,value);

    existing = config_get_line(key, &end_ptr);
    if(existing)
    {
        size_increase = length - (end_ptr - existing);
        if(size_increase != 0)
        {
            memmove(end_ptr + size_increase, end_ptr, config_length - (end_ptr - config_buffer));
        }
        memcpy(existing, new_line, length);
        config_length += size_increase;
    }
    else
    {
        // key doesn't exist, just append to end
        strcat(config_buffer,new_line);
        config_length += length;
    }
}

void config_put_int(char *key, int32_t value)
{
    char buffer[500];
    snprintf(buffer, 500, "%d", value);
    config_put_string(key, buffer);
}
