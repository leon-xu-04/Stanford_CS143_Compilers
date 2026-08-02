/*
 *  The scanner definition for COOL.
 */

/*
 *  Stuff enclosed in %{ %} in the first section is copied verbatim to the
 *  output, so headers and global definitions are placed here to be visible
 * to the code in the file.  Don't remove anything that was here initially
 */
%{
#include <cool-parse.h>
#include <stringtab.h>
#include <utilities.h>

/* The compiler assumes these identifiers. */
#define yylval cool_yylval
#define yylex  cool_yylex

/* Max size of string constants */
#define MAX_STR_CONST 1025
#define YY_NO_UNPUT   /* keep g++ happy */

extern FILE *fin; /* we read from this file */

/* define YY_INPUT so we read from the FILE fin:
 * This change makes it possible to use this scanner in
 * the Cool compiler.
 */
#undef YY_INPUT
#define YY_INPUT(buf,result,max_size) \
	if ( (result = fread( (char*)buf, sizeof(char), max_size, fin)) < 0) \
		YY_FATAL_ERROR( "read() in flex scanner failed");

char string_buf[MAX_STR_CONST]; /* to assemble string constants */
char *string_buf_ptr;

extern int curr_lineno;
extern int verbose_flag;

extern YYSTYPE cool_yylval;

/*
 *  Add Your own definitions here
 */

int comment_depth_counter = 0;

const char *string_error;
%}

%option noyywrap

/*
 * Define names for regular expressions here.
 */

CLASS           [Cc][Ll][Aa][Ss][Ss]
ELSE            [Ee][Ll][Ss][Ee]
FI              [Ff][Ii]
IF              [Ii][Ff]
IN              [Ii][Nn]
INHERITS        [Ii][Nn][Hh][Ee][Rr][Ii][Tt][Ss]
LET             [Ll][Ee][Tt]
LOOP            [Ll][Oo][Oo][Pp]
POOL            [Pp][Oo][Oo][Ll]
THEN            [Tt][Hh][Ee][Nn]
WHILE           [Ww][Hh][Ii][Ll][Ee]
CASE            [Cc][Aa][Ss][Ee]
ESAC            [Ee][Ss][Aa][Cc]
OF              [Oo][Ff]
DARROW          =>
NEW             [Nn][Ee][Ww]
ISVOID          [Ii][Ss][Vv][Oo][Ii][Dd]
NOT             [Nn][Oo][Tt]
TRUE            t[Rr][Uu][Ee]
FALSE           f[Aa][Ll][Ss][Ee]

DIGIT           [0-9]
INT             {DIGIT}+
CAP_LETTER      [A-Z]
LOW_LETTER      [a-z]
LETTER          {CAP_LETTER}|{LOW_LETTER}
WHITE_SPACE     [ \f\r\t\v]
ID_CHAR         {LETTER}|{DIGIT}|"_"
TYPEID          {CAP_LETTER}{ID_CHAR}*
OBJECTID        {LOW_LETTER}{ID_CHAR}*

SINGLE_CHAR     [-()+*/~<={}:;.,@]
%x              STR LINE_COMMENT BLOCK_COMMENT

%%

 /*
  *  Nested comments
  */


 /*
  * Keywords are case-insensitive except for the values true and false,
  * which must begin with a lower-case letter.
  */

{CLASS}     { return (CLASS); }
{ELSE}      { return (ELSE); }
{FI}        { return (FI); }
{IF}        { return (IF); }
{IN}        { return (IN); }
{INHERITS}  { return (INHERITS); }
{LET}       { return (LET); }
{LOOP}      { return (LOOP); }
{POOL}      { return (POOL); }
{THEN}      { return (THEN); }
{WHILE}     { return (WHILE); }
{CASE}      { return (CASE); }
{ESAC}      { return (ESAC); }
{OF}        { return (OF); }
{DARROW}		{ return (DARROW); }
{NEW}       { return (NEW); }
{ISVOID}    { return (ISVOID); }
{NOT}       { return (NOT); }
{TRUE}      { yylval.boolean = true; return (BOOL_CONST); }
{FALSE}     { yylval.boolean = false; return (BOOL_CONST); }

