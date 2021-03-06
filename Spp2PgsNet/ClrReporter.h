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

#include <vcclr.h>

#include <ProgressReporter.h>

#include "IProgressReporter.h"

namespace spp2pgs
{
	using namespace Spp2PgsNet;

	class ClrReporter final
		: public spp2pgs::ProgressReporter
	{
	public:
		ClrReporter(IProgressReporter ^reporter) : reporterNet(AssertClrArgumentNotNull(reporter)) {}
		~ClrReporter() {}

		void ReportAmount(int amount) { reporterNet->Amount = amount; }
		void ReportProgress(int progress) { reporterNet->Progress = progress; }
		void ReportEnd() { reporterNet->OnTaskEnd(); }
		bool IsCanceled() { return reporterNet->IsCanceled; }

	private:
		gcroot<IProgressReporter ^> reporterNet;
	};

}
