#HW2 LD_PROLOAD

traget program : wget

1.set LD_PROLOAD
  `setenv LD_PROLOAD /path/to/my/inject.so`

2.run `wget http://xxx.xxx.xxx`

3.unset LD_PROLOAD
  `unsetenv LD_PROLOAD`

4.inject message is in 0456085.log
