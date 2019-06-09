/*
 * json.h
 *
 *  Created on: 29 May 2019
 *      Author: brandon
 */

#ifndef SRC_JSON_H_
#define SRC_JSON_H_

typedef void (*json_callback)(char *key, char *value, void *param);

char *json_parse_object(char *input, json_callback callback, void *param);


#endif /* SRC_JSON_H_ */
