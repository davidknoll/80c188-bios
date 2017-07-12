;from: "jdm" <jdm1intx@die_spambot_diehome.com>

;***************************************************************
;*
;*
;*       tiny basic for intel 8086
;*
;*
;*        version: 1.1
;*
;*         by
;*
;*        michael sullivan
;*                              based
;*                               on
;*                       li-chen wang's
;*
;*                    8080 tiny basic
;*
;*
;*                    27 june 1982
;*
;*  @copyleft
;*  all wrongs reserved
;*
;* note:
;*  8080 registers have been mapped as follows:
;*
;*  8080  8086
;* -------------------------------------
;*
;*  bc <-> cx
;*  de <-> dx
;*  hl <-> bx
;*
;*
;* vers 1.1 - support ms-dos interupt i/o
;*     improve rnd action
;*     support time and date from ms-dos
;*
;**************************************************************

org 100h         ;standard ms-dos start addr.

section .text

start:
 mov sp,stack    ;set up stack
 mov dx,msg1     ;get sign-on msg
 call prtstg     ;send it
 mov byte [buf_max],80h ;init cmd line buffer

; main
;
; this is the main loop that collects the tiny basic program
; and stores it in memory.
;
; at start, it prints out "(cr)ok(lf)", and initializes the
; stack and some other internal variables. then it prompts
; ">" and reads a line. if the line starts with a nonzero
; number, this number is the line number. the line number
; (in 16 bit binary) and the rest of the line (including
; its (cr))is stored in memory. if a line with the same
; line number is already there, it is replaced by the new
; one. if the rest of the line consists of a (cr) only, it
; is stored and any existing line with the same line
; number is deleted.
;
; after a line is inserted, replaced, or deleted, the
; program loops back and asks for another line. this loop
; will be terminated when it reads a line with zero or no
; line number: control is then transfered to "direct".
;
; the tiny basic program save area starts at the memory
; location labeled "txtbgn" and ends at "txtend". we always
; fill this area starting at "txtbgn", the unfilled portion
; pointed to by the contents of the memory location labeled
; "txtunf".
;
; the memory location "currnt" points to the line number
; that is currently being interpreted. while we ar in this
; loop or while we are interpreting a direct command
; (see next section), "currnt" should point to a 0.

rstart:
 mov sp,stack    ;set stack pointer
_st1:
 call crlf
 mov dx,ok       ;de->string
 sub al,al
 call prtstg     ;print prompt
 mov word [currnt],0 ;current line # = 0
_st2:
 mov word [lopvar],0
 mov word [stkgos],0
_st3:
 mov al,'>'      ;prompt ">" now
 call getln      ;read a line
 push di         ;di -> end of line
_st3a:
 mov dx,buffer   ;dx -> beginning of line
 call tstnum     ;test if it's a number
 mov ah,0
 call ignblnk
 or bx,bx        ;bx:= value of # or 0 if no # found
 pop cx          ;cx -> end of line
 jnz _st3b
 jmp direct
_st3b:
 dec dx
 dec dx
 mov ax,bx       ;get line #
 mov di,dx
 stosw           ;value of line # there
 push cx
 push dx         ;bx,dx -> begin,end
 mov ax,cx
 sub ax,dx
 push ax         ;ax:= # bytes in line
 call fndln      ;find this line in save
 push dx         ;area, dx -> save area
 jnz _st4        ;nz:not found, insert
 push dx         ;z:found, delere it
 call fndnxt     ;find next line
                 ;de -> next lie
 pop cx          ;cx -> line to be deleted
 mov bx,[txtunf] ;bx -> unfilled save area
 call mvup       ;move up to delete
 mov bx,cx       ;txtunf -> unfilled area
 mov [txtunf],bx ;update
