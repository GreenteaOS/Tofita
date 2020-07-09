// The Tofita Kernel
// Copyright (C) 2020  Oleg Petrenko
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, version 3 of the License.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

namespace userCall {
bool userCallHandled(volatile process::Process *process, const TofitaSyscalls syscall) {
	volatile var frame = &process->frame;

	if (syscall == TofitaSyscalls::CreateWindowEx) {
		if (!probeForReadOkay(frame->rdxArg1, sizeof(CreateWindowExPayload)))
			return false;

		var payload = (CreateWindowExPayload *)frame->rdxArg1;

		var window = dwm::OverlappedWindow_create(process->pid);

		frame->raxReturn = window->windowId;
		process->schedulable = true;
		return true;
	}

	if (syscall == TofitaSyscalls::ShowWindow) {
		const uint64_t windowId = frame->rdxArg1;
		const int32_t nCmdShow = frame->r8Arg2;
		var window = dwm::OverlappedWindow_find(process->pid, windowId);

		if (window != null) {
			window->visible = true;
		}

		process->schedulable = true;
		return true;
	}

	if (syscall == TofitaSyscalls::GetMessage) {
		if (!probeForReadOkay(frame->rdxArg1, sizeof(GetMessagePayload)))
			return false;

		var payload = (GetMessagePayload *)frame->rdxArg1;

		if (!probeForWriteOkay((uint64_t)payload->msg, sizeof(wapi::Msg)))
			return false;

		frame->raxReturn = process::getMessage(process, payload);

		// Should sorts of loop forever
		if (frame->raxReturn) {
			process->schedulable = true;
		} else {
			process->awaitsGetMessage = true;
		}
		return true;
	}

	if (syscall == TofitaSyscalls::PostMessage) {
		if (!probeForReadOkay(frame->rdxArg1, sizeof(PostMessagePayload)))
			return false;

		var payload = (PostMessagePayload *)frame->rdxArg1;

		frame->raxReturn = process::postMessage(process, payload);

		process->schedulable = true;
		return true;
	}

	if (syscall == TofitaSyscalls::SwapWindowFramebuffer) {
		if (!probeForWriteOkay(frame->r8Arg2, sizeof(nj::WindowFramebuffer)))
			return false;

		uint64_t windowId = frame->rdxArg1;

		var fb = (nj::WindowFramebuffer *)frame->r8Arg2;
		var window = dwm::OverlappedWindow_find(process->pid, windowId);

		if (window == null)
			return false;

		if (window->fbZeta != null) {
			window->fbCurrentZeta = !window->fbCurrentZeta;
			if (window->fbCurrentZeta) {
				fb->pixels = (nj::Pixel32 *)&window->fbZeta->pixels;
				fb->width = window->fbZeta->width;
				fb->height = window->fbZeta->height;
			} else {
				fb->pixels = (nj::Pixel32 *)&window->fbGama->pixels;
				fb->width = window->fbGama->width;
				fb->height = window->fbGama->height;
			}
		}

		process->schedulable = true;
		return true;
	}

	if (syscall == TofitaSyscalls::GetOrCreateWindowFramebuffer) {
		if (!probeForWriteOkay(frame->r8Arg2, sizeof(nj::WindowFramebuffer)))
			return false;

		uint64_t windowId = frame->rdxArg1;

		var fb = (nj::WindowFramebuffer *)frame->r8Arg2;
		var window = dwm::OverlappedWindow_find(process->pid, windowId);

		if (window == null)
			return false;

		// TODO resize fb *here* on window size change
		if (window->fbZeta == null) {
			let width = (uint32_t)window->width;
			let height = (uint32_t)window->height;

			let bytes = width * height * sizeof(Pixel32);

			// TODO client width
			window->fbZeta = allocateBitmap(width, height);
			window->fbGama = allocateBitmap(width, height);

			memset((void *)window->fbZeta->pixels, 0x66, bytes);
			memset((void *)window->fbGama->pixels, 0x33, bytes);

		}

		if (window->fbCurrentZeta) {
			fb->pixels = (nj::Pixel32 *)&window->fbZeta->pixels;
			fb->width = window->fbZeta->width;
			fb->height = window->fbZeta->height;
		} else {
			fb->pixels = (nj::Pixel32 *)&window->fbGama->pixels;
			fb->width = window->fbGama->width;
			fb->height = window->fbGama->height;
		}

		process->schedulable = true;
		return true;
	}

	return false;
}
} // namespace userCall
