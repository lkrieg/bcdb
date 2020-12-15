#ifndef PARSER_H
#define PARSER_H

int ReadConfKey(char **buf, char **key);
int ReadConfVal(char **buf, char **val);
int SkipWhitespace(char **buf);

#endif // PARSER_H
