(load-extension "runt")

(define freq (ps-mkvar "freq" 440))

(ps-eval 0 "1 1 sine 0 1 biscale dup _freq set 220 440 scale 0.5 sine")
(ps-turnon 0)

(runt-alloc "runt" 512 512)
(define vm (runt-vm "runt"))
(define (on_init)
(begin 
(runt-float vm "freq" freq)

(runt-compile vm 
"'../vg.so' dynload
rec
: blue 110 148 171 255 ;
: red 255 0 0 255 ;
: beige 177 182 150 255 ;
: navy 49 95 128 255 ;
: green 255 0 0 255 ;
: white 255 255 255 255 ;
: circle 40 freq * vgcirc vgf ; 
stop")
(runt-compile vm 
"rec : cb 
blue vgclr 
white vgfc
vgw 0.5 * 80 - vgh 0.5 * circle 
beige vgfc
vgw 0.5 * vgh 0.5 * circle 
navy vgfc
vgw 0.5 * 80 + vgh 0.5 * circle ; 
stop
_cb vgstrt
")))

(define (on_shutdown) (begin (runt-compile vm "'goodbye!' say vgstop")))

(ps-set-init-callback on_init)
(ps-set-shutdown-callback on_shutdown)
