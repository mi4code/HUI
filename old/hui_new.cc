#if !defined(HUI_NEW_CC)
 #define HUI_NEW_CC

/*
	THIS FILE IS SOURCE FOR BUILDING HUI AS LIBRARY (STATIC OR DYNAMIC)
	USED BACKEND IS SELECTED RIGHT HERE IN THIS FILE (AUTOMATICALY OR MANUALY)
	MULTI-ABI: EVERY IMPLEMENTATION SHOULD PROVIDE: HEADER (THIS IMPLIES EXISTENCE OF BINDINGS = C to NATIVE), SOURCE, C API/ABI (NATIVE to C), NATIVE API/ABI (OPTIONAL) 
	
	HEADER FILES SHOULD CONTAIN DEFINITIONS OF HUI OBJECTS IN GIVEN LANGUAGE AND BINDINGS CODE (C to NATIVE, DEACTIVATABLE)
	OPTIONALY MAY CONTAIN PLATFORM SPECIFIC DEFINITINS (REUSABLE FOR BACKEND)
	
	building mode/platform should be defined with g++ define
	when building *.so - build source code from hui_new.cc (hui_<backend>.<any language> ~ includes universal header, contains all required code)
	if you want header only mode, just include hui_new.cc (after hui.hh) + use same libs and defines when building
*/


#include "hui_datatypes.h"
 
#include <functional>
#include <fstream>
#include <stdint.h>

#include <string>
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <stdlib.h>

#if defined(_WIN32)
 #include <windows.h>
 #include <tchar.h>
#endif

// BACKENDS
// NO BACKEND MEANS CPP HEADER WITH WRAPING C METHODS FROM *.so

//backends for webview (its like app with window)
 //#define HUI_BACKEND_GTK3
 #define HUI_BACKEND_QT5
 //#define HUI_BACKEND_DUMMY // TODO: no webview for testing purposes
 //win webview
 //cef

// backends for window managing and embeding
 #define HUI_BACKEND_WINDOWS
 //#define HUI_BACKEND_NONE // for unsupported OSes (WindowControls and ProcessWindow wont be availible for use)
 //x11
 //wayland
 //?universal x11/wayland (just Handle is pair of x11+wayland and functions are if+else)
 //macos


/*#if !defined(HUI_WINDOW_GTK3_H) and !defined(HUI_WINDOW_QT5_H)
 #warning "[HUI::Window] no backend selected/included - auto-selecting" // you should include the desired backend in your code before including this file
 
 #if defined(_WIN32) // windows msys2 mingw g++
  #include "hui_window_qt5.h"
 #else // linux g++
  #include "hui_window_gtk3.h"
 #endif
  
#endif*/

//#include "hui_new.hh"
#include "HUI.hh"


#define HUI_BACKEND_COMMON
#if defined(HUI_BACKEND_COMMON)



namespace HUI { 


std::vector< std::function<void(std::vector<std::string>)> > sent2cpp_handlers;  // TODO: call handlers should be able to use only indexes for other laguages bindings (NOT BINDINGS BUT IMPLEMENTATIONS!)


/// App ///

App::App(const char* name, const char* icon, const char* binary) {
	
	if (binary == NULL) { // TODO: fix for python when it returns interpreter
		// similar code in HUI::filepath
		#if defined(_WIN32) // windows
			char invocation_c[MAX_PATH];
			DWORD invocation_l = GetModuleFileName(nullptr, invocation_c, MAX_PATH);
			Str invocation_s = Str(std::string(invocation_c,invocation_l)).replace("\\","/");
			this->binary = invocation_s.c_str();
		#else // linux
			this->binary = program_invocation_name; // non-resolved path

			/*pid_t pid = getpid();
			char buf[20] = {0};
			sprintf(buf,"%d",pid);
			Str _link = Str({"/proc/",buf,"/exe"});
			char proc[512];
			int ch = readlink(_link.c_str(),proc,512);
			if (ch != -1) {
				proc[ch] = 0;
				Str path = proc;
				this->binary = path; // resolved path, TODO: broken
			}*/

		#endif
	}
	else this->binary = binary;
	
	if (binary == NULL) this->name = this->binary;
	else this->name = name;
	
	this->icon = icon;
}

Window App::new_window(){
	auto window = HUI::Window();
	window.set_title(this->name);
	window.set_id(this->binary);
	// TODO: default icon
	return window;
}


/// WebviewContent ///

std::string WebviewContent::to_js (std::function<void(std::vector<std::string>)> value, const char* process_args){ 
	return call_cpp(value, process_args);
}
std::string WebviewContent::to_js (std::string value){
	auto value_ = HUI::Str(value).replace("\"","\\\"").replace("\n","\\n"); // TODO: other newline encodings (windows vs. unix)
	value_.insert("\"", 0);
	value_.insert("\"", -1);
	return value_.cpp_str();
}
std::string WebviewContent::to_js (long value){
	return HUI::Str(value).cpp_str(); 
}
std::string WebviewContent::to_js (float value){
	return HUI::Str(value).cpp_str();
}
	
void WebviewContent::js_set_value (const char* code, std::string value){ // raw code
	call_js(HUI::Str({code," = ",value,";"}).c_str(), false);	
}
//void WebviewContent::js_set_value (const char* code, auto value){ js_set_value_(code, js_v(value)); }  // escaped 

void WebviewContent::js_set_value (const char* query, const char* property, std::string value, uint16_t index){ 
	if (index == uint16_t(-1)) {
		//call_js(HUI::Str({"document.querySelectorAll(\"",query,"\").forEach(function(e){e.",property," = ",value,";});"}).c_str()); // TODO: doesnt work - forEach is not a function
		call_js(HUI::Str({" Array.from(document.querySelectorAll(\"",query,"\")).forEach(function(e){e.",property," = ",value,";});"}).c_str(), false);
	}
	else {
		call_js(HUI::Str({"document.querySelectorAll(\"",query,"\")[",HUI::Str(index),"].",property," = ",value,";"}).c_str(), false);
	}	
}
//void WebviewContent::js_set_value (const char* query, const char* property, auto value, uint16_t index){ js_set_value_(query, property, js_v(value), index); }  // TODO:  what about js_set_value(Element("query","property", all/index), ...) AND js_set_value(Variable("my_var"), ...)

std::string WebviewContent::js_get_value (const char* code){
	return call_js(code,true); 
}
std::string WebviewContent::js_get_value (const char* query, const char* property, uint16_t index){  
	return js_get_value (HUI::Str({"document.querySelectorAll(\"",query,"\")[",HUI::Str(index),"].",property}).c_str());
}

/*void content2 (HUI::Str filename_or_html, bool its_file=false) { // TODO: set new HTML content and preserve js context
	if (its_file) { // sure that this works - same code is used in li-panel
		std::ifstream filestream (filename_or_html.cpp_str());
		std::stringstream stringstream;
		stringstream << filestream.rdbuf();
		filename_or_html = stringstream.str();
	}
	
	// TODO: fix this - very likely same issue as with hui_theme.css loading -- THE ISSUE ARE DIFFERENT NEWLINE FORMATS
	
	filename_or_html = filename_or_html.replace("\"","\\\"").replace("\n","\\n");
	std::cout<<filename_or_html.cpp_str();

	/ *call_js(Str({ // doesnt work 
		"document.open('text/html');",
		"document.write(",filename_or_html,");",
		"document.close();",
		"document.querySelectorAll('*').forEach(function(e){if (typeof e.onload === 'function'){e.onload();}});"
	}).cpp_str());* /
	
	//call_js(Str({"document.body.parentElement.innerHTML = \"",filename_or_html,"\";"}).cpp_str()); // doesnt work + cant support scripts
	
	//webview->setHtml(filename_or_html.c_str()); // qt5 <https://forum.qt.io/topic/42023/how-to-load-html-source-from-string-code-to-qwebview-solved/2>

	
}*/

/*std::string call_cpp2 (std::function<void(Vec<Str>)> handler){ // TODO: allow both Vec<Str> and std:vector<std::string> + const
	return call_cpp( [&handler](std::vector<std::string> arg){
		Vec<Str> arg2;
		for (auto i : arg) arg2+=i;
		handler(arg2);
		} ); 
}*/


/// WindowBase ///


/// WindowControls ///


};
#endif




