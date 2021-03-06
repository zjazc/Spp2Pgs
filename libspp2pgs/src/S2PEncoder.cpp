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

#include "pch.h"

#include "S2PEncoder.h"
#include "GraphicalTypes.h"
#include "BlurayCommon.h"
#include "FrameStreamAdvisor.h"
#include "S2PExceptions.h"
#include "BgraFrame.h"
#include "PgsEncoder.h"

namespace spp2pgs
{
	bool S2PEncoder::Encode(FrameStream *input, PgsEncoder *pgsEnc, ProgressReporter *reporter)
	{
		const TCHAR * p_msg = nullptr;

		try
		{
			int const& tAmount = input->GetFrameCount();

			this->Logger()->Log(S2PLogger::info + S2PLogger::normal,
				_T("Encoding starting, %d frames in total.\n"), tAmount);

			bool isRptInit = false;
			bool isRpt =
				(reporter != nullptr) &&
				(tAmount > 0);
			int tFrameOfs = 0;
			
			BgraFrame buffer[] = { BgraFrame(input->GetFrameSize()), BgraFrame(input->GetFrameSize()) };
			BgraFrame *pRunningFrame = &buffer[0];
			BgraFrame *pCurrentFrame = &buffer[1];

			bool eos = false;
			bool isBlank = true;

			while (!eos && (reporter == nullptr || !reporter->IsCanceled()))
			{
				int const& tIndex = pCurrentFrame->ReadNextOf(input);

				if (!(eos = tIndex == -1) && isRpt)
				{
					if (!isRptInit)
					{
						tFrameOfs = tIndex;
						reporter->ReportAmount(tAmount - tFrameOfs);
						
						isRptInit = true;
					}
					
					reporter->ReportProgress(tIndex - tFrameOfs);
				}

				if (pCurrentFrame->IsIdenticalTo(pRunningFrame))
				{
					continue;
				}

				BgraFrame *output = pRunningFrame;

				if (!isBlank)
				{
					this->Logger()->Log(S2PLogger::info + S2PLogger::verbose,
						_T("Encountered a key frame at FrameId=%d\n"), output->GetFrameIndex());
					pgsEnc->RegistFrame(output, input->GetCurrentIndex() - output->GetFrameIndex());
				}

				pRunningFrame = pCurrentFrame;
				if (!(isBlank = pRunningFrame->IsBlank()))
				{
					pRunningFrame->Normalize();
				}

				pCurrentFrame = output;
			}

			if (reporter != nullptr)
			{
				reporter->ReportEnd();
			}

			if (eos)
			{
				this->Logger()->Log(S2PLogger::info + S2PLogger::normal, _T("Encoding successfully completed.\n"));
			}
			else
			{
				this->Logger()->Log(S2PLogger::info + S2PLogger::normal, _T("Encoding canceled by user.\n"));
			}

			return true;
		}
		catch (TempOutputException)
		{
			p_msg = _T("Cache file creating failed.\n");
		}
		catch (ImageOperationException)
		{
			p_msg = _T("Invalid image operation.\n");
		}
		catch (EpochManagingException)
		{
			p_msg = _T("Invalid epoch managing operation.\n");
		}
		catch (StreamOperationException)
		{
			p_msg = _T("Stream I/O failed.\n");
		}
		catch (EndUserException ex)
		{
			throw ex;
		}
		catch (S2PException)
		{
			p_msg = _T("Unkown exception. Encoding failed.\n");
		}

		if (p_msg != nullptr)
		{
			throw EndUserException(p_msg);
		}

		return false;
	}

}
