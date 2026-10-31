#include "GlobeDriver.h"

#include "GlobeProtocol.h"

#include <Entry.h>
#include <Message.h>
#include <OS.h>
#include <Roster.h>


namespace {
	const char* const kGlobeSignature = "application/x-vnd.hGlobe";
	const bigtime_t kGlobeReplyTimeout = 5000000;
	const bigtime_t kGlobeStartDelay = 50000;
	const int32 kGlobeStartAttempts = 40;
}


status_t GlobeDriver::CloseDocument(const EditorDocument& document) {
	if (!document.target.IsValid() || document.session == 0)
		return B_BAD_VALUE;

	BMessage request(kMessageGlobeAutomationClose);
	request.AddInt64("session", document.session);
	BMessage reply;
	status_t status = document.target.SendMessage(&request, &reply,
		kGlobeReplyTimeout, kGlobeReplyTimeout);
	if (status != B_OK)
		return status;

	int32 replyStatus = B_ERROR;
	reply.FindInt32("status", &replyStatus);
	return static_cast<status_t>(replyStatus);
}
//---------------------------------------------------------------------------------------------------------------------------------//


status_t GlobeDriver::GetScrollState(const EditorDocument& document,
	EditorScrollState& state) {
	if (!document.target.IsValid() || document.session == 0)
		return B_BAD_VALUE;

	BMessage request(kMessageGlobeAutomationGetScroll);
	request.AddInt64("session", document.session);
	BMessage reply;
	status_t status = document.target.SendMessage(&request, &reply,
		kGlobeReplyTimeout, kGlobeReplyTimeout);
	if (status != B_OK)
		return status;

	return ReadScrollReply(reply, state);
}
//---------------------------------------------------------------------------------------------------------------------------------//


status_t GlobeDriver::OpenDocument(const char* path,
	EditorDocument& document) {
	document.session = 0;
	document.target = BMessenger();
	if (path == NULL || path[0] == '\0')
		return B_BAD_VALUE;

	entry_ref documentReference;
	status_t status = get_ref_for_path(path, &documentReference);
	if (status != B_OK)
		return status;

	status = be_roster->Launch(kGlobeSignature);
	if (status != B_OK && status != B_ALREADY_RUNNING)
		return status;

	BMessenger globe;
	for (int32 attempt = 0; attempt < kGlobeStartAttempts; ++attempt) {
		globe = BMessenger(kGlobeSignature);
		if (globe.IsValid())
			break;
		snooze(kGlobeStartDelay);
	}
	if (!globe.IsValid())
		return B_TIMED_OUT;

	BMessage request(kMessageGlobeAutomationOpen);
	request.AddRef("refs", &documentReference);
	BMessage reply;
	status = globe.SendMessage(&request, &reply, kGlobeReplyTimeout,
		kGlobeReplyTimeout);
	if (status != B_OK)
		return status;

	int32 replyStatus = B_ERROR;
	reply.FindInt32("status", &replyStatus);
	status = static_cast<status_t>(replyStatus);
	if (status != B_OK)
		return status;

	status = reply.FindInt64("session", &document.session);
	if (status != B_OK)
		return status;
	return reply.FindMessenger("target", &document.target);
}
//---------------------------------------------------------------------------------------------------------------------------------//


status_t GlobeDriver::ReadScrollReply(const BMessage& reply,
	EditorScrollState& state) const {
	int32 replyStatus = B_ERROR;
	reply.FindInt32("status", &replyStatus);
	status_t status = static_cast<status_t>(replyStatus);
	if (status != B_OK)
		return status;

	state.ready = false;
	state.minimum = 0.0f;
	state.maximum = 0.0f;
	state.value = 0.0f;
	state.lineHeight = 0.0f;
	reply.FindBool("ready", &state.ready);
	reply.FindFloat("minimum", &state.minimum);
	reply.FindFloat("maximum", &state.maximum);
	reply.FindFloat("value", &state.value);
	reply.FindFloat("line_height", &state.lineHeight);
	return B_OK;
}
//---------------------------------------------------------------------------------------------------------------------------------//


status_t GlobeDriver::SetScrollValue(const EditorDocument& document,
	float value, EditorScrollState& state) {
	if (!document.target.IsValid() || document.session == 0)
		return B_BAD_VALUE;

	BMessage request(kMessageGlobeAutomationSetScroll);
	request.AddInt64("session", document.session);
	request.AddFloat("value", value);
	BMessage reply;
	status_t status = document.target.SendMessage(&request, &reply,
		kGlobeReplyTimeout, kGlobeReplyTimeout);
	if (status != B_OK)
		return status;

	return ReadScrollReply(reply, state);
}
//---------------------------------------------------------------------------------------------------------------------------------//