#if defined(HUI_BACKEND_DUMMY)


namespace HUI { 

void handle_forever(){while(1);}
void handle_once(){}
//void handle_threaded(){}

void exit(){}


/// App ///


/// WebviewContent ///

WebviewContent::WebviewContent () {}

void WebviewContent::content (std::string file){}

std::string WebviewContent::call_js(std::string code, bool return_data){}

std::string WebviewContent::call_cpp(std::function<void(std::vector<std::string>)> handler, const char* process_args){}

void* WebviewContent::backend_widget (){}


/// WindowBase ///

WindowBase::WindowBase () {}

void WindowBase::add (WebviewContent* content){}

void WindowBase::show () {}

void* WindowBase::backend_window(){}


/// WindowControls ///

WindowControls::WindowControls(){}


void WindowControls::use (WindowBase* _window){}


void WindowControls::set_title(const char* title){}
const char* WindowControls::get_title(){}

void WindowControls::set_id(const char* id){}
const char* WindowControls::get_id(){}

void WindowControls::set_type (WindowType type){}
WindowType WindowControls::get_type (){}

void WindowControls::set_geometry (Pixels w, Pixels h, Pixels left, Pixels top, Pixels right, Pixels bottom, Screen relative, int8_t state){}
auto WindowControls::get_geometry (){ struct {Pixels w,h,left,top,right,bottom; Screen relative; int8_t state;} geometry; return geometry; }

void WindowControls::set_layer(WindowLayer layer){}
WindowLayer WindowControls::get_layer(){}

void WindowControls::set_input_mode_keyboard (WindowInputMode mode) {}
WindowInputMode WindowControls::get_input_mode_keyboard (){}

void WindowControls::set_input_mode_mouse (WindowInputMode mode) {}
WindowInputMode WindowControls::get_input_mode_mouse (){}

void WindowControls::set_exclusive_zone (int16_t zone){}
int16_t WindowControls::get_exclusive_zone (){}

void WindowControls::set_focused (bool activated){}
bool WindowControls::get_focused (){}

void WindowControls::start_move_drag (){}

void WindowControls::start_resize_drag (){}

void WindowControls::close () {}


};
#endif




// ----- webviews ----- //

#if defined(HUI_BACKEND_QT5)

#include <QtWidgets/QApplication>
#include <QtWidgets/QDesktopWidget>
#include <QtWebKitWidgets/QWebView>
#include <QtWebKitWidgets/QWebFrame>
//#include <LayerShellQt/Shell>  // TODO, will not be implemented any time soon because layer-shell-qt doesnt have pkg-config
//#include <LayerShellQt/Window> 

// deployment sources (helped but arent completely accurate so better read the readme): <https://stackoverflow.com/questions/25402225/how-to-build-64-bit-mingw-qt-application-on-windows-with-qt-creator> <https://stackoverflow.com/questions/41994485/how-to-fix-could-not-find-or-load-the-qt-platform-plugin-windows-while-using-m>

namespace HUI { 


int qt_argc = 0;
char* qt_argv = "";
QApplication qt_app(qt_argc,&qt_argv);

class Qt_q_object : public QObject {
    Q_OBJECT
  public: 
    Qt_q_object(){}
    Q_INVOKABLE void call_from_js_to_cpp (QString dta){
		
		std::string msgstr = std::string(dta.toStdString());
			
		#if defined (HUI_QT5_DEBUG)
			std::cout<<"\n [HUI::DEBUG] call from js to cpp '"<<msgstr<<"' ";
		#endif
		
		const int hi = stoi(msgstr.substr(0,msgstr.find(",")));
		msgstr = msgstr.substr(msgstr.find(",")+1+1, msgstr.size()-1 -(msgstr.find(",")+1+1) );
		
		std::vector<HUI::Str> msgdata_ = HUI::Str(msgstr).split("\",\"");
		std::vector<std::string> msgdata;
		for (auto it : msgdata_) msgdata.push_back ( it.replace("\\\"","\"").cpp_str() );
		
		sent2cpp_handlers.at(hi)(msgdata);
	
	}
};
Qt_q_object qt_q_object_for_call_cpp; // having this global will save memory (1 QObject, 1 std::vector/std::map)

void handle_forever(){
	qt_app.exec();
}
void handle_once(){
	qt_app.processEvents();
}
//void handle_threaded(){}

void exit(){}


/// App ///


/// WebviewContent ///

WebviewContent::WebviewContent () {
	// window
	window = new QWidget(); //((QWidget*)window)
	//LayerShellQt::Shell::useLayerShell();
	
	// transparency
	((QWidget*)window)->setAttribute(Qt::WA_TranslucentBackground);
	//window->setAutoFillBackground(true); // has no effect there
	//window->setWindowFlags(Qt::FramelessWindowHint); // transparency works only with frameless windows on Windows in Qt (but we dont want all windows to be frameless)
	
	// webview
	webview = new QWebView(); //((QWebView*)webview)
	#if not defined (HUI_QT5_DEBUG) // make it possible to right click and open devtools
		((QWebView*)webview)->setContextMenuPolicy(Qt::NoContextMenu);
	#endif
	
	// transparency
	((QWebView*)webview)->setAttribute(Qt::WA_TranslucentBackground);
	((QWebView*)webview)->setAutoFillBackground(true);
	((QWebView*)webview)->setStyleSheet("background:transparent"); //required
	
	// debug
	#if defined (HUI_QT5_DEBUG)
		std::cout<<"\n [HUI::DEBUG] enabling devtools ";
		((QWebView*)webview)->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled,true);
	#endif
	
	// htui.js, htui.css (maybe will be handled through htui.js - due to dynamic style changes)
	call_js(
		#include "hui.js"
		);
	call_js(std::string(
	  "style = document.createElement('style'); style.textContent = \"")+
	  Str(
	  #include "hui.css" // JSERROR{unexpected EOF} --fixed
	  ).replace("\n","\\n").replace("\"","\\\"").cpp_str()
	  +std::string("\"; document.head.append(style);"
		).c_str(),false);
	
	
	std::string css;
	std::ifstream filestream (HUI::filepath(
	#if defined(_WIN32)
		"%userprofile%/hui_theme.css"
	#else
		"$HOME/.config/hui_theme.css"
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
		#include "hui_theme.css"
		;
	}
	call_js(std::string(
		"style = document.createElement('style'); style.textContent = \"")+
		std::string(Str(std::string(css.c_str())).replace("\n","\\n").replace("\"","\\\"").c_str())
		+std::string("\"; document.head.append(style);"
		),false);
		
	
	// hole in window
	/*QRect r = webview->frameGeometry();
	r.moveTopLeft(webview->mapFromGlobal(r.topLeft()) );
	QRect hole(0, 0, webview->width() - 50, 100);
	hole.moveCenter(webview->rect().center());
	webview->setMask(QRegion(r).subtracted(hole));*/
	
	// js<-->cpp
	((QWebView*)webview)->page()->mainFrame()->addToJavaScriptWindowObject("qt_window", &qt_q_object_for_call_cpp);
	
	// add
	window = webview;
	//window = ((QWidget*) content->backend_widget() );
	//window = content->webview;
	
	// show
	((QWidget*)window)->show();
}

WebviewContent::~WebviewContent () {
	delete ((QWidget*)window);
	//delete ((QWebView*)webview); // causes segfault
}

void WebviewContent::content (std::string file){
	((QWebView*)webview)->load(QUrl(std::string("file://"+file).c_str()));
	((QWebView*)webview)->page()->mainFrame()->evaluateJavaScript(
	#include "hui.js"
	);
}

std::string WebviewContent::call_js(std::string code, bool return_data){  // TODO: fix 'return value' = 'after load'  vs.  'run before load' = 'no return' (we need a way to check whether the page was loaded)
	
	#if defined (HUI_QT5_DEBUG)
		std::cout<<"\n [HUI::DEBUG] call js from cpp '"<<code<<"' ";
	#endif
	
	if (return_data) {
		return ((QWebView*)webview)->page()->mainFrame()->evaluateJavaScript(code.c_str()).toString().toStdString();
	}
	
	else { // TODO: not working (by default return value is used)
		/*if (loaded)*/ ((QWebView*)webview)->page()->mainFrame()->evaluateJavaScript(std::string(code+"\n true;").c_str());
		/*else*/ ((QWebView*)webview)->connect(((QWebView*)webview), &QWebView::loadFinished, ((QWebView*)webview), [=](){ ((QWebView*)webview)->page()->mainFrame()->evaluateJavaScript(code.c_str()); });
		return "";
	}
	
}

std::string WebviewContent::call_cpp(std::function<void(std::vector<std::string>)> handler, const char* process_args){
	sent2cpp_handlers.push_back(handler);
	return "function(..._args_array){qt_window.call_from_js_to_cpp( '"+std::to_string(sent2cpp_handlers.size()-1)+"'+',\"'+("+process_args+")(_args_array).join('\",\"')+'\"' );}";
}

void* WebviewContent::backend_object (){
	return window; // backend widget can be get using backend code from the toplevel container
}

Handle WebviewContent::window_handle (){
	return HWND( ((QWidget*)window)->winId() );
}

#if defined(OLDOLD)
/// WindowBase ///

WindowBase::WindowBase () {
	// window
	window = new QWidget(); //((QWidget*)window)
	//LayerShellQt::Shell::useLayerShell();
	
	// transparency
	((QWidget*)window)->setAttribute(Qt::WA_TranslucentBackground);
	//window->setAutoFillBackground(true); // has no effect there
	//window->setWindowFlags(Qt::FramelessWindowHint); // transparency works only with frameless windows on Windows in Qt (but we dont want all windows to be frameless)

}

WindowBase::~WindowBase () {
	delete ((QWidget*)window);
}

void WindowBase::add (WebviewContent* content){
	window = ((QWidget*) content->backend_widget() );
	//window = content->webview;
}

void WindowBase::show () {
	((QWidget*)window)->show();
}

void* WindowBase::backend_window(){
	return window;
}
#endif


#if !defined(HUI_BACKEND_QT5_building_moc)
  #include "qt5_generated.moc"
#endif

};
#endif


