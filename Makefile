#-------------------------------------------------------------------------------
# This file is part of the DataGatheringSystem distribution
#   (https://github.com/nuncio-bitis/DataGatheringSystem
# Copyright (c) 2021 James P. Parziale.
# 
# This program is free software: you can redistribute it and/or modify  
# it under the terms of the GNU General Public License as published by  
# the Free Software Foundation, version 3.
# 
# This program is distributed in the hope that it will be useful, but 
# WITHOUT ANY WARRANTY; without even the implied warranty of 
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
# General Public License for more details.
# 
# You should have received a copy of the GNU General Public License 
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#-------------------------------------------------------------------------------
# Makefile for DataMonitorSystem
#
# J. Parziale  February 2020
#-------------------------------------------------------------------------------

MAKEFILE= $(subst ../../,,$(lastword $(MAKEFILE_LIST)))

PROJECT_ROOT = $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

TOOLS_DIR=$(PROJECT_ROOT)../Tools
INCLUDES += -I$(TOOLS_DIR)/include

#-------------------------------------------------------------------------------

LIB_FILE = $(TOOLS_DIR)/bin/libcpp_tools.a

LDFLAGS = -lpthread #-lrt

OBJS = DataMonitor.o \
	MasterTask.o \
	SensorTask.o \
	UITask.o \
	DataStore.o \
	FluidTempSensor.o

HEADER_FILES = $(shell ls $(PROJECT_ROOT)*.h 2> /dev/null)

#-------------------------------------------------------------------------------

CFLAGS += -O2 -Wall
CFLAGS += $(INCLUDES)

CXXFLAGS += -std=c++2a
CXXFLAGS += $(CFLAGS)

#-------------------------------------------------------------------------------

# The C compilers
CC = /usr/bin/gcc
CXX = /usr/bin/g++

# GNU Assembler
AS=/usr/bin/as
AL=$(AS)

# Linker
LD=/usr/bin/ld

#-------------------------------------------------------------------------------

all: tools DataMonitor

#-------------------------------------------------------------------------------

tools: $(LIB_FILE)
#	$(MAKE) -C $(TOOLS_DIR) $(LIB_FILE)

$(OBJS): $(HEADER_FILES)

DataMonitor: $(OBJS) $(HEADER_FILES)
	@echo "--------------------------------------------------------------------------------"
	@echo " Building $@"
	@echo "--------------------------------------------------------------------------------"
	$(CXX) $(CFLAGS) -o $@ $(OBJS) $(LIB_FILE) $(LDFLAGS)

$(LIB_FILE):
	$(MAKE) -C $(TOOLS_DIR)

#-------------------------------------------------------------------------------

%.o:	$(PROJECT_ROOT)%.cpp
	@echo "--------------------------------------------------------------------------------"
	@echo " Building $@"
	@echo "--------------------------------------------------------------------------------"
	$(CXX) -c $(CFLAGS) $(CXXFLAGS) $(CPPFLAGS) -o $@ $<

%.o:	$(PROJECT_ROOT)%.c
	@echo "--------------------------------------------------------------------------------"
	@echo " Building $@"
	@echo "--------------------------------------------------------------------------------"
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -o $@ $<

#-------------------------------------------------------------------------------

clean:
	$(RM) -fr DataMonitor $(OBJS)

clobber: clean rmlogs
	$(RM) *.exe
	$(RM) -rf *.dSYM
	$(RM) $(TARGETS)

rmlogs:
	$(RM) *.log

#-------------------------------------------------------------------------------
