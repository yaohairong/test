"mine
if has("win32")
 au GUIEnter * simalt ~x
endif

set nu
set nobk
set noswapfile
set ai
set ci
set si
set is
set hls
set belloff=all
set encoding=utf-8 
set termencoding=utf-8 
set fileencoding=utf-8 
set fileencodings=ucs-bom,utf-8,chinese,cp936

set foldmethod=indent
set foldlevel=100

source $VIMRUNTIME/delmenu.vim 
source $VIMRUNTIME/menu.vim 

language messages zh_CN.utf-8 
set tabstop=4
set softtabstop=4
set shiftwidth=4
set expandtab
augroup Binary  
    au!  
    au BufReadPost *.* if &bin | %!xxd -u -g 1  
    au BufReadPost *.* set ft=xxd | endif  
augroup END  
map <F3> yiw:vim /<C-R>"/j **/*%:e<CR>:cw<CR>
if has("win32")
    nmap <F5> :!start cmd /c .run.bat <CR>
else
    nmap <F5> :silent !.run <CR>
endif