#if defined(HUI_BACKEND_GTK3)

#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <gtk-layer-shell.h>

bool gtk_inited = false;

namespace HUI { 

void handle_forever(){
	gtk_main();
}
void handle_once(){ // can be used like `while(1){ww.handle_once(); usleep(35000);}`
	while (gtk_events_pending()) {gtk_main_iteration();}
}
/*void handle_threaded(){ // wont work (gtk needs everything to be in one thread)
	std::thread hui_handle_thread(gtk_main);
}*/

void exit(){
	gtk_main_quit();
	sent2cpp_handlers.clear(); // solves problem with python bindings mwhen it was impossible to exit due to GIL error in gillstate_tss_set
}


/// App ///


/// WebviewContent ///

WebviewContent::WebviewContent () {
	
	if (!gtk_inited) {gtk_init(NULL, NULL);  gtk_inited = true;}  // init so no extra global init is needed
	
	// webview
	webview = GTK_WIDGET(webkit_web_view_new_with_context(webkit_web_context_new_ephemeral())); // private mode (should be always enabled)
	//webkit_settings_set_enable_private_browsing (webkit_web_view_get_settings(webview), true); // deprecated private mode
	//if (!webkit_web_context_get_sandbox_enabled ( webkit_web_view_get_context (webview))) {std::cout<<"not sandboxed";} // not sandboxed by default
	
	
	// transparency
	gtk_widget_set_visual(GTK_WIDGET(webview), gdk_screen_get_rgba_visual(gdk_screen_get_default()));
	//gtk_widget_set_opacity(GTK_WIDGET(webview),0.5); // sets transparent color for everything (useless here, but can be used for something like forced transparency)
	GdkRGBA transparent = {0,0,0,0};
	webkit_web_view_set_background_color(WEBKIT_WEB_VIEW(webview), &transparent);
	
	
	// htui.js, htui.css (maybe will be handled through htui.js - due to dynamic style changes)
	WebKitUserContentManager *manager = webkit_web_view_get_user_content_manager (WEBKIT_WEB_VIEW(webview));
	
	webkit_user_content_manager_add_script(manager, webkit_user_script_new(
		#include "hui.js"
		,WEBKIT_USER_CONTENT_INJECT_ALL_FRAMES, WEBKIT_USER_SCRIPT_INJECT_AT_DOCUMENT_START, NULL,NULL) ); 

	/*webkit_user_content_manager_add_style_sheet(manager, webkit_user_style_sheet_new ( // doesnt work
		 #include "hui.css"
		,WEBKIT_USER_CONTENT_INJECT_ALL_FRAMES, WEBKIT_USER_STYLE_LEVEL_AUTHOR, NULL,NULL) ); */
		
	call_js(std::string(
		"style = document.createElement('style'); style.textContent = \"")+
		Str(
		#include "hui.css" // JSERROR{unexpected EOF} --fixed
		).replace("\n","\\n").replace("\"","\\\"").cpp_str()
		+std::string("\"; document.head.append(style);"
		).c_str(),false);


	std::string css;
	std::ifstream filestream (HUI::filepath(
	#if defined(_WIN32)
		"%userprofile%/hui_theme.css"
	#else
		"$HOME/.config/hui_theme.css"
	#endif
	).c_str());
	if (filestream.good()) {
		// same as following oneliner
		//std::stringstream stringstream;
		//stringstream << filestream.rdbuf();
		//std::string str = stringstream.str();
		//css = str;
		css = std::string((std::istreambuf_iterator<char>(filestream)), std::istreambuf_iterator<char>()); // TODO: the fucking file must have unix line ends (not windows ones) - otherwise it breaks the js parsing resulting in "Unexpected EOF"
	}
	else {
		css = 
		#include "hui_theme.css"
		;
	}
	call_js(std::string(
		"style = document.createElement('style'); style.textContent = \"")+
		std::string(Str(std::string(css.c_str())).replace("\n","\\n").replace("\"","\\\"").c_str())
		+std::string("\"; document.head.append(style);"
		),false);
	
	// debug
	#if defined(HUI_GTK3_DEBUG)
		std::cout<<"\n [HUI::DEBUG] enabling devtools ";
		webkit_settings_set_enable_developer_extras(webkit_web_view_get_settings(WEBKIT_WEB_VIEW(webview)), true);
		webkit_web_inspector_show(webkit_web_view_get_inspector(WEBKIT_WEB_VIEW(webview))); // takes extra RAM (about the same as the app itself)
	#endif
	
	
	// js<-->cpp
	g_signal_connect (manager, "script-message-received::sent2cpp", G_CALLBACK(+[](WebKitUserContentManager* manager, WebKitJavascriptResult* message, gpointer user_data) -> void {

		JSGlobalContextRef context = webkit_javascript_result_get_global_context(message);
		JSCValue* result = webkit_javascript_result_get_js_value(message);
		std::string msgstr = std::string(jsc_value_to_string(result));
		
		#if defined(HUI_GTK3_DEBUG)
			std::cout<<"\n [HUI::DEBUG] call from js to cpp '"<<msgstr<<"' ";
		#endif
		
		const int hi = stoi(msgstr.substr(0,msgstr.find(",")));
		msgstr = msgstr.substr(msgstr.find(",")+1+1, msgstr.size()-1 -(msgstr.find(",")+1+1) );
		
		std::vector<HUI::Str> msgdata_ = HUI::Str(msgstr).split("\",\"");
		std::vector<std::string> msgdata;
		for (auto it : msgdata_) msgdata.push_back ( it.replace("\\\"","\"").cpp_str() );

		sent2cpp_handlers.at(hi)(msgdata);
		
	}), NULL);

	webkit_user_content_manager_register_script_message_handler (manager, "sent2cpp");
	
	// TODO: fix GTK3 WebKit2 hover issue when mouse leaves the window before it enters non-hover area (following code should have fixed the issue but it didnt because mouseleave is broken the same way as hover)
	/*g_signal_connect (GTK_WIDGET(webview), "leave-notify-event", G_CALLBACK(+[](GtkWidget* self, GdkEventCrossing event, gpointer user_data) -> void {
		#if defined(HUI_GTK3_DEBUG)
			std::cout<<"\n [HUI::DEBUG] window mouse leave";
		#endif		
		//auto* th = static_cast<WebviewContent*>(user_data);
		//th->call_js("style = document.createElement('style'); style.textContent = '*{pointer-events: none;}'; style.id = 'hui-fix-hover'; document.head.append(style);"); 
		webkit_web_view_evaluate_javascript (WEBKIT_WEB_VIEW(self),"style = document.createElement('style'); style.textContent = '*{pointer-events: none;}'; style.id = 'hui-fix-hover'; document.head.append(style);",-1,NULL,NULL,NULL,NULL,NULL);
	}) , NULL);
	g_signal_connect (GTK_WIDGET(webview), "enter-notify-event", G_CALLBACK(+[](GtkWidget* self, GdkEventCrossing event, gpointer user_data) -> void {
		#if defined(HUI_GTK3_DEBUG)
			std::cout<<"\n [HUI::DEBUG] window mouse enter";
		#endif		
		//static_cast<WebviewContent*>(user_data)->call_js("document.querySelector('#hui-fix-hover').remove();"); 
		webkit_web_view_evaluate_javascript (WEBKIT_WEB_VIEW(self),"document.querySelector('#hui-fix-hover').remove();",-1,NULL,NULL,NULL,NULL,NULL);
	}) , NULL);*/


	// window
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	//g_signal_connect(window, "delete-event", G_CALLBACK(gtk_main_quit), NULL); 
	//g_signal_connect(window, "destroy", G_CALLBACK(+[](){ gtk_main_quit(); }), NULL);
	
	// transparency
	gtk_widget_set_app_paintable(GTK_WIDGET(window), TRUE);
	gtk_widget_set_visual(GTK_WIDGET(window), gdk_screen_get_rgba_visual(gdk_screen_get_default()));
	
	// add
	//gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(content->backend_widget()));
	gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(webview));
	
