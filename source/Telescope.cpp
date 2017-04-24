/*
**	Telescope (.hpp/.cpp)
**	handling class for telescope
**
**	piScope project https://github.com/march42/piScope
**	(C) Copyright 2017 by Marc Hefter
**
**	This program is free software; you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation; either version 2 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program; if not, write to the Free Software
**	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
**	MA 02110-1301 USA.
*/

#include "Telescope.hpp"

//#include <cstdio>
//#include <cstdlib>
//#include <cstring>
//#include <unistd.h>

//#include <cmath>
//#include <climits>

namespace piScope
{

	Telescope::Telescope()
	{
		//	prepare log
		this->SetLogName("telescope");
	}
	Telescope::~Telescope()
	{
		//	deinit log
		this->~LogFile();
	}


};
