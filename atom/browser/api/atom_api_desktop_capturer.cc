// Copyright (c) 2015 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#include "atom/browser/api/atom_api_desktop_capturer.h"

using base::PlatformThreadRef;

#include "atom/common/api/atom_api_native_image.h"
#include "atom/common/native_mate_converters/gfx_converter.h"
#include "base/strings/utf_string_conversions.h"
#include "chrome/browser/media/desktop_media_list.h"
#include "content/public/browser/desktop_capture.h"
#include "native_mate/dictionary.h"
#include "third_party/webrtc/modules/desktop_capture/desktop_capture_options.h"
#include "third_party/webrtc/modules/desktop_capture/desktop_capturer.h"

#include "atom/common/node_includes.h"

namespace mate {

template<>
struct Converter<DesktopMediaList::Source> {
  static v8::Local<v8::Value> ToV8(v8::Isolate* isolate,
                                   const DesktopMediaList::Source& source) {
    mate::Dictionary dict(isolate, v8::Object::New(isolate));
    content::DesktopMediaID id = source.id;
    dict.Set("name", base::UTF16ToUTF8(source.name));
    dict.Set("id", id.ToString());
    dict.Set(
        "thumbnail",
        atom::api::NativeImage::Create(isolate, gfx::Image(source.thumbnail)));
    return ConvertToV8(isolate, dict);
  }
};

}  // namespace mate

namespace atom {

namespace api {

DesktopCapturer::DesktopCapturer(v8::Isolate* isolate) {
  Init(isolate);
}

DesktopCapturer::~DesktopCapturer() {
}

void DesktopCapturer::StartHandling(bool capture_window,
                                    bool capture_screen,
                                    const gfx::Size& thumbnail_size) {
  webrtc::DesktopCaptureOptions options =
    content::CreateDesktopCaptureOptions();

  std::unique_ptr<webrtc::DesktopCapturer> screen_capturer(
      capture_screen ? webrtc::DesktopCapturer::CreateScreenCapturer(options)
                     : nullptr);
  std::unique_ptr<webrtc::DesktopCapturer> window_capturer(
      capture_window ? webrtc::DesktopCapturer::CreateWindowCapturer(options)
                     : nullptr);
  media_list_.reset(new NativeDesktopMediaList(
      std::move(screen_capturer), std::move(window_capturer)));

  media_list_->SetThumbnailSize(thumbnail_size);
  media_list_->StartUpdating(this);
}

void DesktopCapturer::OnSourceAdded(int index) {
}

void DesktopCapturer::OnSourceRemoved(int index) {
}

void DesktopCapturer::OnSourceMoved(int old_index, int new_index) {
}

void DesktopCapturer::OnSourceNameChanged(int index) {
}

void DesktopCapturer::OnSourceThumbnailChanged(int index) {
}

bool DesktopCapturer::OnRefreshFinished() {
  Emit("finished", media_list_->GetSources());
  return false;
}

// static
mate::Handle<DesktopCapturer> DesktopCapturer::Create(v8::Isolate* isolate) {
  return mate::CreateHandle(isolate, new DesktopCapturer(isolate));
}

// static
void DesktopCapturer::BuildPrototype(
    v8::Isolate* isolate, v8::Local<v8::FunctionTemplate> prototype) {
  prototype->SetClassName(mate::StringToV8(isolate, "DesktopCapturer"));
  mate::ObjectTemplateBuilder(isolate, prototype->PrototypeTemplate())
      .SetMethod("startHandling", &DesktopCapturer::StartHandling);
}

}  // namespace api

}  // namespace atom

namespace {

void Initialize(v8::Local<v8::Object> exports, v8::Local<v8::Value> unused,
                v8::Local<v8::Context> context, void* priv) {
  v8::Isolate* isolate = context->GetIsolate();
  mate::Dictionary dict(isolate, exports);
  dict.Set("desktopCapturer", atom::api::DesktopCapturer::Create(isolate));
}

}  // namespace

NODE_BUILTIN_MODULE_CONTEXT_AWARE(atom_browser_desktop_capturer, Initialize);
