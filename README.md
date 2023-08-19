#####################################################################
# CS:APP Malloc Lab
# Handout files for students
#
# Copyright (c) 2002, R. Bryant and D. O'Hallaron, All rights reserved.
# May not be used, modified, or copied without permission.
#
######################################################################

## :hourglass: 2023.05.12 ~ 2023.05.18
## :bulb: 시스템 구조 이해 및 심화 자료구조 학습

:bookmark: 블로그 정리
- 동적 메모리 할당 개념 : https://soo-note.tistory.com/69
- implicit, first-fit : https://soo-note.tistory.com/70
- implicit, next-fit : https://soo-note.tistory.com/71
- explicit, first-fit : https://soo-note.tistory.com/72
- segregated list : https://soo-note.tistory.com/73
- segregated, best-fit : https://soo-note.tistory.com/74

## :pushpin: 할당기 구현 방법 : branch로 구분
+ implicit/first-fit :묵시적 가용리스트 - first-fit 으로 구현
+ implicit/next-fit : 묵시적 가용리스트 - next-fit 으로 구현
+ explicit/LIFO : 명시적 가용리스트 - first-fit(LIFO) 으로 구현
+ segregated/best-fit : 분리 가용리스트 - best-fit 으로 구현
+ main : 중간에 브랜치를 생성하여 나눔 = 여러 코드가 합쳐짐

***********
Main Files:
***********

mm.{c,h}	
	Your solution malloc package. mm.c is the file that you
	will be handing in, and is the only file you should modify.

mdriver.c	
	The malloc driver that tests your mm.c file

short{1,2}-bal.rep
	Two tiny tracefiles to help you get started. 

Makefile	
	Builds the driver

**********************************
Other support files for the driver
**********************************

config.h	Configures the malloc lab driver
fsecs.{c,h}	Wrapper function for the different timer packages
clock.{c,h}	Routines for accessing the Pentium and Alpha cycle counters
fcyc.{c,h}	Timer functions based on cycle counters
ftimer.{c,h}	Timer functions based on interval timers and gettimeofday()
memlib.{c,h}	Models the heap and sbrk function

*******************************
Building and running the driver
*******************************
To build the driver, type "make" to the shell.

To run the driver on a tiny test trace:

	unix> mdriver -V -f short1-bal.rep

The -V option prints out helpful tracing and summary information.

To get a list of the driver flags:

	unix> mdriver -h