_st4:
 pop cx          ;get ready to insert
 mov bx,[txtunf] ;but first check if

 pop ax          ;ax = # chars in line
 push bx         ;is 3 (line # and cr)
 cmp al,3        ;then do not insert
 jz rstart       ;must clear the stack

 add ax,bx       ; compute new txtunf
 mov bx,ax       ; bx -> new unfilled area
_st4a:
 mov dx,txtend   ;check to see if there
 cmp bx,dx       ;is enough space
 jc _st4b        ;sorry, no room for it
 jmp qsorry
_st4b:
 mov [txtunf],bx ;ok, update txtunf
 pop dx          ;dx -> old unfilled area
 call mvdown
 pop dx          ;dx -> begin, bx -> end
 pop bx
 call mvup       ;move new line to save area
 jmp _st3

tstv:
 mov ah,64       ;test variables
 call ignblnk
 jc ret01
tstv1:
 jnz tv1         ;not @ array
 call parn       ;@ should be followed
 add bx,bx
 jnc ss1b        ;is index too big?
 jmp qhow
ss1b:
 push dx         ;will it overwrite
 xchg dx,bx      ;text?
 call size       ;find size of free
 cmp bx,dx       ;and check that
 jnc ss1a        ;iff so, say "sorry"
 jmp asorry
ss1a:
 mov bx,varbgn   ;iff not, get address
 sub bx,dx       ;of @(expr) and put it
 pop dx          ;in hl
ret01:
 ret             ;c flag is cleared
tv1:
 cmp al,27       ;not @, is it a to z?
 jc ret2         ;iff not, return c flag
 inc dx
tv1a:
 mov bx,varbgn   ;compute address of
 mov ah,0        ;clear upper byte
 add ax,ax       ;ax:=ax*2 (word storage)
 add bx,ax       ;bx:=varbgn+2*al
ret2:
 ret             ;use carry as error indicator

;
; tstnum - at entry dx -> buffer of ascii characters
;
tstnum:
 mov bx,0        ;****tstnum****
 mov ch,bh       ;test iff the text is
 mov ah,0        ;for cmp in ignblnk
 call ignblnk    ;a number.
tn1:
 cmp al,'0'      ;iff not, return 0 in
 jc ret2         ;b and hl
 cmp al,':'      ;iff numbers, convert
 jnc ret2        ;to binary in bx and
 mov al,0f0h     ;set al to # of digits
 and al,bh       ;iff bh>255, there is no
 jnz qhow        ;room for next digit
 inc ch          ;ch counts number of digits
 push cx

 mov ax,bx       ;bx:=10*bx+(new digit)
 mov cx,10
 push dx         ;save dx
;mul ax,cx
 mul cx
 mov bx,ax       ;partial result now in bx
 pop dx          ;restore
 mov si,dx
 lodsb           ;ascii digit in al now
 sub al,48       ;convert to binary
 mov ah,0
 add bx,ax       ;full result now in bx
 pop cx
 lodsb           ;repeat for more digits
 lahf            ;save flags
 inc dx
 sahf            ;restore flags
 jns tn1         ;quit if no num or overflow

qhow:
 push dx         ;****error: "how?"****
ahow:
 mov dx,how
 jmp error

how:
 db 'how?',0dh
ok:
 db 'ok',0dh
what:
 db 'what?',0dh
sorry:
 db 'sorry',0dh

;
;*
;**********************************************************
;*
;* *** tables *** direct *** & exec ***
;*
;* this section of the code tests a string against a table.
;* when a match is found, control is transferred to the section
;* of code according to the table.
;*
;* at 'exec' dx should point to the string and bx should point
;* to the table-1. at 'direct', dx should point to the string,
;* bx will be set up to point to tab1-1, which is the table of
;* all direct and statement commands.
;*
;* a '.' in the string will terminate the test and the partial
;* match will be considered as a match. e.g., 'pr.',
;* 'pri.', 'prin.', or 'print' will all match 'print'.
;*
;* the table consists of any number of items. each item
;* is a string of characters with bit 7 set to 1 in last char
;* a jump address is stored following each character entry.
;*
;* end of table is an item with a jump address only. if the
;* string does not match any of the other items, it will
;* match this null item as default. the default is indicated
;* by following the 80h default indicator.
;*

tab1:   equ $    ;direct commands
 db 'lis','t' | 80h
 dw list         ;execution addresses
 db 'edi','t' | 80h
 dw edit
 db 'e' | 80h
 dw edit         ;have short form defined also
 db 'ru','n' | 80h
 dw run
 db 'ne','w' | 80h
 dw new
 db 'loa','d' | 80h
 dw dload
 db 'sav','e' | 80h
 dw dsave
 db 'by','e' | 80h ;exit tbasic
 dw bye
tab2: equ $      ;direct/statement
 db 'nex','t' | 80h
 dw next         ;execution addresses
 db 'le','t' | 80h
 dw let
 db 'ou','t' | 80h
 dw outcmd
 db 'pok','e' | 80h
 dw poke
 db 'wai','t' | 80h
 dw waitcm
 db 'i','f' | 80h
 dw iff
 db 'got','o' | 80h
 dw goto
 db 'gosu','b' | 80h
 dw gosub
 db 'retur','n' | 80h
 dw return
 db 're','m' | 80h
 dw rem
 db 'fo','r' | 80h
 dw for
 db 'inpu','t' | 80h
 dw input
 db 'prin','t' | 80h
 dw print
 db 'sto','p' | 80h
 dw stop
 db 128          ;signals end
                 ;remember to move default down.
 dw deflt        ;last posibility

tab4: equ $      ;functions
 db 'rn','d' | 80h
 dw rnd
 db 'in','p' | 80h
 dw inp
 db 'pee','k' | 80h
 dw peek
 db 'us','r' | 80h
 dw usr
 db 'ab','s' | 80h
 dw myabs
 db 'siz','e' | 80h
 dw size
 db 128          ;signals end
                 ;you can add more functions but remember
                 ;to move xp40 down
 dw xp40

tab5: equ $      ;"to" in "for"
 db 't','o' | 80h
tab5a: dw fr1
 db 128
 dw qwhat
tab6: equ $      ;"step" in "for"
 db 'ste','p' | 80h
tab6a: dw fr2
 db 128
 dw fr3
tab8: equ $      ;relation operators
 db '>','=' | 80h
 dw xp11         ;execution address
 db '#' | 80h
 dw xp12
 db '>' | 80h
 dw xp13
 db '=' | 80h
 dw xp15
 db '<','=' | 80h
 dw xp14
 db '<' | 80h
 dw xp16
 db 128
 dw xp17
;
; end of parser action table
;

;
; at entry bx -> command table (above)
;    dx -> command line (i.e. "buffer")

direct:
 mov bx,tab1-1   ;***direct***
         ;*
exec: equ $      ;***exec***
ex0:
 mov ah,0
 call ignblnk    ;ignore leading blanks
 push dx         ;save pointer
 mov si,dx
ex1:
 lodsb           ;get char where dx ->
 inc dx          ;preserve pointer
 cmp al,'.'      ;we declare a match
 jz ex4
 inc bx
 mov ah,[bx]
 and ah,127      ;strip bit 7
; and al,0dfh     ; uppercase al
 or al, 20h      ; lowercase al

 cmp al,ah       ;comparison now easy
 jz ex2
                 ;no match - check next entry
ex0a:
 cmp byte [bx],128 ;byte compare
 jnc ex0b
 inc bx
 jmp ex0a
                 ;at this point have last letter
ex0b:
 add bx,3        ;get past execution address
 cmp byte [bx],128 ;found default?
 jz ex3a         ;if so, execute default
 dec bx          ;correct for pre-increment
 pop dx          ;restore pointer
 jmp ex0         ;look some more for a match
ex4:
 inc bx
 cmp byte [bx],128
 jc ex4
 jmp ex3
;
ex3a:
 dec si
 jmp ex3         ;correct si for default execution
ex2:
 cmp byte [bx],128 ;end of reserved word?
 jc ex1          ;no - check some more
                 ;at this point need to get execution address

ex3: inc bx      ;bx -> execution address
 pop ax          ;clear stack
 mov dx,si       ;reset pointer
 jmp [bx]        ;do it

;
; what follows is the code to ececute direct and statement com-
; mands. control is transfered to these points via the  command
; table lookup code of 'direct' and 'exec' in the last section.
; after the command is executed,  control  is  transferred   to
; other sections as follows:
;
; for 'list','new', ans 'stop': go back to 'rstart'
;
; for 'run',: go execute the first stored line iff any; else
;   go back to rstart.
;
; for 'goto' and 'gosub': go execute the target line.
;
; for 'return' and 'next': go back to saved return line.
;
; for all others: iff 'currnt' -> 0, go to 'rstart', else
;   go execute next command. (this is done
;   in 'finish'.)
;
;
; ****new****stop****run (& friends)****goto****
;
; 'new(cr)' sets 'txtunf' to point to 'txtbgn'
;
; 'stop(cr)' goes back to 'rstart'
;
; 'run(cr)' finds the first stroed line, stores its address
; (in 'currnt'), and start to execute it. note that only
; those commands in tab2 are legal for stored programs.
;
; there are three more entries in 'run':
;
; 'runnxl' finds next line, stores its addr and exec it.
; 'runtsl' stores the address of this line and executes it
; 'runsml' continues the execution on same line.
;
; 'goto(expr)' evaluates the expression, finds the target line,
; and jumps to 'runtsl' to do it.
;
; 'dload' loads a named program from disk (anyname.tbi)
;
; 'dsave' saves a named program on disk
;
; 'fcbset' sets up the msdos file control block for subsequent
; disk i/o.
;
new:
 mov word [txtunf],txtbgn

stop:
 call endchk     ;****stop(cr)****
 jmp rstart

run:
 call endchk     ;****run(cr)****
 mov dx,txtbgn   ;first saved line

runnxl:
 mov bx,0        ;****runnxl****
 call fndlnp     ;find whatever line
 jnc runtsl      ;c: passed txtunf, quit
 jmp rstart

runtsl:
 xchg dx,bx      ;****runtsl****
 mov [currnt],bx ;set 'currnt"->line #
 xchg dx,bx
 inc dx
 inc dx

runsml:
 call chkio      ;****runsml****
 mov bx,tab2-1   ;find command in table 2
 jmp exec        ;and execute it

goto:
 call exp        ;****goto(expr)****
 push dx         ;save for error routine
 call endchk     ;must find a 0dh (cr)
 call fndln      ;find the target line
 jz gt1          ;no such line #
 jmp ahow
gt1: pop ax
 jmp runtsl      ;go do it

bye: equ 0       ;exit address

dload:
 mov ah,0
 call ignblnk    ;ignore blanks
 call tochs      ;query of cylinder-head-sector (CHS) address
 push di
 mov bx,txtbgn
 mov ah,02h	;try read 3x, so drive spins up
 int 13h
 jnc fend	;jump if no error
 mov ah,0	;do a drive reset
 int 13h
 mov ah,0	;try again
 int 13h
 jnc fend	;jump if no error
 mov ah,0	;do a drive reset
 int 13h
 mov ah,0	;try one more time
 int 13h
 jc dloadhow	;error out	

fend:
 mov bp,txtbgn
 mov cx,100h     ;max loops
rdm1:
 inc bp          ;pre inc
 cmp word [bp],0 ;found delimiter?
 loopnz rdm1     ;keep looking
 cmp cl,0        ;mac loops executed?
 jz qhow
 mov [txtunf],bp ;update pointer
 pop dx
 call finish     ;finish
dloadhow:
 pop dx
 jmp qhow

dsave:
 cmp word [txtunf],txtbgn ;see if anything to save
 jnz ds1a
 jmp qwhat
ds1a:
 cmp word [txtunf],txtbgn+512
 jb ds2
 jmp qsorry
ds2:
 mov bp,[txtunf]
 mov word [bp],0 ;set delimiter
 mov ah,0
 call ignblnk    ;ignore blanks
 call tochs      ;query of cylinder-head-sector (CHS) address
 push di
 mov bx,txtbgn
 mov ah,03h
 int 13h
 jc dloadhow
 pop dx
 call finish

tochs:
 mov di,dx
 xor ax,ax
parsec:
 xor bx,bx
 mov bl,[di]
 cmp bl,','
 je parsecend
 cmp bl,'0'
 jnb ds3
 jmp tochswhat
ds3:
 cmp bl,'9'
 ja tochswhat
 sub bl,'0'
 mov cx,10
 mul cx
 test dx,dx
 jnz tochswhat
 add ax,bx
 jc tochswhat
 inc di
 jmp parsec
parsecend:
 inc di
 test ax,0fc00h
 jnz tochswhat
 mov cl,ah
 shl cx,6
 mov ch,al
 push cx
 xor ax,ax
parseh:
 mov bl,[di]
 cmp bl,','
 je parsehend
 cmp bl,'0'
 jb tochswhat
 cmp bl,'9'
 ja tochswhat
 sub bl,'0'
 mov cl,10
 mul cl
 test ah,ah
 jnz tochswhat
 add al,bl
 jc tochswhat
 inc di
 jmp parseh
parsehend:
 inc di
 mov dh,al
 mov dl,[driveID]
 push dx
 xor ax,ax
parses:
 mov bl,[di]
 cmp bl,0dh
 je parsesend
 cmp bl,'0'
 jb tochswhat
 cmp bl,'9'
 ja tochswhat
 sub bl,'0'
 mov cl,10
 mul cl
 test ah,ah
 jnz tochswhat
 add al,bl
 jc tochswhat
 inc di
 jmp parses
parsesend:
 pop dx
 test al,0c0h
 jnz tochswhat
 pop cx
 or cl,al
 mov al,1
 ret
tochswhat:
 mov dx,di
 jmp qwhat

;
; ****list**** and ****print**** and ****edit****
;
; list has two forms:
; 'list(cr)' lists all saved lines
; 'list #(cr)' start list at this line #
; you can stop listing by control c key
;
; print command is 'print ....;' or 'print ....(cr)'
; where '....' is a list of expresions, formats, backarrows, and
; strings. these items are seperated by commas.
;
; a format is a pound sign followed by a number. it controls the
; number of spaces the value of an expression is to be printed.
; ted. it stays effective for the rest of the print, unless
; changed by another format. if no format spec, 6 positions
; will be used.
;
; a string is quoted in a pair of single quotes or double
; quotes.
;
; a back-arrow means generate a (cr) without (lf).
;
; a (crlf) is generated after the entire list has been print or
; if the list is a null list. however if the list ended with a
; comma, no (cr) is generated.
;

list:
 call tstnum     ;test iff there is a #
 call endchk     ;iff no # we get a 0
 call fndln      ;find this or next line
ls1:
 jnc ls2         ;c: passed txtunf
 jmp rstart
ls2:
 call prtln      ;print the line
 call chkio      ;see if ^x or ^c
 call fndlnp     ;find next line
 jmp ls1         ;loop back
;
edit:
 call tstnum     ;test if there is a #
 call endchk     ;at end?
 call fndln      ;find spec line or next line
 push dx         ;save line #
 jnc ed2         ;c: passed txtunf
 pop dx          ;throw away line #
ed1:
 jmp rstart
ed2:
 call prtln      ;print the line
 pop dx          ;get line # back
 mov byte [ocsw],0 ;direct output to buffer
 mov byte [buffer-1],0 ;clear char count
 mov byte [prtln1+1],4 ;print one less space
 mov di,buffer   ;prepare to move
 call prtln
 mov byte [ocsw],0ffh ;redirect output to console
 dec byte [buffer-1] ;avoid cr?
 mov byte [prtln1+1],5 ;restore prtln
 jmp _st3        ;prompt and getline only
print:
 mov cl,6        ;c:= # of spaces
 mov ah,';'      ;check for ';' in ignblnk
 call ignblnk    ;ignore blanks
 jnz pr2         ;jump if ';' not found
 call crlf       ;give cr,lf and
 jmp runsml      ;continue same line
pr2:
 mov ah,0dh
 call ignblnk
 jnz pr0
 call crlf       ;also give crlf and
 jmp runnxl      ;goto next line
pr0:
 mov ah,'#'
 call ignblnk
 jnz pr1
 call exp        ;yes, evaluate expr
 mov cl,bl       ;and save it in c
 jmp pr3         ;look for more to print
pr1:
 call qtstg      ;or is it a string?
 jmp pr8         ;iff not, must be expression
pr3:
 mov ah,','
 call ignblnk
 jnz pr6
 call fin        ;in the list
 jmp pr0         ;list continues
pr6:
 call crlf       ;list ends
 call finish
pr8:
 call exp        ;eval the expr
 push cx
 call prtnum     ;print the value
 pop cx
 jmp pr3         ;more to print?
;
;
; ****gosub**** and ****return****
;
; 'gosub (expr);' or 'gosub expr(cr)' is like the 'goto' command
; except that the current text pointer, stack pointer etc.   are
; saved so that execution can be continued after the  subroutine
; 'return'. in order that 'gosub' can be nested (and even recur-
; sive), the save area must be  stacked.  the  stack  pointer is
; saved in 'stkgos'. the old 'stkgos' is saved in the stack. if
; we are in the main routine, 'stkgos' is zero (this was done by
; the "main" section of the code),  but  we  still  save  it  as
; a flag for no further returns.
;
; 'return(cr)' undoes everything that 'gosub' did, and thus  re-
; turns the execution to the command after the most recent  'go-
; sub'. iff 'stkgos' is zero, it indicates that we never  had  a
; 'gosub' and is thus an error.
;
;
gosub:
 call _pusha     ;save the current 'for'
 call exp        ;parameters
 push dx
 call fndln      ;find the target line
 jz gs1          ;not there, say "how?"
 jmp ahow
gs1:
 mov bx,[currnt] ;found it, save old
 push bx         ;'currnt' old 'stkgos'
 mov bx,[stkgos]
 push bx
 mov bx,0        ;and load new ones
 mov [lopvar],bx
 add bx,sp
 mov [stkgos],bx
 jmp runtsl      ;then run that line
return:
 call endchk     ;there must be a 0dh
 mov bx,[stkgos] ;old stack pointer
 or bx,bx
 jnz ret1        ;so, we say: "what?"
 jmp qwhat
ret1:
 xchg bx,sp      ;else restore it
 pop bx          ;else restore it
 mov [stkgos],bx ;and the old 'stkgos'
 pop bx
 mov [currnt],bx ;and the old 'currnt'
 pop dx          ;old text pointer
 call _popa      ;old "for" parameters
 call finish     ;and we are back home
;
;
; ****for**** and ****next****
;
; 'for' has two forms:
; 'for var=exp1 to exp2 step exp3'
; 'for var=exp1 to exp2'
; the second form means the same as the first form with exp3=1.
;
; tbi will find the variable var and set its value to the cur-
; rent value of exp1. it also  evaluates  exp2  and  exp3  and
; saves all of these together  with  the  text  pointer etc in
; the 'for' save area, which consists of 'lopvar',   'lopinc',
; 'loplmt', 'lopln', and 'loppt'. iff there is already   some-
; thing in the save area (this is  indicated  by  a   non-zero
; 'lopvar'), then the old save area is saved in the stack  be-
; fore the new one overwrites it.
;
; tbi will then dig in the  stack  and  find  out iff     this
; same variable was used in  another  currently  active    for
; loop. it that is the case then the old 'for'   loop is   de-
; ivated (purged from the stack).
;
; 'next var' serves as the logical (not necessarilly physical)
; end of the 'for' loop. the control variable var. is  checked
; with the 'lopvar'. iff they are not the same, tbi diggs   in
; the stack to find the right one  and  purges  all those that
; did not match. either way, tbi then adds the 'step' to  that
; variable and checks the result with the limit.  iff  it   is
; within the limit, control loops back to the command  follow-
; ing the 'for'. iff outside the limit, the save area is purg-
; ed and execution continues.
;
for:
 call _pusha     ;save the old save area
 call setval     ;set the control var.
 dec bx
 mov [lopvar],bx ;save tgat
 mov bx,tab5-1   ;use 'exec' to look
 jmp exec        ;for the word 'to'
fr1:
 call exp        ;evaluate the limit
 mov [loplmt],bx ;save that
 mov bx,tab6-1   ;used 'exec' to look
 jmp exec        ;for the word 'step'
fr2:
 call exp        ;found it, get step
 jmp fr4         ;found it, get step
fr3:
 mov bx,1        ;not found, set to one
fr4:
 mov [lopinc],bx ;save that too
fr5:
 mov bx,[currnt] ;save current line #
 mov [lopln],bx
 xchg dx,bx      ;and text pointer
 mov [loppt],bx
 mov cx,10       ;dig into stack to
 mov bx,[lopvar] ;find 'lopvar'
 xchg dx,bx
 mov bx,cx       ;bx:=10 now
 add bx,sp
 jmp fr7a
fr7:
 add bx,cx
fr7a:
 mov ax,[bx]     ;get that old 'lopvar'
 or ax,ax
 jz fr8          ;0 says no more in it
 cmp ax,dx       ;same as this one?
 jnz fr7
 xchg dx,bx
 mov bx,0        ;the other half?
 add bx,sp
 mov cx,bx
 mov bx,10
 add bx,dx
 call mvdown     ;and purge 10 words
 xchg bx,sp      ;in the stack
fr8:
 mov bx,[loppt]  ;job done, restore de
 xchg dx,bx
 call finish     ;and continue
next:
 call tstv       ;get addr of var
 jnc nx4         ;no variable, "what?"
 jmp qwhat
nx4:
 mov [varnxt],bx ;yes, save it
nx0:
 push dx         ;save text pointer
 xchg dx,bx
 mov bx,[lopvar] ;get var in 'for'
 mov al,bh
 or al,bl        ;0 say never had one
 jnz nx5         ;so we ask: "what?"
 jmp awhat
nx5:
 cmp dx,bx       ;else we check them
 jz nx3          ;ok, they agree
 pop dx          ;no, let's see
 call _popa      ;purge current loop
 mov bx,[varnxt] ;and pop one level
 jmp nx0         ;go check again
nx3:
 mov dl,[bx]     ;come here when agreed
 inc bx
 mov dh,[bx]     ;de = val of var
 mov bx,[lopinc]
 push bx
 add bx,dx
 xchg dx,bx      ;add one step
 mov bx,[lopvar] ;put it back
 mov [bx],dl
 inc bx
 mov [bx],dh
 mov bx,[loplmt] ;hl-> limit
 pop ax
 xchg ah,al
 or ax,ax
 jns nx1         ;step > 0
 xchg dx,bx
nx1:
 call ckhlde     ;compare with limit
 pop dx          ;restore text pointer
 jc nx2          ;outside limit
 mov bx,[lopln]  ;within limit, go
 mov [currnt],bx ;back to the saved
 mov bx,[loppt]  ;'currnt' and text
 xchg dx,bx      ;pointer
 call finish     ;pointer
nx2:
 call _popa      ;purge this loop
 call finish

;
; ****rem**** and ****if**** and ****let*****
;
; 'rem' can be followed by anything and is ignored by tbi. tbi
; treats it like an 'if' with a false condition.
;
; 'if' is followed by an expr. as a condition and one or  more
; commands (including other 'if's) seperated  by  semi-colons.
; note that the word 'then' is not used.  tbi  evaluates   the
; expr. iff it is non-zero, execution continues. iff the expr.
; is zero, the commands that follow are ignored and  execution
; continues at the next line.
;
; 'iput' commans is like the 'print' command, and is  followed
; by a list of items. iff the item is a  string  in  single or
; double quotes, or is a back-arroword  it has the same effedt as
; printed out followed by a colon. then tbi waits for an expr.
; to be typen in. the variable is then  set  to  the  value of
; this expr. iff the variable is proceded by a string  printed
; followed by a colon. tbi then waits for input expr. and sets
; the variable to the value of the expr.
;
; iff the input expr. is invalid,  tbi  will  print  "what?" ,
; "how?",or "sorry" and reprint the prompt and redo the input.
; the execution will not terminate unless you type control-c .
; this is handled in 'inperr'.
;
; 'let' is followed by a list of items seperated  by  commas .
; each item consists of a variable,  an  equal  sign,  and  an
; expr. tbi evaluates the expr. and sets the variable to  that
; value. tbi will also handle 'let' command without the   word
; 'let'. this is done by 'deflt'.
;
rem:
 mov bx,0        ;****rem****
 jmp iff1a       ;jump around expr

iff:
 call exp        ;****if****
iff1a:
 cmp bx,0        ;is the expr = 0?
 jz iff1         ;no, continue
 jmp runsml
iff1:
 call fndskp     ;yes, sikp rest of line
 jc iff2         ;yes, sikp rest of line
 jmp runtsl
iff2:
 jmp rstart      ;yes, sikp rest of line

inperr:
 mov bx,[stkinp] ;****inperr****
 xchg bx,sp      ;restore old stack pointer
 pop bx          ;and old 'currnt'
 mov [currnt],bx
 pop dx
 pop dx          ;redo input

input: equ $     ;****input****
ip1:
 push dx         ;save in case of error
 call qtstg      ;is next item a string?
 jmp ip2         ;no
 call tstv       ;yes, but followed by a
 jc ip4          ;variable? no.
 jmp ip3         ;yes. input var.
ip2:
 push dx         ;save for 'prtstg'
 call tstv       ;must be a var now
 jnc ip2a        ;"what" it is not!
 jmp qwhat
ip2a:
 mov si,dx
 lodsb           ;get ready for 'rtstg'
 mov cl,al
 sub al,al
 mov di,dx
 stosb
 pop dx
 call prtstg     ;print string as prompt
 mov al,cl
 dec dx
 mov di,dx
 stosb
ip3:
 push dx
 xchg dx,bx
 mov bx,[currnt] ;also save 'currnt'
 push bx
 mov bx,ip1
 mov [currnt],bx ;neg number as flag
 mov [stkinp],sp
 push dx         ;old hl
 mov al,':'      ;print this too
 call getln      ;and get a line
ip3a:
 mov dx,buffer   ;points to buffer
 call exp        ;evaluate input
 nop             ;can be 'call endchk'
 nop             ;can be 'call endchk'
 nop             ;can be 'call endchk'
 pop dx          ;ok,get old hl
 xchg dx,bx      ;ok,get old hl
 mov [bx],dx
 pop bx          ;get old 'currnt'
 mov [currnt],bx
 pop dx          ;and get old text pointer
ip4:
 pop ax
 mov ah,','
 call ignblnk
 jnz ip5
 jmp ip1         ;yes, more items
ip5:
 call finish

deflt:
 mov si,dx
 lodsb           ;****deflt****
 cmp al,0dh      ;empty line is ok
 jz lt1          ;else it is 'let'

let:
 call setval     ;****let****
 mov ah,','
 call ignblnk
 jnz lt1
 jmp let         ;item by item
lt1:
 call finish     ;until finish

;
; ****expr****
;
; 'expr' evaluates arithmetical or logical expressions.
; <expr>::=<expr2>
;    <expr2><rel.op><expr2>
;
; where <rel.op> is one of the operators in tab8 and the re-
; sult of these operations is 1 iff true and  0  iff  false.
;
; <expr2>::=(+ or -)<expr3>(+ or -<expr3>(....)
;
; where () are optional and (....) are optional repeats.
;
; <expr3>::=<expr4>(<* or /><expr4>)(....)
; <expr4>::=<variable>
;   <function>
;   (<expr>)
;
; <expr> is recursive so that variable '@' can have an expr
; as index, fuctions can have an <expr> as arguments,   and
; <expr4> can be an <expr> in parantheses.
;

exp:
 call expr2
 push bx
expr1:
 mov bx,tab8-1   ;lookup rel.op
 jmp exec        ;go do it
xp11:
 call xp18
 jc ret4         ;no return hl=0
 mov bl,al       ;yes, return hl=1
 ret
xp12:
 call xp18
 jz ret4         ;false, return hl=0
 mov bl,al       ;true, return hl=1
ret4:
 ret
xp13:
 call xp18       ;rel.op '>'
 jz ret5         ;false
 jc ret5         ;also false, hl=0
 mov bl,al       ;true, hl=1
ret5:
 ret
xp14:
 call xp18       ;rel op '<='
 mov bl,al       ;set hl=1
 jz ret6         ;rel. true, return
 jc ret6         ;rel. true, return
 mov bl,bh       ;else set hl=0
ret6:
 ret
xp15:
 call xp18       ;rel op '='
 jnz ret7        ;false, return hl=0
 mov bl,al       ;else set hl=1
ret7:
 ret
xp16:
 call xp18       ;rel.op '<'
 jnc ret8        ;false, return hl=0
 mov bl,al       ;else set hl=1
ret8:
 ret
xp17:
 pop bx          ;not rel op
 ret             ;return hl=<eptr2>
xp18:
 mov al,cl       ;subroutine for all
 pop bx          ;rel.op's
 pop cx          ;rel.op's
 push bx         ;reverse top of stack
 push cx         ;reverse top of stack
 mov cl,al
 call expr2      ;get 2nd expression
 xchg dx,bx      ;value in de now
 pop ax
 push bx
 mov bx,ax       ;last 3 instr for xthl inst!
 call ckhlde     ;compare 1st with second
 pop dx
 mov bx,0        ;set hl=0, a=1
 mov al,1        ;set hl=0, a=1
 ret

expr2:
 mov ah,'-'
 call ignblnk    ;negative sign?
 jnz xp21
 mov bx,0        ;yes, fake '0-'
 jmp xp26        ;treat like subtract
xp21:
 mov ah,'+'      ;positive sign?
 call ignblnk
xp22:
 call expr3      ;1st <expr3>
xp23:
 mov ah,'+'
 call ignblnk    ;add?
 jnz xp25        ;note offset whas 21 bytes in 8080 version
 push bx         ;yes, save value
 call expr3      ;get 2nd <expr3>
xp24:
 xchg dx,bx      ;2nd in de
 pop ax          ;this + next 2 lines for 8080 xthl inst!!
 push bx
 mov bx,ax       ;bx <-> [sp] now, [sp]->buffer,bx=old expr3
 add bx,dx
 pop dx
 jno xp23        ;check for overflow
xp24a:
 jmp qhow        ;else we have overflow
xp25:
 mov ah,'-'
 call ignblnk    ;subtract?
 jnz ret9
xp26: push bx    ;yes, save 1st <expr3>
 call expr3      ;get 2nd <expr3>
 call chgsgn
 jmp xp24

expr3:
 call expr4      ;get 1st <expr4>
xp31:
 mov ah,'*'
 call ignblnk    ;multiply?
 jnz xp34
 push bx         ;yes, save 1st
 call expr4      ;and get 2nd <expr4>
 xchg dx,bx      ;2nd in de now
 pop ax          ;subsitute for 8080 xthl
 push bx
 imul dx         ;ax:=ax*dx, 8088 1 operand only, mtm
 jo xp32         ;see intel book on overflow flag
 mov bx,ax       ;result now in bx
 jmp xp35        ;look for more
xp34:
 mov ah,'/'
 call ignblnk    ;divide?
 jnz ret9
 push bx         ;yes, save 1st <expr4>
 call expr4      ;and get second one
 xchg dx,bx      ;put 2nd in de
 pop ax          ;replacement for xthl
 push bx
 mov bx,ax
 or dx,dx
 jnz xp34a       ;say "how?"
xp32:
 jmp ahow
xp34a:
 call divide     ;use subroutine
 mov bx,cx       ;get result
 mov cx,6        ;six spaces
xp35:
 pop dx          ;and text pointer
 jmp xp31        ;look for more terms

expr4:
 mov bx,tab4-1   ;find fucntion in tab4
 jmp exec        ;and got do it
xp40:
 call tstv       ;no, not a function
 jc xp41         ;nor a variable
 mov al,[bx]     ;variable
 lahf
 inc bx
 sahf
 mov bh,[bx]     ;value in hl
 mov bl,al       ;value in hl
ret9:
 ret
xp41:
 call tstnum     ;or is it a number?
 mov al,ch       ;# of digits
 or al,al
 jnz xp42        ;ok
parn:
 mov ah,'('
 call ignblnk    ;no digit, must be
 jnz parn1
 call exp        ;"(expr)"
parn1: mov ah,')'
 call ignblnk    ;"(expr)"
 jnz xp43        ;******why check this?******
xp42:
 ret
xp43:
 jmp qwhat       ;else say: "what?"

rnd:
 call parn       ;****rnd(expr)****
 or bx,bx
 jns rnd1        ;must be positive
 jnz rnd1        ;and non-zero
 jmp qhow
rnd1:
 push cx
 push dx
 push ds
 mov ax, 40h
 mov ds, ax
 mov ax, [06ch]
 mov dl, 20
 div dl
 shr ax, 8
 mov dl, 5
 mul dl
 mov dl, al
 pop ds
 mov ax,327
 mov dh,0
;mul ax,dx       ;0<=ax<=32700
 mul dx
 xchg dx,bx
 mov bx,ax
 call divide     ;rnd(n)=mod(m,n)+1
 pop dx
 pop cx
 inc bx
 ret

myabs:
 call parn       ;****abs(expr)****
 call chksgn     ;check sign
 or ax,bx
 jns ret10       ;ok
 jmp qhow        ;so say: "how?"

size:
 mov bx,[txtunf] ;****size****
 push dx         ;get the number of free
 xchg dx,bx      ;bytes between 'txtunf'
sizea:
 mov bx,varbgn   ;and 'varbgn'
 sub bx,dx
 pop dx
ret10:
 ret

;
; ****out**** and ****inp**** and ****wait**** and
; ****poke**** and ****peek**** and ****usr****
;
; 'out i,j(,k,l)'
;
; outputs expression 'j' to port 'i', and may be repeated as
; in data 'l' to port 'k' as many times as needed. this com-
; mand modifies *, a small section of code above address 2k.
;
; 'inp (i)'
;
; this function returns data read from  input  port 'i'  as
; its value. it also modifies code just above 2k.
;
; 'wait i,j,k'
;
; this command reads the status of port 'i', exclusive  or's
; the result with 'k', if the result is one, or if not  with
; zero, and's with 'j' and returns when the result is   non-
; zero. its modified code is also above 2k.
;
; 'poke i,j(,k,l)
;
; this command works like out except that it puts  data  'j'
; into memory location 'i'.
;
; 'peek (i)'
;
; this function works like inp except that it puts data  'j'
; from memory location 'i'.
;
; 'usr(i(,j))'
;
; usr call a machine language subroutine at location 'i'  if
; the optional parameter 'j' is used its value is passed  in
; hl. the value of the function should be returned in hl.
;
outcmd:
 call exp
 mov al,bl
 mov [outio+1],al
 mov ah,','
 call ignblnk
 jz out1         ;found more to work on
 jmp qwhat
out1:
 call exp
 mov al,bl
 call outio
 mov ah,','
 call ignblnk
 jnz outcmd1
 jmp outcmd
outcmd1:
 call finish

waitcm:
 call exp
 mov al,bl
 mov [waitio+1],al
 mov ah,','
 call ignblnk
 jz wt1
 jmp qwhat
wt1:
 call exp
 push bx
 mov ah,','
 call ignblnk
 jnz wait1
 call exp
 mov al,bl
 pop bx
 mov bl,al
 jmp wait2
wait1:
 mov bh,0
wait2:
 jmp waitio

inp:
 call parn
 mov al,bl
 mov [inpio+1],al
 mov bx,0
 jmp inpio
 jmp qwt

poke:
 call exp
 push bx
 mov ah,','
 call ignblnk
 jz pok1
 jmp qwhat
pok1:
 call exp
 mov al,bl
 pop bx
 mov [bx],al
 mov ah,','
 call ignblnk
 jnz pok2
 jmp poke
pok2:
 call finish

peek:
 call parn
 mov bl,[bx]
 mov bh,0
 ret
; jmp qwhat

usr:
 push cx
 mov ah,'('
 call ignblnk
 jnz qwt
 call exp        ;expr
 mov ah,')'
 call ignblnk    ;expr
 jnz pasprm
 push dx
 mov dx,usret
 push dx
 push bx
 ret             ;call usr routine
pasprm:
 mov ah,','
 call ignblnk
 jnz usret1
 push bx
 call exp
 mov ah,')'
 call ignblnk
 jnz usret1
 pop cx
 push dx
 mov dx,usret
 push dx
 push cx
 ret             ;call usr routine
usret:
 pop dx
usret1:
 pop cx
 ret
qwt:
 jmp qwhat

;
; ****divide**** and ****chksgn****
; ****chksgn**** and ****ckhlde****
;
;
; 'divide divides bx by dx, result in cx, remainder in bx
;
; 'chksgn' checks sign of bx. iff +, no change. iff -, change
; sign and flip sign of c
;
; 'chgsgn' changes sign of bx and cl unconditionally.
;
; 'ckhlde' check sign of bx and dx. iff different, bx and dx
; are interchanged. iff same sign, not interchanged.   either
; case, bx and dx are then compared to set the flags.
;

divide:
 push dx         ;preserve dx accross call
 push dx
 xor dx,dx
 pop cx
 mov ax,bx
; idiv ax,cx
 div cx
 mov cx,ax       ;quotient
 mov bx,dx       ;remainder
 pop dx          ;dx restored
 ret

chksgn:
 or bx,bx        ;set flags to check sign
 jns ret11       ;iff -, change sign

chgsgn:
 not bx          ;****chgsgn****
 inc bx
 xor ch,128
ret11:
 ret

ckhlde:
 mov al,bh
 xor al,dh       ;same sign?
 jns ck1         ;yes, compare
 xchg dx,bx
ck1:
 cmp bx,dx
 ret

;
; ****setval**** and ****fin**** and ****endchk****
; ****error**** and friends
;
;
; 'setval' expects a variable, followed by an equal sign and
; then an expr. it evaluates the expr and sets the  variable
; to that value.
;
; 'fin' checks the end of a command. iff it ended with ";" ,
; execution continues. iff it ended with a cr, it finds  the
; next line and continues from there.
;
; 'endchk' checks iff a command is ended with a cr, this  is
; required in certain commands. (goto, return, and stop,etc)
;
; 'error' prints the string pointed by dx (and ends  with  a
; cr). it then prints the line pointed by 'currnt' with a ?.
; inserted at where the old text pointer (should  be  on top
; of the stack) points to. execution of tb is  stopped   and
; tbi is restarted. however, iff 'currnt' -> zero (indicat -
; ing a direct command), the direct command is not printed ,
; and iff 'currnt' -> negative # (indicating 'input' command
; the input line is not printed and execution is not termin-
; ated bur continued at 'inperr').
;
; related to 'error' are the following:
;
; 'qwhat' saves text pointer in stack and gets message
;  "what?"
; 'awhat' just gets message "what?" and jumps to error
;
; 'qsorry' and 'asorry' do the same kind of thing.
;
; 'qhow' and 'ahow' in the zero page section also   do
;  this.
;
setval:
 call tstv       ;see it it's a variable
 jc qwhat        ;"what" no variable
 push bx         ;save addr of variable
 mov ah,'='
 call ignblnk
 jnz sv1
 call exp
 mov cx,bx       ;value in cx now
 pop bx          ;get addr
 mov [bx],cl     ;save value
 inc bx
 mov [bx],ch     ;save value
 ret
sv1:
 jmp qwhat       ;no '=' sign

fin:
 mov ah,';'
 call ignblnk
 jnz fi1
 pop ax
 jmp runsml
fi1:
 mov ah,0dh
 call ignblnk
 jnz fi2
 pop ax
 jmp runnxl      ;run next line
fi2:
 ret             ;else return to caller

endchk:
 mov ah,0dh      ;end with cr?
 call ignblnk
 jz fi2          ;ok, else say "what?"

qwhat:
 push dx         ;****qwhat****
awhat:
 mov dx,what     ;****awhat****
error:
 sub al,al       ;****error****
 call prtstg     ;print 'what?','how?'
 pop dx
 mov si,dx
 lodsb
 push ax         ;save the character
 sub al,al       ;and put a zero there
 mov di,dx
 stosb
 mov bx,[currnt] ;get current line #
 cmp word [currnt],0 ;direct command?
 jnz err1        ;iff zero, just restart
 jmp err2        ;save a byte
err1: mov al,[bx] ;iff negative,
 or al,al
 jns err1a
 jmp inperr      ;redo input
err1a: call prtln ;else print the line
 dec dx
 pop ax
 mov di,dx
 stosb           ;restore the char
 mov al,63       ;print a '?'
 call chrout
 sub al,al       ;and the rest of the
 call prtstg     ;line
err2: jmp rstart
qsorry:
 push dx         ;****qsorry****
asorry:
 mov dx,sorry    ;****asorry****
 jmp error

;
; ****getln**** and ****fndln****
;
;
; 'getln' reads an input line into 'buffer'. it first prompts
; the character in a (given by the caller), then it fills the
; buffer and echos it. it uses bdos primitives to  accomplish
; this. once a full line is read in, 'getln' returns.
;
; 'fndln' finds a line with a given line #(in bx) in the text
; save area. dx is used as the text pointer. iff the line  is
; found, dx will point to the beginning of that line iff that
; line (i.e. the low byte of the line #), and flags are nc&z.
; iff that line is not there and a line with a higher line  #
; is found, dx points to there and flags are nc&nz.  iff   we
; reached the end of text save area and cannot find the line,
; flags are c&nz.
; 'fndln' will initialize dx to the  beginning  of  the  text
; save area to start the search. some other entries  of  this
; routine will not initialize dx and do the search.
;
; 'fndlnp' will start with dx and search for the line #.
;
; 'fndnxt' will bump dx by  2, find a 0dh and then start  the
; search.
; 'fndskp' uses dx to find a cr, and then starts the search.
;

getln:
 call chrout     ;****getln****
 push bx
 cld
 mov di,buffer
gl1:
 push di
 mov ah,0
 int 16h
 pop di
 cmp al,08h
 jne gl2
 cmp di,buffer
 jna gl1
 dec di
 mov ax,0e08h
 mov bx,7
 int 10h
 mov ax,0e20h
 int 10h
 mov ax,0e08h
 int 10h
 jmp gl1
gl2:
 stosb
 cmp al,0dh
 je gl1e
 cmp di,bufend
 je gl3
 mov ah,0eh
 mov bx,7
 int 10h
 jmp gl1
gl3:
 dec di
 jmp gl1
gl1e:
 push di
 call chrout
 pop di
 mov bx,di
 sub bx,buffer
 dec bx
 mov [buffer-1],bl
 pop bx
 mov dx,di
 ret

;
; at entry bx -> line # to be found
;

fndln:
 or bx,bx        ;check sign of bx
 jns fnd1        ;it can't be -
 jmp qhow        ;error
fnd1:
 mov dx,txtbgn
fndlnp:
fl1:
 push bx         ;save line #
 mov bx,[txtunf] ;check iff we passed end
 dec bx
 cmp bx,dx       ;substitute for call 4
 pop bx          ;get line # back
 jc ret13        ;c, nz passed end
 mov si,dx
 lodsw
 cmp ax,bx
 jc fl2
ret13:
 ret             ;nc,z:found;nc,nz:not found

fndnxt:          ;****fndnxt****
 inc dx
fl2:
 inc dx

fndskp:
 mov si,dx
 lodsb           ;****fndskp****
 cmp al,0dh      ;try to find cr
 jnz fl2         ;keep looking
 inc dx
 jmp fl1         ;check iff end of text

;
; **** prtstg **** qtstg **** prtnum **** prtln ****
;
;
; 'prtstg prints a string pointed to by dx. it stops printing
; and returns to caller when either a 0dh is printed or  when
; the next byte is the sames as what was in a  ( given by the
; caller). old al is stored in ch, old ch is lost.
;
; 'qtstg' looks for a back-slash,  single quote,   or  double
; quote. iff none of these, return to caller. if back slash \
; output a odh without a lf. iff single or double quote,print
; the string in the quote and demands a matching unquote. af-
; ter the printing the next 3 bytes of the caller  is skipped
; over (usually a jmp instruction).
;
; 'prtnum' prints the number in hl. leading blanks  are added
; iff needed to pad the number of spaces to the number in  c.
; nowever, iff the number of digits is larger than the number
; in c, all digits are printed anyway. negative sign is  also
; printed and counted in, positive sign is not.
;
; 'prtln' prints a saved text line with line # and all.
;

prtstg:
 mov ch,al       ;****prtstg****
ps1:
 mov si,dx
 lodsb           ;get a char
 lahf            ;preserve flags
 inc dx
 sahf            ;restore flags
 cmp al,ch       ;same as old a?
 jnz ps2         ;yes, return
 ret
ps2:
 call chrout     ;else, print it
 cmp al,0Dh      ;was it a cr?
 jnz ps1         ;no, next
 ret

qtstg:
 mov ah,'"'
 call ignblnk
 jnz qt3
 mov al,34       ;it is a '"'
qt1:
 call prtstg     ;print until another
 cmp al,0dh      ;was last one a cr?
 pop bx          ;return address
 jnz qt2         ;was cr, run next line
 jmp runnxl
qt2:
 inc bx          ;skips two bytes on return!!!!
 inc bx
 jmp bx          ;jump to address in bx
qt3:
 mov ah,39       ;is it a single quote (')?
 call ignblnk
 jnz qt4
 mov al,39       ;yes, do same
 jmp qt1         ;as in ' " '
qt4:
 mov ah,'\'
 call ignblnk    ;is it back-slash?('\')
 jnz qt5
 mov al,141      ;yes, 0dh without lf!
 call chrout     ;do it twice
 call chrout     ;to give tty enough time
 pop bx          ;return address
 jmp qt2
qt5:
 ret             ;none of the above

; on entry bx = binary #,cl = # spaces
;
prtnum:
 push dx         ;****prtnum****
 mov dx,10       ;decimal
 push dx         ;save as a flag
 mov ch,dh       ;ch=sign
 dec cl          ;cl=spaces
 call chksgn     ;check sign
 jns pn1         ;no sign
 mov ch,45       ;ch=sign
 dec cl          ;'-' takes space
pn1:
 push cx         ;save sign % space
pn2:
 call divide     ;divide bx by 10 (in dx)
 or cx,cx        ;cx has quotient
 jz pn3          ;yes, we got all
 pop ax          ;get sign and space count
 push bx         ;save remainder
 dec al          ;dec space count
 push ax         ;save new sign and space count
 mov bx,cx       ;move result to bx
 jmp pn2         ;and divide by 10
pn3:
 pop cx          ;we got all digits in
pn4:
 dec cl          ;the stack
 mov al,cl       ;look at space count
 or al,al
 js pn5          ;no leading blanks
 mov al,32       ;leading blanks
 call chrout
 jmp pn4
pn5:
 mov al,ch       ;print sign
 call chrout     ;maybe, or null
 mov dl,bl       ;last remainder in e
pn6:
 mov al,dl       ;check digit in e
 cmp al,10       ;10 is flag for no more
 pop dx
 jz ret14        ;iff so, return
 add al,48       ;else convert to ascii
 call chrout     ;and print the digit
 jmp pn6         ;go back for more

prtln:
 mov si,dx
 lodsw
 mov bx,ax
 inc dx
 inc dx          ;move pointer
prtln1:
 mov cl,5        ;print 5 digit line #
 call prtnum
 mov al,32       ;followed by a blank
 call chrout
 sub al,al       ;and then the text
 call prtstg
ret14:
 ret

;
;
; **** mvup **** mvdown **** popa **** pusha ****
;
; 'mvup' moves a block up from where dx -> where cx -> until
; dx = bx
;
; 'mvdown' moves a block down from where dx -> to where bx->
; until dx = cx.
;
; 'popa' restores the 'for' loop var save area from the stack.
;
; 'pusha' stacks the 'for' loop variable save area in the stack
;

mvup:
 cmp dx,bx       ;***mvup***
 jz ret15        ;de = hl, return
 mov si,dx
 lodsb           ;get one byte
 mov di,cx
 stosb           ;move it
 inc dx
 inc cx
 jmp mvup        ;until done

mvdown:
 cmp dx,cx
 jz ret15        ;yes, return
md1:
 lahf
 dec dx
 dec bx
 mov si,dx
 lodsb           ;both pointers and
 mov [bx],al     ;then do it
 jmp mvdown      ;loop back

_popa:
 pop cx          ;cx = return addr
 pop bx          ;restore lopvar, but
 mov [lopvar],bx ;=0 means no more
 or bx,bx
 jz pp1          ;yes, go return
 pop bx          ;no, restore others
 mov [lopinc],bx
 pop bx
 mov [loplmt],bx
 pop bx
 mov [lopln],bx
 pop bx
 mov [loppt],bx
pp1:
 push cx         ;cx = return addr
ret15:
 ret

_pusha:
 mov bx,stklmt   ;****pusha****
 call chgsgn
 pop cx          ;cx=ret addr
 add bx,sp
 jc pushb        ;yes, sorry for that.
 jmp qsorry
pushb:
 mov bx,[lopvar] ;else save loop vars
 or bx,bx        ;that will be all
 jz pu1
 mov bx,[loppt]  ;else, more to save
 push bx
 mov bx,[lopln]  ;else, more to save
 push bx
 mov bx,[loplmt]
 push bx
 mov bx,[lopinc]
 push bx
 mov bx,[lopvar]
pu1:
 push bx
 push cx         ;cx = return addr
 ret

;
; **** outc **** chkio ****
;
;
; these are the only i/o routines in tbi.
;
;
; 'chkio' checks the input, iff no input, it will return to  the
; caller with the z flag set. iff there is input, the z flag  is
; cleared and the input byre is in a. however, iff the input  is
; a control-o, the 'ocsw' is complimented, and the z flag is re-
; turned. iff a control-c is read, 'chkio' will restart tbi  and
; does not return to the caller.
;

crlf:
 mov al,0dh      ;****crlf****
chrout:
 cmp byte [ocsw],0
 jz cout1        ;see if output redirected
 push cx         ;save cx on stack
 push dx         ;and dx
 push bx         ;and bx too
 mov [outcar],al ;save chatacter
 mov ah,0eh
 mov bx,7
 int 10h
 mov al,[outcar] ;get char. back
 cmp al,0dh      ;was it a 'cr'?
 jnz done        ;no,done
 mov ax, 0e0ah
 mov bx,7
 int 10h
done:
 mov al,[outcar] ;get char back
idone:
 pop bx          ;get h back
 pop dx          ;and d
 pop cx          ;then h
 ret             ;done at last

cout1:
 cmp byte al,0   ;is it null?
 jz ret16        ;skip it
 stosb           ;store al (char) in buffer
 inc byte [buffer-1] ;increment counter
ret16:
 ret             ;done

chkio:
 push cx         ;save b on stack
 push dx         ;and d
 push bx         ;then h
 mov ah,1
 int 16h
 jnz ci1         ;if ready, get char
 jmp idone       ;restore and return
ci1:
 mov ah,0
 int 16h
 push ax
 mov ah,0eh
 mov bx,7
 int 10h
 pop ax
ci2:
 cmp al,18h      ;is ti control-x?
 jnz idone       ;return and restore if not
 jmp rstart      ;yes, restart tbi

lstrom: equ $    ;all above can be rom
outio:
 out 0ffh,al
 ret

waitio:
 in al, 0ffh
 xor al,bh
 and al,bl
 jz waitio
 call finish

inpio:
 in al, 0ffh
 mov bl,al
 ret

;
; ignblnk
;
; deblanks where dx->
; if (dx)=ah then dx:=dx+1
;

ignblnk:
 mov si,dx
ign1:
 lodsb           ;get char in al
 cmp al,32       ;ignore blanks
 jnz ign2        ;in text (where dx ->)
 inc dx
 jmp ign1
ign2:
 cmp al,ah       ;is search character found at (dx)?
 jnz _ret        ;no, return, pointer (dx) stays
 lahf            ;save results of comparison
 inc dx          ;inc pointer if character matches
 sahf            ;return result of comparison to flags
 _ret:
 ret

finish: pop ax
 call fin        ;check end of command
 jmp qwhat       ;print "what?" iff wrong

section .data

msg1: db '8086 tiny basic v1.1 27 june 82',0dh
ocsw db 0ffh     ;output switch
txtunf dw txtbgn ;-> unfilled text area
driveID db 0     ;drive identifier for LOAD/SAVE operations

section .bss

outcar resb 1    ;output char storage
currnt resw 1    ;points to current line
stkgos resw 1    ;saves sp in 'gosub'
varnxt resw 1    ;temp storage
stkinp resw 1    ;saves sp in 'input'
lopvar resw 1    ;'for' loop save area
lopinc resw 1    ;increment
loplmt resw 1    ;limit
lopln  resw 1    ;line number
loppt  resw 1    ;test pointer
ranpnt resw 1    ;random number pointer

txtbgn resb 2000h
txtend:          ; text area save area ends

buf_max resb 1   ; max chars in buffer
buf_cnt resb 1   ; char count
buffer  resb 80h
bufend:

varbgn resb 54

stklmt resw 400h ;top limit for stack
stack:           ;stack starts here
