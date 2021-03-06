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
#include "PgsEncoder.h"
#include "EpochEncoder.h"

namespace spp2pgs
{
	PgsEncoder::PgsEncoder(S2PContext const *context, StreamEx *output, Size size, BdViFrameRate rate) :
		S2PControllerBase(context), frameRate(rate), frameSize(size),
		output(new PgsWriter(context, size, rate, output)),
		bufMgr(new GxBufferManager(context)), epochManager(context, size, rate),
		imageBuffer(size)
	{
		this->Log(S2PLogger::info + S2PLogger::normal, _T("PgsEncoder Created.\n"));
	}

	PgsEncoder::~PgsEncoder()
	{
		this->EncodeEpoch();
	}

	void PgsEncoder::RegistFrame(BgraFrame *frame, int length)
	{
		if (! frame->IsBlank())
		{
			DisplaySetThumb *thumb = new DisplaySetThumb(frame, length, frameRate, this->Settings()->MaxImageBlockSize());

			if (epochManager.IsNewEpochStart(thumb))
			{
				this->Log(S2PLogger::info + S2PLogger::normal, _T("Epoch#%d started at FrameId=%d\n"
					"Start encoding the previous epoch, %d compositions in total...\n"), ++epochCount,
					frame->GetFrameIndex(), epochManager.Length());
				this->EncodeEpoch();
				this->Log(S2PLogger::info + S2PLogger::normal, _T("Epoch#%d is successfully encoded.\n"), epochCount);
			}
			
			imageBuffer.ImportFrom(frame, thumb->GetDataCrop());

			epochManager.EnqueueDisplaySet(thumb, &imageBuffer);
		}
	}

	void PgsEncoder::EncodeEpoch()
	{
		if (epochManager.IsEmpty())
		{
			return;
		}

		EpochEncoder epochEnc{ this->context, bufMgr.get(), this->epochManager.GetMergedBinaryBoard(), this->epochManager.GetMaxOfCropSize() };
		auto wndDesc = epochEnc.GetWindowsDescriptor();
		output->StartEpoch(wndDesc);

		for (;;)
		{
			for (int i = 0; i < wndDesc->count; ++i)
			{
				imageBuffer.Erase(wndDesc->windows[i]);
			}
			auto thumb = epochManager.DequeueDisplaySet(&imageBuffer);

			if (thumb == nullptr)
			{
				break;
			}

			try
			{
				this->Log(S2PLogger::info + S2PLogger::verbose, _T("Encoding composition at PTS=%lld\n"), thumb->GetPTS());

				auto cmpn = epochEnc.EncodeComposition(thumb, &imageBuffer);

				this->Log(S2PLogger::info + S2PLogger::verbose, _T("Encoded a composition\n"
					"\tPTS=%12lld, PalRef=%d, ObjRef0=%2d, ObjRef1=%2d\n"),
					thumb->GetPTS(), cmpn->paletteRef->GetId(),
					cmpn->cmpnObjs[0].objectRef->GetId(), (cmpn->cmpnObjsCount == 2) ? cmpn->cmpnObjs[1].objectRef->GetId() : -1);

				output->WriteComposition(cmpn);
				this->Log(S2PLogger::info + S2PLogger::verbose, _T("\tThe Composition is written.\n"));

			}
			catch (GxBufferException)
			{
				output->IgnoreComposition(thumb->GetPTS(), thumb->GetETS());

				this->Log(S2PLogger::warning + S2PLogger::normal, _T("Ingnored a composition at PTS=%lld ~ ETS=%lld.\n"
					"\tAllocate buffer for composition failed.\n"), thumb->GetPTS(), thumb->GetETS());
			}
			catch (...)
			{
				do
				{
					delete thumb;
					thumb = epochManager.DequeueDisplaySet(&imageBuffer);

				} while (thumb != nullptr);
				
				throw;
			}

			if (thumb != nullptr)
			{
				delete thumb;
				thumb = nullptr;
			}
		}

		output->EndEpoch();
	}

}