	// TODO: show automatically but make possible layer-shell
	gtk_widget_show_all(GTK_WIDGET(window)); // this makes the window show up

}

WebviewContent::~WebviewContent () {
	//delete GTK_WIDGET(webview);  // causes  free(): invalid pointer
	//delete GTK_WINDOW(window); // causes  free(): invalid pointer
}

void WebviewContent::content (std::string file){
	//webkit_web_view_load_html (webview, std::string("<html></html>").c_str() , "none");
	webkit_web_view_load_uri(WEBKIT_WEB_VIEW(webview), std::string("file://"+file).c_str() );
	
	
	/*WebKitUserContentManager *manager = webkit_web_view_get_user_content_manager (WEBKIT_WEB_VIEW(webview));
	
	webkit_user_content_manager_add_script(manager, webkit_user_script_new(
		#include "hui.js"
		,WEBKIT_USER_CONTENT_INJECT_ALL_FRAMES, WEBKIT_USER_SCRIPT_INJECT_AT_DOCUMENT_START, NULL,NULL) ); 

	webkit_user_content_manager_add_style_sheet(manager, webkit_user_style_sheet_new (
		 #include "hui.css"
		,WEBKIT_USER_CONTENT_INJECT_ALL_FRAMES, WEBKIT_USER_STYLE_LEVEL_AUTHOR, NULL,NULL) );*/
}

std::string WebviewContent::call_js(std::string code, bool return_data){  // TODO: fix 'return value' = 'after load'  vs.  'run before load' = 'no return' (we need a way to check whether the page was loaded)
	
	#if defined (HUI_GTK3_DEBUG)
		std::cout<<"\n [HUI::DEBUG] call js from cpp '"<<code<<"' ";
	#endif
	
	if (return_data) {
		struct upoint {HUI::Str str = ""; bool ok = false;} stat;
		
		webkit_web_view_evaluate_javascript (WEBKIT_WEB_VIEW(webview), code.c_str(), -1, NULL, NULL, NULL, +[](GObject *object, GAsyncResult *result, gpointer user_data) -> void {
			JSCValue *value;
			GError *error = NULL;
			value = webkit_web_view_evaluate_javascript_finish (WEBKIT_WEB_VIEW (object), result, &error);
			((upoint*)user_data)->str = jsc_value_to_string (value);
			((upoint*)user_data)->ok = true;
		}, &stat );
		
		while (!stat.ok) handle_once();

		return stat.str.c_str();;
	}
	
	else {
		webkit_web_view_evaluate_javascript (WEBKIT_WEB_VIEW(webview),code.c_str(),-1,NULL,NULL,NULL,NULL,NULL); //doesnt work at the begining of execution
		webkit_user_content_manager_add_script( webkit_web_view_get_user_content_manager (WEBKIT_WEB_VIEW(webview)), webkit_user_script_new(code.c_str(),WEBKIT_USER_CONTENT_INJECT_ALL_FRAMES,WEBKIT_USER_SCRIPT_INJECT_AT_DOCUMENT_END,NULL,NULL) );
		return "";
	}
	
}

std::string WebviewContent::call_cpp(std::function<void(std::vector<std::string>)> handler, const char* process_args){
	sent2cpp_handlers.push_back(handler);
	return "function(..._args_array){ window.webkit.messageHandlers['sent2cpp'].postMessage( '"+std::to_string(sent2cpp_handlers.size()-1)+"'+',\"'+("+process_args+")(_args_array).join('\",\"')+'\"' ); }";
}

void* WebviewContent::backend_object (){
	return window;
}


#if defined(OLDOLD)
/// WindowBase ///

WindowBase::WindowBase () {
	
	if (!gtk_inited) {gtk_init(NULL, NULL);  gtk_inited = true;}  // init so no extra global init is required

	// window
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	//g_signal_connect(window, "delete-event", G_CALLBACK(gtk_main_quit), NULL); 
	//g_signal_connect(window, "destroy", G_CALLBACK(+[](){ gtk_main_quit(); }), NULL);
	
	// transparency
	gtk_widget_set_app_paintable(GTK_WIDGET(window), TRUE);
	gtk_widget_set_visual(GTK_WIDGET(window), gdk_screen_get_rgba_visual(gdk_screen_get_default()));
		
}

WindowBase::~WindowBase () {
	//delete GTK_WINDOW(window); // causes  free(): invalid pointer
}

void WindowBase::add (WebviewContent* content){
	//void add (GtkWidget/GtkPlug/GtkSocket/WindowHandle){}
	//content = widget;
	gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(content->backend_widget()));
	//gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(content->webview));

}

void WindowBase::show () {
	gtk_widget_show_all(GTK_WIDGET(window)); // this makes the window show up
}

void* WindowBase::backend_window(){
	return window;
}


/// WindowControls ///

WindowControls::WindowControls(){}
//_WindowControls(auto object){use(object);}
	
void WindowControls::use (WindowBase* _window){
	window = GTK_WIDGET(_window->backend_window());
	//window = /*&(*/_window->window/*)*/;
	g_signal_connect(G_OBJECT(window), "delete-event", G_CALLBACK(+[](GtkWidget *widget, GdkEvent *event, gpointer data){((WindowControls*) data)->on_closing(); return true;}), this);
}
/*void use (GtkWidget*& _window){
	window = _window;
	g_signal_connect(G_OBJECT(window), "delete-event", G_CALLBACK(+[](GtkWidget *widget, GdkEvent *event, gpointer data){((_WindowControls*) data)->on_closing(); return true;}), this);
}*/


void WindowControls::set_title(const char* title){
	gtk_window_set_title(GTK_WINDOW(window), title);
}
const char* WindowControls::get_title(){
	return gtk_window_get_title(GTK_WINDOW(window));
}

void WindowControls::set_id(const char* id){
	if (gtk_layer_is_layer_window(GTK_WINDOW(window))) gtk_layer_set_namespace(GTK_WINDOW(window), id); // wayland layer shell
	g_set_prgname (id); // should work as wayland app_id but TODO: per window
	gtk_window_set_startup_id(GTK_WINDOW(window), id); // x11
}
const char* WindowControls::get_id(){
	if (gtk_layer_is_layer_window(GTK_WINDOW(window))) return gtk_layer_get_namespace(GTK_WINDOW(window));
	return g_get_prgname();
	return ""; // TODO: get startup id
}
		
