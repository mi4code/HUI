#ifndef _hui_backend_
#define _hui_backend_

#include <ciso646>

#define _hui_webview_use_common_code
#include "HUI.hh"

#include "hui_datatypes.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <windows.h>
#include <string>
#include <list>
#include <sstream>

#include "include/cef_command_line.h"
//#include "include/cef_sandbox_win.h"  // not using sandbox to save memory
#include "include/cef_app.h"
#include "include/cef_browser.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/wrapper/cef_helpers.h"
#include "include/cef_client.h"
#include "include/base/cef_callback.h"
#include "include/cef_parser.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/wrapper/cef_message_router.h"

#include <chrono>
#include <thread>

// previously based on <https://github.com/chromiumembedded/cef/tree/master/tests/cefsimple> now its mostly ai code
// single process mode <https://github.com/salvadordf/CEF4Delphi/issues/291>
// C++ <--> js comunication <https://bitbucket.org/chromiumembedded/cef/wiki/JavaScriptIntegration.md#markdown-header-functions-and-extensions> <https://gist.github.com/gipi/3028454> <https://magpcss.org/ceforum/viewtopic.php?f=6&t=17511> <https://bitbucket.org/chromiumembedded/cef-project/src/master/examples/message_router/>
// unused resources <https://me.kxd.dev/2013/01/14/chromium-embedded-framework-3-bare-bones/> <https://bitbucket.org/chromiumembedded/cef/wiki/Tutorial> <https://github.com/acristoffers/CEF3SimpleSample>


namespace HUI {


std::vector< std::function<void(std::vector<std::string>)> > sent2cpp_handlers;

std::vector<std::string> cpp_handlers2call;

std::string GetDataURI(const std::string& data, const std::string& mime_type) {
  return "data:" + mime_type + ";base64," + CefURIEncode(CefBase64Encode(data.data(), data.size()), false).ToString();
}

class SimpleWindowDelegate : public CefWindowDelegate { // when using views this provides the delegate for the CefWindow that hosting the browser
 public:
  SimpleWindowDelegate(CefRefPtr<CefBrowserView> browser_view, cef_runtime_style_t runtime_style, cef_show_state_t initial_show_state) : browser_view_(browser_view), runtime_style_(runtime_style), initial_show_state_(initial_show_state) {}

  void OnWindowCreated(CefRefPtr<CefWindow> window) override {
    // add the browser view and show the window
    window->AddChildView(browser_view_);

    if (initial_show_state_ != CEF_SHOW_STATE_HIDDEN) {
      window->Show();
    }

    if (initial_show_state_ != CEF_SHOW_STATE_MINIMIZED && initial_show_state_ != CEF_SHOW_STATE_HIDDEN) {
      // give keyboard focus to the browser view
      browser_view_->RequestFocus();
    }
  }

  void OnWindowDestroyed(CefRefPtr<CefWindow> window) override {
    browser_view_ = nullptr;
  }

  bool CanClose(CefRefPtr<CefWindow> window) override {
    // allow the window to close if the browser says its OK
    CefRefPtr<CefBrowser> browser = browser_view_->GetBrowser();
    if (browser) {
      return browser->GetHost()->TryCloseBrowser();
    }
    return true;
  }

  /*CefSize GetPreferredSize(CefRefPtr<CefView> view) override {
    return CefSize(800, 600);
  }*/

  cef_show_state_t GetInitialShowState(CefRefPtr<CefWindow> window) override {
    return initial_show_state_;
  }

  cef_runtime_style_t GetWindowRuntimeStyle() override {
    return runtime_style_;
  }

 private:
  CefRefPtr<CefBrowserView> browser_view_;
  const cef_runtime_style_t runtime_style_;
  const cef_show_state_t initial_show_state_;

  IMPLEMENT_REFCOUNTING(SimpleWindowDelegate);
  DISALLOW_COPY_AND_ASSIGN(SimpleWindowDelegate);
};

class SimpleBrowserViewDelegate : public CefBrowserViewDelegate {
 public:
  explicit SimpleBrowserViewDelegate(cef_runtime_style_t runtime_style)
      : runtime_style_(runtime_style) {}

  bool OnPopupBrowserViewCreated(CefRefPtr<CefBrowserView> browser_view, CefRefPtr<CefBrowserView> popup_browser_view, bool is_devtools) override {
    // create a new top-level window, will show itself after creation
    CefWindow::CreateTopLevelWindow(new SimpleWindowDelegate(popup_browser_view, runtime_style_, CEF_SHOW_STATE_NORMAL));
    return true;
  }

  cef_runtime_style_t GetBrowserRuntimeStyle() override {
    return runtime_style_;
  }

 private:
  const cef_runtime_style_t runtime_style_;

  IMPLEMENT_REFCOUNTING(SimpleBrowserViewDelegate);
  DISALLOW_COPY_AND_ASSIGN(SimpleBrowserViewDelegate);
};

class CustomV8Handler : public CefV8Handler {
  public:
    bool Execute(const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval, CefString& exception) override {
        //CEF_REQUIRE_UI_THREAD();
		
		if (name == "hui_cef_call_native") {
			if (arguments.size() > 0 && arguments[0]->IsString()){
				
				std::string msgstr = arguments[0]->GetStringValue().ToString();
				cpp_handlers2call.push_back(msgstr);  // save the message and call the cpp function outside the message loop handling
					
				HUI_DEBUG_PRINT("call from js to cpp "<<msgstr);
				
			}
			
            retval = CefV8Value::CreateString("ok");
            return true;
        }
        return false;
    }
	
    IMPLEMENT_REFCOUNTING(CustomV8Handler);
};

class RenderProcessHandler_cef : public CefRenderProcessHandler {
public:
    void OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context) override {
        CefRefPtr<CefV8Value> global = context->GetGlobal();
        CefRefPtr<CustomV8Handler> handler = new CustomV8Handler();
        global->SetValue("hui_cef_call_native", CefV8Value::CreateFunction("hui_cef_call_native", handler), V8_PROPERTY_ATTRIBUTE_NONE);
    }
	
    IMPLEMENT_REFCOUNTING(RenderProcessHandler_cef);
};

class App_cef : public CefApp, public CefBrowserProcessHandler, public CefRenderProcessHandler {
  public:
    CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override { return this; }
    CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override { return new RenderProcessHandler_cef(); }
	
	void OnContextInitialized() override {}
	
	void OnBeforeCommandLineProcessing(const CefString& process_type, CefRefPtr<CefCommandLine> command_line) override { // run in single process - workaround for call_native (wont work without that because vector is appended and read in different processes causing it to not work) + may help when built as dll
        command_line->AppendSwitch("single-process");  // enable single-process mode
        command_line->AppendSwitch("no-sandbox");  // disable sandbox
		command_line->AppendSwitch( std::string()+"cache-path=.\\cache_"+ std::to_string(reinterpret_cast<uintptr_t>(GetModuleHandle(NULL))) ); // TODO: ensure this really works OR better use system temporary dir (this may be potentionally cause of "Opening in existing browser session." <https://cef-builds.spotifycdn.com/docs/130.1/classCefBrowserProcessHandler.html#a052a91639483467c0b546d57a05c2f06>)
		command_line->AppendSwitch("disable-gpu-compositing"); // TODO: fix and allow gpu
		command_line->AppendSwitch("transparent-painting-enabled"); // enables transparency TODO: fix it -> <https://chatgpt.com/share/67c79507-9334-8006-a775-89e75340aa2e>
    }
	
	IMPLEMENT_REFCOUNTING(App_cef);

};

class Client_cef : public CefClient {
public:
    bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefProcessId source_process, CefRefPtr<CefProcessMessage> message) override { return false; }
    IMPLEMENT_REFCOUNTING(Client_cef);
};


uint8_t cef_webviews = 0; 

// variables for call_js
bool ret = true;
std::string ret_data = "#";
bool rr = false;

int cef_init(){

	int exit_code;
  
  /*#if defined(ARCH_CPU_32_BITS) // TODO (if nessesary or if it can save RAM)
	// Run the main thread on 32-bit Windows using a fiber with the preferred 4MiB stack size. This function must be called at the top of the executable entry point function (`main()` or `wWinMain()`). It is used in combination with the initial stack size of 0.5MiB configured via the `/STACK:0x80000` linker flag on executable targets. This saves significant memory on threads (like those in the Windows thread pool, and others) whose stack size can only be controlled via the linker flag.
	exit_code = CefRunWinMainWithPreferredStackSize(wWinMain, hInstance, lpCmdLine, nCmdShow);
	if (exit_code >= 0) return exit_code; // The fiber has completed so return here.
  #endif*/
	
	void* sandbox_info = nullptr;
  /*#if defined(CEF_USE_SANDBOX)
	// Manage the life span of the sandbox information object. This is necessary for sandbox support on Windows. See cef_sandbox_win.h for complete details.
	CefScopedSandboxInfo scoped_sandbox;
	sandbox_info = scoped_sandbox.sandbox_info();
  #endif*/

	HINSTANCE hInstance = GetModuleHandle(NULL);
	CefMainArgs main_args(hInstance);
    //CefMainArgs main_args(argc, argv); // TODO: linux
	
	CefSettings settings;
	//settings.single_process = true; // discontinued
	//settings.multi_threaded_message_loop = true; // breaks the app
  
  //#if !defined(CEF_USE_SANDBOX)
	settings.no_sandbox = true;
  //#endif
	
	CefRefPtr<App_cef> app = new App_cef();  // TODO: shouldnt be global?
	
	exit_code = CefExecuteProcess(main_args, app, nullptr);
    if (exit_code >= 0) return exit_code;
	
	CefInitialize(main_args, settings, app, nullptr);
	
	// now, we can create windows
	
}


struct WebView::pImpl {
	CefRefPtr<CefBrowserView> browser_view = nullptr;
	CefRefPtr<CefBrowser> browser = nullptr;
};


WebView::WebView () {
	
	impl = std::make_unique<pImpl>();
	
	if (cef_webviews == 0) cef_init();
	cef_webviews++;
	
    CefRefPtr<Client_cef> client = new Client_cef();
    CefBrowserSettings browser_settings;
	
	browser_settings.background_color = CefColorSetARGB(0x00, 0x00, 0x00, 0x00);  // TODO: doesnt work (may be windows issue)
   
	impl->browser_view = CefBrowserView::CreateBrowserView(client, "", browser_settings, nullptr, nullptr, new SimpleBrowserViewDelegate(CEF_RUNTIME_STYLE_DEFAULT)); // we are using the 'Views framework' - other option is 'native platform framework', which brings up full browser UI
    CefWindow::CreateTopLevelWindow(new SimpleWindowDelegate(impl->browser_view, CEF_RUNTIME_STYLE_DEFAULT, CEF_SHOW_STATE_NORMAL));
	
	if (impl->browser_view) { impl->browser = impl->browser_view->GetBrowser(); }
	
	load_uri("about:blank"); // load blank page to make element api work
	
	hui_tweaks();
   
}

WebView::~WebView () {
	cef_webviews--;
	if (cef_webviews == 0) CefShutdown(); 
}


void WebView::load_file (std::string file){

	CefRefPtr<CefBrowser> browser = impl->browser_view->GetBrowser();
	if (browser) {
		browser->GetMainFrame()->LoadURL(std::string()+"file://"+file);
		//browser->GetMainFrame()->LoadURL(std::string()+"file://"+HUI::Str(file).replace("\\","/"));
	}
	hui_tweaks();

}

void WebView::load_uri (std::string uri){

	CefRefPtr<CefBrowser> browser = impl->browser_view->GetBrowser();
	if (browser) {
		browser->GetMainFrame()->LoadURL(uri);
	}
	hui_tweaks();

}

void WebView::load_str (std::string str){

	CefRefPtr<CefBrowser> browser = impl->browser_view->GetBrowser();
	if (browser) {
		browser->GetMainFrame()->LoadURL(GetDataURI(str,"text/html"));
	}
	hui_tweaks();

}

void WebView::hui_tweaks (){
	call_js(
		#include "hui.js"
		);
	call_js(std::string(
	  "style = document.createElement('style'); style.textContent = \"")+
	  HUI::Str(
	  #include "hui.css" // JSERROR{unexpected EOF} --fixed
	  ).replace("\n","\\n").replace("\"","\\\"").cpp_str()
	  +std::string("\"; document.head.append(style);"
		).c_str(),false);
	
	
	std::string css;
	std::ifstream filestream (HUI::filepath(
	#if defined(_WIN32)
		"%userprofile%/HUI_theme.css"
	#else
		"$HOME/.config/HUI_theme.css"
	#endif
	).c_str());
	if (filestream.good()) {
		// same as following oneliner
		//std::stringstream stringstream;
		//stringstream << filestream.rdbuf();
		//std::string str = stringstream.str();
		//css = str;
		css = std::string((std::istreambuf_iterator<char>(filestream)), std::istreambuf_iterator<char>());
	}
	else {
		css = 
		#include "HUI_theme.css"
		;
	}
	call_js(std::string(
		"style = document.createElement('style'); style.textContent = \"")+
		std::string(HUI::Str(std::string(css.c_str())).replace("\n","\\n").replace("\"","\\\"").c_str())
		+std::string("\"; document.head.append(style);"
		),false);
}


std::string WebView::call_js (std::string code, bool return_data){
    if (return_data){ // expects the code to be one expresion // TODO: check/remove this limitation
	
		//CEF_REQUIRE_UI_THREAD();
		
		ret = true;
		ret_data = "#";
		
		if(!rr) {
			call_js(std::string("let hui_js_return = ") + call_native([](std::vector<std::string> args){ret_data = args[0]; ret=false;})+";" , false);
			rr=true;
		}
		
		while(code[code.size()-1] == ' ' or  code[code.size()-1] == ';') code.pop_back();
		
		call_js(std::string("hui_js_return(") + code + std::string(");"), false);
		
		HUI_DEBUG_PRINT("called js, waiting for return...");
		
        auto start_time = std::chrono::steady_clock::now();
        auto timeout_duration = std::chrono::seconds(5);
		
		while (ret){
			
			handle_once();
			
			//std::cout<<".";
			
           auto current_time = std::chrono::steady_clock::now();
            if (current_time - start_time > timeout_duration) {
				HUI_DEBUG_PRINT("timed out waiting for js return");
                break;
            }
			
		}
		
		return ret_data;

	}
	else {
		
		if (impl->browser_view) {
			//CefRefPtr<CefBrowser> browser = impl->browser_view->GetBrowser(); // the browser is stored globally as it seems it cant be obtained sometimes
			if (impl->browser) {
				// execute the js in the main frame
				impl->browser->GetMainFrame()->ExecuteJavaScript(code, impl->browser->GetMainFrame()->GetURL(), 0);
				HUI_DEBUG_PRINT("js called: "<<code);
			} 
			else HUI_DEBUG_PRINT("error calling js (browser)");
		} 
		else HUI_DEBUG_PRINT("error calling js (browser_view)");
		
	}
	
	return "";
}

std::string WebView::call_native (std::function<void(std::vector<std::string>)> handler, std::string process_args){
	CEF_REQUIRE_UI_THREAD();
	sent2cpp_handlers.push_back(handler);
	return "function(..._args_array){hui_cef_call_native( '"+std::to_string(sent2cpp_handlers.size()-1)+"'+',\"'+("+process_args+")(_args_array).join('\",\"')+'\"' );}";
}


std::string WebView::backend_name (){
	return "blink-cef";
}

void* WebView::backend_object (){
	return impl->browser_view.get();
}

std::string WebView::window_type (){
	#if defined(_WIN32) // windows
	  return "windows";
	#else // TODO: whatever
	  return "unknown";
	#endif
}

void* WebView::window_handle (){
	return impl->browser->GetHost()->GetWindowHandle();
	//return window->GetWindowHandle(); // Windows HWND = PVOID = void* = pointer (https://learn.microsoft.com/en-us/windows/win32/winprog/windows-data-types)
}

 
void WebView::handle_forever (){
	//CefRunMessageLoop(); // run blocking CEF message loop (we cant use it due to the way we handle js api)
	while(1){ 
		handle_once(); 
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}
	CefShutdown();
}

void WebView::handle_once (){
	CefDoMessageLoopWork();
	
	// run all callbacks that were requested to be run from js
	for (int i = 0; i < cpp_handlers2call.size() ;) {
		
		auto msgstr = cpp_handlers2call[i];
		cpp_handlers2call.erase(cpp_handlers2call.begin() + i);
		
		const int hi = stoi(msgstr.substr(0,msgstr.find(",")));
		msgstr = msgstr.substr(msgstr.find(",")+1+1, msgstr.size()-1 -(msgstr.find(",")+1+1) );
		
		std::vector<HUI::Str> msgdata_ = HUI::Str(msgstr).split("\",\"");
		std::vector<std::string> msgdata;
		for (auto it : msgdata_) msgdata.push_back ( it.replace("\\\"","\"").cpp_str() );
		
		HUI_DEBUG_PRINT("call from js: "<<msgstr);
		
		sent2cpp_handlers.at(hi)(msgdata);
	}
	
	cpp_handlers2call.clear();
	
}

void/*std:thread*/ WebView::handle_threaded (){
	
}

void WebView::exit() {
	CefShutdown();
}


#include "./hui_windowcontrols__dummy.cc"

} // HUI

#endif // _hui_backend_