set foldmethod=indent
set foldlevel=100
if has("gui_running")
    " 关闭菜单
    set guioptions-=m
    " 关闭工具栏
    set guioptions-=T
    " 关闭左侧滚动条
    set guioptions-=L
    set guioptions-=l
    " 关闭右侧滚动条
    set guioptions-=r
    " 标签页使用文本显示
    set guioptions-=e
    " 启动时最大化
    autocmd GUIEnter * simalt ~x
endif

set belloff=all
set showcmd
" 在命令模式下按 Tab 键，展示候选词
set wildmenu

" ================================================================
" 编辑器配置
" ================================================================
" 显示行号
set number
" 显示语法高亮
syntax enable
syntax on
" 突出显示当前行
set cursorline
" 开启自动缩进
set autoindent
" 智能缩进
set smartindent

" 编辑时按一个 Tab 键相当于输入4个空格
set tabstop=4
" 格式化时缩进尺寸为4个空格，即 >>、<< 、==（取消全部缩进）时，每一级的字符数。
set shiftwidth=4
" 让 Vim 把连续的空格视为一个 Tab, 删除时可以一次删掉一个 Tab 的空格数量
set softtabstop=4
" 把制表符转换为多个空格, 具体空格数量参考 tabstop 和 shiftwidth
set expandtab
" 在行和段的开始出使用 Tab
set smarttab

" ================================================================
" 编码
" ================================================================
" 设置vim内部编码
set encoding=utf-8
" 设置编辑文件时的编码
set fileencoding=utf-8
" 设置 Vim 能识别的编码
set fileencodings=ucs-bom,utf-8,cp936,gb18030,gb2312,big5,cuc-jp,cuc-kr,latin
" 设置终端模式（非 GUI 模式）下的编码
set termencoding=utf-8
" 防止特殊符号无法显示
set ambiwidth=double
set fileformats=unix,dos,mac
" 解决 console 输出乱码
language messages zh_CN.utf-8

" 自动检测文件类型和缩进格式, 并根据文件类型加载插件
"filetype plugin indent on
" 文件被外部改动后, 自动加载
"set autoread
" 不生成备份文件
set nobackup
" 不生成临时文件
set noswapfile
" 不生成 undo 文件
set noundofile

" ================================================================
" 搜索和匹配
" ================================================================
" 高亮显示匹配的括号
set showmatch
" 高亮显示搜索到的关键字
set hlsearch
" 即时搜索
set incsearch
" 智能大小写敏感, 只要有一个字母大写, 就大小写敏感, 否则不敏感
set ignorecase smartcase

autocmd! bufwritepost *.vimrc source <sfile>

map <C-n> :lne<CR>
map <C-p> :lp<CR>

" 移动分割窗口
nmap <C-j> <C-W>j
nmap <C-k> <C-W>k
nmap <C-h> <C-W>h
nmap <C-l> <C-W>l

" 正常模式下 alt+(-+<>对应键) 调整分割窗口大小
nnoremap <M-=> <C-W>+
nnoremap <M--> <C-W>-
nnoremap <M-,> <C-W><
nnoremap <M-.> <C-W>>

function SearchInFiles()
    let $ext = expand("%:e")
    let word = expand("<cword>")
    if $ext == "c" || $ext == "cpp" || $ext == "h"
    else
        "execute "<C-W>b:Grep" . '"\<"' . word . '"\>"' . "." . ext
        "execute 'yiw<CR><C-W>b:Grep "\<<C-R>"\>" *.lua<CR>'
    endif
endfunction

" 命令模式下的行首尾
if has("win32")
    set grepprg=ag
    command! -complete=filetype -nargs=1 FiletypeReload bufdo if &ft =~# '\<<args>\>' | doautocmd FileType | endif
    "command! -nargs=+ Grep execute 'silent lgrep! /s /r <args>' | botright lopen
    command! -nargs=+ Grep execute 'silent lgrep! -f --vimgrep <args>' | botright lopen
    map <F3> :Grep <C-R><C-W> *%:e<CR>
    augroup search_in_files
        autocmd!
        autocmd filetype lua map <leader>f yiw<CR><C-W>b:Grep --lua -w <C-R>" <CR>
        autocmd filetype yaml map <leader>f yiw<CR><C-W>b:Grep --yaml -w <C-R>" <CR>
        autocmd filetype h,c,cpp map <leader>f yiw<CR><C-W>b:Grep --cpp --cc -w <C-R>" <CR>
        autocmd filetype cs map <leader>f yiw<CR><C-W>b:Grep --cs -w <C-R>" <CR>
        "autocmd filetype h,c,cpp map <leader>f yiw<CR><C-W>b:Grep -g "*.{cpp,c,h}" -w <C-R>" <CR>
    augroup end
    map <leader>g :call SearchInFiles()<CR>
    map <leader>o :FZF<CR>
    map <leader>up :silent !adb push % /storage/emulated/0/Android/data/com.joywinds.rot2/files/%:.:gs?\\?/?<CR>
    map <leader>rm :silent !adb shell rm /storage/emulated/0/Android/data/com.joywinds.rot2/files/%:.:gs?\\?/?<CR>
    map <leader>re :silent !adb shell am start -S -n com.joywinds.rot2/com.unity3d.player.UnityPlayerActivity<CR>
    nmap <M-c> <C-C><C-W>c
    " 打开当前目录 windows
    map <leader>ex :silent !explorer %:p:h<CR>

    " 打开当前目录CMD
    map <leader>cmd :silent !start<cr>
    nmap <C-F5> :!start cmd /c .run.bat <CR>
    " 设置字体
    "set guifont=Courier_New:h10
    set guifont=DejaVu_Sans_Mono:h10
    set guifontwide=楷体
else
    nmap <C-F5> :silent !.run <CR>
    set guifont=Consolas\ 10
endif

let g:netrw_list_hide= netrw_gitignore#Hide().'.*\.meta$'
let g:ft_ignore_pat = '\.\(Z\|gz\|bz2\|zip\|tgz\|meta\)$'
set wildignore=*.obj,*.o,*~,*.pyc,*.meta

" Put these lines at the very end of your vimrc file.

" Load all plugins now.
" Plugins need to be added to runtimepath before helptags can be generated.
packloadall
" Load all of the helptags now, after plugins have been loaded.
" All messages and errors will be ignored.
silent! helptags ALL
let g:ale_set_loclist=0
let g:ale_set_quickfix=1
"let b:ale_lua_luacheck_executable='luacheck'
source $VIMRUNTIME/delmenu.vim 
source $VIMRUNTIME/menu.vim 

