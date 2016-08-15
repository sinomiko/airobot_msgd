DEBUG ?= 1
CC = gcc
CCFLAGS = -g -O2 -std=gnu99 
CXX = g++
CXXFLAGS = -g -O2 -std=c++11
PACKAGE = airobot_msgd
PACKAGE_NAME = $(PACKAGE)
PACKAGE_STRING = $(PACKAGE_NAME)1.0
PACKAGE_VERSION = 1.0
SHELL = /bin/bash
VERSION = 1.0
SRC_DIRS = ./source
EXTRAFLAGS = -I./include $(shell mysql_config --cflags --libs) -lboost_system

OBJDIR = ./obj

vpath %.cpp $(SRC_DIRS)

srcs = $(filter-out main.cpp, $(notdir $(wildcard $(SRC_DIRS)/*.cpp)))
objs = $(srcs:%.cpp=$(OBJDIR)/%.o)

all : $(PACKAGE)
.PHONY : all

$(PACKAGE) : $(objs) 
	- @mkdir -p $(OBJDIR)
	$(CXX) -c $(CXXFLAGS) $(EXTRAFLAGS) $(SRC_DIRS)/main.cpp -o $(OBJDIR)/main.o
	$(CXX) $(OBJDIR)/main.o $^ $(CXXFLAGS) $(EXTRAFLAGS) -o $@

$(objs) : $(OBJDIR)/%.o: %.cpp
	@mkdir -p $(OBJDIR)
	$(CC) -MMD -c $(CCFLAGS) $(EXTRAFLAGS) $< -o $@ 

#check header for obj reconstruction
-include $(OBJDIR)/*.d

.PHONY : clean 
clean :	
	-rm -fr $(OBJDIR)
	-rm -fr $(PACKAGE)