void WindowControls::set_type (WindowType type){ // TODO: fix: only before show -> create new window and move all content there // should be set_type
	switch(type){
		case WT_TOPLEVEL_SSD:
			// i can do shit about that
			break;
		case WT_TOPLEVEL_CSD:
			{
			auto placeholder = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0);
			gtk_widget_set_size_request(placeholder, -1, 0);
			gtk_window_set_titlebar(GTK_WINDOW(window), placeholder);
			}
			break;
		case WT_DESKTOP_COMPONENT:
			if (gtk_layer_is_supported ()) gtk_layer_init_for_window(GTK_WINDOW(window)); // should be called before gtk_widget_show_all (otherwise not applied) 
			gtk_window_set_decorated(GTK_WINDOW(window), false);
			gtk_window_set_skip_pager_hint(GTK_WINDOW(window), true);
			gtk_window_set_skip_taskbar_hint(GTK_WINDOW(window), true);
			break;
		case WT_POPOVER:
			// TODO: popovers
			break;
	}	
}
WindowType WindowControls::get_type (){  
	if (gtk_window_get_titlebar(GTK_WINDOW(window)) != NULL) return WT_TOPLEVEL_CSD;
	
    if ( !gtk_window_get_decorated(GTK_WINDOW(window)) or gtk_window_get_skip_pager_hint(GTK_WINDOW(window)) or gtk_window_get_skip_taskbar_hint(GTK_WINDOW(window)) ) return WT_DESKTOP_COMPONENT;

	return WT_TOPLEVEL_SSD;
}

void WindowControls::set_geometry (Pixels w, Pixels h, Pixels left, Pixels top, Pixels right, Pixels bottom, Screen relative, int8_t state){
	// keep in mind that this doesnt take panels into account as well as invisible areas of the multiscreen rectangle -- TODO: there should be arg for this in layer shell
	// TODO: relative to usable space https://docs.gtk.org/gdk3/method.Monitor.get_workarea.html
	// set position
	if (left != -1 or top != -1 or right != -1 or bottom != -1) { // setting only screen doesnt make sense
		if (gtk_layer_is_layer_window(GTK_WINDOW(window))){

			if (relative == -2) {  // recalculate to screen with overflow
				Pixels sw;
				Pixels sh;
				for (uint8_t ss = gdk_screen_get_n_monitors(gdk_screen_get_default())-1; ss>=0; ss--){
					GdkRectangle rect;
					gdk_screen_get_monitor_geometry(
						gdk_screen_get_default(), 
						ss,
						&rect
						);
					sw = std::max(int(sw), rect.x+rect.width);
					sh = std::max(int(sh), rect.y+rect.height);
				}
			
				if (right != -1) {
					w = sw-right-left;
					right = -1;
				}
				if (bottom != -1) {
					h = sh-bottom-top;
					bottom = -1;
				}
				
				relative = gdk_screen_get_monitor_at_point(gdk_screen_get_default(), left, top);
				GdkRectangle rect;
				gdk_screen_get_monitor_geometry(gdk_screen_get_default(), relative, &rect);
				left-=rect.x;
				top-=rect.y;
				
			}
			
			if (relative != -1)  gtk_layer_set_monitor(GTK_WINDOW(window), relative!=-3 ? gdk_display_get_monitor(gdk_display_get_default(), relative) : gdk_display_get_primary_monitor(gdk_display_get_default()) ); // old bugfix says that the window may not show up if this in not the last thing set
			
			if (top != -1) {
				gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_TOP, true);
				gtk_layer_set_margin(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_TOP, top);
			}
			else {
				gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_TOP, false);
				gtk_layer_set_margin(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_TOP, 0);
			}
			
			if (left != -1) {
				gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_LEFT, true);
				gtk_layer_set_margin(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_LEFT, left);
			}
			else {
				gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_LEFT, false);
				gtk_layer_set_margin(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_LEFT, 0);
			}
			
			if (right != -1) {
				gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_RIGHT, true);
				gtk_layer_set_margin(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_RIGHT, right);
			}
			else {
				gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_RIGHT, false);
				gtk_layer_set_margin(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_RIGHT, 0);
			}
			
			if (bottom != -1) {
				gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_BOTTOM, true);
				gtk_layer_set_margin(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_BOTTOM, bottom);
			}
			else {
				gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_BOTTOM, false);
				gtk_layer_set_margin(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_BOTTOM, 0);
			}
			
			if (left != -1 and right != -1) w = -1;
			if (top != -1 and bottom != -1) h = -1;	
		}
		else {
			// get screen size and recalculate to relative=-2 and right=bottom=-1
			Pixels sw;
			Pixels sh;
			if (relative == -1 or relative == -2) { // default or multiscreen
				for (int8_t ss = gdk_screen_get_n_monitors(gdk_screen_get_default())-1; ss>=0; ss--){
				//for (uint8_t ss = gdk_display_get_n_monitors(gdk_display_get_default())-1; ss>=0; ss--){
					GdkRectangle rect;
					gdk_screen_get_monitor_geometry(
						gdk_screen_get_default(),
						ss,
						&rect
						);
					sw = std::max(int(sw), rect.x+rect.width);
					sh = std::max(int(sh), rect.y+rect.height);
				}
				
			}
			else { // screen
				GdkRectangle rect;
				gdk_screen_get_monitor_geometry(
					gdk_screen_get_default(), 
					relative!=-3 ? relative : gdk_screen_get_primary_monitor(gdk_screen_get_default()),
					&rect
					);
				if (left != -1) left+=rect.x;
				if (top != -1) top+=rect.y;
				if (right != -1) right+=rect.x;
				if (bottom != -1) bottom+=rect.y;
				sw = rect.width;
				sh = rect.height;
			}
			if (left != -1 and right != -1){
				w = sw-left-right;
				right = -1;
			}
			if (top != -1 and bottom != -1){
				h = sh-top-bottom;
				bottom = -1;
			}
			if (right != -1){
				int ww,wh;	
				if (w == -1) gtk_window_get_size(GTK_WINDOW(window), &ww, &wh);
				else ww = w;
				left = sw-right-w;
				right = -1;
			}
			if (bottom != -1){
				int ww,wh;	
				if (h == -1) gtk_window_get_size(GTK_WINDOW(window), &ww, &wh);
				else wh = h;
				top = sh-bottom-wh;
				bottom = -1;
			}
			// set position
			gtk_window_move(GTK_WINDOW(window), left, top);
		}
	}
	// set size
	if (w != -1 or h != -1){
		if (gtk_layer_is_layer_window(GTK_WINDOW(window))) gtk_widget_set_size_request(GTK_WIDGET(window), w, h); 
		else gtk_window_resize(GTK_WINDOW(window), w, h);
	}
	// set state // TODO: only for toplevels (hacks: WT_DESKTOP_COMPONENT fulscreen, multiscreen fullscreen)
	if (state != 0 and state != -1){
		if (state == (state|WS_MAXIMIZED)) gtk_window_maximize(GTK_WINDOW(window));
		else if (state == (state|WS_N_MAXIMIZED)) gtk_window_unmaximize (GTK_WINDOW(window));
		if (state == (state|WS_FULLSCREEN)) {
			if (relative >=0) gtk_window_fullscreen_on_monitor(GTK_WINDOW(window), gdk_screen_get_default(), relative);
			else gtk_window_fullscreen(GTK_WINDOW(window));
		}
		else if (state == (state|WS_N_FULLSCREEN)) gtk_window_unfullscreen(GTK_WINDOW(window));
		if (state == (state|WS_MINIMIZED)) gtk_window_iconify(GTK_WINDOW(window));
		else if (state == (state|WS_N_MINIMIZED)) gtk_window_deiconify(GTK_WINDOW(window));
	}
}
auto WindowControls::get_geometry (){
	struct {Pixels w,h,left,top,right,bottom; Screen relative; int8_t state;} geometry;
	if (gtk_layer_is_layer_window(GTK_WINDOW(window))){
		if (gtk_layer_get_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_TOP)) geometry.top = gtk_layer_get_margin(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_TOP);
		else geometry.top = -1;	
		
		if (gtk_layer_get_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_LEFT)) geometry.left = gtk_layer_get_margin(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_LEFT);
		else geometry.left = -1;	
		
		if (gtk_layer_get_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_RIGHT)) geometry.right = gtk_layer_get_margin(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_RIGHT);
		else geometry.right = -1;	
		
		if (gtk_layer_get_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_BOTTOM)) geometry.bottom = gtk_layer_get_margin(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_BOTTOM);
		else geometry.bottom = gdk_screen_get_monitor_at_window(gdk_screen_get_default(), GDK_WINDOW(window));
	
		geometry.relative = -1;
		geometry.state = 0;
	}
	else {
		int w,h,x,y;
		gtk_window_get_size(GTK_WINDOW(window), &w, &h);
		gtk_window_get_position(GTK_WINDOW(window), &x, &y);
		
		geometry.w = w;
		geometry.h = h;
		geometry.left = y;
		geometry.top = x;
		geometry.right = -1;
		geometry.bottom = -1;
		
		geometry.relative = -2; // TODO: return monitor the window is at while preserving logic (position is relative to multiscreen)
		geometry.state = 0;
		if (gtk_window_is_maximized(GTK_WINDOW(window))) geometry.state = geometry.state|WS_MAXIMIZED;
		else geometry.state = geometry.state|WS_N_MAXIMIZED;
		// TODO: fullscreen (gtk_window_is_fullscreen(GTK_WINDOW(window)); availible only in Gtk4 -- alternative) - compare screen size and window size
		// TODO: minimized
		
	}
	return geometry;
}