{INT}       { yylval.symbol = inttable.add_string(yytext); return (INT_CONST); }

"<-"        { return (ASSIGN); }
"<="        { return (LE); }
"\n"        { ++curr_lineno; }
{SINGLE_CHAR}   { return (yytext[0]); }
{WHITE_SPACE}   {}

"--"        BEGIN(LINE_COMMENT);
"(*"        { BEGIN(BLOCK_COMMENT); ++comment_depth_counter;}
"*)"        { yylval.error_msg = "Unmatched *)"; return (ERROR); }
"\""        { string_buf_ptr = string_buf; string_error = NULL; BEGIN(STR); }

{TYPEID}   { yylval.symbol = idtable.add_string(yytext); return (TYPEID); }
{OBJECTID} { yylval.symbol = idtable.add_string(yytext); return (OBJECTID); }

<LINE_COMMENT>{
  [^\n]*        {}
  "\n"          { ++curr_lineno; BEGIN(INITIAL); }
}

<BLOCK_COMMENT>{
  \n      { ++curr_lineno; }
  "(*"    { ++comment_depth_counter; }
  "*)"    { --comment_depth_counter; if (comment_depth_counter == 0) {BEGIN(INITIAL);} }
  <<EOF>> { yylval.error_msg = "EOF in comment"; BEGIN(INITIAL); return (ERROR); }
  .       {}
}

<STR>{
  \\([^\0])  {
    if (string_error) ;
    else if (string_buf_ptr >= (string_buf + MAX_STR_CONST - 1)) {
      string_error = "String constant too long";
    } else {
      char c = yytext[1];
      if (c == 'b')         *string_buf_ptr++ = '\b';
      else if (c == 't')    *string_buf_ptr++ = '\t';
      else if (c == 'n')    *string_buf_ptr++ = '\n';
      else if (c == 'f')    *string_buf_ptr++ = '\f';
      else                  *string_buf_ptr++ = c;
    }
    if (yytext[1] == '\n')  ++curr_lineno;
  }
  \n            {
                  BEGIN(INITIAL);
                  ++curr_lineno;
                  yylval.error_msg = (char *)(string_error ? string_error : "Unterminated string constant");
                  return (ERROR);
                }
  \0            {
                  if (!string_error) {
                    string_error = "String contains null character";
                  }
                }
  \"            {
                  BEGIN(INITIAL);
                  if (string_error) {
                    yylval.error_msg = (char *)string_error;
                    return (ERROR);
                  }
                  *string_buf_ptr = '\0';
                  yylval.symbol = stringtable.add_string(string_buf);
                  string_buf_ptr = string_buf;
                  return (STR_CONST);
                }
  <<EOF>>       {
                  BEGIN(INITIAL);
                  yylval.error_msg = (char *)(string_error ? string_error : "EOF in string constant");
                  return (ERROR);
                }
  .             {
                  if (string_error) ;
                  else if (string_buf_ptr >= (string_buf + MAX_STR_CONST - 1)) {
                    string_error = "String constant too long";
                  } else {
                    *string_buf_ptr++ = yytext[0];
                  }
                }
  
}

.               { yylval.error_msg = strdup(yytext); return (ERROR); }

 /*
  *  String constants (C syntax)
  *  Escape sequence \c is accepted for all characters c. Except for 
  *  \n \t \b \f, the result is c.
  *
  */

<<EOF>>      { yyterminate(); }

%%

/*
 *  User code section
 */
void reset_lexer_state() {
  string_buf_ptr = string_buf;
  comment_depth_counter = 0;
  BEGIN(INITIAL);
}