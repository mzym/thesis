#! /usr/bin/make -f
# $Id$

SUBDIRS	=  
ROOT	= ../

default: clean

dic: dic-run.o dic.o bitmap.o itemsets.o profiler.o csvparser.o

dic.phi: dic-run.o.phi dic.o.phi bitmap.o.phi itemsets.o.phi profiler.o.phi csvparser.o.phi

mkbitmap: mkbitmap.o bitmap.o csvparser.o

subzero: subzero.o bitmap.o csvparser.o

subzero_rmendblanks: subzero_rmendblanks.o bitmap.o csvparser.o

include $(ROOT)Makefile.Rules






