/*
 * this is a simple parser for JSON formatted string.
 * we use it for parsing incoming HTTPS, or MQTT bodies
 * and to updated the config structure of conf_t
 */

#ifndef __JSMN_H_
#define __JSMN_H_

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * this conf structure holds the config values of the IoT device boards
 */
typedef struct conf{
    char *device;
    char *id;
    char *ip;

}conf_t;

/**
 * JSON type identifier. Basic types are:
 * 	o Object
 * 	o Array
 * 	o String
 * 	o Other primitive: number, boolean (true/false) or null
 */
typedef enum {
	JSMN_UNDEFINED = 0,
	JSMN_OBJECT = 1,
	JSMN_ARRAY = 2,
	JSMN_STRING = 3,
	JSMN_PRIMITIVE = 4
} jsmntype_t;

enum jsmnerr {
	/* Not enough tokens were provided */
	JSMN_ERROR_NOMEM = -1,
	/* Invalid character inside JSON string */
	JSMN_ERROR_INVAL = -2,
	/* The string is not a full JSON packet, more bytes expected */
	JSMN_ERROR_PART = -3
};

/**
 * JSON token description.
 * type		type (object, array, string etc.)
 * start	start position in JSON data string
 * end		end position in JSON data string
 */
typedef struct {
	jsmntype_t type;
	int start;
	int end;
	int size;
#ifdef JSMN_PARENT_LINKS
	int parent;
#endif
} jsmntok_t;

/**
 * JSON parser. Contains an array of token blocks available. Also stores
 * the string being parsed now and current position in that string
 */
typedef struct {
	unsigned int pos; /* offset in the JSON string */
	unsigned int toknext; /* next token to allocate */
	int toksuper; /* superior token node, e.g parent object or array */
} jsmn_parser;

/**
 * Create JSON parser over an array of tokens
 */
void jsmn_init(jsmn_parser *parser);

/**
 * Run JSON parser. It parses a JSON data string into and array of tokens, each describing
 * a single JSON object.
 */
int jsmn_parse(jsmn_parser *parser, const char *js, size_t len,
		jsmntok_t *tokens, unsigned int num_tokens);

/*
 * not standard JSMN library elements.
 * were created for the smart home project
 */

/*
 * checks if the jsmn token's key value is equal to the searches key value
 * json: the original json string
 * tok: jsmn token
 * s: looked up key value
 */
int jsoneq(const char *json, jsmntok_t *tok, const char *s);
/*
 * returns a copy of a string (pointer) from the memory location of s1
 * up to n bytes long
 */
char *strndup(const char *s1, size_t n);
/*
 * paprses a JSON_STRING, and copies its value to the corresponding
 * members of the conf_struct.
 */
int parse_JSON(conf_t *conf_struct, char *JSON_STRING);


/*
 ***************************************************************
 */



#ifdef __cplusplus
}
#endif

#endif /* __JSMN_H_ */