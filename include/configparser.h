#ifndef CONFIGPARSER_H
#define CONFIGPARSER_H

#include "http.h"

#define MAX_KEYWORDS 8
#define FILE_PATH_SIZE 2048
#define KEY_WORD_SIZE 32
#define VALUE_SIZE 32

/**
 * @brief struct for saving words and values
 * 
 */
typedef struct
{
    char keyWord[KEY_WORD_SIZE];
    char value[VALUE_SIZE];
} keyValue;

/**
 * @brief struct which save file path and information from file
 * 
 */
typedef struct
{
    char filePath[FILE_PATH_SIZE];
    keyValue keys[MAX_KEYWORDS];
    int keyCount;
} configParser;

/**
 * @brief parse config file
 * 
 * @param cParser struct which will be filles
 */
void configParse(configParser *cParser);

/**
 * @brief Set the Params object
 * 
 * @param httpStruct 
 * @param cParser 
 */
void setParams(http *httpStruct, configParser cParser);

/**
 * @brief Get the Value 
 * 
 * @param key key
 * @param cParser struct with config data
 * @return value of the key
 */
char* getValue(char *key, configParser *cParser);

#endif