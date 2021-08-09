#ifndef UTILS_H
#define UTILS_H

#define IPv4 1
#define IPv6 2

/**
 * @brief calculate system time
 * 
 * @return time as string 
 */
char* getTime();

/**
 * @brief 
 * 
 * @param IP ip address
 * @return is ip valid
 */
int isIP(const char *IP);

#endif
