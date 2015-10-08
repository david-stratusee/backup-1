" File: project.vim
" Author: Jeffy Du <jeffy.du@163.com>
" Version: 0.1
"
" Description:
" ------------
" This plugin provides a solution for creating project tags and cscope files.
" If you want to run this plugin in Win32 system, you need add the system-callings
" (ctags,cscope,find,grep,sort) to your system path. Usually, you can find these
" system-callings in Cygwin.
"
" Installation:
" -------------
" 1. Copy project.vim to one of the following directories:
"
"       $HOME/.vim/plugin    - Unix like systems
"       $VIM/vimfiles/plugin - MS-Windows
"
" 2. Start Vim on your project root path.
" 3. Use command ":ColorLoad" to load project.

if exists('loaded_color')
    finish
endif
let loaded_color=1

if v:version < 700
    finish
endif

" Line continuation used here
let s:cpo_save = &cpo
set cpo&vim

" flag for tags type
" "d" - macro define
" "e" - enum item
" "f" - function
let s:HLUDType = [' ', ' ', ' ']

" HLUDLoad                      {{{1
" load user types
function! s:HLUDLoad(udtagfile)
	if filereadable(a:udtagfile)
		let s:HLUDType = readfile(a:udtagfile)
	endif
endfunction

" HLUDColor                     {{{1
" highlight tags data
" NR-16   NR-8    COLOR NAME
" 0       0       Black 
" 1       4       DarkBlue
" 2       2       DarkGreen
" 3       6       DarkCyan
" 4       1       DarkRed
" 5       5       DarkMagenta
" 6       3       Brown, DarkYellow
" 7       7       LightGray, LightGrey, Gray, Grey
" 8       0*      DarkGray, DarkGrey
" 9       4*      Blue, LightBlue
" 10      2*      Green, LightGreen
" 11      6*      Cyan, LightCyan
" 12      1*      Red, LightRed
" 13      5*      Magenta, LightMagenta
" 14      3*      Yellow, LightYellow
" 15      7*      White
function! s:HLUDColor()
	if !filereadable('./udtags')
		return
	endif

	if strlen(s:HLUDType[0]) > 0
		exec 'syn keyword cUserTypes    ' . s:HLUDType[0]
	endif
	if strlen(s:HLUDType[1]) > 0
		exec 'syn keyword cUserDefines  ' . s:HLUDType[1]
	endif
	if strlen(s:HLUDType[2]) > 0
		exec 'syn keyword cUserFunctions    ' . s:HLUDType[2]
	endif
	if strlen(s:HLUDType[3]) > 0
		exec 'syn keyword cTodo    contained ' . s:HLUDType[3]
	endif

    "exec 'hi cUserTypes ctermfg=DarkCyan guifg=DarkCyan'
    "exec 'hi cUserFunctions ctermfg=Blue guifg=Blue'
    "exec 'hi cUserDefines ctermfg=Brown guifg=Brown'
    "
    "exec 'hi cUserTypes ctermfg=Cyan guifg=DarkCyan'
    "exec 'hi cUserFunctions ctermfg=Green guifg=Blue'
    "exec 'hi cUserDefines ctermfg=LightCyan guifg=Brown'
    "
    exec 'hi link cUserTypes Type'
    exec 'hi link cUserFunctions Identifier'
    exec 'hi link cUserDefines Constant'

endfunction

" ColorLoad                   {{{1
" load project data
function! s:ColorLoad()
	if !filereadable('./udtags')
		return 1
	endif

    "color user defined.
    call s:HLUDLoad('./udtags')
    call s:HLUDColor()

    echon "Load color done."
    return 1
endfunction

" }}}

command! -nargs=0 -complete=file ColorLoad call s:ColorLoad()

aug Project
    au BufEnter,FileType c,cpp,cc,h,hpp call s:ColorLoad()
aug END

nnoremap <leader>jl :ColorLoad<cr>

" restore 'cpo'
let &cpo = s:cpo_save
unlet s:cpo_save

" vim:set foldenable foldmethod=marker:
