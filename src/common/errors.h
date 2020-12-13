#ifndef ERRORS_H
#define ERRORS_H

#define E_ASSERT  "Assertion failure"
#define E_NOMEM   "Memory allocation failure"
#define E_FSIZE   "File size is too large"
#define E_EXPECT  "Expected character"
#define E_NOREAD  "Unable to read file"
#define E_SPACES  "Invalid whitespace found"
#define E_GETCFG  "Check the config file for errors"
#define E_GETARG  "See --help for more information"
#define E_GETKEY  "Invalid configuration key"
#define E_GETVAL  "Invalid configuration value"
#define E_CFGNUM  "Too many configuration values"
#define E_CFGLEN  "Configuration value too long"
#define E_NOVAL   "Required option value missing"
#define E_NOTNUM  "Invalid numeric value"
#define E_NOTSTR  "Invalid string value"
#define E_NOTBOL  "Invalid boolean value"
#define E_NOSOCK  "Unable to bind to socket"

#endif // ERRORS_H