void WindowControls::set_layer(WindowLayer layer){ // TODO: fix: x11 desktop - only before window.show()
	if (gtk_layer_is_layer_window(GTK_WINDOW(window))){
		switch(layer){
			case WL_BACKGROUND:
				gtk_layer_set_layer(GTK_WINDOW(window), GTK_LAYER_SHELL_LAYER_BACKGROUND);
				break;
			case WL_BOTTOM:
				gtk_layer_set_layer(GTK_WINDOW(window), GTK_LAYER_SHELL_LAYER_BOTTOM);
				break;
			case WL_TOP:
				gtk_layer_set_layer(GTK_WINDOW(window), GTK_LAYER_SHELL_LAYER_TOP);
				break;
			case WL_OVERLAY:
				gtk_layer_set_layer(GTK_WINDOW(window), GTK_LAYER_SHELL_LAYER_OVERLAY);
				break;
		}
	}
	else {
		switch(layer){
			case WL_BACKGROUND:
				gtk_window_set_keep_above(GTK_WINDOW(window), false); 
				gtk_window_set_keep_below(GTK_WINDOW(window), false); // should be true of false? - doesnt make a difference ; is there for compatibility ; but doesnt work under windows anyway
				gtk_window_set_type_hint(GTK_WINDOW(window), GDK_WINDOW_TYPE_HINT_DESKTOP); // only for not-toplevel - works only before show
				break;
			case WL_TOPLEAST:
				gtk_window_set_keep_below(GTK_WINDOW(window), true);
				gtk_window_set_keep_above(GTK_WINDOW(window), false);
				break;
			case WL_TOP:
				gtk_window_set_keep_above(GTK_WINDOW(window), false);
				gtk_window_set_keep_below(GTK_WINDOW(window), false);
				break;
			case WL_TOPMOST:
				gtk_window_set_keep_above(GTK_WINDOW(window), true);
				gtk_window_set_keep_below(GTK_WINDOW(window), false);
				break;
		}

	}
}
WindowLayer WindowControls::get_layer(){
	if (gtk_layer_is_layer_window(GTK_WINDOW(window))){
		GtkLayerShellLayer layer = gtk_layer_get_layer (GTK_WINDOW(window));
		switch(layer){
			case GTK_LAYER_SHELL_LAYER_BACKGROUND:
				return WL_BACKGROUND;
				break;
		
			case GTK_LAYER_SHELL_LAYER_BOTTOM:
				return WL_BOTTOM;
				break;
		
			case GTK_LAYER_SHELL_LAYER_TOP:
				return WL_TOP;
				break;
		
			case GTK_LAYER_SHELL_LAYER_OVERLAY:
				return WL_OVERLAY;
				break;
		}
		return WL_TOP;
	}
	else {
		// TODO: not possible to get topmost/topleast values from gtk3
		return WL_TOP;
	}
}

void WindowControls::set_input_mode_keyboard (WindowInputMode mode) {
	if (gtk_layer_is_layer_window(GTK_WINDOW(window))){ // [before & after] show (!always for non-layer shell)
		switch(mode){
			case WIM_ALWAYS:
				gtk_layer_set_keyboard_mode (GTK_WINDOW(window), GTK_LAYER_SHELL_KEYBOARD_MODE_EXCLUSIVE);
				break;
			case WIM_AUTO_WINDOW:
				gtk_layer_set_keyboard_mode (GTK_WINDOW(window), GTK_LAYER_SHELL_KEYBOARD_MODE_ON_DEMAND);
				break;
			case WIM_NEVER:
				gtk_layer_set_keyboard_mode (GTK_WINDOW(window), GTK_LAYER_SHELL_KEYBOARD_MODE_NONE);
				break;
		}
	}
	else {
		switch(mode){
			case WIM_ALWAYS:
				// TODO: can be callback .. on_unfocus = [](){window.activate()}
				break;
			case WIM_AUTO_WINDOW:
				gtk_window_set_accept_focus (GTK_WINDOW(window), true);
				break;
			case WIM_NEVER:
				gtk_window_set_accept_focus (GTK_WINDOW(window), false);
				break;
		}
	}
}
WindowInputMode WindowControls::get_input_mode_keyboard (){
	if (gtk_layer_is_layer_window(GTK_WINDOW(window))){
		GtkLayerShellKeyboardMode mode = gtk_layer_get_keyboard_mode (GTK_WINDOW(window));
		switch(mode){
				case GTK_LAYER_SHELL_KEYBOARD_MODE_EXCLUSIVE:
					return WIM_ALWAYS;
					break;
		
				case GTK_LAYER_SHELL_KEYBOARD_MODE_ON_DEMAND:
					return WIM_AUTO_WINDOW;
					break;
		
				case GTK_LAYER_SHELL_KEYBOARD_MODE_NONE:
					return WIM_NEVER;
					break;
		}
		return WIM_AUTO_WINDOW;
	}
	else {
		if (gtk_window_get_accept_focus(GTK_WINDOW(window))) return WIM_AUTO_WINDOW;
		else return WIM_NEVER;
		// TODO: WIM_ALWAYS
	}
}

void WindowControls::set_input_mode_mouse (WindowInputMode mode) {}
WindowInputMode WindowControls::get_input_mode_mouse (){}

void WindowControls::set_exclusive_zone (int16_t zone){
	if (gtk_layer_is_layer_window(GTK_WINDOW(window))){
		if (zone == -2) gtk_layer_auto_exclusive_zone_enable(GTK_WINDOW(window));
		else gtk_layer_set_exclusive_zone(GTK_WINDOW(window), zone);
	}
	else {
		// TODO: <https://stackoverflow.com/questions/3859045/preventing-window-overlap-in-gtk>
	}
}
int16_t WindowControls::get_exclusive_zone (){
	//gtk_layer_auto_exclusive_zone_is_enabled(GTK_WINDOW(window)); // useless in fact
	return gtk_layer_get_exclusive_zone(GTK_WINDOW(window));
}


void WindowControls::set_focused (bool activated){
	gtk_window_activate_default(GTK_WINDOW(window));
	// TODO: deativate (hack: create new invisible window)
}
bool WindowControls::get_focused (){
	return bool( gtk_window_is_active(GTK_WINDOW(window)) );
}  




void WindowControls::start_move_drag (){
	gtk_window_begin_move_drag (GTK_WINDOW(window), NULL, NULL, NULL, NULL);
}

void WindowControls::start_resize_drag (){ // TODO: select / auto-select resize edge/corner (by the mouse position)
	gtk_window_begin_resize_drag (GTK_WINDOW(window), GDK_WINDOW_EDGE_SOUTH_EAST, NULL, NULL, NULL, NULL);
}



