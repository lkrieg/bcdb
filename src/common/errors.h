#ifndef ERRORS_H
#define ERRORS_H

#define E_ASSERT  "Assertion failure"
#define E_NOMEM   "Memory allocation failure"
#define E_FSIZE   "File size is too large"
#define E_EXPECT  "Expected character"
#define E_NOREAD  "Unable to read file"
#define E_SPACES  "Invalid whitespace found"
#define E_GETKEY  "Invalid configuration key"
#define E_GETVAL  "Invalid configuration value"
#define E_NOVAL   "Missing configuration value"
#define E_ARGVAL  "Invalid argument: Try --help"
#define E_ARGNUM  "Too many command-line arguments"
#define E_ARGLEN  "Command-line argument too long"
#define E_KEYLEN  "Configuration key too long"
#define E_VALLEN  "Configuration value too long"
#define E_NOTNUM  "Invalid numeric value given"
#define E_NOBOOL  "Invalid boolean value given"
#define E_NOSOCK  "Unable to bind to socket"

#endif // ERRORS_H
