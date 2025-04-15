#ifndef _hui_backend_
#define _hui_backend_


#define _hui_webview_use_common_code
#include "HUI.hh"


#include "./hui_datatypes.h"
#include <fstream>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDesktopWidget>
#include <QtWebKitWidgets/QWebView>
#include <QtWebKitWidgets/QWebFrame>
//#include <LayerShellQt/Shell>  // TODO, will not be implemented any time soon because layer-shell-qt doesnt have pkg-config
//#include <LayerShellQt/Window> 


namespace HUI {

std::vector< std::function<void(std::vector<std::string>)> > sent2cpp_handlers;  // TODO: call handlers should be able to use only indexes for other laguages bindings (NOT BINDINGS BUT IMPLEMENTATIONS!)

int qt_argc = 0;
char* qt_argv = "";
QApplication qt_app(qt_argc,&qt_argv);

class Qt_q_object : public QObject {
    Q_OBJECT
  public: 
    Qt_q_object(){}
    Q_INVOKABLE void call_from_js_to_cpp (QString dta){
		
		std::string msgstr = std::string(dta.toStdString());
		
		HUI_DEBUG_PRINT("call from js to cpp '"<<msgstr<<"' ");
		
		const int hi = stoi(msgstr.substr(0,msgstr.find(",")));
		msgstr = msgstr.substr(msgstr.find(",")+1+1, msgstr.size()-1 -(msgstr.find(",")+1+1) );
		
		std::vector<HUI::Str> msgdata_ = HUI::Str(msgstr).split("\",\"");
		std::vector<std::string> msgdata;
		for (auto it : msgdata_) msgdata.push_back ( it.replace("\\\"","\"").cpp_str() );
		
		sent2cpp_handlers.at(hi)(msgdata);
	
	}
};
Qt_q_object qt_q_object_for_call_cpp; // having this can save memory (1 QObject, 1 std::vector/std::map)


struct WebView::pImpl {
	QWidget* window;
	QWebView* webview;
};

WebView::WebView () {
	impl = std::make_unique<pImpl>();
	
	// window
	//impl->window = new QWidget();
	//LayerShellQt::Shell::useLayerShell();
	
	// transparency
	//impl->window->setAttribute(Qt::WA_TranslucentBackground);
	//window->setAutoFillBackground(true); // has no effect there
	//window->setWindowFlags(Qt::FramelessWindowHint); // transparency works only with frameless windows on Windows in Qt (but we dont want all windows to be frameless)
	
	// webview
	impl->webview = new QWebView();
	#if not defined (HUI_DEBUG_ENABLED) // make it possible to right click and open devtools
		impl->webview->setContextMenuPolicy(Qt::NoContextMenu);
	#endif
	
	// transparency
	impl->webview->setAttribute(Qt::WA_TranslucentBackground);
	impl->webview->setAutoFillBackground(true);
	impl->webview->setStyleSheet("background:transparent"); // required
	//window->setWindowFlags(Qt::FramelessWindowHint); // transparency works only with frameless windows on Windows in Qt (but we dont want all windows to be frameless)
	// TODO: fix transparency on Windows - easiest way is to use client-side decorations or to give up resizability and create transparent window and zero height window as titlebar
	
	// debug
	#if defined (HUI_DEBUG_ENABLED)
		HUI_DEBUG_PRINT("enabling devtools ");
		impl->webview->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled,true);
	#endif
	
	// hui.js, hui.css
	hui_tweaks();
	// just to unify all backends
	impl->webview->load(QUrl("about:blank"));
	
	// hole in window
	/*QRect r = webview->frameGeometry();
	r.moveTopLeft(webview->mapFromGlobal(r.topLeft()) );
	QRect hole(0, 0, webview->width() - 50, 100);
	hole.moveCenter(webview->rect().center());
	webview->setMask(QRegion(r).subtracted(hole));*/
	
	// js<-->cpp
	impl->webview->page()->mainFrame()->addToJavaScriptWindowObject("qt_window", &qt_q_object_for_call_cpp);
	
	// add
	impl->window = impl->webview; // window and webview are same object (in qt, widget alone becomes window)
	
	// show
	impl->window->show();
	
}
WebView::~WebView () {
	delete impl->window;
}

void WebView::load_file (std::string file){
	impl->webview->load(QUrl(std::string("file://"+file).c_str()));

	hui_tweaks();
}
void WebView::load_uri (std::string uri){
	impl->webview->load(QUrl(uri.c_str()));

	hui_tweaks();
}
void WebView::load_str (std::string str){
	impl->webview->setHtml(str.c_str());  // <https://forum.qt.io/topic/42023/how-to-load-html-source-from-string-code-to-qwebview-solved/12>
	hui_tweaks();
}
void WebView::hui_tweaks (){
	/*impl->webview->page()->mainFrame()->evaluateJavaScript(
	#include "hui.js"
	);*/
	call_js(
		#include "hui.js"
		);
	call_js(std::string(
	  "style = document.createElement('style'); style.textContent = \"")+
	  HUI::Str(
	  #include "hui.css"
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
	
	HUI_DEBUG_PRINT("\n [HUI::DEBUG] call js from cpp '"<<code<<"' ");
	
	if (return_data) {
		return impl->webview->page()->mainFrame()->evaluateJavaScript(code.c_str()).toString().toStdString();
	}
	
	else {
		// there were previously some issues, but this seems to work fine
		/*if (loaded)*/ impl->webview->page()->mainFrame()->evaluateJavaScript(std::string(code+"\n true;").c_str());
		/*else*/ impl->webview->connect(impl->webview, &QWebView::loadFinished, impl->webview, [=](){ impl->webview->page()->mainFrame()->evaluateJavaScript(code.c_str()); });
		return "";
	}
	
}
std::string WebView::call_native (std::function<void(std::vector<std::string>)> handler, std::string process_args){
	sent2cpp_handlers.push_back(handler);
	return "function(..._args_array){qt_window.call_from_js_to_cpp( '"+std::to_string(sent2cpp_handlers.size()-1)+"'+',\"'+("+process_args+")(_args_array).join('\",\"')+'\"' );}";
}

std::string WebView::backend_name (){
	return "webkit-qt5";
}
void* WebView::backend_object (){
	return static_cast<void*>(&(impl->window)); // backend widget can be get using backend code from the toplevel container
}
std::string WebView::window_type (){
	#if defined(_WIN32) // windows
	  return "windows";
	#else // TODO: whatever
	  return "unknown";
	#endif
}
void* WebView::window_handle (){
	#if defined(_WIN32) // windows
	  return HWND( impl->window->winId() );
	#else // TODO: whatever
	  return impl->window->winId();
	#endif
}

void WebView::handle_forever (){
	qt_app.exec();
}
void WebView::handle_once (){
	qt_app.processEvents();
}
void/*std:thread*/ WebView::handle_threaded (){
	
}
void WebView::exit (){
	QCoreApplication::exit(0);
}


#include "./hui_windowcontrols__qt5.cc"

#if !defined(HUI_BACKEND_QT5_building_moc)
  #include "qt5_generated.moc"
#endif

} // HUI

#endif // _hui_backend_