void WindowControls::close () { // TODO: close = destroy = delete variable vs. request close = decorations buttons
	gdk_window_destroy(gtk_widget_get_window(GTK_WIDGET(window)));
}
 #endif


};
#endif




// ----- window managers ----- //

#if defined(HUI_BACKEND_NONE)
namespace HUI {
	
/// ProcessWindow (TODO) ///

Handle ProcessWindow(const char* run) {}

/// WindowBase2 ///

WindowBase2::WindowBase2() {}
WindowBase2::~WindowBase2() {}

void WindowBase2::embed (Handle child, Rect(*positioner)(Rect) ) {}
void WindowBase2::detach (Handle handle) {}

Handle WindowBase2::window_handle() {
    return 0;
}

void WindowBase2::run_blocking() {}
void WindowBase2::run_nonblocking() {}
std::thread WindowBase2::run_inthread() {}


/// WindowControls ///

WindowControls::WindowControls(){}

void WindowControls::use (WebviewContent* _window) {}

void WindowControls::show() {}
void WindowControls::hide() {}

void WindowControls::set_title(const char* title) {}
const char* WindowControls::get_title(){
    return nullptr;
}

void WindowControls::set_id(const char* id) {}
const char* WindowControls::get_id(){
    return nullptr;
}

void WindowControls::set_type (WindowType type) {}
WindowType WindowControls::get_type (){}

void WindowControls::set_geometry (Pixels w, Pixels h, Pixels left, Pixels top, Pixels right, Pixels bottom, Screen relative, int8_t state) {}
auto WindowControls::get_geometry (){ 
    struct {Pixels w,h,left,top,right,bottom; Screen relative; int8_t state;} geometry; 
    return geometry; 
}

void WindowControls::set_layer(WindowLayer layer) {}
WindowLayer WindowControls::get_layer(){
    return WL_TOP;
}

void WindowControls::set_input_mode_keyboard (WindowInputMode mode) {}
WindowInputMode WindowControls::get_input_mode_keyboard (){
    return WIM_AUTO_WINDOW;
}

void WindowControls::set_input_mode_mouse (WindowInputMode mode) {}
WindowInputMode WindowControls::get_input_mode_mouse (){
    return WIM_AUTO_WINDOW;
}

void WindowControls::set_exclusive_zone (int16_t zone){}
int16_t WindowControls::get_exclusive_zone (){}

void WindowControls::set_focused (bool activated) {}
bool WindowControls::get_focused (){
    return false;
}

void WindowControls::start_move_drag () {}

void WindowControls::start_resize_drag () {}

void WindowControls::close () {}




};
#endif


