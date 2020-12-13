#ifndef ERRORS_H
#define ERRORS_H

#define E_ASSERT  "Assertion failure"
#define E_NOMEM   "Memory allocation failure"
#define E_FSIZE   "File size is too large"
#define E_EXPECT  "Expected character"
#define E_NOREAD  "Unable to read file"
#define E_SPACES  "Invalid whitespace found"
#define E_GETCFG  "Unable to load config file"
#define E_GETARG  "Unable to get user options"
#define E_GETKEY  "Invalid configuration key"
#define E_GETVAL  "Invalid configuration value"
#define E_ARGVAL  "Invalid argument: Try --help"
#define E_CFGNUM  "Too many configuration values"
#define E_CFGLEN  "Configuration value too long"
#define E_NOTNUM  "Invalid numeric value given"
#define E_NOTSTR  "Invalid textual value given"
#define E_NOTBOL  "Invalid boolean value given"
#define E_NOSOCK  "Unable to bind to socket"

#endif // ERRORS_H
