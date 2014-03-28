/*--------------------------------------------------------------------------*/
/*                                                                          */
/*       parser1                                                            */
/*                                                                          */
/*       Group Members:         ID numbers                                  */
/*                                                                          */
/*       Ethan O'Brien          11134798                                    */
/*       Hammad Aljeddani       09003021                                    */
/*       Hristo Trifonov        11060905                                    */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*--------------------------------------------------------------------------*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "scanner.h"
#include "line.h"


/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  Global variables used by this parser.                                   */
/*                                                                          */
/*--------------------------------------------------------------------------*/

PRIVATE FILE *InputFile;           /*  CPL source comes from here.          */
PRIVATE FILE *ListFile;            /*  For nicely-formatted syntax errors.  */

PRIVATE TOKEN  CurrentToken;       /*  Parser lookahead token.  Updated by  */
                                   /*  routine Accept (below).  Must be     */
                                   /*  initialised before parser starts.    */


/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  Function prototypes                                                     */
/*                                                                          */
/*--------------------------------------------------------------------------*/

PRIVATE int  OpenFiles( int argc, char *argv[] );
PRIVATE void Accept( int code );
PRIVATE void ReadToEndOfFile( void );


PRIVATE void ParseProgram(void);
PRIVATE void ParseDeclarations(void);
PRIVATE void ParseProcDeclaration(void);
PRIVATE void ParseParameterList(void);
PRIVATE void ParseFormalParameter(void);
PRIVATE void ParseBlock(void);
PRIVATE void ParseStatement(void);
PRIVATE void ParseSimpleStatement(void);
PRIVATE void ParseRestOfStatement(void);
PRIVATE void ParseProcCallList(void);
PRIVATE void ParseAssignment(void);
PRIVATE void ParseActualParameter(void);
PRIVATE void ParseWhileStatement(void);
PRIVATE void ParseIfStatement(void);
PRIVATE void ParseReadStatement(void);
PRIVATE void ParseWriteStatement(void);
PRIVATE void ParseExpression(void);
PRIVATE void ParseCompoundTerm(void);
PRIVATE void ParseTerm(void);
PRIVATE void ParseSubTerm(void);
PRIVATE void ParseBooleanExpression(void);
PRIVATE void ParseAddOp(void);
PRIVATE void ParseMultOp(void);
PRIVATE void ParseRelOp(void);
PRIVATE void ParseVariable(void);
PRIVATE void ParseIntConst(void);
PRIVATE void ParseIdentifier(void);


