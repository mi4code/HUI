//#include "hui_webview__webkit_qt5.cc"  // in case you want to build backend with your app, you can include it instead of HUI.hh and then modify the build command corespondingly
//#include "hui_webview__webkit_gtk3.cc"
#include "HUI.hh"


//std::vector<HUI::WebView> windows;
std::vector<HUI::WebView*> windows;

int main (int argc, char* argv[]){
	//auto window = HUI::WebView(); // currently not supported by api
	HUI::WebView window;
	
	window.html_element("body button#open");
	window.html_element("button#open","innerText","'open new window'");
	window.html_element("button#open","onclick", window.call_native([&](std::vector<std::string> args) -> void {
		//HUI::WebView w; // window would imidiately close after its creation because it would get out of scope
		//windows[windows.size()] = HUI::WebView(); // currently not supported by api
		windows[windows.size()] = new HUI::WebView();
		// TODO: fix api for window move/copy/delete + multiwindow
	}));
	
	window.html_element("body button#exit");
	window.html_element("button#exit","innerText","'exit application'");
	window.html_element("button#exit","onclick", window.call_native([&](std::vector<std::string> args) -> void {
		HUI::WebView::exit(); // closes all windows and exits
	}));
	
	window.call_js(std::string()+"window.onbeforeunload = function(){ ("+window.call_native([&](std::vector<std::string> args) -> void {  // woraround for missing close callback 
		HUI::WebView w;
		w.html_element("body p");
		w.html_element("body p","innerText","'main window was closed'");
		window.call_js(std::string()+"window.onbeforeunload = function(){ ("+window.call_native([&](std::vector<std::string> args) -> void { HUI::WebView::exit(); })+")(); /*return '';*/}");
		
		HUI::WebView::handle_forever();
	})+")(); /*return '';*/}");
	
	HUI::WebView::handle_forever();
	//HUI_WebView_handle_forever(); // in C++ C calls work too
}