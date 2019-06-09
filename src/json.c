/*
 * json.c
 *
 *  Created on: 29 May 2019
 *      Author: brandon
 */

#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include "json.h"

/**
 *   {
 *     "key1":"value1",
 *     "key2":"value2",
 *     "key3": {"keya":"valuea","keyb":"valueb"}
 *   }
 */

void json_split(char *input, char **key, char **value);
char *json_get_object(char *input);

char *json_parse_object(char *input, json_callback callback, void *param)
{
    char *input_end;
    char *object_end;
    char *key;
    char *value;
    if(*input != '{')
    {
        return 0;
    }

    input_end = json_get_object(input);
    if(*input_end != '}')
    {
        return 0;
    }

    input_end --;
    input ++;

    while(input < input_end)
    {
        object_end = json_get_object(input);
        if(object_end == NULL)
        {
            break;
        }
        *object_end='\0';
        json_split(input, &key, &value);
        callback(key, value, param);
        input = object_end + 1;
    }
    // return pointer to after the closing brace
    return input_end+2;
}


/**
 * returns a pointer to end of current key value pair
 */
char *json_get_object(char *input)
{
    char *ptr;
    char *closing_brace=NULL;
    int16_t  brace_counter;
    int16_t  bracket_counter;
    brace_counter = 0;
    bracket_counter = 0;

    ptr = input;
    while(*ptr != '\0')
    {
        ptr = strpbrk(ptr,",{}[]");
        switch(*ptr)
        {
        case ',':
            if((brace_counter == 0) && (bracket_counter == 0))
            {
                goto success;
            }
            break;
        case '[':
            bracket_counter++;
            break;
        case ']':
            bracket_counter--;
            break;
        case '{':
            brace_counter++;
            break;
        case '}':
            brace_counter--;
            closing_brace = ptr; // just incase we are at the end
            break;
        }
        ptr++;

    }
    if((brace_counter == -1)&&(closing_brace != NULL))
    {
        return closing_brace;
    }
    else if((brace_counter == 0)&&(bracket_counter == 0))
    {
        return ptr-1;
    }
    return NULL;
success:
    return ptr;
}

/**
 * returns pointers to nul terminated strings
 */
void json_split(char *input, char **key, char **value)
{
    char *local_key;
    char *local_value;
    local_value = strchr(input,':');
    if(local_value != NULL)
    {
        local_value ++;
    }
    local_key = strchr(input,'\"');
    local_key++;
    strchr(local_key, '\"')[0]=0;
    while(isspace(*local_value))
    {
        local_value++;
    }
    if(*local_value == '\"')
    {
        local_value++;
        strchr(local_value,'\"')[0]=0;
    }

    *key = local_key;
    *value = local_value;
}
