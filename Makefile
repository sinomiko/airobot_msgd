DEBUG ?= 1
CC = gcc
CCFLAGS = -g -O0 -std=gnu99 
CXX = g++
CXXFLAGS = -g -O0 -std=c++11
PACKAGE = airobot_msgd
PACKAGE_NAME = $(PACKAGE)
PACKAGE_STRING = $(PACKAGE_NAME)1.0
PACKAGE_VERSION = 1.0
SHELL = /bin/bash
VERSION = 1.0
SRC_DIRS = ./source
EXTRAFLAGS =  -I/home/user/Dropbox/ReadTheCode/boost/installdir/include -L/home/user/Dropbox/ReadTheCode/boost/installdir/lib
EXTRAFLAGS += -DBOOST_LOG_DYN_LINK 
EXTRAFLAGS += -lboost_system -lboost_thread -lboost_date_time -lboost_regex -lboost_log -lboost_log_setup 
EXTRAFLAGS += -I./include $(shell mysql_config --cflags --libs)
EXTRAFLAGS += -Wall -Wextra -march=native

OBJDIR = ./obj

vpath %.cpp $(SRC_DIRS)

srcs = $(filter-out main.cpp, $(notdir $(wildcard $(SRC_DIRS)/*.cpp)))
objs = $(srcs:%.cpp=$(OBJDIR)/%.o)

all : $(PACKAGE)
.PHONY : all

ifeq ($(DEBUG),1)
TARGET_DIR=Debug
else
TARGET_DIR=Release
endif

$(PACKAGE) : $(objs) 
	@test -d $(OBJDIR) || mkdir $(OBJDIR)
	@test -d $(TARGET_DIR) || mkdir $(TARGET_DIR)
	$(CXX) -c $(CXXFLAGS) $(EXTRAFLAGS) $(SRC_DIRS)/main.cpp -o $(OBJDIR)/main.o
	$(CXX) $(OBJDIR)/main.o $^ $(CXXFLAGS) $(EXTRAFLAGS) -o $(TARGET_DIR)/$(PACKAGE)

$(objs) : $(OBJDIR)/%.o: %.cpp
	@test -d $(OBJDIR) || mkdir $(OBJDIR)
	$(CXX) -MMD -c $(CXXFLAGS) $(EXTRAFLAGS) $< -o $@ 

#check header for obj reconstruction
-include $(OBJDIR)/*.d

.PHONY : clean 
clean :	
	-rm -fr $(OBJDIR)
	-rm -fr $(TARGET_DIR)/$(PACKAGE)
