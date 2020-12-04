# Copyright 2019 Tjark Weber <tjark.weber@it.uu.se>
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

CXXFLAGS=-Wall -std=c++11 -O3
LDFLAGS=-static

.PHONY: all clean

all: par

par: par.o
	$(CXX) $< -o $@ $(LDFLAGS)

clean:
	rm -f par.o par
