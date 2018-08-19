name adlipt

file adlipt
file cmdline
file res_opl2
file res_glue
file res_end

system dos

order
  clname CODE
    segment BEGTEXT
    segment RESIDENT
    segment RESEND
    segment _TEXT
  clname FAR_DATA
  clname BEGDATA
  clname DATA
  clname BSS noemit
  clname STACK noemit

option map
option quiet