/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  Main: Smallparser entry point.  Sets up parser globals (opens input and */
/*        output files, initialises current lookahead), then calls          */
/*        "ParseProgram" to start the parse.                                */
/*                                                                          */
/*--------------------------------------------------------------------------*/

PUBLIC int main ( int argc, char *argv[] )
{   
    if ( OpenFiles( argc, argv ) )  
    {
    InitCharProcessor( InputFile, ListFile );
    CurrentToken = GetToken();
    ParseProgram();
    fclose( InputFile );
    fclose( ListFile );
    printf("Valid\n");
    return  EXIT_SUCCESS;
    }
    else 
    {
        printf("Syntax Error\n");
    return EXIT_FAILURE;
    }
}


/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  Parser routines: Recursive-descent implementaion of the grammar's       */
/*                   productions.                                           */
/*                                                                          */
/*                                                                          */
/*  ParseProgram implements:                                                */
/*                                                                          */
/*        <Program>     :== "PROGRAM" <Identifier> ";" [<Declarations>]     */
/*                         {ProcDeclaration} <Block> "."                    */
/*                                                                          */
/*                                                                          */
/*--------------------------------------------------------------------------*/



PRIVATE void ParseProgram(void)
{
    Accept(PROGRAM);
    Accept(IDENTIFIER);
    Accept(SEMICOLON);
    
    if(CurrentToken.code == VAR)    
        ParseDeclarations();
        
    while (CurrentToken.code == PROCEDURE)
        ParseProcDeclaration(); 
    
    ParseBlock();
    Accept(ENDOFPROGRAM);   
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  ParseDeclarations implements:                       */
/*                                                                          */
/*      <Declarations> :== "VAR" <Variable> {"," <Variable>} ";"            */
/*                                                                          */
/*                                                                          */
/*--------------------------------------------------------------------------*/

PRIVATE void ParseDeclarations(void)
{
    Accept(VAR);
    ParseVariable();
    
    while (CurrentToken.code == COMMA)
    {
        Accept(COMMA);
        ParseVariable();
    }
    Accept(SEMICOLON);  
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  ParseProcDeclaration implements:                        */
/*                                                                          */
/*      <ProcDeclarations> :== "PRODEDURE" <Identifire> [<ParameterList>]   */
/*              ";" [<Declarations>] {<ProcDeclaration>}    */
/*              <Block> ";"                     */
/*                                                                          */
/*                                                                          */
/*--------------------------------------------------------------------------*/

PRIVATE void ParseProcDeclaration(void)
{
    Accept(PROCEDURE);
    Accept(IDENTIFIER);
    
    if(CurrentToken.code == LEFTPARENTHESIS)
        ParseParameterList();
        
    Accept(SEMICOLON);
    if(CurrentToken.code == VAR)
        ParseDeclarations();
        
    while (CurrentToken.code == PROCEDURE)  
        ParseProcDeclaration();
    
    ParseBlock();
    Accept(SEMICOLON);
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  ParseParameterList implements:                                          */
/*                                                                          */
/*      <ParameterList> :== "(" <FormalParameter> {"," <FormalParameter>}   */
/*                              ")"                                         */
/*                                                                          */                                                                          */
/*--------------------------------------------------------------------------*/

PRIVATE void ParseParameterList(void)
{
    Accept(LEFTPARENTHESIS);
    ParseFormalParameter();
    
    while (CurrentToken.code == COMMA)
    {
        Accept(COMMA);
        ParseFormalParameter();
    }
    Accept(RIGHTPARENTHESIS);
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  ParseFormalParameter implements:                                        */
/*                                                                          */
/*      <FormalParameter> :== ["REF"] <Variable>                            */
/*                                                                          */
/*                                                                          */
/*--------------------------------------------------------------------------*/

PRIVATE void ParseFormalParameter(void)
{
    if(CurrentToken.code == REF)
        Accept(REF);
        
    ParseVariable();
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  ParseBlock implements:                                                  */
/*                                                                          */
/*      <Block> :== "BEGIN" {<Statement> ";"} "END"                         */
/*                                                                          */
/*                                                                          */
/*--------------------------------------------------------------------------*/

PRIVATE void ParseBlock(void)
{   
    Accept(BEGIN);
        
        while (CurrentToken.code == IDENTIFIER || CurrentToken.code == WHILE ||
            CurrentToken.code == IF || CurrentToken.code == READ ||
            CurrentToken.code == WRITE)  
        {   
        ParseStatement();
        Accept(SEMICOLON);
        }
        Accept(END);
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  ParseStatement implements:                                              */
/*                                                                          */
/*      <Statement> :== <SimpleStatement> | <WhileStatement> |              */
/*                          <IfStatement> | <ReadStatement> |               */
/*                          <WriteStatement>                                */
/*                                                                          */
/*--------------------------------------------------------------------------*/


PRIVATE void ParseStatement(void)
{
    switch(CurrentToken.code)
    {
        case IDENTIFIER:ParseSimpleStatement(); break;
        case WHILE:ParseWhileStatement(); break;
        case IF:ParseIfStatement(); break;
        case READ:ParseReadStatement(); break;
        case WRITE:ParseWriteStatement(); break;
        default:
                    SyntaxError( IDENTIFIER, CurrentToken );
                ReadToEndOfFile();
                    fclose( InputFile );
                    fclose( ListFile );
                    exit( EXIT_FAILURE );
                    break;
    }
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  ParseSimpleStatement implements:                                        */
/*                                                                          */
/*      <SimpleStatement> :== <Variable> <RestOfStatement>                  */
/*                                                                          */
/*--------------------------------------------------------------------------*/


PRIVATE void ParseSimpleStatement(void)
{
    ParseVariable();
    ParseRestOfStatement();
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  ParseRestOfStatement implements:                                        */
/*                                                                          */
/*      <ParseRestOfStatement> :== <ProcCallList> | <Assignment> | ϵ        */
/*                                                                          */
/*--------------------------------------------------------------------------*/


PRIVATE void ParseRestOfStatement(void)
{
    switch(CurrentToken.code)
    {
        case LEFTPARENTHESIS:ParseProcCallList(); break;
        case ASSIGNMENT:ParseAssignment(); break;
    }   
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  ParseProcCallList implements:                                           */
/*                                                                          */
/*       <ProcCallList> :== "(" <ActualParameter> {"," <ActualParameter>}   */
/*                          ")"                                             */
/*                                                                          */
/*--------------------------------------------------------------------------*/


PRIVATE void ParseProcCallList(void)
{   
    Accept(LEFTPARENTHESIS);

    ParseActualParameter();
    while (CurrentToken.code == COMMA)
    {       
        Accept(COMMA);
        ParseActualParameter();
        ParseExpression();  
    }
    Accept(RIGHTPARENTHESIS);   
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  ParseAssignment implements:                                             */
/*                                                                          */
/*       <Assignment> :== ":=" <Expression>                                 */
/*                                                                          */
/*--------------------------------------------------------------------------*/


PRIVATE void ParseAssignment(void)
{
    Accept(ASSIGNMENT);
    ParseExpression();
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  ParseActualParameter implements:                                        */
/*                                                                          */
/*       <ActualParameter> :== <Variable> | <Expression>                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/


PRIVATE void ParseActualParameter(void)
{
    if (CurrentToken.code == SUBTRACT)
        Accept(SUBTRACT);
    ParseVariable();
    ParseExpression();
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  ParseWhileStatement implements:                                         */
/*                                                                          */
/*       <WhileStatement> :== "WHILE" <BooleanExpression> "DO" <Block>      */
/*                                                                          */
/*--------------------------------------------------------------------------*/


PRIVATE void ParseWhileStatement(void)
{
    Accept(WHILE);
    ParseBooleanExpression();
    Accept(DO);
    ParseBlock();   
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  ParseIfStatement implements:                                            */
/*                                                                          */
/*       <IfStatement> :== "IF" <BooleanExpression> "THEN" <Block> ["ELSE"  */
/*                            <Block>]                                      */
/*                                                                          */
/*--------------------------------------------------------------------------*/


PRIVATE void ParseIfStatement(void)
{
    Accept(IF);
    ParseBooleanExpression();
    Accept(THEN);
    ParseBlock();
    
    if (CurrentToken.code == ELSE)
    {   
        Accept(ELSE);
        ParseBlock();
    }
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  ParseReadStatement implements:                                          */
/*                                                                          */
/*       <ReadStatement> :== "READ" <ProcCallList>                          */
/*                                                                          */
/*--------------------------------------------------------------------------*/


PRIVATE void ParseReadStatement(void)
{
    Accept(READ);
    ParseProcCallList();
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  ParseWriteStatement implements:                                         */
/*                                                                          */
/*       <WriteStatement> :== "WRITE" <ProcCallList>                        */
/*                                                                          */
/*--------------------------------------------------------------------------*/


PRIVATE void ParseWriteStatement(void)
{
    Accept(WRITE);
    ParseProcCallList();
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  ParseExpression implements:                                             */
/*                                                                          */
/*       <Expression> :== <CompountTerm> {<AddOp> <CompountTerm>}           */
/*                                                                          */
/*--------------------------------------------------------------------------*/


PRIVATE void ParseExpression(void)
{
    ParseCompoundTerm();
    
    while(CurrentToken.code == ADD || CurrentToken.code == SUBTRACT)
    {
        ParseAddOp();
        ParseCompoundTerm();
    }
}
 
/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  ParseCompoundTerm implements:                                           */
/*                                                                          */
/*       <CompoundTerm> :== <Term> {<MultOp> <Term>}                        */
/*                                                                          */
/*--------------------------------------------------------------------------*/
               

PRIVATE void ParseCompoundTerm(void)
{
    ParseTerm();
    while(CurrentToken.code == MULTIPLY || CurrentToken.code == DIVIDE)
    {
        ParseMultOp();
        ParseTerm();
    }
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  ParseTerm implements:                                                   */
/*                                                                          */
/*       <Term> :== ["-"] <SubTerm>                                         */
/*                                                                          */
/*--------------------------------------------------------------------------*/


PRIVATE void ParseTerm(void)
{
    if (CurrentToken.code == SUBTRACT)
        Accept(SUBTRACT);
        
    ParseSubTerm();     
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  ParseSubTerm implements:                                                */
/*                                                                          */
/*       <SubTerm> :== <Variable> | <IntConst> | "(" <Expression> ")"       */
/*                                                                          */
/*--------------------------------------------------------------------------*/



PRIVATE void ParseSubTerm(void)
{
    switch(CurrentToken.code)
    {
        case IDENTIFIER:ParseVariable(); break;
        case INTCONST:ParseIntConst(); break;
        case LEFTPARENTHESIS:Accept(LEFTPARENTHESIS); ParseExpression(); Accept(RIGHTPARENTHESIS); break;
        /*default:
                    SyntaxError( IDENTIFIER, CurrentToken );
                ReadToEndOfFile();
                    fclose( InputFile );
                fclose( ListFile );
                    exit( EXIT_FAILURE );
                break;  */  
    }
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  ParseBooleanExpression implements:                                      */
/*                                                                          */
/*       <BooleanExpression> :== <Expression> <RelOp> <Expression>          */
/*                                                                          */
/*--------------------------------------------------------------------------*/


PRIVATE void ParseBooleanExpression(void)
{
    ParseExpression();
    ParseRelOp();
    ParseExpression();
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  ParseAddOp implements:                                                  */
/*                                                                          */
/*       <AddOp> :== "+" | "-"                                              */
/*                                                                          */
/*--------------------------------------------------------------------------*/


PRIVATE void ParseAddOp(void)
{
    switch(CurrentToken.code)
    {
        case ADD:Accept(ADD); break;
        case SUBTRACT:Accept(SUBTRACT); break;
        default:
                    SyntaxError( MULTIPLY, CurrentToken );
                    ReadToEndOfFile();
                    fclose( InputFile );
                    fclose( ListFile );
                    exit( EXIT_FAILURE );
                    break;
    }
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  ParseMultOp implements:                                                 */
/*                                                                          */
/*       <MultOp> :== "*" | "/"                                             */
/*                                                                          */
/*--------------------------------------------------------------------------*/


PRIVATE void ParseMultOp(void)
{
    switch(CurrentToken.code)
    {
        case MULTIPLY:Accept(MULTIPLY); break;
        case DIVIDE:Accept(DIVIDE); break;
        default:
                    SyntaxError( MULTIPLY, CurrentToken );
                ReadToEndOfFile();
                    fclose( InputFile );
                    fclose( ListFile );
                    exit( EXIT_FAILURE );
                    break;
    }
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  ParseRelOp implements:                                                  */
/*                                                                          */
/*       <RelOp> :== "=" | "<=" | ">=" | "<" | ">"                          */
/*                                                                          */
/*--------------------------------------------------------------------------*/


PRIVATE void ParseRelOp(void)
{
    switch(CurrentToken.code)
    {
        case EQUALITY:Accept(EQUALITY); break;
        case LESSEQUAL:Accept(LESSEQUAL); break;
        case GREATEREQUAL:Accept(GREATEREQUAL); break;
        case LESS:Accept(LESS); break;
        case GREATER:Accept(GREATER); break;
        default:
                    SyntaxError( MULTIPLY, CurrentToken );
                ReadToEndOfFile();
                    fclose( InputFile );
                    fclose( ListFile );
                    exit( EXIT_FAILURE );
                    break;
    }
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  ParseVariable implements:                                               */
/*                                                                          */
/*       <Variable> :== <Identifier>                                        */
/*                                                                          */
/*--------------------------------------------------------------------------*/


PRIVATE void ParseVariable(void)
{
    ParseIdentifier();
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  ParseIntConst implements:                                               */
/*                                                                          */
/*       <Variable> :== <Digit> { <Digit> }                                 */
/*                                                                          */
/*--------------------------------------------------------------------------*/


PRIVATE void ParseIntConst(void)
{
    Accept(INTCONST);
    while(CurrentToken.code == INTCONST)
    {
        Accept(INTCONST);
    }   
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  ParseIdentifier implements:                                             */
/*                                                                          */
/*       <Identifier> :== <Alpha> { <AlphaNum> }                            */
/*                                                                          */
/*--------------------------------------------------------------------------*/


PRIVATE void ParseIdentifier(void)
{
    Accept(IDENTIFIER);
    while(CurrentToken.code == IDENTIFIER)
    {       
        Accept(IDENTIFIER);
    }
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  End of parser.  Support routines follow.                                */
/*                                                                          */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  Accept:  Takes an expected token name as argument, and if the current   */
/*           lookahead matches this, advances the lookahead and returns.    */
/*                                                                          */
/*           If the expected token fails to match the current lookahead,    */
/*           this routine reports a syntax error and exits ("crash & burn"  */
/*           parsing).  Note the use of routine "SyntaxError"               */
/*           (from "scanner.h") which puts the error message on the         */
/*           standard output and on the listing file, and the helper        */
/*           "ReadToEndOfFile" which just ensures that the listing file is  */
/*           completely generated.                                          */
/*                                                                          */
/*                                                                          */
/*    Inputs:       Integer code of expected token                          */
/*                                                                          */
/*    Outputs:      None                                                    */
/*                                                                          */
/*    Returns:      Nothing                                                 */
/*                                                                          */
/*    Side Effects: If successful, advances the current lookahead token     */
/*                  "CurrentToken".                                         */
/*                                                                          */
/*--------------------------------------------------------------------------*/

PRIVATE void Accept( int ExpectedToken )
{
    int static recovery = 0;
    if(recovery){
        while(CurrentToken.code != ExpectedToken && CurrentToken.code != ENDOFINPUT)
            CurrentToken = GetToken();
        recovery = 0
    }
    if(CurrentToken.code != ExpectedToken ){
        SyntaxError(ExpectedToken , CurrentToken);
        recovery = 1;
    }
    else  CurrentToken = GetToken();
}


/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  OpenFiles:  Reads strings from the command-line and opens the           */
/*              associated input and listing files.                         */
/*                                                                          */
/*    Note that this routine mmodifies the globals "InputFile" and          */
/*    "ListingFile".  It returns 1 ("true" in C-speak) if the input and     */
/*    listing files are successfully opened, 0 if not, allowing the caller  */
/*    to make a graceful exit if the opening process failed.                */
/*                                                                          */
/*                                                                          */
/*    Inputs:       1) Integer argument count (standard C "argc").          */
/*                  2) Array of pointers to C-strings containing arguments  */
/*                  (standard C "argv").                                    */
/*                                                                          */
/*    Outputs:      No direct outputs, but note side effects.               */
/*                                                                          */
/*    Returns:      Boolean success flag (i.e., an "int":  1 or 0)          */
/*                                                                          */
/*    Side Effects: If successful, modifies globals "InputFile" and         */
/*                  "ListingFile".                                          */
/*                                                                          */
/*--------------------------------------------------------------------------*/

PRIVATE int  OpenFiles( int argc, char *argv[] )
{
    

    if ( argc != 3 )  {
        fprintf( stderr, "%s <inputfile> <listfile>\n", argv[0] );
        return 0;
    }

    if ( NULL == ( InputFile = fopen( argv[1], "r" ) ) )  {
    fprintf( stderr, "cannot open \"%s\" for input\n", argv[1] );
    return 0;
    }

    if ( NULL == ( ListFile = fopen( argv[2], "w" ) ) )  {
    fprintf( stderr, "cannot open \"%s\" for output\n", argv[2] );
    fclose( InputFile );
    return 0;
    }

    return 1;
}

