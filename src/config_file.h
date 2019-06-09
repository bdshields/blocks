/*
 * config_file.h
 *
 *  Created on: 6 Jun. 2019
 *      Author: brandon
 */

#ifndef SRC_CONFIG_FILE_H_
#define SRC_CONFIG_FILE_H_

#include <stdint.h>

#define FILE_NAME ".blocks"

void config_init(char *file_name);
void config_save(void);
void config_close(void);

uint16_t config_get_string(char *key, char *value);
uint16_t config_get_int(char *key, int32_t *value);

void config_put_string(char *key, char *value);
void config_put_int(char *key, int32_t value);


#endif /* SRC_CONFIG_FILE_H_ */
