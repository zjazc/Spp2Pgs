/*----------------------------------------------------------------------------
* spp2pgs - Generates BluRay PG Stream from Subtitles or AviSynth scripts
* by Giton Xu <adm@subelf.net>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*----------------------------------------------------------------------------*/

#pragma once

#include <WinDef.h>
#include <tchar.h>

namespace spp2pgs
{
	class ExceptionFormatter
	{
	public:
		ExceptionFormatter() {};
		virtual ~ExceptionFormatter() {};

		bool virtual Invoke(TCHAR * buffer, int bufferSize) const = 0;
	};


	class SystemExceptionFormatter : public ExceptionFormatter
	{
	public:
		SystemExceptionFormatter(const DWORD * pErrno) : pErrno(pErrno) {};
		~SystemExceptionFormatter() {};

		bool Invoke(TCHAR * buffer, int bufferSize) const
		{
			return !FormatMessage(
				FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				*pErrno,
				0,
				buffer,
				bufferSize, NULL);
		}

	private:
		const DWORD * const pErrno;
	};


	class StaticExceptionFormatter : public ExceptionFormatter
	{
	public:
		StaticExceptionFormatter(const TCHAR * const pMessage) : pMessage(pMessage) {};
		~StaticExceptionFormatter() {};

		bool Invoke(TCHAR * buffer, int bufferSize) const
		{
			_tcscpy(buffer, pMessage);
			return true;
		}

	private:
		const TCHAR * const pMessage;
	};
}