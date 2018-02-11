# Copyright (c) 1994-2018 Matt Samudio (Albert Lea Data)  All Rights Reserved.
# Contact information for Albert Lea Data available at http://www.albertleadata.org
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
LPX = 64
PGM = msikb
C_SRC = msikb.c
LDP = -lusb-1.0
CFLG = -g -c -fPIC $(INC)
LFLG = -g -fPIC

all: $(PGM)

clean:
	rm -f $(PGM) $(C_SRC:.c=.o) core

$(PGM): $(C_SRC:.c=.o)
	gcc -o $@ $(LFLG) $(C_SRC:.c=.o) $(LDP)

%.o : %.cc
	g++ -o $*.o -g -Wno-write-strings -c -fPIC $(INC) $(DEFS) $<

%.o: %.c
	gcc $(CFLG) -o $*.o $<