#if defined(HUI_BACKEND_WINDOWS)
namespace HUI {
	
/// ProcessWindow (TODO) ///

HWND ProcessWindow(const char* run) { // TODO: make this class with handle getter and kill destructor + proper threading
    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    HWND hwnd = NULL;

    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_SHOWMINIMIZED;
	
	//si.dwFlags = STARTF_USESHOWWINDOW;
    //si.wShowWindow = SW_HIDE; // TODO: doesnt work cos the window doesnt have handle when it starts hidden; may help: <https://stackoverflow.com/questions/49123062/unable-to-restore-windows-hidden-with-showwindow>

    if (CreateProcess(NULL, const_cast<LPSTR>(run), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        std::cout << "Started process: " << run << ", PID: " << pi.dwProcessId << std::endl;

		for (uint16_t ms = 0; ms < 2000; ms += 5) {

			std::this_thread::sleep_for(std::chrono::milliseconds(5));

			hwnd = 
			[](DWORD processID)->HWND {
				HWND bestHandle = NULL;
				struct HandleData {
					DWORD processID;
					HWND bestHandle;
				} data = { processID, bestHandle };

				EnumWindows([](HWND handle, LPARAM lParam) -> BOOL {
					HandleData& data = *(HandleData*)lParam;
					DWORD processID = 0;
					GetWindowThreadProcessId(handle, &processID);
					if (data.processID == processID && GetWindow(handle, GW_OWNER) == 0 && IsWindowVisible(handle)) {
						data.bestHandle = handle;
						return FALSE;
					}
					return TRUE;
				}, (LPARAM)&data);

				return data.bestHandle;
			
			} (pi.dwProcessId); //FindMainWindow
			
			if (hwnd) break;
		
		}
		
        if (!hwnd) {
            std::cerr << "Main window not found, enumerating all windows" << std::endl;
            HWND tempHandle = NULL;
            for (HWND window = GetTopWindow(NULL); window != NULL; window = GetNextWindow(window, GW_HWNDNEXT)) {
                DWORD processID = 0;
                GetWindowThreadProcessId(window, &processID);
                if (processID == pi.dwProcessId && IsWindowVisible(window)) {
                    tempHandle = window;
                    break;
                }
            }
            hwnd = tempHandle;
        }
		
		        if (hwnd) {
            // Show the window now that it's created
            ShowWindow(hwnd, SW_SHOW);
            UpdateWindow(hwnd);
        }

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    } else {
        std::cerr << "Failed to start process: " << run << std::endl;
    }

    std::cout << "Process window handle: " << hwnd << std::endl;
		  //ShowWindow(hwnd, SW_HIDE);
    return hwnd;
}


/// WindowBase2 ///

WindowBase2::WindowBase2(){
	hInst = GetModuleHandle(NULL);
	// TODO: add support for transparency <https://learn.microsoft.com/en-us/answers/questions/1190872/how-do-i-create-a-simple-transparent-window-in-win>
    WNDCLASS wc = {0};
    //wc.lpfnWndProc   = WndProc;
    wc.lpfnWndProc   = [](HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) ->LRESULT CALLBACK {
		switch (msg) {
			case WM_SIZE:
				{
					RECT rcClient;
					GetClientRect(hwnd, &rcClient);

					//int halfWidth = rcClient.right / 2;

					WindowBase2* pThis = (WindowBase2*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
					if (pThis) {
						/*if (pThis->hwndChild1) {
							MoveWindow(pThis->hwndChild1, 0, 0, halfWidth, rcClient.bottom, TRUE);
						}
						if (pThis->hwndChild2) {
							MoveWindow(pThis->hwndChild2, halfWidth, 0, halfWidth, rcClient.bottom, TRUE);
						}*/
						
						for (auto a : pThis->windows){
							std::cout<<rcClient.right<<" " <<rcClient.bottom;
							Rect rect = a.positioner(Rect(0,0,rcClient.right,rcClient.bottom));
							MoveWindow(a.handle, rect.x, rect.y, rect.w, rect.h, TRUE);
						}
					}
				}
				break;

			case WM_CREATE:
				{
					LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
					WindowBase2* pThis = (WindowBase2*)pcs->lpCreateParams;
					SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
				}
				break;

			case WM_DESTROY:
				PostQuitMessage(0);
				break;

			default:
				return DefWindowProc(hwnd, msg, wParam, lParam);
		}
		return 0;
	};
    
	wc.hInstance     = hInst;
    wc.lpszClassName = _T("ParentWindowClass");
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);

    if (!RegisterClass(&wc)) {
        std::cerr << "Failed to register window class" << std::endl;
        std::exit(1);
    }

    hwndParent = CreateWindow(wc.lpszClassName, _T("Embedded Applications"),
                              WS_OVERLAPPEDWINDOW,
                              CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
                              NULL, NULL, hInst, this);

    if (!hwndParent) {
        std::cerr << "Failed to create parent window" << std::endl;
        std::exit(1);
    }
}

WindowBase2::~WindowBase2() {
    if (hwndParent) {
        DestroyWindow(hwndParent);
    }
}

void WindowBase2::embed (HWND child, Rect(*positioner)(Rect) ) {
	            //ShowWindow(child, SW_SHOW);
           // UpdateWindow(child);
	  
    SetParent(child, hwndParent);
    LONG style = GetWindowLong(child, GWL_STYLE);
    style &= ~(WS_POPUP | WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU);
    style |= WS_CHILD | WS_VISIBLE;
    SetWindowLong(child, GWL_STYLE, style);

    LONG exStyle = GetWindowLong(child, GWL_EXSTYLE);
    exStyle &= ~(WS_EX_APPWINDOW | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE);
    exStyle |= WS_EX_CONTROLPARENT;
	exStyle &= ~(WS_EX_NOACTIVATE);
    SetWindowLong(child, GWL_EXSTYLE, exStyle);

    //SetWindowPos(child, NULL, x, y, width, height, SWP_NOZORDER /*| SWP_NOACTIVATE*/ | SWP_FRAMECHANGED | SWP_SHOWWINDOW);
	this->windows.push_back({child, positioner});
	  
      

    std::cout << "Window embedded: " << child << std::endl;
}

void WindowBase2::detach (HWND handle){
	SetParent(handle, NULL);
	SetWindowLong(handle, GWL_STYLE, /* WS_THICKFRAME | */WS_CAPTION | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU | WS_THICKFRAME | WS_VISIBLE);
	SetWindowLong(handle, GWL_EXSTYLE, WS_EX_WINDOWEDGE);
}

HWND WindowBase2::window_handle() {
    return hwndParent;
}

void WindowBase2::run_blocking() {
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void WindowBase2::run_nonblocking() {
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

std::thread WindowBase2::run_inthread() { // won't ever work because "thread that creates the window owns the window" (except maybe that we created the window in the thread)
    return std::thread([this]() {
        run_blocking();
        
    });
	// TODO: <https://stackoverflow.com/questions/7489175/are-win32-windows-thread-safe> <https://learn.microsoft.com/cs-cz/windows/win32/api/winuser/nf-winuser-postmessagea?redirectedfrom=MSDN>
}


/// WindowControls ///

WindowControls::WindowControls(){}

void WindowControls::use (WebviewContent* _window){
	//handle = HWND( ((QWidget*)_window->backend_object())->window()->winId() );
	handle = _window->window_handle();
}

void WindowControls::show() {
    ShowWindow(handle, SW_SHOW);
    UpdateWindow(handle);
}

void WindowControls::hide() {
    ShowWindow(handle, SW_HIDE);
}

void WindowControls::set_title(const char* title){
	SetWindowText(handle, title);
}
const char* WindowControls::get_title(){
    char buffer[1024];
    GetWindowText(handle, buffer, sizeof(buffer)/sizeof(buffer[0]));
	return std::string(buffer).c_str();
}

void WindowControls::set_id(const char* id){
	// TODO: window Class name - cannot be changed after the window creation
}
const char* WindowControls::get_id(){
    char buffer[1024];
    GetClassName(handle, buffer, sizeof(buffer)/sizeof(buffer[0]));
	return std::string(buffer).c_str();
}

void WindowControls::set_type (WindowType type){
	// panel and window list
	SetWindowLongA(handle, GWL_EXSTYLE, GetWindowLongA(handle, GWL_EXSTYLE) | WS_EX_APPWINDOW);
	SetWindowLongA(handle, GWL_EXSTYLE, GetWindowLongA(handle, GWL_EXSTYLE) & ~(WS_EX_APPWINDOW));
	
	// frame = resizable
	
	// titlebar
	
	
	// WS_THICKFRAME resizable, WS_CAPTION titlebar, 
	//SetWindowLongA(handle, GWL_STYLE, GetWindowLongA(handle, GWL_STYLE) & ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU)); //no titlebar, no frame, no resize, no panel
	SetWindowLongA(handle, GWL_STYLE, GetWindowLongA(handle, GWL_STYLE) /*& ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE)*//*| WS_MINIMIZE| WS_SYSMENU| WS_SYSMENU| WS_THICKFRAME  | WS_MAXIMIZE | WS_SYSMENU*/); //no titlebar
	SetWindowLongA(handle, GWL_EXSTYLE, GetWindowLongA(handle, GWL_EXSTYLE) | WS_EX_APPWINDOW);
	SetWindowPos(handle, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
}
WindowType WindowControls::get_type (){}

void WindowControls::set_geometry (Pixels w, Pixels h, Pixels left, Pixels top, Pixels right, Pixels bottom, Screen relative, int8_t state){
	// TODO: relative, anchors, set only some, state
	SetWindowPos(handle, HWND_BOTTOM, left, top, w, h, SWP_NOZORDER /*| SWP_NOMOVE | SWP_NOSIZE*/);
}
auto WindowControls::get_geometry (){ struct {Pixels w,h,left,top,right,bottom; Screen relative; int8_t state;} geometry; return geometry; }

void WindowControls::set_layer(WindowLayer layer){
	switch(layer){
		case WL_BACKGROUND:
			// TODO: same trick as topleast + frameless BUT this covers desktop icons (should it?)
			SetWindowPos(handle, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			break;
		case WL_TOPLEAST:
			// TODO: <https://stackoverflow.com/questions/365094/window-on-desktop>
			SetWindowPos(handle, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			break;
		case WL_TOP:
			SetWindowPos(handle, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			break;
		case WL_TOPMOST:
			SetWindowPos(handle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			break;
	}
}
WindowLayer WindowControls::get_layer(){
	int exStyle = GetWindowLong(handle, GWL_EXSTYLE);
	if ((exStyle & WS_EX_TOPMOST) == WS_EX_TOPMOST) return WL_TOPMOST;
	// TODO: topleast and background
	return WL_TOP;
}

void WindowControls::set_input_mode_keyboard (WindowInputMode mode) {
	switch(mode){
		case WIM_ALWAYS:
			// TODO: can be callback .. on_unfocus = [](){window.activate()}
			break;
		case WIM_AUTO_WINDOW:
			SetWindowLongA(handle, GWL_EXSTYLE, GetWindowLongA(handle, GWL_EXSTYLE) & ~(WS_EX_NOACTIVATE));
			break;
		case WIM_NEVER:
			SetWindowLongA(handle, GWL_EXSTYLE, GetWindowLongA(handle, GWL_EXSTYLE) | WS_EX_NOACTIVATE);
			break;
	}
}
WindowInputMode WindowControls::get_input_mode_keyboard (){
	int exStyle = GetWindowLong(handle, GWL_EXSTYLE);
	if ((exStyle & WS_EX_NOACTIVATE) == WS_EX_NOACTIVATE) return WIM_NEVER;
	return WIM_AUTO_WINDOW;
}

void WindowControls::set_input_mode_mouse (WindowInputMode mode) {
	switch(mode){
		case WIM_ALWAYS:
			// same as WIM_AUTO_WINDOW
			SetWindowLongA(handle, GWL_EXSTYLE, GetWindowLongA(handle, GWL_EXSTYLE) & ~(WS_EX_COMPOSITED | WS_EX_LAYERED | WS_EX_TRANSPARENT));
			break;
		case WIM_AUTO_WINDOW:
			// same as WIM_ALWAYS
			SetWindowLongA(handle, GWL_EXSTYLE, GetWindowLongA(handle, GWL_EXSTYLE) & ~(WS_EX_COMPOSITED | WS_EX_LAYERED | WS_EX_TRANSPARENT));
			break;
		case WIM_NEVER:
			SetWindowLongA(handle, GWL_EXSTYLE, GetWindowLongA(handle, GWL_EXSTYLE) | WS_EX_COMPOSITED | WS_EX_LAYERED | WS_EX_TRANSPARENT);
			break;
	}
}
WindowInputMode WindowControls::get_input_mode_mouse (){
	int exStyle = GetWindowLong(handle, GWL_EXSTYLE);
	if ((exStyle & (WS_EX_COMPOSITED | WS_EX_LAYERED | WS_EX_TRANSPARENT)) == (WS_EX_COMPOSITED | WS_EX_LAYERED | WS_EX_TRANSPARENT)) return WIM_NEVER;
	return WIM_AUTO_WINDOW;
}

void WindowControls::set_exclusive_zone (int16_t zone){}
int16_t WindowControls::get_exclusive_zone (){}

void WindowControls::set_focused (bool activated){
	if (activated) SetFocus(handle); // TODO: "The window must be attached to the calling thread's message queue." from <https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setfocus>
	else {} // TODO: another window has to be focused in order to unfocus this one
}
bool WindowControls::get_focused (){
	return GetActiveWindow() == handle;
}

void WindowControls::start_move_drag (){
	// TODO
}

void WindowControls::start_resize_drag (){
	// TODO
}

void WindowControls::close () {}


};
#endif


#endif //HUI_NEW_CC