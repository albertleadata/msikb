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

This program is named "msikb", because its original purpose
is to manipulate the backlighting for keyboards that alot of
MSi laptops are sold with (made by SteelSeries).

This initial version only allows setting of the color on
all three sectors of the keyboard (left, middle, and right)
to the same color.  Future versions will allow setting
different colors to each sector, and other features of
the backlighting, such as "breathe" and "blink" modes.

The program takes one command-line argument, which should
look like:

	--rgb=FFFFFF

... which would set the backlighting color for the entire
keyboard to white.  The 6-digit hex value passed is a typical
representation of RGB pixel color, with two hex digits per
color.

The only dependency for the program is libusb-1.0, which
needs to be in the dynamic linker's search path at run-time.

