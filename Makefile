pwm: pwm.c
	clang pwm.c -lX11 -o pwm

clean:
	rm -f *.o pwm

default: pwm

.PHONY: clean
